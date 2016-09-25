/*   Haiyun Jin
 *   Sept. 23, 2016
 *   mysh.c
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define LINESIZE 512
#define ARGC 64

void usage() {
    fprintf(stderr,"Usage: mysh [batchFile]\n");
    exit(1);
}

void interact();
void batchmode(char *argv[]);

int main (int argc, char *argv[]) {
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


void parse(char *commands, char **argv) {
    while ( *commands != '\n' ) { // loop before the command line ends
    // For interative mode, the line is ended with '\n'
        while ( *commands == ' ' || *commands == '\t' || *commands == '\n' ) {
            *commands++ = '\0';  // change all specical char to \0
        }
        *argv++ = commands; // store the argument
        while ( *commands != '\0' && *commands != ' ' && *commands != '\n' 
                && *commands != '\t') {
            commands++; // skip until next special char
        }
    }
    *commands = '\0';
    *argv = NULL;
    //*argv = '\0'; // end of argument list
    //printf("parse argv[0] \"%s\"\n", argv[0]);
    //argv[0] = commands;
    //argv[0][strlen(argv[0])-1]='\0';
    //argv[1] = NULL;
    //*argv++ = '\0';
}

void execute(char **argv) {
    pid_t pid;
    pid = fork();
    if ( pid < 0 ) {
        printf("Fork error.\n");
    } else if (pid == 0 ) {
        if ( execvp(argv[0],argv) < 0 ) {
            printf("Invalid exec.\n");
        }
        printf("Child process complete.\n");
        exit(1);
    } else {
        waitpid(-1,pid,0);
    }
}

void interact() {
    printf("Enter interactive mode\n");
    // loop to receive user input

    char commands[LINESIZE];
    char *argv[ARGC];
    int i;
    for ( i = 0 ; i < ARGC ; ++i ) {
        argv[i] = NULL ;
    }
    //char **argv;
    while(1) {
        printf("promot> "); 
        if ( NULL == fgets(commands,LINESIZE,stdin) ) {
            return;
        }
        printf("\n"); 
    printf("after read stdin\n");
        parse(commands,argv);
    printf("after parse\n");
    printf("Command: %s\n",*argv);
     printf("argv[0] \"%s\"\n", argv[0] ) ;
     printf("argv[1] \"%s\"\n", argv[1] ) ;
     printf("argv[2] \"%s\"\n", argv[2] ) ;
     printf("argv[3] \"%s\"\n", argv[3] ) ;
     printf("argv[4] \"%s\"\n", argv[4] ) ;
     printf("argv[5] \"%s\"\n", argv[5] ) ;
     printf("argv[6] \"%s\"\n", argv[6] ) ;
     printf("strcmp %d\n", strcmp( argv[0],"exit") ) ;
    if ( strcmp( argv[0],"exit") == 0 )  {
//    if (argv[0][0] == 'e' && argv[0][1] == 'x' && 
//        argv[0][2] == 'i' && argv[0][3] == 't' && argv[0][4] == '\0')  {
        printf("exit typed\n");
        exit(1);
    }
        execute(argv);
//    printf("after execute\n");
//       exit(1);
    }

}

void batchmode(char *argv[]) {
    printf("Enter batch mode\n");
    printf("Using batch file: %s\n",argv[1]);
}


