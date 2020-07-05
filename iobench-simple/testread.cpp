/*
 * testread.cpp
 *
 * Copyright (C) 2019-2020 Sudarsun Kannan <sudarsun.kannan@@rutgers.edu>
 *
 * Licensed under the GNU GPL. See the file COPYING for details.
 *
 */

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

#define MB    (1024*1024)
#define FILESIZE 2ULL*1024*1024*1024
#define ENABLE_IO_PREFETCH
#define NUM_THREADS 32
#define WRMODE "w+"
#define RDMODE "r"
#define READ 0
#define WRITE 1

#pragma GCC diagnostic ignored "-fpermissive"


#define _USE_FSYNC
#define SYNCINTERVAL 100000
/*Use one file for all threads*/
//#define SHARED_FILE_THREADS

#define ZIPFSILE "zipfs.out"

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
double g_alpha;
unsigned long g_numblocks;
unsigned int *g_ziparr;
threadpool g_thpool;
struct sigaction g_action;
float g_thrput;
off_t g_iobytes, g_filesize;
int g_ntimes, g_ntimes_done;
std::chrono::duration<double> g_elapsed;
std::chrono::time_point<std::chrono::system_clock> g_start, g_end;

typedef struct filestruct {
    int fd;
    off_t iobytesdone;
    long bytes;
    char *buf;
    bool done;
    int buffsize;
    int writeop;
    int isSeq;
    off_t startoff;
}filestruct;
filestruct g_fstruct[NUM_THREADS+1];

typedef struct memstruct {
    off_t iobytesdone;
    int isSeq;
    double timer;
    char *buf;
    unsigned long worksize;
}memstruct;
memstruct g_memstruct[NUM_THREADS+1];


int g_fdarr[NUM_THREADS+1];
FILE *g_fparr[NUM_THREADS+1];
off_t filesizearr[NUM_THREADS+1];

void term(int signum);

