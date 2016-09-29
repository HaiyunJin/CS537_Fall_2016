/*   Haiyun Jin
 *   Sept. 23, 2016
 *   mysh.c
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#define LINESIZE 512
#define ARGC 256
#include "parse.c"
#include "linkedlist.c"


/* Job list, globally accessible */
node_t *joblist = NULL;

void usage() {
    fprintf(stderr,"Usage: mysh [batchFile]\n");
    exit(1);
}

void interact();
void batchmode(char *argv[]);

int main (int argc, char *argv[]) {
    joblist = createNode(0,0,0,NULL);

    if ( argc == 1 ) {
        // interactive mode
        interact();
    } else if ( argc == 2 ) {
        // batch mode
        batchmode(argv);
    } else {
        usage();
    }
    return 1;
}


void parse(char *commands, int *argc, char **argv, int *background);

void execute(int argc, char **argv, int *background) {
    pid_t pid;
    int status;
    static int jid = 0;
    
    /* Check for j or myw */



    pid = fork();
    if ( pid < 0 ) {
        printf("Fork error.\n");
    } else if ( pid == 0 ) { // child process
//        printf("I will exec the command!!\n");
        if ( execvp(argv[0],argv) < 0 ) {
            //printf("Invalid exec.\n");
            printf("%s: Command not found\n",argv[0]);

        } else {
//            printf("I can do it\n");
        }
        // the following code will not be reached
        exit(1);
    } else {

        /* keep track of the jid */
        node_t *jobNode;
// problem here:
// all nodes points to the same argv memory address
// So need to make a copy of the argv
        char **argvNew = malloc(sizeof(char *)*argc);
        int i;
        for ( i = 0 ; i < argc ; ++i )
            argvNew[i] = strdup(argv[i]); // remember to free
        
        jobNode = createNode(++jid, pid, argc, argvNew);
//        printf("%d: ", jid);
//        for ( i = 0 ; i < argc ; ++i )
//            printf(" %s", argv[i]);
//        printf("\n");
//        printf("%d: ", jid);
//        for ( i = 0 ; i < jobNode->argc ; ++i )
//            printf(" %s", (jobNode->argv)[i]);
//        printf("\n");

        push(joblist, jobNode);

//        printf("after for child, determine wait or not\n");
        //printf("background value: %d\n",*background);
        if ( *background )  {
//            printf("    No wait\n");
            printf("[%d] %d\n", 1, pid);
        } else {
//            printf("    Decided to wait\n");
            struct timeval start, end;
            gettimeofday(&start, NULL);
            waitpid(pid, &status, 0);
            gettimeofday(&end, NULL);
//            printf("time waited: %ld\n", ((end.tv_sec * 1000000 + end.tv_usec)
//                              - (start.tv_sec * 1000000 + start.tv_usec)));
        }

    }
}

void interact() {
//    printf("Enter interactive mode\n");
    // loop to receive user input

    static char commands[LINESIZE];
    int argc;
    char *argv[ARGC];
    int background ; // background flag
    int i; // loop 
    while(1) {
        // initialize all
        for ( i = 0 ; i < ARGC ; ++i ) 
            argv[i] = NULL ;
        background = 0;
        printf("mysh> "); 
        if ( NULL == fgets(commands,LINESIZE,stdin) ) {
            return;
        }
//        printf("\n"); 
//        printf("after read stdin\n");
//        printf("Command: %s\n",commands);
//        printf("i value: %d\n", i);
//        printf("background value: %d\n", background);
        // if ( ! strcmp( commands, '\n' ) ) {

        
        /* If blank command, just ask for another */
        if ( *commands == '\n' ) 
            continue;

        parse(commands,&argc,argv,&background);
//    printf("after parse\n");
//    printf("background value: %d\n", background);
//    printf("Command: %s\n",*argv);
//    printf("argv[0] \"%s\"\n", argv[0] ) ;
//    printf("argv[1] \"%s\"\n", argv[1] ) ;
//    printf("argv[2] \"%s\"\n", argv[2] ) ;
//    printf("argv[3] \"%s\"\n", argv[3] ) ;
        if ( strcmp( argv[0],"j") == 0 ) {
//            printf("j typed\n");
            removeStoppedJobs(joblist);
            printList(joblist);
        } else if ( strcmp( argv[0],"myw") == 0 ) {
        
        } else if ( strcmp( argv[0],"exit") == 0 )  {
//            printf("exit typed\n");
            exit(1);
        } else {
//            printf("in execute\n");
            execute(argc,argv,&background);
        }
//    printf("after execute\n");
    }
}

void batchmode(char *argv[]) {
    printf("Enter batch mode\n");
    printf("Using batch file: %s\n",argv[1]);
    close(STDIN_FILENO);
    open(argv[1],O_RDONLY);
    interact(argv);

}


