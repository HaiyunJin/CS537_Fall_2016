// #include "types.h"
// #include "stat.h"
// #include "user.h"
// #include "pstat.h"
// 
/* // #define check(exp, msg) if(exp) {} else {\
//   printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
//   exit();}  
*/
// 
// 
// int
// main(int argc, char *argv[])
// {
//   struct pstat pstat;
// //  int i; 
//   check(getpinfo(&pstat) == 0, "getpinfo"); 
// 
//   int i;
//   for ( i = 0 ; i < NPROC ; ++i) {
//     if ( pstat.inuse[i] ) {
//       printf(1, "pid: %d priority: %d\n", pstat.pid[i], pstat.priority[i]);
//     }
//   }
//   return 0;
// }
// 
#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

#define check(exp, msg) if(exp) {} else {\
  printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
  exit();}

int
main(int argc, char *argv[])
{
  struct pstat st;

  check(getpinfo(&st) == 0, "getpinfo");

  int i;
  // int j;
//  int pid = getpid();
//  int found_self = 0;
  for(i = 0; i < NPROC; i++) {
    if (st.inuse[i]) {
      printf(1, "pid: %d priority: %d\t ", st.pid[i], st.priority[i]);
//      if (st.pid[i] == pid) {
//        found_self = 1;
//      }
      
      //for (j = 0; j < 4; j++) {
        //printf(1, "\t level %d ticks used %d\n", j, st.ticks[i][j]);
        printf(1, "[%d\t%d\t%d\t%d]\n", st.ticks[i][0]
                ,st.ticks[i][1],st.ticks[i][2],st.ticks[i][3]);
      //}
    }
  }

//  if (found_self) {
//    printf(1, "TEST PASSED\n");
//  } else {
//    printf(1, "TEST FAILED\n");
//  }

  exit();
}