/*Print result and stats*/
void print_results(){
  printf("%lf MB/sec, Elapsed %lf, bytes %ld, iters %d\n",
          g_thrput, g_elapsed.count(), g_iobytes, g_ntimes_done);
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
  off_t bytes = 0;

  fprintf(stderr,"Handling terminate \n");
  g_end = std::chrono::system_clock::now();
  g_elapsed = g_end - g_start;
  g_iobytes = 0;

  for (i = 0; i < g_num_threads; i++) {
    bytes += g_fstruct[i].iobytesdone;
  }
  g_iobytes = (g_filesize * g_ntimes_done) + bytes;

  g_thrput = ((g_iobytes)/MB)/g_elapsed.count();
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



void* thrdrandwrite (void *arg) {

  filestruct *str = (filestruct *)(arg);
  int fd = (int)str->fd;
  char *buf = (char *)str->buf;
  off_t filesize = (long)str->bytes;
  off_t bytes = 0, prev=0;
  int buffsize = (int)str->buffsize;
  int iswrite = (int)str->writeop;	
  int isSeq = (int)str->isSeq;
  off_t offset = 0, index=0;
 
  srand (time(NULL)); 

  while (bytes < filesize) {
    if(iswrite) { 
      bytes += pwrite(fd, buf, buffsize, offset);
      //bytes += write(fd, buf, buffsize-1);
    }else {
      bytes += pread(fd, buf, buffsize, offset);
    }


    if (isSeq) {
      offset += buffsize;
    }else {
      offset =  (lrand48())  % g_filesize;
    }
#ifdef _USE_FSYNC
      if(iswrite && (index % SYNCINTERVAL == 0)) {
        fsync(fd);
      }
#endif
    str->iobytesdone = bytes;
    if(prev == bytes) {
      return NULL;
    }
    prev = bytes;
    index++;
  }
  return NULL;
}


off_t random(char *buf, int buffsize, FILE *fp, off_t filesize,
                char **thrd_buffs, int isWrite, int isSeq) {
    int i=0;
    off_t bytes = 0;

    size_t thrd_bytes = g_filesize/g_num_threads;

    if( g_num_threads > 1) {
      g_thpool = thpool_init(g_num_threads-1);
      assert(g_thpool);
    }
    //fprintf(stderr, "g_ntimes_done %lu, g_ntimes %lu \n", g_ntimes_done, g_ntimes);
    while (g_ntimes_done < g_ntimes) {
      for (i = 0; i < g_num_threads; i++) {
          g_fstruct[i].fd = g_fdarr[i];
          g_fstruct[i].buf = thrd_buffs[i];
          g_fstruct[i].bytes = thrd_bytes; 
          g_fstruct[i].buffsize = buffsize;
          g_fstruct[i].iobytesdone = 0;
          g_fstruct[i].writeop = isWrite;
          g_fstruct[i].isSeq = isSeq;
          g_fstruct[i].startoff = i*thrd_bytes;
	  if (i == g_num_threads-1)
             thrdrandwrite(&g_fstruct[i]);
          else 
           thpool_add_work(g_thpool, (void*)thrdrandwrite, &g_fstruct[i]);
      }
      if( g_num_threads > 1) {
        thpool_wait(g_thpool);
      }
      g_ntimes_done++;
      for (i = 0; i < g_num_threads; i++) {
        bytes += g_fstruct[i].iobytesdone;
      }
    }
    return bytes;
}


long GetFileSize(char *filename)
{
    struct stat stat_buf;
    int rc = stat((const char*)filename, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}


int OpenFiles(char *fname, const char *mode) {

  int i =0;
  char buffer[256];
  size_t thrdfilesz = g_filesize/g_num_threads;
  size_t filesz = 0;

  for(i=0; i < g_num_threads; i++) {

#if defined(SHARED_FILE_THREADS)
    /*All threads use shared file*/
    strcpy(buffer, fname);
    strcat(buffer, "_");
    snprintf(buffer+strlen(buffer), sizeof(buffer), "%d", 0);
#else
    strcpy(buffer, fname);
    strcat(buffer, "_");
    snprintf(buffer+strlen(buffer), sizeof(buffer), "%d", i);
#endif
 
    /*Always open in "WRMODE*/
    g_fparr[i] = fopen(buffer, mode);  
    assert(g_fparr[i]);
    g_fdarr[i] = fileno(g_fparr[i]);

    //fprintf(stderr,"thrdfilesz %lu \n",thrdfilesz); 
    if (g_fparr[i] != NULL) {
      filesz = GetFileSize(buffer);
      g_filesize = filesz;
      /*if (!strcmp(mode,RDMODE) && (filesz != thrdfilesz)) {
        if (ftruncate(g_fdarr[i], thrdfilesz) != 0) {
          perror("Failed ftruncate \n");
          exit(-1);
         }
        filesz = GetFileSize(buffer);
        if(filesz != thrdfilesz) {
          assert(0);
        }
      }*/ 
#if defined(ENABLE_IO_PREFETCH)
       posix_fadvise(g_fdarr[i], 0, thrdfilesz, POSIX_FADV_SEQUENTIAL);
#endif
      /*Finally assign the size*/	
      filesizearr[i] = thrdfilesz;
    }
  }
}


int do_IO(int seq, char *fname, int buffsize, int isWrite) {

  off_t filesize = 0;
  int i = 0;
  char *buf = NULL;
  char **thrd_buffs;
  size_t thrdfilesz = 0;

  thrd_buffs = (char **)malloc(g_num_threads * sizeof(char *));
  assert(thrd_buffs);

  for(i = 0; i < g_num_threads; i++) {
    thrd_buffs[i] = (char *)malloc(buffsize * sizeof(char));
    assert(thrd_buffs[i]);
  }

  if(isWrite) {
    OpenFiles(fname, (const char *)WRMODE);
  }else {
    OpenFiles(fname, (const char *)RDMODE);
  }
  if(!g_filesize)
    g_filesize = FILESIZE;

  /*Start the time measurement*/
  g_start = std::chrono::system_clock::now();
  if(isWrite) {
    g_iobytes = random(buf, buffsize, g_fparr[0], filesize, thrd_buffs,1, seq);
  }else {
    g_iobytes = random(buf, buffsize, g_fparr[0], filesize, thrd_buffs,0, seq);
  }
  /*End the time measurement*/
  g_end = std::chrono::system_clock::now();
  g_elapsed = g_end -  g_start;
  g_thrput = (g_iobytes/MB)/g_elapsed.count();

  if(isWrite) {
    if(seq) {
      printf("Seq. write BW:");
    }else { 
      printf("Random. write BW:");
    }
  }else {
    if(seq) {
      printf("Seq. read bw:");
    }else {
      printf("Random. read bw:");
    }
  }

  thrdfilesz = g_filesize/g_num_threads;
  printf("%lf MB/sec , Avg-latency (buffered): %lf usec\n", 
		  g_thrput,  
		  (double)(((double)g_elapsed.count()*1000000)/(double)(thrdfilesz/4096)));

  for(i = 0; i < g_num_threads; i++) {
    fclose(g_fparr[i]);
  }

  thpool_destroy(g_thpool);

  for(i = 0; i < g_num_threads; i++) {
    free(thrd_buffs[i]);
    thrd_buffs[i] = NULL;
  }
  //if(thrd_buffs != NULL)
    //free(thrd_buffs);
}

int read_zipf_number() {

  FILE *fp;
  unsigned int value;
  unsigned int i = 0; /* EDIT have i start at -1 :) */

  if ((fp = fopen (ZIPFSILE, "r")) == NULL)
    assert(0);

 g_ziparr = (unsigned int *)malloc(sizeof(unsigned int) * g_numblocks);
 assert(g_ziparr);
  while (!feof (fp) && fscanf (fp, "%d", &value)) {
    g_ziparr[i] = value;
    //fprintf(stderr,"value %d \n",value);
    i++;
  }
  fclose (fp);
  return 0;
}


int main(int argc, char *argv[]) {
  char *fname;
  int buffsize =0;
  int isSeq = 0;

  //default
  g_ntimes = 1;

  init_sighandler();

  if (argc < 2) {
    fprintf(stderr,"Usage: ./testread fname iosize Optional:rand,seq \n");
    exit(0);	
  }
  fname = (char *)argv[1];
  buffsize = atoi(argv[2]);

  if (argc > 3) {	
    g_ntimes = atoi (argv[3]);
  }else {
    g_ntimes = 1;
  }
  if (argc > 4) {
    g_num_threads = atoi (argv[4]);
  }else {
    g_num_threads = 1;
  }
  if (argc > 5) {
    isSeq = atoi (argv[5]);
  }

  if (argc > 6) {
    g_filesize = atoi (argv[6]);
  }
  if (!g_filesize) {
    g_filesize = FILESIZE;
  }

  /*Currently only for single thread*/
  //g_alpha = 0.9999;
  g_numblocks = g_filesize/buffsize;
  //fprintf(stderr,"g_numblocks %u %u \n", g_filesize, g_numblocks);
  //rand_val(g_numblocks-10000);
  //read_zipf_number();

   //run_mem_bw(isSeq);
  do_IO(isSeq, fname, buffsize, WRITE);
  /*READ is 0 and write is 1*/
  //do_IO(isSeq, fname, buffsize, READ); 
  ///unlink (fname);
  //Write(1, fname, buffsize);
  //Read(1, fname, buffsize); 
  return 0;
}

