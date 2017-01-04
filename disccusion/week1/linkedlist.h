
#ifndef 
#define 

typedef struct __linked_list_node {
    void *data;
    linkedlistnode *next ; 
 } linked_list_node


int main(int argc, char *argv[]) {
    int i;
    linked_list_node header;
    linked_list_node node;
    header.next=&node;
    linkedlistnode *curr;
    curr = header.next; 

    for ( i = 0 ; i < argc ; ++i ) {
        curr.(*data) = malloc(sizeof(*argv[i]));
        curr.(*data) = *argv[i] ;
        linked_list_node newnode ;
        curr.next = &newnode;
        curr = curr.next;
    
    }


}

