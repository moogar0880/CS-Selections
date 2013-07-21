#include <stdio.h>

#define status(part,total) fprintf(stderr, "(%d/%d)\n", part, total)

extern long thread_create(void (*)(void*), void*);
extern void thread_yield();

void thread1(void* info);

main()
{
  thread_create(thread1, "info passed correctly");
  status(0,2);
  thread_yield();
  status(2,2);
}

void thread1(void* info) 
{
  status(1,2);
  fprintf(stderr, "%s\n", (char*)info);
  thread_yield();
  status(3,2); /* error if program gets here */
}
