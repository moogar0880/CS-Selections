/* pi2.c (Uses CS520 threads) */

/* the master plays the role of slave 0 in this version */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

// CS520 threads
#include "thread.h"

#define INTERVALS 500000000

#define MAX_T 32

thread_mutex_t mu;              /* Mutex for "sum" and "bar_cnt" */

thread_cond_t cv;               /* Condition variable to hold master
                                       until all slaves done */

int cnt;                        /* Counter for exiting slaves */

double sum;                     /* Global sum of areas */

double width = 1.0/INTERVALS;   /* Width of interval */

int chunk;                      /* Number of intervals per thread */

int split;                      /* Processor ids less than split
                                      have one extra interval     */

int n;                          /* Number of threads to create */

void usage(char *);

void error(char *);

void work(void *);

int main (int argc, char *argv[])
{
   long i;

   if (argc !=2) usage("pi numofthreads\n");

   n = atoi(argv[1]);

   if (n <= 0) usage("number of threads must be > 0");

   printf("%d (slave) threads used\n", n);

   chunk = INTERVALS / n;
   split = INTERVALS % n;
   if (split == 0)
   {
      split = n;
      chunk -= 1;
   }

   if (thread_mutex_init(&mu) == 0)
      error("can't init mutex");

   if (thread_cond_init(&cv) == 0)
      error("can't init condition variable");

   cnt = 0;   /* number of slaves that have exited */

   sum = 0.0;

   for (i=1; i < n; i++)
   {
      /* create threads; DANGER: thread logical id (int) passed as "void *" */
      if (thread_create(work, (void *) i) == 0)
         error("error in thread create");
   }

   work(0);

   if (++cnt != n)
   {
      if (thread_cond_wait(&cv, &mu) == 0) /* wait for all slaves to finish */
         error("error in cond_wait by master");
   }

   sum *= 1.0/INTERVALS;

   printf ("Estimation of pi is %14.12f\n", sum);

   return 0;
}

void work(void * in)
{
   int i;
   int low;                           /* First interval to be processed */
   int high;			      /* First interval *not* to be processed */
   double local_sum = 0.0;            /* Sum for intervals being processed */
   double x;			      /* Mid-point of an interval   */
   long id = (long) in;                 /* Logical thread id (0..n-1) */

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

   x = (low+0.5)*width;
   for (i = low; i < high; i++)
   {
      local_sum += (4.0/(1.0+x*x));
      x += width;	              /* could cause round-off errors? */
   }

   if (thread_mutex_lock(&mu) == 0)
      error("error in mutex_lock in slave");

   sum += local_sum;

   if (id == 0)  /* master continues to hold lock! */
      return;

   if (++cnt == n)  /* all threads are now done */
   {
      if (thread_cond_signal(&cv) == 0)  /* wakeup master */
            error("error in cond_signal");
   }

   if (thread_mutex_unlock(&mu) == 0)
      error("error in mutex_unlock in slave");

}

void error(char *str)
{
    fprintf(stderr, "%s\n", str);
    exit(-1);
}

void usage(char *str)
{
   fprintf(stderr, "usage: %s\n",str);
   exit(-1);
}

