//  Jonathan Nappi cs520 Program4
//  main.c
//  Program4
//
//  Created by Jon Nappi on 3/24/12.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>

#define HASHSIZE 2048
#define MAXWORDLEN 50
#define MAXTOPWORDS 20
#define MAXTHREADS 9
#define HashEntry   struct HashEntry
#define Parameter   struct Parameter

HashEntry {
    HashEntry *prev;
	HashEntry *next;
	char    word[MAXWORDLEN];
	int     wordCount;
	int     status;
    pthread_mutex_t myMutex;
};

Parameter {
    int chunkSize;
    int acceptableStatus;
    int id;
    char *buf;
};

pthread_mutex_t mutex;
pthread_cond_t	cond;
pthread_t threads[MAXTHREADS];
int deadThreads = 0;
HashEntry *hashTable[HASHSIZE];
HashEntry *head, *tail;

int  hash( char *word );
void add( char *word, int acceptableStatus );
int  buildWord( int i, char *buf, int acceptableStatus );
int  getThreadCounts( char *fileName );
int  getFileSizes( void* p );
void printCounts( void );
void* count( void* p );
void processTable( int acceptableStatus );
void insertWCount( HashEntry *tmp );

int main (int argc, const char * argv[])
{
    head = tail = 0;
    if(argc == 1)
    {
        fprintf(stdout, "Error: No file given to be read\n");
        exit( -1 );
    }

	if(pthread_mutex_init(&mutex, NULL) != 0)
      	fprintf( stderr, "can't init mutex\n");

   	if(pthread_cond_init(&cond, NULL) != 0)
      	fprintf( stderr, "can't init condition variable\n");

    int i = 1;
    while( i < argc )
    {
        int fd = open( argv[i], O_RDONLY );
        
        if( fd == -1 )//file open fails
        {
            fprintf(stderr, "Can't open %s for reading!\n", argv[i] );
        }
        else//file open succeeded
        {
            int j = 0;
            deadThreads = 0;
            int buflen = (int)lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);
            char buf[buflen];
            read( fd, buf, buflen );
            while( j < MAXTHREADS )
            {
                Parameter *param = malloc( sizeof( Parameter ) );
                param->acceptableStatus = i;
                if( buflen >= MAXTHREADS )
                    param->chunkSize = ( buflen/MAXTHREADS);
                else
                    param->chunkSize = buflen;
                param->id = j;
                param->buf = buf;
                if(pthread_create(&threads[j], NULL, count, (void*) param) != 0)
                    fprintf( stderr, "error in thread create\n");
                ++j;
            }
        }    
        close(fd);
        if( deadThreads != MAXTHREADS )
        {
            if( pthread_cond_wait( &cond, &mutex ) != 0)
                fprintf( stderr, "error in cond_wait by master\n" );
        }
        ++i;
    }
    
    processTable( argc - 1 );
    printCounts( );
    return 0;
}

/*
 * This function opens and reads the file by processing all of the words as
 * they come in. It then closes the file.
 */
void* count( void* p )
{
    Parameter *params = (Parameter*)p;
    int minRange;
    if( params->id == 0 )
        minRange = 0;
    else
        minRange = params->chunkSize * params->id - 1; 
    int maxRange = minRange + params->chunkSize - 1;
    
    int i = minRange;
    if( params->id != 0 )
    {
        if( isalpha(params->buf[i]) )
        {
            while( isalpha(params->buf[i]) )//skip until first non-alpha reached
                ++i;
        }
    }
    
    while( i < maxRange )
    {
        if( isalpha(params->buf[i]) )
        {
            char word[MAXWORDLEN];
            int j = 0;
            while( isalpha(params->buf[i]) )
            {
                word[j] = tolower( params->buf[i] );
                ++j;
                ++i;
            }
            if( j > 5 && j < MAXWORDLEN )
            {
                word[j] = '\0';
                add( word, params->acceptableStatus );
            }
        }
        ++i;
    }
    
    if( ++deadThreads == MAXTHREADS )//all threads are now done
   	{
      	if( pthread_cond_signal( &cond ) != 0 )//wakeup master
            fprintf( stderr, "error in cond_signal\n" );
   	}
    return 0;
}

/*
 * This function begins building a c-style string off of the information passed
 * from it's calling function of the last byte read, the file descriptor, and
 * the buffer that the read function is using
 */
int buildWord( int i, char *buf, int acceptableStatus )
{
    char word[MAXWORDLEN];
    int j = 0;
    while( isalpha(buf[i]) )
    {
        word[j] = tolower( buf[i] );
        ++j;
        ++i;
    }
    if( j > 5 && j < MAXWORDLEN )
    {
        word[j] = '\0';
        add( word, acceptableStatus );
    }
    return i;
}

/*
 * This function adds the passed in word into the hash table, or, if the word
 * is already in the hash table, incriments that words count
 */
