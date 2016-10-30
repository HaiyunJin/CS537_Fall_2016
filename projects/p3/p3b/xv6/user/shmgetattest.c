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

  int key = atoi(argv[0]);
  int num_pages = atoi(argv[1]);

  if ( ( i = (int)shmgetat(key,num_pages) ) < 0 )
      printf(1,"shmgetat error");
  printf(stdout, "shmgetat %x\n", (unsigned int) i);


  if ( (i = shm_refcount(key)) < 0 )
      printf(1,"shm_refcount error");
  printf(stdout, "shm_ref_count %d\n", i);
  exit();
}
