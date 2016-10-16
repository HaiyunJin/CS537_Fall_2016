#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  int i;
  int j;
  int x = 0;

  if(argc < 2){
    exit();
  }
  for(i=1; i<atoi(argv[1]); i++)
      for ( j = 1 ; j < 1000000 ; ++j) {
        x += 2;
      }
  exit();
  return x;
}
