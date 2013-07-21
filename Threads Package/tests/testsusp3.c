#include <stdio.h>

#define status(part,total) fprintf(stderr, "(%d/%d)\n", part, total)

extern long thread_create(void (*)(void*), void*);
extern void thread_yield();
extern long thread_suspend(long thread_id);
extern long thread_continue(long thread_id);
extern long thread_self(void);

void thread1(void* info);
void thread2(void* info);

main()
{
  long tid1, tid2;
  tid1 = thread_create(thread1, 0);
  tid2 = thread_create(thread2, 0);

  if (thread_suspend(tid1) != 1) 
    fprintf(stderr, "** Incorrect return value for successful suspend\n");
  if (thread_suspend(tid1) != 1) 
    fprintf(stderr, "** Incorrect return value for successful suspend\n");
  if (thread_suspend(tid1+456) != 0) 
    fprintf(stderr, "** Incorrect return value for unsuccessful suspend\n");

  thread_suspend(tid2);
  thread_suspend(thread_self()); /* this should cause program to halt */

  fprintf(stderr, "** Error: only running thread able to suspend itself\n");
}

void thread1(void* info) 
{
  thread_yield();
}

void thread2(void* info)
{
  thread_yield();
}
