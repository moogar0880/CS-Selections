#include <stdio.h>

#define status(part,total) fprintf(stderr, "(%d/%d)\n", part, total)

extern long thread_create(void (*)(void*), void*);
extern void thread_yield();
extern long thread_suspend(long thread_id);
extern long thread_continue(long thread_id);
extern long thread_self(void);
extern long thread_kill(long thread_id);

void thread1(void* info);

main()
{
  long tid1,tidmain;
  
  tidmain = thread_self();
  tid1 = thread_create(thread1, &tidmain);
  //fprintf(stderr, " Main thread is now yielding itself\n");
  status(0,4);
  thread_yield();
  status(4,4);

}

void thread1(void* info) 
{
  long tidmain = *(long*)info;
  status(1,4);
  if(thread_suspend(tidmain) ==1)
    //fprintf(stderr, " ** Main thread successfully suspended from thread 1\n");
    status(2,4);
  else
    fprintf(stderr," **Error: Main thread could not be suspended from thread 1\n");
  if(thread_kill(thread_self()) == 1)
    fprintf(stderr," **Error: Only running thread trying to kill itself\n");
  thread_continue(tidmain);  
  status(3,4);
}

