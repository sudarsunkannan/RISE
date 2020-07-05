#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include "thpool.h"

using namespace std;

#define ITERATIONS 60000000000
//#define DISABLE_IO_PREFETCH

#define NUM_THREADS 32
#define NTIMES 1

/* Period parameters */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */   
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/*global handlers*/
int g_num_threads;
threadpool g_thpool;
struct sigaction g_action;
float g_thrput;
off_t g_computeiters;
int g_ntimes;
std::chrono::duration<double> g_elapsed;
std::chrono::time_point<std::chrono::system_clock> g_start, g_end;

typedef struct read_struct {
    long itersdone;
    long iters;
    char *buf;
    bool done;
    int buffsize;
}read_struct;
read_struct g_str[NUM_THREADS+1];


void term(int signum);

/*Print result and stats*/
void print_results(){
  printf("%lf MB/sec, Elapsed %lf, iters %ld\n",g_thrput, g_elapsed.count(), g_computeiters);
}

void init_sighandler() {
  memset(&g_action, 0, sizeof(struct sigaction));
  g_action.sa_handler = term;
  sigaction(SIGTERM, &g_action, NULL);
}
/* Handler termination*/
void term(int signum)
{
  int i = 0;
  fprintf(stderr,"Handling terminate \n");
  g_end = std::chrono::system_clock::now();
  g_elapsed = g_end - g_start;

  for (i = 0; i < g_num_threads; i++) {
    g_computeiters += g_str[i].itersdone;
  }
  g_thrput = (g_computeiters/1048576)/g_elapsed.count();
  print_results();
  exit(0);
}

/* Initializing the array with a seed */
void sgenrand(unsigned long seed)
{
    int i;

    for (i=0;i<N;i++) {
         mt[i] = seed & 0xffff0000;
         seed = 69069 * seed + 1;
         mt[i] |= (seed & 0xffff0000) >> 16;
         seed = 69069 * seed + 1;
    }
    mti = N;
}


unsigned long  genrand()
{
    unsigned long y;
    static unsigned long mag01[2]={0x0, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if sgenrand() has not been called, */
            sgenrand(4357); /* a default initial seed is used   */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

        mti = 0;
    }
  
    y = mt[mti++];
    y ^= TEMPERING_SHIFT_U(y);
    y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
    y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
    y ^= TEMPERING_SHIFT_L(y);

    return y; 
}

#define MB    (1024*1024)
#define START_SIZE 0
#define STOP_SIZE  16384
#define STEPS_N 2*MB
#define CACHE_LN_SZ 64
/////////////////////////////////////////////////////////
// node structure padded to cache line size
/////////////////////////////////////////////////////////
struct node {
  int val;
  struct node *next;
  struct node *prev;
  //pad to make each node a cache line sz;
  char padding[CACHE_LN_SZ - 2 * sizeof(struct node*) - sizeof(int)];
};


struct node* delete_node( struct node *j) {

  if(!j) return NULL;

  j->prev->next = j->next;
  j->next->prev = j->prev;
  //ok I am free
  return j;
}

struct node* insert_node( struct node *i, struct node *j) {

  if(!j) return NULL;

  i->next->prev = j;
  j->next = i->next;
  j->prev = i;
  i->next = j;
  return i;
}

