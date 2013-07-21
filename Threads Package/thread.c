/** Jonathan Nappi Program6 CS520
 *  main.c
 *  Program6
 *
 *  Created by Jonathan Nappi on 4/6/12.
 *
 *  Custom threads package capable of generating new threads as well as
 *  suspending, killing, and continuing any valid thread ID that is passed in
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include "thread.h"

TCB *head, *tail, *current, *deathQ, *suspendQ;
long validAddresses[1000];
int lastValidAddress = 0;
int mainAddress, initDone = 0;
int notBusy = 1;

/**
 * This function creates a thread, starts it, and puts it at the end of the
 * thread queue
 */
long thread_create( void (*func)(void*), void* info )
{
    notBusy = 0;
	if( !initDone )
		init( );
    TCB *new = malloc( sizeof(TCB ) );
    int* stack = malloc(16384);
    stack[4095] = (int)info; //push info onto bottom of the stack
    stack[4094] = (int)func;
    stack[4092] = (int)thread_start;
    new->esp = (int)&stack[4091];
    new->next = 0;
    new->prev = tail;
    new->esi = new->edi = new->ebx = 0;
    new->isSuspended = 1;

    tail->next = new;
    tail = new;
    
    validAddresses[lastValidAddress] = (long)new;
    lastValidAddress++;
    notBusy = 1;
    return (long)new;
}

/**
 * This function pauses the currently executing thread, moves it to the
 * suspended queue, and then moves the next thread to be executed
 */
void thread_yield( )
{
    if( head != tail && head->next != 0)
    {
        notBusy = 0;
    	TCB *oldCur = head;
    	head = head->next;
        head->prev = 0;
    	tail->next = oldCur;
        oldCur->prev = tail;
    	tail = oldCur;
    	oldCur->next = 0;
        asm_yield( oldCur, head );
        notBusy = 1;
    }
}

/**
 * This function starts a thread with the parameters passed in
 */
void thread_start( void (*func)(void*), void* info )
{
    notBusy = 1;
    func( info );
    special_yield();
}

/**
 * This function is called when a thread has finished executing it's method
 * and can now terminate.
 */
void special_yield(  )
{
    notBusy = 0;
	if( deathQ )
	{
		free( deathQ->stack );
		free( deathQ );
	}
	TCB *oldCur = head;
    head = head->next;
    head->prev = 0;
    oldCur->next = 0;
    deathQ = oldCur;
    
   	asm_specialyield( head, head );
}

/**
 * This function handles initializing a TCB for the main function as well as]
 * initializing all global variables
 */
static void init( )
{
	deathQ = suspendQ = 0;
	
	TCB *main = malloc(sizeof(TCB));
    head = tail = current = main;
    main->stack = malloc(16384);
    mainAddress = (int)&main;
    main->next = 0;
    main->prev = 0;
    main->isSuspended = 1;
    
    validAddresses[0] = (long)main;
    lastValidAddress = 1;
    
    struct sigaction* sa = malloc( sizeof( sigaction ) );
	sa->sa_handler = handler;
	
	sigemptyset( &sa->sa_mask );
	
	sa->sa_flags = SA_NODEFER;
	
	sigaction( SIGVTALRM, sa, NULL );
	
	struct itimerval* it = malloc( sizeof( struct itimerval ) );
	it->it_interval.tv_sec = 0;
	it->it_interval.tv_usec = 10000;
	it->it_value.tv_sec = 0;
	it->it_value.tv_usec = 10000;
	setitimer( ITIMER_VIRTUAL, it, NULL );
    
    initDone = 1;
}

/**
 * This is a simple handler function called by the signal timer
 */
static void handler( int signal )
{
    if( notBusy )
        thread_yield();
}

/**
 * This function initializes the mutex variable passed in
 */
int thread_mutex_init( thread_mutex_t *mutex )
{
    notBusy = 0;
    if( mutex != NULL )
    {
        if( mutex->init != 1 )
        {
            mutex->lock = 0;
            mutex->ownerID = -1;
            mutex->qHead = 0;
            mutex->init = 1;
            notBusy = 1;
            return 1;
        }
        notBusy = 1;
        return 0;
    }
    else
    {
        notBusy = 1;
        return 0;
    }
}

/**
 * This function locks the mutex on the current thread or if there are already
 * threads waiting on this mutex at which point the calling thread is then
 * added to this mutex's queue
 */
