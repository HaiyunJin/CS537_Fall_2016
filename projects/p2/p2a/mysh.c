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
    argv[0] = commands;
    argv[0][strlen(argv[0])-1]='\0';
    argv[1] = NULL;
    //*argv++ = '\0';
}

void execute(char **argv) {
     printf("argv[0] \"%s\"\n", argv[0] ) ;
     printf("strcmp %d\n", strcmp( argv[0],"exit") ) ;
    //if ( strcmp( argv[0],"exit") == 0 )  {
    if (argv[0][0] == 'e' && argv[0][1] == 'x' && 
        argv[0][2] == 'i' && argv[0][3] == 't' && argv[0][4] == '\0')  {
        printf("exit typed\n");
        exit(1);
    }
    pid_t pid;
    pid = fork();
    if ( pid < 0 ) {
        printf("Fork error.\n");
    } else if (pid == 0 ) {
        if ( execvp(argv[0],argv) < 0 ) {
            printf("Invalid exec.\n");
        }
        exit(1);
    } else {
        waitpid(-1,pid,0);
    }
}

void interact() {
    printf("Enter interactive mode\n");
    // loop to receive user input

    char commands[LINESIZE];
    char *argv[2];
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
        execute(argv);
//    printf("after execute\n");
//       exit(1);
    }


}

void batchmode(char *argv[]) {
    printf("Enter batch mode\n");
    printf("Using batch file: %s\n",argv[1]);
}


