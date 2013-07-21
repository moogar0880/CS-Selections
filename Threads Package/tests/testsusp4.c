#include <stdio.h>

#define status(part,total) fprintf(stderr, "(%d/%d)\n", part, total)

extern long thread_create(void (*)(void*), void*);
extern long thread_suspend(long thread_id);

void thread1(void* info);

main()
{
  long tid1;
  tid1 = thread_create(thread1, 0);

  if (thread_suspend(tid1) == 1){ 
    //fprintf(stderr, "** Thread 1 suspended once successfully\n");
    status(0,2);
  }
  else{
    fprintf(stderr, "** Error: Thread 1 not suspended even once\n");
  }
  if (thread_suspend(tid1) != 1)
    fprintf(stderr, "** Error: Suspended Thread 1 could not be suspended again\n");
  else
    //fprintf(stderr, "** Suspended Thread 1 suspended again successfully\n");
    status(1,2);
  status(2,2);
}

void thread1(void* info) 
{
  thread_yield();
}