int thread_mutex_lock( thread_mutex_t *mutex )
{
    notBusy = 0;
    if( mutex != NULL )
    {
        if( mutex->ownerID == -1 ) //no one owns this mutex
        {
            mutex->ownerID = (long)head;
            mutex->lock = 1;
            notBusy = 1;
            return 1;
        }
        else if( mutex->ownerID == (long)head ) //you're the owner
        {
            notBusy = 1;
            return 0;
        }
        else //mutex is currently locked
        {
            if( mutex->qHead != 0 )
            {
                TCB *scan = mutex->qHead;
                while( scan->next )
                    scan = scan->next;
                TCB* oldcur = head;
                head = head->next;
                head->prev = 0;
                scan->next = oldcur;
                oldcur->next = 0;
                oldcur->prev = scan;
                asm_yield( oldcur, head );
            }
            else 
            {
                TCB* oldcur = head;
                head = head->next;
                head->prev = 0;
                oldcur->prev = oldcur->next = 0;
                mutex->qHead = oldcur;
                asm_yield( oldcur, head );
            }
            notBusy = 1;
            return 1;
        }
    }
    else
    {
        notBusy = 1;
        return 0;
    }
}

/**
 * This function unlocks the passed in mutex if it is locked. If there were any
 * threads on the mutex's queue then the longest waiting thread is given the
 * the lock
 */
int thread_mutex_unlock( thread_mutex_t *mutex )
{
    notBusy = 0;
    if( mutex->ownerID == (long)head )
    {
        mutex->lock = 0;
        mutex->ownerID = -1;
        if( mutex->qHead != 0 )
        {
            TCB* getsLock = mutex->qHead;
            mutex->qHead = mutex->qHead->next;
            mutex->ownerID = (long)getsLock;
            mutex->lock = 1;
            tail->next = getsLock;
            getsLock->prev = tail;
            getsLock->next = 0;
            tail = getsLock;
        }
        notBusy = 1;
        return 1;
    }
    notBusy = 1;
    return 0;
}

/**
 * This function initializes the passed in cond variable
 */
int thread_cond_init( thread_cond_t *cond )
{
    notBusy = 0;
    if( cond != NULL )
    {
        if( cond->init != 1 )
        {
            cond->init = 1;
            cond->qHead = 0;
            return 1;
        }
        notBusy = 1;
        return 0;
    }
    else
    {
        notBusy = 1;
        return 0;
    }
}

/**
 * This function unlocks the passed in mutex and gives the lock to the currently
 * executing thread which also blocks on the passed in condition variable. If
 * there are already threads blocked on the cond variable, then the calling
 * thread is added to the cond's queue.
 */
int thread_cond_wait( thread_cond_t *cond, thread_mutex_t *mutex )
{
    notBusy = 0;
    if( cond == NULL || mutex == NULL )//cond or mutex are NULL
    {
        notBusy = 1;
        return 0;
    }
    
    if( mutex->ownerID != (long)head ) //calling thread not locked by mutex
    {
        notBusy = 1;
        return 0;
    }
    
    if( cond->ownerID != -1 )
    {
        TCB* scan = cond->qHead;
        while( scan->next )
            scan = scan->next;
        scan->next = head;
        TCB* oldcur = head;
        oldcur->prev = scan;
        head = head->next;
        head->prev = 0;
        oldcur->next = 0;
        cond->qHead = oldcur;
        if( mutex->qHead != 0 )
        {
            TCB *scan = mutex->qHead;
            while( scan->next )
                scan = scan->next;
            TCB* oldcur = (TCB*)mutex->ownerID;
            scan->next = oldcur;
            mutex->ownerID = (long)head;
        }
        else 
        {
            TCB* oldcur = (TCB*)mutex->ownerID;
            mutex->qHead = oldcur;
            mutex->ownerID = (long)head;
        }
        asm_yield( oldcur, head );
    }
    else
    {
        cond->ownerID = (long)head;
    }
    
    if( mutex->qHead != 0 )
    {
        TCB *scan = mutex->qHead;
        while( scan->next )
            scan = scan->next;
        TCB* oldcur = (TCB*)mutex->ownerID;
        scan->next = oldcur;
        mutex->ownerID = (long)head;
    }
    else 
    {
        TCB* oldcur = (TCB*)mutex->ownerID;
        mutex->qHead = oldcur;
        mutex->ownerID = (long)head;
    }
    notBusy = 1;
    return 1;
}

/**
 * This function unblocks the thread that has been blocked on this cond the
 * longest
 */
int thread_cond_signal( thread_cond_t *cond )
{
    notBusy = 0;
    if( cond == NULL )
    {
        notBusy = 1;
        return 0;
    }
    return 1;
}