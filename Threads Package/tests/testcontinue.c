#include <stdio.h>

#define status(part,total) fprintf(stderr, "(%d/%d)\n", part, total)

extern long thread_create(void (*)(void*), void*);
extern long thread_continue(long thread_id);
extern void thread_yield();
extern long thread_suspend(long thread_id);
extern long thread_self(void);

void thread1(void* info);

main()
{
  long tid1;
  tid1 = thread_create(thread1, 0);

  status(0,2);
  if (thread_continue(tid1) != 0) 
    fprintf(stderr, "** Error: Unsuspended thread being continued\n");
  thread_suspend(tid1);
  status(1,2);
  if (thread_continue(tid1) != 1)
   fprintf(stderr, "** Error: Incorrect return value for successful continue\n");
  if (thread_continue(tid1 + 456) != 0)
    fprintf(stderr, "** Error: Incorrect return value for unsuccessful continue\n");
  status(2,2);
}

void thread1(void* info) 
{
  thread_yield();
}

