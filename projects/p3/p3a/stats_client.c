#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ipc.h>        /* shared memory */
#include <sys/shm.h>
#include <sys/types.h>

#include <fcntl.h>          /* For O_* constants */
#include <sys/stat.h>       /* For mode constants */
#include <semaphore.h>

#include <sys/time.h>       /* setpriority/getpriority */
#include <sys/resource.h>   /* setpriority/getpriority */

#include <time.h>          /* clock_gettime */

#include <signal.h>         /* signal */

#include "stats.h"
#include "defs.h"

sem_t *mutex;
char* validbitaddr;
key_t key;

void exit_handler(int signo) {
    /* 
     * Clear the shared memory field before exit
     * No need to clear content since invalid
     */
    if ( stats_unlink(key) != 0 ) {
        perror("Stats_unlink error");
        exit(1);
    }  // else ok
if(debug) { printf("\nEXITING!!!!\n"); }
    exit(0);
}

void usage() {
    printf("Usage: stats_client [-k key] [-p priority] [-s sleeptime_ns]");
    printf(" [-c cputime_ns]\n");
    exit(1);
}

int main(int argc, char *argv[] ) {
    /* Signal handling */
    struct sigaction myexit;
    myexit.sa_handler = exit_handler;
    sigaction(SIGINT, &myexit, NULL);
    sigaction(SIGTERM, &myexit, NULL);
    sigaction(SIGKILL, &myexit, NULL);
    sigaction(SIGTSTP, &myexit, NULL);

    /* Argument parsing*/
    int opt;
    key = 0;
    int priority = 0, sleeptime_ns = 0, cputime_ns = 0;
    while ((opt = getopt(argc, argv, "k:p:s:c:")) != -1) {
        switch (opt) {
        case 'k':
            key = (key_t) atoi(optarg);
            break;
        case 'p':
            priority = atoi(optarg);
            break;
        case 's':
            sleeptime_ns = atoi(optarg);
            break;
        case 'c':
            cputime_ns = atoi(optarg);
            break;
        default:
            usage();
        }
    }
    /* Check for missing params and set to default */
    if ( key == 0 ) {
//         fprintf(stdout, "Use Default key: 12345\n");
        key = 12345;
    }
if(debug) { printf("key: %d\n", key); }
    if ( priority == 0 ) {
//         fprintf(stdout, "Use Default priority: 10\n");
        priority = 10;
    }
    if ( sleeptime_ns == 0 ) {
//         fprintf(stdout, "Use Default sleeptime in ns: 1000\n");
        sleeptime_ns = 1000;
    }
    if ( cputime_ns == 0 ) {
//         fprintf(stdout, "Use Default cputime in ns: 1000000\n");
        cputime_ns = 1000000;
    }

    if ((mutex = sem_open("sem_haiyun_zhewen", O_CREAT, 0644, 1))
            == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
if(debug) printf("client open mutex %ld, at %p\n", mutex->__align, mutex);

    /* Attach the process to shared memory */
    stats_t* buffaddr;
    if ( (buffaddr = stats_init(key)) == NULL ) {
        fprintf(stderr, "No more free buffers!\n");
        exit(1);
    }

    /* Set priority of the process */
    setpriority(PRIO_PROCESS, 0, priority);

    /* Write permanent info */
    int i;
    buffaddr->pid = (int) getpid();
    for ( i = 0 ; i < NAME_LENGTH ; ++i )
        (buffaddr->procname)[i] = argv[0][i];
    buffaddr->procname[NAME_LENGTH] = '\0';  // truncate to 15 meaningful char
if(debug) printf("procname: %s\n", buffaddr->procname);
    /* 
     * Start loop:
     * Do stuff and write statics forever until signal arrives
     */
    struct timespec starttime, currtime, endtime, sleeptime, timeused;
    int counter = 0;
    // Initialize priority and sleeptime
    buffaddr->priority = getpriority(PRIO_PROCESS, 0);
    sleeptime.tv_sec = sleeptime_ns / NS_PER_SEC;
    sleeptime.tv_nsec = sleeptime_ns % NS_PER_SEC;

    while (1) {
        /* Compute for specified time */
        if ( clock_gettime(CLOCK_REALTIME_COARSE, &starttime) == -1 ) {
            perror("Start Clock error");
            exit(1);
        }
if(debug) printf("starttime nsec: %ld\n", starttime.tv_nsec);

        endtime.tv_sec = starttime.tv_sec + \
                   (starttime.tv_nsec + cputime_ns)/NS_PER_SEC;
        endtime.tv_nsec = (starttime.tv_nsec+ cputime_ns)%NS_PER_SEC;
        currtime = starttime;
        while ( currtime.tv_sec < endtime.tv_sec
              || (currtime.tv_sec == endtime.tv_sec &&
                        currtime.tv_nsec < endtime.tv_nsec) ) {
            if ( clock_gettime(CLOCK_REALTIME_COARSE, &currtime) == -1 ) {
                perror("Get Clock error");
                exit(1);
            }
if(debug > 2) { printf("currtime nsec: %ld\n", currtime.tv_nsec); }
        }

        /* Sleep for specified time */
        nanosleep(&sleeptime, NULL);

        /* Calculate cummulative  cpu time*/
        if ( clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timeused) == -1 ) {
            perror("clock get cpu time error");
            exit(1);
        }

        /* Get current priority */
        priority = getpriority(PRIO_PROCESS, 0);

        /* Increase counter */
         ++counter;

        /* write statics */
        buffaddr->counter = counter;
        buffaddr->priority = priority;
        buffaddr->cpu_secs = timeused.tv_sec \
                             + (double)timeused.tv_nsec/NS_PER_SEC;
if(debug) { printf("cpu_secs: %.2f\n", buffaddr->cpu_secs); }
    }

    return 0;
}

