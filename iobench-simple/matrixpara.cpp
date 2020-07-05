#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
 
#define SIZE 2400   // Size by g_size matrices
int g_size;
int num_thrd;   // number of threads
 
//int A[g_size][g_size], B[g_size][g_size], C[g_size][g_size];
int **A = NULL;
int **B = NULL;
int **C = NULL; 

// initialize a matrix
void init_matrix(int **m)
{
  int i, j, val = 0;
  for (i = 0; i < g_size; i++)
    for (j = 0; j < g_size; j++)
      m[i][j] = val++;
}
 
void print_matrix(int **m)
{
  int i, j;
  for (i = 0; i < g_size; i++) {
    printf("\n\t| ");
    for (j = 0; j < g_size; j++)
      printf("%2d ", m[i][j]);
    printf("|");
  }
}
 
// thread function: taking "slice" as its argument
void* multiply(void* slice)
{
  int s = (int)slice;   // retrive the slice info
  int from = (s * g_size)/num_thrd; // note that this 'slicing' works fine
  int to = ((s+1) * g_size)/num_thrd; // even if g_size is not divisible by num_thrd
  int i,j,k;
 
  //printf("computing slice %d (from row %d to %d)\n", s, from, to-1);
  for (i = from; i < to; i++)
  {  
    for (j = 0; j < g_size; j++)
    {
      C[i][j] = 0;
      for ( k = 0; k < g_size; k++)
 C[i][j] += A[i][k]*B[k][j];
    }
  }
  //printf("finished slice %d\n", s);
  return 0;
}
 
int main(int argc, char* argv[])
{
  pthread_t* thread;  // pointer to a group of threads
  int i;
 
  if (argc > 3)
  {
    printf("Usage: %s number_of_threads matrixsize\n",argv[0]);
    exit(-1);
  }

  g_size = atoi(argv[2]);
  if(!g_size) {
    g_size = SIZE;
  }

  A = (int **)malloc(g_size * sizeof(int **));
  B = (int **)malloc(g_size *  sizeof(int **));
  C = (int **)malloc(g_size *  sizeof(int **));

  for (i = 0; i < g_size; i++) {
    A[i] = (int *)malloc(sizeof(int) * g_size);
    B[i] = (int *)malloc(sizeof(int) * g_size);
    C[i] = (int *)malloc(sizeof(int) * g_size);
  }
 
  num_thrd = atoi(argv[1]);
  init_matrix(A);
  init_matrix(B);
  thread = (pthread_t*) malloc(num_thrd*sizeof(pthread_t));
 
  // this for loop not entered if threadd number is specified as 1
  for (i = 1; i < num_thrd; i++)
  {
    // creates each thread working on its own slice of i
    if (pthread_create (&thread[i], NULL, multiply, (void*)i) != 0 )
    {
      perror("Can't create thread");
      free(thread);
      exit(-1);
    }
  }
 
  // main thread works on slice 0
  // so everybody is busy
  // main thread does everything if threadd number is specified as 1
  multiply(0);
 
  // main thead waiting for other thread to complete
  for (i = 1; i < num_thrd; i++)
 pthread_join (thread[i], NULL);
 
  /*printf("\n\n");
  print_matrix(A);
  printf("\n\n\t       * \n");
  print_matrix(B);
  printf("\n\n\t       = \n");
  print_matrix(C);
  printf("\n\n");*/
 
  free(thread);
 
  return 0;
 
}
