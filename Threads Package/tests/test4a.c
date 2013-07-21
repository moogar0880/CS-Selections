#include <stdio.h>

#define status(part,total) fprintf(stderr, "(%d/%d)\n", part, total)
extern void thread_create(void (*)(void*), void*);
extern void thread_yield();

void thread1(void*);

main()
{
  thread_create(thread1, 0);
  thread_yield();

  /* at this point, this is the only thread */
  thread_yield();
  //fprintf(stderr," **Only running thread yielded and continued\n");
  status(1,1);
}

void thread1(void* fubar) 
{
  status(0,1);
  //fprintf(stderr," ** Thread 1 running after main yielded\n");
  /* does nothing */
}
