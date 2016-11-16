#include "cs537.h"
#include "request.h"

// haiyun
int debug = 0;

#include <signal.h>         /* signal handling */

int numthread, numbuff;
pthread_t **p;
int *buff;
int count;  // count of requests
pthread_cond_t fill, empty;
pthread_mutex_t m;
int write_buffer_no, read_buffer_no;

void exit_handler(int signo) {
  // free
  int i;
  for ( i = 0 ; i < numthread; ++i ) {
    free(p[i]);
  }
  free(p);
  free(buff);
if(debug) printf("EXITING!!!\n");
  exit(0);
}

int parseerror(char *msg) {
  fprintf(stderr,"ERROR: %s\n",msg);
  exit(1);
}

int usage(char *filename){
    fprintf(stderr, "Usage: %s <portnum> <threads> <buffers>\n", filename);
    exit(1);
}

// CS537: Parse the new arguments too
void getargs(int *port, int *numthread, int *numbuff, int argc, char *argv[])
{
    char temp;
    if (argc != 4) usage(argv[0]);

if(debug) printf("in getarg\n");
    if ( sscanf(argv[1],"%d%c",port,&temp) != 1 )  usage(argv[0]);
    if ( *port <= 0 ) parseerror("port number should be positive");
if(debug) printf("port read\n");
    if ( sscanf(argv[2],"%d%c",numthread,&temp) != 1 )  usage(argv[0]);
    if ( *numthread < 1 ) parseerror("number of threads should larger than 1");
if(debug) printf("numthread read\n");
    if ( sscanf(argv[3],"%d%c",numbuff,&temp) != 1 )  usage(argv[0]);
    if ( *numbuff < 1 ) parseerror("number of buffers should larger than 1"); 
if(debug) printf("number of buffer read\n");
}

void *workerthread(void *arg) {
if(debug) printf("In worker\n");
  int connfd;
  while (1) {
      Pthread_mutex_lock(&m);
if(debug) printf("worker in lock\n");
if(debug) printf("worker count %d\n",count);
      while ( count == 0 ) {
if(debug) printf("worker wait for fill\n");
        Pthread_cond_wait(&fill, &m);
if(debug) printf("worker count %d\n",count);
      }
      // find a request in the buffer 
      connfd = buff[read_buffer_no++];
      read_buffer_no = read_buffer_no%numbuff;
      Pthread_cond_signal(&empty);
      count--;
if(debug) printf("worker release lock\n");
      Pthread_mutex_unlock(&m);
      // and handle it
      requestHandle(connfd);
      Close(connfd);
  }
  return NULL;
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

if(debug) printf("\n");
if(debug) printf("begin server\n");

    /* Signal handling */
    struct sigaction myexit;
    myexit.sa_handler = exit_handler;
    sigemptyset(&myexit.sa_mask);
    myexit.sa_flags = 0;
    sigaction(SIGINT, &myexit, NULL);

if(debug) printf("before get arg\n");
    getargs(&port, &numthread, &numbuff, argc, argv);

    // init cv and mutex lock
    Pthread_cond_init(&fill, NULL);
    Pthread_cond_init(&empty, NULL);
    Pthread_mutex_init(&m, NULL);

    // allocate buff
    buff = malloc(numbuff*sizeof(int));

    write_buffer_no = 0;
    read_buffer_no = 0;
    count = 0;
    
    // CS537: Create some threads...
if(debug) printf("before p malloc\n");
    p = malloc(numthread*sizeof(pthread_t *));
    int i;
if(debug) printf("before p[] malloc\n");
    for ( i = 0 ; i < numthread; ++i ) {
      p[i] = malloc(sizeof(pthread_t));
      Pthread_create(p[i], NULL, &workerthread, NULL);
    }

    // listen and put into buffer
    listenfd = Open_listenfd(port);
    while (1) {

if(debug) printf("master Wait for request\n");
      clientlen = sizeof(clientaddr);
      connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
if(debug) printf("master Accept request\n");

      Pthread_mutex_lock(&m);
if(debug) printf("master in lock\n");
if(debug) printf("master count %d\n",count);
      while ( count == numbuff ) {
if(debug) printf("master wait to fill\n");
        Pthread_cond_wait(&empty, &m);
if(debug) printf("master count %d\n",count);
      }
      // put into the buffer
      buff[write_buffer_no++] = connfd;
      write_buffer_no = write_buffer_no%numbuff;
      count++;
      Pthread_cond_signal(&fill);
if(debug) printf("master release lock\n");
      Pthread_mutex_unlock(&m);
      
    }
}

