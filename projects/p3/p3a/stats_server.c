#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "stats.h"
#include "defs.h"
#include <string.h>

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>

#include <signal.h>         /* signal handling */

int id;  // shared memory id: shmid

void exit_handler(int signo) {
    // 1. remove page
    shmctl(id, IPC_RMID, NULL);
    // 2. remove semaphore
    sem_unlink("sem_haiyun_zhewen");
if(debug) { printf("Exit!!!!\n"); }
    exit(0);
}

void usage() {
    fprintf(stderr, "Usage: stat_server [-k key]\n");
    exit(1);
}

int main(int argc, char *argv[] ) {
    /* Signal handling*/
    struct sigaction myexit;
    myexit.sa_handler = exit_handler;
    sigaction(SIGINT, &myexit, NULL);
    sigaction(SIGTERM, &myexit, NULL);
    sigaction(SIGTSTP, &myexit, NULL);
    sigaction(SIGKILL, &myexit, NULL);

    /* Argument parsing*/
    int opt;
    key_t key = 0;
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
        fprintf(stdout, "Use Default key: 12345\n");
        key = 12345;
    }
if(debug) printf("key: %d\n", key);

    size_t pagesize = (size_t) getpagesize();
if(debug > 1) { printf("pagesize: %d", (int)pagesize); }
    size_t buffersize = pagesize/NUMBER_OF_BUFFERS;
    id = shmget(key, pagesize, IPC_CREAT | SHM_R | SHM_W);
    char* pageaddr = (char*)shmat(id, NULL, 0);
    memset((void *)pageaddr, '\0', pagesize);

    /* Create semaphore for clients */
    sem_t *mutex;
    if ((mutex = sem_open("sem_haiyun_zhewen", O_CREAT, 0644, 1)) \
            == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
if(debug) printf("server start mutex %ld, at %p\n", mutex->__align, mutex);

    int counter = 0;
    while (1) {
        /* Check client code for comments */
        int buffno;
        char* validbitaddr;
        stats_t* buffaddr;
        for (buffno = 0 ; buffno < NUMBER_OF_BUFFERS ; ++buffno) {
            validbitaddr = pageaddr + buffno*buffersize;
            buffaddr = (stats_t*)(validbitaddr + 1);
if(debug) { printf("Validbit is '%c'\n", *validbitaddr); }
            if ( *validbitaddr != '\0' ) {
if(debug) { printf("buffno %d is good to use\n", buffno); }
                fprintf(stdout, "%d ", counter);
                fprintf(stdout, "%d ", buffaddr->pid);
                fprintf(stdout, "%s ", buffaddr->procname);
                fprintf(stdout, "%d ", buffaddr->counter);
                fprintf(stdout, "%.2f ", buffaddr->cpu_secs);
                fprintf(stdout, "%d ", buffaddr->priority);
                fprintf(stdout, "\n");
            }
        }
        fprintf(stdout, "\n");
        ++counter;
        sleep(1);
    }

    return 0;
}
