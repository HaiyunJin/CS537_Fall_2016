/*   Haiyun Jin   Sept. 23, 2016 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#define LINESIZE 512
#define ARGC 256

typedef struct node {
    int jid, pid, argc;
    char **argv;
    struct node *next;
} node_t;

/* Job list, globally accessible */
node_t *joblist = NULL;

void usage() {
    fprintf(stderr,"Usage: mysh [batchFile]\n");
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
    newnode->next=NULL;
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
        printf("%d:", curr->jid);
        for ( i = 0; i < curr->argc; ++i ) printf(" %s", (curr->argv)[i]);
        printf("\n");
        curr = curr->next;
    }
}

/* Add a item to the end*/
void push(node_t *head, node_t *newnode) {
    node_t *curr = head;
    while ( curr->next != NULL ) curr = curr->next;
    curr->next = newnode;
}

void parse(char *commands, int *argc, char **argv, int *background) {
    const char *delim = " \t\n";
    char *token;
    int i = 0;
    token = strtok(commands,delim);  /* get the first token */
    do argv[i++] = token;  /* walk through other tokens */
    while (( token = strtok(NULL,delim)) != NULL );
    argv[i] = NULL; /* argvs ended with NULL */
    /* if the last arg is &, run background, & in the middle is ok */
    if ( ( *background = (*argv[i-1] == '&') ) == 1 )
        argv[--i] = NULL; // remove the last & arg and decrease the argc
    *argc = i ; // pass the argc 
}

void execute(int argc, char **argv, int *background) {
    static int jid = 0;
    pid_t pid = fork();
    if ( pid < 0 ) printf("Fork error.\n");
    else if ( pid == 0 ) { // child process
        execvp(argv[0],argv);
        printf("%s: Command not found\n",argv[0]);
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
        if (*background) printf("[%d] %d\n", 1, pid);
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
    static char commands[LINESIZE];
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
        if ( fileno == stdin ) printf("mysh> "); // print promot
        fgetstatus = fgets(commands, LINESIZE, fileno);
        if ( fileno != stdin ) printf("%s",commands); // Print the cmd
        if ( NULL == fgetstatus ) return;
        if ( *commands == '\n' ) continue; /* If blank, ask for another */
        parse(commands,&argc,argv,&background);
        if ( strcmp(argv[0],"j")==0 && argv[1]==NULL ) printList(joblist);
        else if ( strcmp( argv[0],"myw") == 0 && argv[2] == NULL ) {
            jid = (int) strtol(argv[1], &endstr,10);
            pid = jid2pid(jid);
            if ( strcmp(endstr,"") != 0 || pid == -1 ) {
                printf("Invalid jid %s\n",argv[1]);
                //write(STDERR_FILENO,,);
                continue;
            } else if ( pid == -2 )
                timewait = 0; /*Finished Jobs */
            else {
                gettimeofday(&start, NULL);
                waitpid(pid, NULL, 0); /* Wait for jid */
                gettimeofday(&end, NULL);
                timewait =  ((end.tv_sec * 1000000 + end.tv_usec)
                        - (start.tv_sec * 1000000 + start.tv_usec));
            }
            printf("%ld : Job %d terminated\n", timewait, jid);
            write(STDOUT_FILENO, (char *)&timewait, sizeof(timewait));
            write(STDOUT_FILENO, " : Job ", sizeof(" : Job ")-1);
            write(STDOUT_FILENO, (char *)&jid, sizeof(jid));
            write(STDOUT_FILENO," terminated\n",sizeof(" terminated\n")-1);
        } else if ( strcmp(argv[0], "exit") == 0 && argv[1] == NULL )
            exit(1);
        else  execute(argc,argv,&background);
    }
}

int main (int argc, char *argv[]) {
    FILE *fileno = stdin;
    joblist = createNode(0,0,0,NULL);
    if ( argc > 2 ) usage();
    if ( argc == 2 ) {
        fileno = fopen(argv[1],"r");
        if ( fileno == NULL ) {
            printf("Error: Cannot open file %s\n",argv[1]);
            exit(1);
        }
    } 
    readcommands(fileno);
    freeList(joblist);
    return 1;
}
