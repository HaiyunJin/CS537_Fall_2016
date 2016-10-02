/*   Haiyun Jin   Sept. 23, 2016 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#define LSIZE 512 // Max linesize
#define ARGC 256

typedef struct node {
    int jid, pid, argc;
    char **argv;
    struct node *next;
} node_t;

/* Job list, globally accessible */
node_t *joblist = NULL;
char str[LSIZE];

void usage() {
    // fprintf(stderr,"Usage: mysh [batchFile]\n");
    sprintf(str, "Usage: mysh [batchFile]\n");
    write(STDERR_FILENO, str, strlen(str));
    exit(1);
}

/* Construct a node */
node_t *createNode(int jid, int pid, int argc, char **argv) {
    node_t *newnode = NULL;
    newnode = malloc(sizeof(node_t)); // remember to free
    newnode->jid = jid;
    newnode->pid = pid;
    newnode->argc = argc;
    newnode->argv = argv;
    newnode->next = NULL;
    return newnode;
}

/* free a node*/
void freenode(node_t *curr) {
    int i, argc = curr->argc;
    for ( i = 0; i < argc; ++i ) free(curr->argv[i]);
    free(curr->argv); // free the argv
    free(curr);
}

void freeList(node_t *head) {
    node_t *curr;
    while ( (curr = head ) != NULL ) {
        head = curr->next;
        freenode(curr);
    }
}

/* Clean the list by remove stopped jobs*/
void removeStoppedJobs(node_t *head) {
    node_t *curr = head;
    node_t *nextNext;
    while ( curr != NULL ) {  // stopped job return 0
        while ( curr->next != NULL && 
                waitpid(curr->next->pid,NULL,WNOHANG) != 0  ) {
            nextNext = curr->next->next;
            freenode(curr->next);
            curr->next = nextNext;
        }
        curr = curr->next;
    }
}

void printList(node_t *head) {
    removeStoppedJobs(head); // Purge first 
    node_t *curr = head->next;
    int i;
    while ( curr != NULL ) {
        sprintf(str,"%d:", curr->jid);
        write(STDOUT_FILENO, str, strlen(str));
        for ( i = 0; i < curr->argc; ++i ) {
            sprintf(str, " %s", (curr->argv)[i]);
            write(STDOUT_FILENO, str, strlen(str));
        }
        // printf("\n");
        write(STDOUT_FILENO, "\n", 1);
        curr = curr->next;
    }
}

/* Add a item to the end*/
void push(node_t *head, node_t *newnode) {
    node_t *curr = head;
    while ( curr->next != NULL ) curr = curr->next;
    curr->next = newnode;
}

int parse(char *commands, int *argc, char **argv, int *background) {

    const char *delim = " \t\n";
    char *token;
    int i = 0;
    token = strtok(commands,delim);  /* get the first token */
    do argv[i++] = token;  /* walk through other tokens */
    while (( token = strtok(NULL,delim)) != NULL );

    /* if first token is null, return */
    if ( argv[0] == NULL ) return -1;
    //if ( (*argv[0] == '&') && i == 1  )  return -1;

    argv[i] = NULL; /* argvs ended with NULL */
    /* if the last arg is &, run background, & in the middle is ok */
    if ( ( *background = (*argv[i-1] == '&') ) == 1 )
        argv[--i] = NULL; // remove the last & arg and decrease the argc
    *argc = i ; // pass the argc 
    return *argc;
}

void execute(int argc, char **argv, int *background) {
    static int jid = 0;
    pid_t pid = fork();
    if ( pid < 0 ) {
        sprintf(str, "ERROR: Fork error.\n");
        write(STDERR_FILENO, str, strlen(str));
    }
    else if ( pid == 0 ) { // child process
        execvp(argv[0],argv);
        sprintf(str, "%s: Command not found\n",argv[0]);
        write(STDERR_FILENO, str, strlen(str));
        exit(1);
    } else {
        /* First, keep track of the jid */
        node_t *jobNode;
        char **argvNew = malloc(sizeof(char *)*argc);
        int i;
        for ( i = 0; i < argc; ++i ) argvNew[i] = strdup(argv[i]); 
        jobNode = createNode(++jid, pid, argc, argvNew);
        joblist->jid++;  // jid in joblist is the current max jid
        push(joblist, jobNode);
        /* Then, determine background or foreground */
        if ( *background == 1 ) {
            // sprintf(str, "[%d] %d\n", 1, pid);
            // write(STDOUT_FILENO, str, strlen(str));
        }
        else waitpid(pid, NULL, 0);
    }
}

