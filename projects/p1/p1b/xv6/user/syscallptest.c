#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int start;
  int end;

  if ( argc != 2 ){
    printf(1,"Usage: syscallptest N\n");
    exit();
  }

  int numcalls = atoi(argv[1]);

  start = getnumsyscallp();
  int i;
  for (i = 0; i < numcalls ; ++i ) {
      getpid();
  }
  end = getnumsyscallp();
  printf(1,"%d\n",start);
  printf(1,"%d\n",end);
  exit();
}
