#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

void spin(int n){
  int i, j;
  int x = 0;
  for(i=1; i<n; i++) {
    //  for ( j = 1 ; j < 1000000 ; ++j)
      for ( j = 1 ; j < 740740 ; ++j)
        x += 2;
  }
}

// haiyun show statics
// #include  "getpinfotest_show.c"

//1.  one proc, downgrade property
//  #include  "getpinfotest_1.c"

//2. two proc, one comes late but grab CPU first
// #include  "getpinfotest_2.c"

//3. similar to 2 but sleep parent until child reach level 3
// #include  "getpinfotest_3.c"

//4. No test to run here
//5. No test to run here
//6. No test to run here
//7. use 2 to show
//8. use 2 to show
//9. both long run, one sleep, when resume, continue timetick
 #include  "getpinfotest_9.c"

//10. 7 long run jobs, show upgrade
// #include  "getpinfotest_10.c"