/* Find the pid given jid */
int jid2pid(int jid) {
    if ( jid > joblist->jid ) return -1; // -1 for large jid
    removeStoppedJobs(joblist);
    node_t *curr = joblist->next;
    while ( curr != NULL ) {
        if ( jid == curr->jid ) return curr->pid;
        curr = curr->next;
    }
    return -2; // -2 for finished jobs
}

void readcommands(FILE *fileno) {
    /* loop to receive user input */
    //static char commands[LSIZE];
    char commands[LSIZE+1]; // +1 for \0
    char *argv[ARGC], *endstr;
    int background ; // background flag
    int argc, i, jid, pid;
    long int timewait;
    char *fgetstatus = "";
    struct timeval start, end; // for myw
    while( fgetstatus ) {
        /* initialize all */
        for ( i = 0; i < ARGC; ++i ) argv[i] = NULL ;
        background = 0;
        /* print promot */
        if ( fileno == stdin ) write(STDOUT_FILENO, "mysh> ", 6); 
        if ( (fgetstatus = fgets(commands, LSIZE+1, fileno)) == NULL ) return;
// printf("Commands strlen: %d\n", (int)strlen(commands));
// printf("\\n at %ld\n", (unsigned long int) strchr(commands, '\n'));
        //if (strlen(commands) >= LSIZE && strchr(commands, '\n') == NULL ) {
        if ( strchr(commands, '\n') == NULL ) { // if the line is not fin
            //while (strlen(fgets(commands, LSIZE+1, fileno)) >= LSIZE);
            do fgets(commands, LSIZE+1, fileno);
            while ( strchr(commands, '\n') == NULL) ;
//printf(" reading the rest\n");
            sprintf(str, "Command is too long! (Max: %d)\n", LSIZE);
            write(STDERR_FILENO, str, strlen(str));
            continue;
        }
// printf("Finish read all rest\n");

        if ( fileno != stdin ) {
            sprintf(str, "%s",commands); // Print the cmd
            write(STDOUT_FILENO, str, strlen(str));
        }
        if ( *commands == '\n' ) continue; /* If blank, ask for another */
        if ( parse(commands, &argc, argv, &background) <= 0 ) continue;
        if ( strcmp(argv[0], "j") == 0 && argv[1] == NULL ) printList(joblist);
        else if ( strcmp( argv[0], "myw") == 0 && argv[2] == NULL ) {
            jid = (int) strtol(argv[1], &endstr, 10);
            pid = jid2pid(jid);
            if ( strcmp(endstr,"") != 0 || pid == -1 ) {
                sprintf(str, "Invalid jid %s\n", argv[1]);
                write(STDERR_FILENO, str, strlen(str));
                continue;
            } else if ( pid == -2 )
                timewait = 0; /* Finished Jobs */
            else {
                gettimeofday(&start, NULL);
                waitpid(pid, NULL, 0); /* Wait for jid */
                gettimeofday(&end, NULL);
                timewait =  ((end.tv_sec * 1000000 + end.tv_usec)
                        - (start.tv_sec * 1000000 + start.tv_usec));
            }
            //printf("%ld : Job %d terminated\n", timewait, jid);
            sprintf(str, "%ld : Job %d terminated\n", timewait, jid);
            write(STDOUT_FILENO, str, strlen(str));
        } else if ( strcmp(argv[0], "exit") == 0 && argv[1] == NULL )
            exit(1);
        else  execute(argc,argv,&background);
    }
}

int main (int argc, char *argv[]) {
    if ( argc > 2 ) usage();
    FILE *fileno = stdin;
    joblist = createNode(0,0,0,NULL);
    if ( argc == 2 && (fileno = fopen(argv[1], "r")) == NULL ) {
        sprintf(str, "Error: Cannot open file %s\n",argv[1]);
        write(STDERR_FILENO, str, strlen(str));
        exit(1);
    } 
    readcommands(fileno);
    freeList(joblist);
    return 1;
}
