// interface for CS520 threads library
//
// Note: thread_mutex_t and thread_cond_t definitions are incomplete!
//

#define TCB struct TCB
#define thread_mutex_t struct thread_mutex_t
#define thread_cond_t struct thread_cond_t

long thread_create( void (*func)(void*), void* info );
void thread_start( void (*func)(void*), void* info );
void thread_yield( void );
void special_yield( void );
long thread_self(void);
static void init( void );
static int isValidID( long thread_id );
static void removeID( long thread_id );
static void handler( int signal );
void printRdyQueue( void );

long thread_create(void (*)(void*), void*);
void thread_yield(void);

TCB {
    int esi;
    int edi;
    int ebx;
    int esp;
    TCB* next;
    TCB* prev;
    int *stack;
    int isSuspended;
};

static void asm_yield( TCB* curr, TCB* next );
static void asm_specialyield( TCB* curr, TCB* curr2 );

thread_mutex_t {
    TCB* qHead;
    long ownerID;
    int  lock;
    int  init;
};

int thread_mutex_init(thread_mutex_t *);
int thread_mutex_lock(thread_mutex_t *);
int thread_mutex_unlock(thread_mutex_t *);
void printMutexQueue( thread_mutex_t *mutex );

thread_cond_t {
    TCB* qHead;
    long ownerID;
    int init;
};

int thread_cond_init(thread_cond_t *);
int thread_cond_wait(thread_cond_t *, thread_mutex_t *);
int thread_cond_signal(thread_cond_t *);

