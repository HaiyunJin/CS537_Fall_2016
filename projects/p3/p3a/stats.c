#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "stats.h"
#include "defs.h"

#include <fcntl.h>          /* For O_* constants */
#include <sys/stat.h>       /* For mode constants */
#include <semaphore.h>

extern sem_t *mutex;
extern char * validbitaddr;

stats_t* stats_init(key_t key) {
if(debug) printf("Entering stats_init\n");
    /* 
     * To ensure there are no more than 16 clients, 
     * the page is divided into 16 regions.
     * Use the first char as valid bit, '\0' if free, otherwise used.
     * The data is followed at validbitaddr+1.
     */
    size_t pagesize = (size_t) getpagesize();
    size_t buffersize = pagesize/NUMBER_OF_BUFFERS;
    int id;
    if ( (id = shmget(key, pagesize, SHM_R|SHM_W)) == -1 ) {
        perror("shmget error: No memory created by server");
        exit(1);
    }
if(debug) { printf("id %d\n", id); }
    char* pageaddr = (char*)shmat(id, NULL, 0);
// if(debug) printf("");
    /* find free buffer */
    int buffno;

    /* find semaphore and acquire lock, set valid bit */
    int found = 0;
    for (buffno = 0 ; buffno < NUMBER_OF_BUFFERS ; ++buffno) {
if(debug) { printf("client use  mutex %ld, at %p\n", mutex->__align, mutex); }
        sem_wait(mutex);  // entering critical section
if(debug) { printf("mutex wait call   %ld, at %p\n", mutex->__align, mutex); }
        if ( *(pageaddr + buffno*buffersize) == '\0' ) {
if(debug) printf("After find free buffer\n");
            /* set valid bit and content*/
            validbitaddr = pageaddr + buffno*buffersize;
            *validbitaddr = '0';  // any char works except '\0'
if(debug) { printf("After set validbitaddr\n"); }
            found = 1;
        }
        sem_post(mutex);   // end of critical section
if(debug) { printf("mutex post call   %ld, at %p\n", mutex->__align, mutex); }
        if (found) break;
    }
    if ( buffno == NUMBER_OF_BUFFERS ) {
        return NULL;
    }

    stats_t* buffaddr;
    buffaddr = (stats_t*)(pageaddr + buffno*buffersize + 1);

    return buffaddr;
}

int stats_unlink(key_t key) {
    if ( *validbitaddr == '\0' ) return -1;  // the process did not call init
    sem_wait(mutex);  // entering critical section
    *validbitaddr = '\0';  // '\0' for buffer not in-use
    sem_post(mutex);  // end of critical section
    return 0;
}