double MemoryTimingTest(void)
{    
  double retval;
  int j = 0,sequential=0, i=0;
  int index = 0;
  unsigned long avg_memref[4],avg=0,ws=0;
  unsigned int idx=0;
  struct node *arr=NULL;

  //arr = (struct node *)malloc(sizeof(struct node) * STOP_SIZE);

  for(i=START_SIZE; i<STOP_SIZE; i=i<<1) {
    ws = i;
    //first link all the node continuos
    /*for(idx =1; idx < ws-1; idx++){
      arr[idx].val = 1;
      arr[idx].prev = &arr[idx-1];
      arr[idx].next = &arr[idx+1];
    }
    //set the boundary values
    arr[0].prev = &arr[ws-1];
    arr[0].next = &arr[1];
    arr[0].val =1;
    arr[ws-1].prev= &arr[ws-2];
    arr[ws-1].next = &arr[0];
    arr[ws-1].val = 1;*/
    //fill random
    //srand ( time(NULL) );

    if(!sequential){
      /*Now start linking random nodes
       delete old node links and then create
       new links*/
      for(idx =0; idx < ws; idx++) {
        //generate a random id
        j = rand()% (ws-1);
        //insert between i and its next
        //insert_node(&arr[idx],&arr[j]);
      }
    }
    index++;
  }
  //free(arr);
  //arr=NULL;
  return 0;
}

  void* thrdrandwrite (void *arg) {

    read_struct *str = (read_struct *)(arg);
    /*char *buf = (char *)str->buf;
    int buffsize = (int)str->buffsize;*/
    off_t itersize = (long)str->iters;
    int offset = 0, ntimes=0, idx=0;

    double a[STOP_SIZE], b[STOP_SIZE], c[STOP_SIZE];
    double q = 1.0000086;

    //struct node *arr = (struct node *)malloc(sizeof(struct node) * STOP_SIZE);
    fprintf(stderr,"itersize %ld per thread, times %d \n", itersize, g_ntimes);
    while (ntimes < g_ntimes) {
      while (str->itersdone < itersize) {
        str->itersdone += 1;
        for(idx =START_SIZE; idx < STOP_SIZE; idx++){
          offset =  genrand() % (STOP_SIZE-1) ;
          a[offset] = b[offset];
          a[offset] = q*b[offset];
          a[offset] = b[offset] + c[offset];
          a[offset] = b[offset] + q*c[offset];  
        }
        //MemoryTimingTest();
      }
      ntimes++;
    }
    return NULL;
  }


long random(char *buf, int buffsize, FILE *fp, off_t itersize,
                char **thrd_buffs, int isWrite) {
    int res = 0, i=0;
    long iters = 0;

    size_t thrd_iters = itersize/g_num_threads;
     for (i = 0; i < g_num_threads; i++) {
          g_str[i].buf = thrd_buffs[i];
          g_str[i].iters = thrd_iters; 
          g_str[i].buffsize = buffsize;
          g_str[i].itersdone = 0;
	  if (i == g_num_threads-1)
             thrdrandwrite(&g_str[i]);
          else 
            thpool_add_work(g_thpool, (void*)thrdrandwrite, &g_str[i]);
     }
     thpool_wait(g_thpool);
     for (i = 0; i < g_num_threads; i++) {
       iters += g_str[i].itersdone;
     }
     return iters;
}


off_t GetComputeIter()
{
    return g_computeiters;
}


int Read(int seq, char *fname, int buffsize) {

  long iters = 0;
  char *buf = (char *)malloc(sizeof(char)*buffsize);
  assert(buf);

  char **thrd_buffs = (char **)malloc(g_num_threads);
  int i = 0;
  assert(thrd_buffs);
  for(i = 0; i < g_num_threads; i++) {
    thrd_buffs[i] = (char *)malloc(buffsize);
    assert(thrd_buffs[i]);
  }

  off_t itersize =  GetComputeIter();  
  g_start = std::chrono::system_clock::now();
  random(buf, buffsize, NULL, itersize, thrd_buffs, 0);
  g_end = std::chrono::system_clock::now();

  g_elapsed = g_end - g_start;
  g_thrput = (g_computeiters/1048576)/g_elapsed.count();

  printf("%lf MB/sec, Elapsed %lf, iters %ld\n",g_thrput, g_elapsed.count(), g_computeiters);
  free(buf);

  thpool_destroy(g_thpool);
  for(i = 0; i < g_num_threads; i++) {
    free(thrd_buffs[i]);
    thrd_buffs[i] = NULL;
  }
  if(thrd_buffs != NULL)
    free(thrd_buffs);
}


int main(int argc, char *argv[]) {
  char *fname;
  int buffsize =4096;
  int i=0;

  //init_sighandler();

  if (argc < 2) {
    fprintf(stderr,"Usage: ./testread fname iterations\n");
    exit(0);	
  }

  fname = (char *)argv[1];

  if (argc > 2) {	
    g_computeiters = atoi (argv[2]);
  }else {
    g_computeiters = ITERATIONS;
  }
  if (argc > 3) {
    g_num_threads = atoi (argv[3]);
  }else {
    g_num_threads = 1;
  }
  if (argc > 4) {
    g_ntimes = atoi (argv[4]);
  }else {
    g_ntimes = NTIMES;
  }

  g_thpool = thpool_init(g_num_threads+1);
  assert(g_thpool);
  Read(0, fname, buffsize);
  return 0;
}
