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

  if (thread_kill(tid1) != 1) 
    fprintf(stderr, "** Incorrect return value for successful kill\n");
  if (thread_kill(tid1+456) != 0) 
    fprintf(stderr, "** Incorrect return value for unsuccessful kill\n");

//  thread_kill(tid2);

  if (thread_kill(thread_self()) == 1)
    fprintf(stderr, "** incorrect return value for attempt to kill main\n");
  status(0,2);
  thread_kill(tid2);
  status(1,2);
  thread_yield();
  status(2,2);
}

void thread1(void* info) 
{
  fprintf(stderr, "** error: killed thread 1  running\n");
  thread_yield();
}

void thread2(void* info)
{
  fprintf(stderr, "** error: Attempt to kill main successful\n");
  thread_yield();
}
