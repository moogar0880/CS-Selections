#
# Makefile for thread tests
#

CC = gcc
CFLAGS = -g
EXES = pi1 pi2 sync0 sync1 sync2 \
       test1 test2 test3 test4a test4b test5 testkill1 testkill2 testkill3 \
       testkill4 testsusp1 testsusp2 testsusp3 testsusp4 testsusp5 testcontinuecleanup

all: $(EXES)

pi1: pi1.o thread.o thr_asm.o
	$(CC) $(CFLAGS) pi1.o thread.o thr_asm.o -o pi1

pi2: pi2.o thread.o thr_asm.o
	$(CC) $(CFLAGS) pi2.o thread.o thr_asm.o -o pi2

sync0: sync0.o thread.o thr_asm.o
	$(CC) $(CFLAGS) sync0.o thread.o thr_asm.o -o sync0

sync1: sync1.o thread.o thr_asm.o
	$(CC) $(CFLAGS) sync1.o thread.o thr_asm.o -o sync1

sync2: sync2.o thread.o thr_asm.o
	$(CC) $(CFLAGS) sync2.o thread.o thr_asm.o -o sync2

test1: test1.o thread.o thr_asm.o
	$(CC) $(CFLAGS) test1.o thread.o thr_asm.o -o test1

test2: test2.o thread.o thr_asm.o
	$(CC) $(CFLAGS) test2.o thread.o thr_asm.o -o test2

test3: test3.o thread.o thr_asm.o
	$(CC) $(CFLAGS) test3.o thread.o thr_asm.o -o test3

test4a: test4a.o thread.o thr_asm.o
	$(CC) $(CFLAGS) test4a.o thread.o thr_asm.o -o test4a

test4b: test4b.o thread.o thr_asm.o
	$(CC) $(CFLAGS) test4b.o thread.o thr_asm.o -o test4b

test5: test5.o thread.o thr_asm.o
	$(CC) $(CFLAGS) test5.o thread.o thr_asm.o -o test5

testkill1: testkill1.o thread.o thr_asm.o
	$(CC) $(CFLAGS) testkill1.o thread.o thr_asm.o -o testkill1

testkill2: testkill2.o thread.o thr_asm.o
	$(CC) $(CFLAGS) testkill2.o thread.o thr_asm.o -o testkill2

testkill3: testkill3.o thread.o thr_asm.o
	$(CC) $(CFLAGS) testkill3.o thread.o thr_asm.o -o testkill3

testkill4: testkill4.o thread.o thr_asm.o
	$(CC) $(CFLAGS) testkill4.o thread.o thr_asm.o -o testkill4

testsusp1: testsusp1.o thread.o thr_asm.o
	$(CC) $(CFLAGS) testsusp1.o thread.o thr_asm.o -o testsusp1

testsusp2: testsusp2.o thread.o thr_asm.o
	$(CC) $(CFLAGS) testsusp2.o thread.o thr_asm.o -o testsusp2

testsusp3: testsusp3.o thread.o thr_asm.o
	$(CC) $(CFLAGS) testsusp3.o thread.o thr_asm.o -o testsusp3

testsusp4: testsusp4.o thread.o thr_asm.o
	$(CC) $(CFLAGS) testsusp4.o thread.o thr_asm.o -o testsusp4

testsusp5: testsusp5.o thread.o thr_asm.o
	$(CC) $(CFLAGS) testsusp5.o thread.o thr_asm.o -o testsusp5

testcontinue: testcontinue.o thread.o thr_asm.o
	$(CC) $(CFLAGS) testcontinue.o thread.o thr_asm.o -o testcontinue

thread.o: thread.c
	$(CC) $(CFLAGS) -c thread.c

thr_asm.o: thr_asm.s
	$(CC) $(CFLAGS) -c thr_asm.s

clean:
	-rm -f *.o $(EXES)

cleanup:
	-rm -f *.o

