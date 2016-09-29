#include <stdlib.h>
typedef struct node {
    int jid, pid, argc;
    char **argv;
    struct node *next;
} node_t;

/* Construct a node */
node_t *createNode(int jid, int pid, int argc, char **argv) {
    node_t *newnode = NULL;
    newnode = malloc(sizeof(node_t));
    newnode->jid = jid;
    newnode->pid = pid;
    newnode->argc = argc;
    newnode->argv = argv;
    newnode->next=NULL;

        int i;
        printf("%d: ", jid);
        for ( i = 0 ; i < argc ; ++i )
            printf(" %s", argv[i]);
        printf("\n");

    return newnode;
}

/* Free list*/
void freeList(node_t *head) {
    node_t *curr = head;
    while (curr != NULL ) {
        head = curr->next;
        free(curr);
        curr = head;
    }
}

/* Print the list */
void printList(node_t *head) {
    node_t *curr  = head->next;
    int i;
    while ( curr != NULL ) {
        printf("%d:", curr->jid);
        printf(" %s", *(curr->argv));
        for ( i = 0 ; i < curr->argc ; ++i )
            printf(" %s", (curr->argv)[i]);
        printf("\n");
        curr = curr->next;
    }
}

/* Add a item to the end*/
void push(node_t *head, node_t *newnode) {
    node_t *curr = head;
    while ( curr->next != NULL ) {
        curr = curr->next;
    }
    curr->next = newnode;

}

/* Clean the list by remove stopped jobs*/
void removeStoppedJobs(node_t *head) {
    node_t *curr = head;
    while ( curr != NULL ) {
        while ( waitpid(curr->next->pid,NULL,WNOHANG) != 0 ) {
                            // existing job return 0
            curr->next = curr->next->next;
            free(curr->next); // free the delete node
        }
        curr = curr->next;
    }
}

