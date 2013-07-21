//
// Use CS520 mutex to protect access to shared variables.
//   This version uses busy waiting by the main thread.
//
 
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

// CS520 threads
#include "thread.h"

// number of additions to perform
#define N 50000

// number of trials to perform
#define TRIALS 100

// mutex to protect shared globals
thread_mutex_t mu;

// counter for exiting threads
int cnt;

// global sum
int sum;

// number of additions per thread
int chunk;

// processor ids less than split have one extra additions
int split;

// number of threads to create
long n;

// local function prototypes
void initialize(void);
void error(char *);
void work(void *);

int main (int argc, char *argv[])
{
   long i;
   int j;
   int correctSum = 0;
   int trials = TRIALS;
   int passed = 0;
   char *chkArg;

   if (argc != 2) 
   {
     fprintf(stderr, "Usage: sync numberOfThreads\n");
     exit(-1);
   }

   n = strtol(argv[1], &chkArg, 10);

   // validate the command-line argument

   // errno would indicate overflow,
   // null string is also not allowed of course,
   // and, the full argument must be consumed
   if (errno || (*argv[1]  == '\0') || (*chkArg != '\0'))
   {
     fprintf(stderr, "invalid number of threads\n");
     exit(-1);
   }

   // finally, given integer must be in the correct range
   if (n <= 0)
   {
     fprintf(stderr, "number of threads must be > 0\n");
     exit(-1);
   }

   printf("using %d child threads\n", n);

   // compute the correct sum
   for (i = 0; i < N; i++)
   {
     correctSum += i;
   }

   // distribute the work
   chunk = N / n;
   split = N % n;
   if (split == 0)
   {
      split = n;
      chunk -= 1;
   }

   // initialize the mutex
   if (thread_mutex_init(&mu) == 0)
      error("can't init mutex");

   // do the trials
   for (j = 0; j < trials; j++)
   {
     cnt = 0;   // number of child threads that have exited

     sum = 0;

     for (i=0; i < n; i++)
     {
        // create threads; DANGER: thread logical id (int) passed as "void *" 
        if (thread_create(work, (void *) i) == 0)
           error("error in thread create");
     }

     // wait for all children to finish
     while (cnt != n)
     {
     }

     if (sum == correctSum)
     {
       
       passed += 1;
	//fprintf( stdout, "passed = %d", passed );
     }
  }

  printf ("%d of %d trials passed\n", passed, trials);

  return 0;
}

void work(void * in)
{
   // loop variable
   int i;

   // first addition to be done
   int low;

   // first addition *not* to be done
   int high;

   // logical thread id (0..n-1)
   long id = (long) in;

   // figure out exactly what work is to be done by this thread
   if (id < split)
   {
      low = (id * (chunk + 1));
      high = low + (chunk + 1);
   }
   else
   {
      low = (split * (chunk + 1)) + ((id - split) * chunk);
      high = low + chunk;
   }

   // do the work
   int localsum = 0;
   for (i = low; i < high; i++)
   {
      localsum += i;
   }

   // lock the mutex before updating global sum and cnt
   if (thread_mutex_lock(&mu) == 0)
      error("error in mutex_lock in child thread");

   sum += localsum;
   cnt += 1;

   if (thread_mutex_unlock(&mu) == 0)
      error("error in mutex_unlock in child thread");

   return;
}

void error(char *str)
{
    fprintf(stderr, "%s\n", str);
    exit(-1);
}

