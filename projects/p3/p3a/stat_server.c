#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "stats.h"
#include "defs.h"
#include <string.h>

void usage() {
    fprintf(stderr,"Usage: stat_server -k key\n");
    exit(1);
}

int main(int argc, char *argv[] ) {
    if ( argc != 3 ) usage();
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
        fprintf(stderr,"Invalid key\n");
        exit(1);
    }
    size_t pagesize = (size_t) getpagesize();
//     printf("pagesize: %d", pagesize);
    size_t buffersize = pagesize/NUMBER_OF_BUFFERS;
    int id  = shmget(key, pagesize, IPC_CREAT | SHM_R | SHM_W);
    char* pageaddr = (char*)shmat(id, NULL, 0);
    memset((void *)pageaddr, '\0', pagesize);

    int counter = 0;
    while (1) {
        /* Check client code for comments */
        int buffno;
        char* validbitaddr;
        char* buffaddr;
        for (buffno = 0 ; buffno < NUMBER_OF_BUFFERS ; ++buffno) {
        // Info to be printed
        // [server_iter] [pid] [argv[0]] [counter] [cpu_secs] [priority]
            validbitaddr = pageaddr + buffno*buffersize;
            buffaddr = validbitaddr + 1;
            if ( *validbitaddr != '\0' ) {
                fprintf(stdout,"%d ",counter);
        /* sample code */
                fprintf(stdout,"Message[key %d]:",key);
                fprintf(stdout,"%s\n", buffaddr);
        /* sample code */
            }
        }

        fprintf(stdout,"\n");
        ++counter;
        sleep(1);
    }

    return 0;   
}

