#include <stdio.h>

#define status(part,total) fprintf(stderr, "(%d/%d)\n", part, total)

extern long thread_create(void (*)(void*), void*);
extern void thread_yield();

void thread1(void* info);
void thread2(void* info);
void thread3(void* info);

main()
{
  thread_create(thread1, "thread one");
  thread_create(thread2, "thread two");
  thread_create(thread3, "thread three");
  status(0,8);
  thread_yield();
  status(4,8);
  thread_yield();
  status(8,8);
}

void thread1(void* info) 
{
  status(1,8);
  fprintf(stderr, "Hello, my name is %s\n", (char*)info);
  thread_yield();
  status(5,8);
}

void thread2(void* info) 
{
  status(2,8);
  fprintf(stderr, "Hello, my name is %s\n", (char*)info);
  thread_yield();
  status(6,8);
}

void thread3(void* info) 
{
  status(3,8);
  fprintf(stderr, "Hello, my name is %s\n", (char*)info);
  thread_yield();
  status(7,8);
  thread_yield();
}
