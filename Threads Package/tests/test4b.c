#include <stdio.h>
#define status(part,total) fprintf(stderr, "(%d/%d)\n", part, total)
extern void thread_yield();

main()
{
  status(0,1);
  thread_yield();
  status(1,1);
}
