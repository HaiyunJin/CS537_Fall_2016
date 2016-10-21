#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

void usage() {
    printf("Usage: stat_server -k key\n");
    exit(1);
}

int main(int argc, char *argv[] ) {
    if ( argc != 3 ) 
        usage();
    key_t key = (key_t) atoi(argv[2]);
    if ( key == 0 ) {
        perror("Invalid key\n");
        exit(1);
    }
    size_t pagesize = (size_t) getpagesize();
    int id  = shmget(key, pagesize, IPC_CREAT | SHM_R |SHM_W);
    char* addr = (char*)shmat(id, NULL, 0);

    while ( 1) {
        printf("Message[key %d]:",key);
        printf("%s\n", addr);
        sleep(1);
    }

    return 0;   
}

