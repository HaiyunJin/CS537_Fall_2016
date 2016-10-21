
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

void usage() {
    printf("Usage: stats_client -k key message\n");
    exit(1);
}

int main(int argc, char *argv[] ) {
    if ( argc != 4 )
        usage();
    key_t key = (key_t) atoi(argv[2]);
    if ( key == 0 ) {
        perror("Invalid key");
        exit(1);
    }
    char *message = argv[3];

    size_t pagesize = (size_t) getpagesize();
    int id  = shmget(key, pagesize, IPC_CREAT | SHM_R |SHM_W);
    char* addr = (char*)shmat(id, NULL, 0);

    int i = 0;
    while (message[i] != '\0' ) {
        addr[i] = message[i];
        ++i;
    }
    addr[i] = '\0';
    return 0;
}

