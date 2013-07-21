#include <stdio.h>

#define status(part,total) fprintf(stderr, "(%d/%d)\n", part, total)

extern long thread_create(void (*)(void*), void*);
extern void thread_yield();
extern long thread_suspend(long thread_id);
extern long thread_continue(long thread_id);
extern long thread_self(void);
extern long thread_kill(long thread_id);

void thread1(void* info);
void thread2(void* info);

main()
{
  long tid1, tid2;

  tid1 = thread_create(thread1, 0);
  tid2 = thread_create(thread2, 0);
  status(0,5);
  thread_yield();
  status(3,5);
  thread_kill(tid2);
  thread_yield();
  status(5,5);
}

void thread1(void* info) 
{
  status(1,5);
  thread_yield();
  status(4,5);
}

void thread2(void* info)
{
  status(2,5);
  thread_yield();
  fprintf(stderr, "** error: killed thread is executing\n");
}
