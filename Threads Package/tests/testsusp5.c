#include <stdio.h>

#define status(part,total) fprintf(stderr, "(%d/%d)\n", part, total)

extern long thread_create(void (*)(void*), void*);
extern long thread_suspend(long thread_id);
extern long thread_kill(long thread_id);

void thread1(void* info);

main()
{
  long tid1;
  tid1 = thread_create(thread1, 0);

  if (thread_suspend(tid1) == 1) 
    //fprintf(stderr, "** Successful suspend of thread 1\n");
    status(0,2);
  if (thread_kill(tid1) == 1) 
    //fprintf(stderr, "** Successfull kill of suspended thread\n");
    status(1,2);
  else
    fprintf(stderr, "** Error: Suspended thread could not be killed\n");
  //fprintf(stderr, "** Exiting main thread\n");
  status(2,2);

}

void thread1(void* info) 
{
  thread_yield();
}
