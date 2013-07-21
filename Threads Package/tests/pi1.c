/*  Multi-threaded computation of Pi.
 *
 *  Uses CS520 pre-emptive threads, but without mutexes or condition variables.
 *  Master does busy waiting on the slaves producing partial results.
 *
 *  Pi is the area under the curve f(x) = 4 / (1 + x*x) on the interval [0,1].
 *
 *  The area is approximated using numeric integration.
 *
 *  The number of threads to use is given as the only command-line argument.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

/* use cs520 threads */
#include "thread.h"

/* how finely to divide [0,1] during numeric integration */
#define INTERVALS 50000000

/* maximum number of threads */
#define MAX_T 24
#define makestring(x) # x

/* thread ids stored here, but they are not used */
static long pt[MAX_T];

/* threads place partial sums here */
static double partial_sum[MAX_T];

/* width of interval */
static double width = 1.0/INTERVALS;

/* number of intervals per thread */
static int chunk;

/* thread logical ids less than split have one extra interval */
static int split;

/* forward reference: function prototype */
static void work(void *);

int main (int argc, char *argv[])
{
  long i;
  double sum = 0.0;
  int n;

  if (argc != 2)
  {
    fprintf(stderr, "usage: pi numofthreads\n");
    exit(-1);
  }

  n = atoi(argv[1]);

  if (n <= 0)
  {
    fprintf(stderr, "usage: number of threads must be > 0\n");
    exit(-1);
  }

  if (n > MAX_T)
  {
    fprintf(stderr, "usage: number of threads must be <= %d\n", MAX_T);
    exit(-1);
  }

  printf("%d threads used\n", n);

  chunk = INTERVALS / n;
  split = INTERVALS % n;
  if (split == 0)
  {
    split = n;
    chunk -= 1;
  }

  for (i=0; i < n; i++)
  {
    partial_sum[i] = 0.0;

    /* create threads; DANGER: thread logical id (int) passed as "void *" */
    if ((pt[i] = thread_create(work, (void *) i)) == 0)
    {
      fprintf(stderr, "error in thread create\n");
      exit(-1);
    }
  }

  for (i = 0; i < n; i++)
  {
    /* wait for the i-th thread to finish */
    while (partial_sum[i] == 0.0)
    {
      // do nothing!
    }

    sum += partial_sum[i];
    fprintf(stderr, "main thread done with work(%d)\n", i);
  }

   sum *= 1.0/INTERVALS;

   printf ("Estimation of pi is %14.12f\n", sum);

   return 0;
}

void work(void * in)
{
  int i;
  int low;                    /* First interval to be processed */
  int high;   		      /* First interval *not* to be processed */
  double local_sum = 0.0;     /* Sum for intervals being processed */
  double x;                   /* Mid-point of an interval   */
  long id = (long) in;        /* Logical thread id (0..n-1) */

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
    x += width;
  }

  partial_sum[id] = local_sum;
  fprintf(stderr, "work %ld finished!\n", id);
}