void add( char *word, int acceptableStatus )
{
    int hashIndex = hash( word );
    if( acceptableStatus == 1 )
    {
        pthread_mutex_lock(&mutex);
        if( hashTable[hashIndex] == NULL )//hashIndex empty
        {
            HashEntry *new = malloc( sizeof(HashEntry) );
            strcpy( new->word, word );
            new->next = new->prev = 0;
            new->wordCount = 1;
            new->status = 1;
            if(pthread_mutex_init(&new->myMutex, NULL) != 0)
                fprintf( stderr, "can't init mutex\n");
            hashTable[hashIndex] = new;
        }
        else
        {
            HashEntry *tmp = hashTable[hashIndex];
            while( tmp )
            {
                if( strcmp( tmp->word, word ) == 0 ) //word found at hashIndex
                {
                    tmp->wordCount = tmp->wordCount + 1;
                    if( tmp->status == ( acceptableStatus - 1 ) )
                        tmp->status = tmp->status + 1;
                    break;
                }
                else if( tmp->next == 0 ) //word not found
                {                                                  //at hashIndex
                    HashEntry *new = malloc( sizeof(HashEntry) );
                    strcpy( new->word, word );
                    new->wordCount = 1;
                    new->status = 1;
                    if(pthread_mutex_init(&new->myMutex, NULL) != 0)
                        fprintf( stderr, "can't init mutex\n");
                    new->next = 0;
                    new->prev = tmp;
                    tmp->next = new;
                    break;
                }
                else
                    tmp = tmp->next;
            }
        }
        pthread_mutex_unlock(&mutex);
    }
    else
    {
        HashEntry *tmp = hashTable[hashIndex];
        while( tmp )
        {
            if( strcmp( tmp->word, word ) == 0 ) //word found at hashIndex
            {
                pthread_mutex_lock(&tmp->myMutex);
                tmp->wordCount = tmp->wordCount + 1;
                if( tmp->status == ( acceptableStatus - 1 ) )
                    tmp->status = tmp->status + 1;
                pthread_mutex_unlock(&tmp->myMutex);
                break;
            }
            tmp = tmp->next;
        }
    }
}

/*
 * This is a hash function that returns a words index in the hash table
 */
int hash( char *word )
{
    char w[MAXWORDLEN];
    strcpy( w, word );
    int length = strlen(w);
    unsigned int hash = 0;
    int i;
    for( i = 0; i < length; i++ )
        hash = 33 * hash ^ w[i];
    
    return hash & 0x7ff;
}

/*
 * This function goes through the entire hash table and prints out all of the
 * words and their counts. Only run after cleanup.
 */
void printCounts( )
{
    fprintf(stdout, "----------------------------\n");
	int i = 0;
    HashEntry *tmp = head;
    if( tmp == NULL )
    {
        fprintf(stdout, "No Words Found in All Files\n" );
        exit(0);
    }
    while( tmp != NULL && i < MAXTOPWORDS )
    {
        fprintf( stdout, "#%d:\t%s\n", i+1, tmp->word );
        tmp = tmp->next;
        ++i;
    }
    if( tmp && tmp->next )
    {
        while( tmp->next->wordCount == tmp->wordCount )
        {
            fprintf( stdout, "#%d:\t%s\n", i, tmp->word );
            tmp = tmp->next;
        }
    }
    fprintf(stdout, "----------------------------\n" );
    exit(0);
}

/*
 * This function searches through the table for a list of all the words that
 * have been found in all files to be put into the print queue
 */
void processTable( int acceptableStatus )
{
    int i = 0;
    while( i < HASHSIZE )
    {
        HashEntry *tmp = hashTable[i];
        while( tmp )
        {
            if( tmp->status == acceptableStatus )
                insertWCount( tmp );
            tmp = tmp->next;
        }
        ++i;
    }
}

/*
 * This function inserts the words found to be applicable for the print queue
 * into the print queue
 */
void insertWCount( HashEntry *tmp )
{
    HashEntry *new = malloc( sizeof( HashEntry ) );
    strcpy(new->word, tmp->word);
    new->wordCount = tmp->wordCount;
    new->next = new->prev = 0;
    
    if( head != 0 )
    {
        HashEntry *scan = head;
        while( scan != tail && scan->wordCount > new->wordCount )
        {
            scan = scan->next;
        }
        
        if( scan == head && scan == tail )
        {
            if( new->wordCount > scan->wordCount )
            {
                new->next = head;
                head->prev = new;
                head = new;
            }
            else
            {
                new->prev = tail;
                tail->next = new;
                tail = new;
            }
        }
        else if( scan == head )
        {
            new->next = head;
            head->prev = new;
            head = new;
        }
        else if( scan == tail )
        {
            if( scan->wordCount > new->wordCount )
            {
                new->prev = tail;
                tail->next = new;
                tail = new;
            }
            else
            {
                new->next = tail;
                new->prev = tail->prev;
                if( tail->prev )
                    tail->prev->next = new;
                tail->prev = new;
            }
        }
        else
        {
            new->next = scan;
            new->prev = scan->prev;
            if( scan->prev )
                scan->prev->next = new;
            
            scan->prev = new;  
        }
    }
    else
        head = tail = new;
}