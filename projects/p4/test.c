#include "p4a/cs537.h"

// haiyun

#include <signal.h>         /* signal handling */

struct twoint {
  int int1;
  int int2;
};

pthread_mutex_t m;

void *workerthread2(void *arg) {
  struct twoint *args;
  args = (struct twoint *) arg;
  pthread_mutex_lock(&m);
  printf("in thread %d %d pid %d ppid %d \n", args->int1, args->int2,\
      getpid(),getppid());
//   printf("in thread pid %d ppid %d \n",getpid(),getppid());
  pthread_mutex_unlock(&m);
//   sleep(20);
  if ( fork() == 0 ) {
    pthread_mutex_lock(&m);
    printf("forked in %d %d pid %d ppid %d\n", args->int1, args->int2,\
        getpid(),getppid());

    pthread_mutex_unlock(&m);
    exit(1);
  }
  pthread_exit(NULL);
}

void *workerthread(void *arg) {
  pthread_mutex_lock(&m);
  printf("in thread %d   pid %d ppid %d\n", *(int *)arg,getpid(), getppid());
//  printf("in thread pid %d ppid %d\n",getpid(), getppid());
  pthread_t p1;
  pthread_t p2;
  pthread_t p3;
  pthread_t p4;

  struct twoint arg1;
  struct twoint arg2;
  struct twoint arg3;
  struct twoint arg4;
  arg1.int1 = *(int *)arg;
  arg2.int1 = *(int *)arg;
  arg3.int1 = *(int *)arg;
  arg4.int1 = *(int *)arg;
  arg1.int2 = 1;
  arg2.int2 = 2;
  arg3.int2 = 3;
  arg4.int2 = 4;
  pthread_mutex_unlock(&m);

  pthread_create(&p1, NULL, workerthread2, &arg1);
  pthread_create(&p2, NULL, workerthread2, &arg2);
//   pthread_create(&p3, NULL, workerthread2, &arg3);
//   pthread_create(&p4, NULL, workerthread2, &arg4);

  pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
  pthread_mutex_init(&m, NULL);

  pthread_t p1;
  pthread_t p2;
  pthread_t p3;
  pthread_t p4;

  int arg1 = 1;
  int arg2 = 2;
  int arg3 = 3;
  int arg4 = 4;

  pthread_mutex_lock(&m);
  printf("in main\n");
  printf("in main       pid %d ppid %d\n",getpid(), getppid());
  pthread_mutex_unlock(&m);

  pthread_create(&p1, NULL, workerthread, &arg1);
  pthread_create(&p2, NULL, workerthread, &arg2);
//   pthread_create(&p3, NULL, workerthread, &arg3);
//   pthread_create(&p4, NULL, workerthread, &arg4);

  pthread_exit(NULL);
   
}

