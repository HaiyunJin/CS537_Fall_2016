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
  int i = (int)shmgetat(1,1);
  printf(stdout, "shmgetat %d\n", i);
  i = shm_refcount(1);
  printf(stdout, "shm_ref_count %d\n", i);
  exit();
}
