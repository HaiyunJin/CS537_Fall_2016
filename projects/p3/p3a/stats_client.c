#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>

#include "stats.h"
#include "defs.h"




void usage() {
    printf("Usage: stats_client -k key message\n");
    exit(1);
}

int main(int argc, char *argv[] ) {
    if ( argc != 4 ) usage();
    int opt;
    key_t key;
    while ((opt = getopt(argc, argv, "k:")) != -1) {
        switch (opt) {
        case 'k':
            key = (key_t) atoi(optarg);
            break;
        default:
            usage();
        }
    }    
    if ( key == 0 ) {
        perror("Invalid key");
        exit(1);
    }
    char *message = argv[3];

    /* 
     * To ensure there are no more than 16 clients, 
     * the page is divided into 16 regions.
     * Use the first char as valid bit, '\0' if free, otherwise used.
     * The data is followed at validbitaddr+1.
     */
    size_t pagesize = (size_t) getpagesize();
    size_t buffersize = pagesize/NUMBER_OF_BUFFERS;
    int id  = shmget(key, pagesize, IPC_CREAT | SHM_R | SHM_W);
    char* pageaddr = (char*)shmat(id, NULL, 0);
    // find free buffer
    int buffno;
    for (buffno = 0 ; buffno < NUMBER_OF_BUFFERS ; ++buffno) {
        if ( *(pageaddr + buffno*buffersize) == '\0' ) break;
    }
    if ( buffno == NUMBER_OF_BUFFERS ) {
        fprintf(stderr,"No more free buffers!\n");
        exit(1);
    }
    char* validbitaddr;
    char* buffaddr;
    validbitaddr = pageaddr + buffno*buffersize;
    buffaddr = validbitaddr + 1;

    // set valid
    *validbitaddr = '1'; // any char works except '\0'


    /* Simple demo code */
    int i = 0;
    while (message[i] != '\0') {
        buffaddr[i] = message[i];
        ++i;
    }
    buffaddr[i] = '\0';
    /* End Simple demo code */

    return 0;
}

