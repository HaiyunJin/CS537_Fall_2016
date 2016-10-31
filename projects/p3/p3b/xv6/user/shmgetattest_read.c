#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"

int stdout = 1;

int
main(int argc, char *argv[])
{
  int i;

  int *ptr;
  int key = atoi(argv[1]);
  int num_pages = atoi(argv[2]);

  if ( ( ptr = (int *)shmgetat(key,num_pages) ) < 0 )
      printf(1,"shmgetat error\n");
  printf(stdout, "shmgetat %x\n", (unsigned int) ptr);

  printf(stdout, "read from shm %d\n", *ptr);

  if ( (i = shm_refcount(key)) < 0 )
      printf(1,"shm_refcount error\n");
  printf(stdout, "shm_ref_count %d\n", i);
  exit();
}
