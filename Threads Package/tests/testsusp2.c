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
  tid2 = thread_create(thread2, &tid1);
  status(0,8);
  thread_yield();
  status(3,8);
  thread_yield();
  status(5,8);
  thread_yield();
  status(8,8);
}

void thread1(void* info) 
{
  status(1,8);
  thread_suspend(thread_self());
  status(6,8);
}

void thread2(void* info)
{
  long tid1 = *(long*)info;

  status(2,8);
  thread_yield();
  status(4,8);
  thread_continue(tid1);
  thread_yield();
  status(7,8);
}
