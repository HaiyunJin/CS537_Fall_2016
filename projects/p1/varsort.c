/*
 * varsort.c
 *
 *  Created on: Sep 6, 2016
 *      Author: haiyun
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"
#include <sys/types.h>
#include <sys/stat.h>


void usage(char *prog){
  fprintf(stderr, "usage: %s <-i inFile> <-o outFile>\n", prog);
  exit(1);
}

int CompareDataptr(rec_dataptr_t *r1,rec_dataptr_t *r2) {
    return (*r1).key - (*r2).key;
}



int main(int argc, char *argv[] ) {

    printf("Start\n");
    // arguments
    char *inFile  =  "/no/such/file";
    char *outFile =  "/no/such/file";
    int c;
    printf("after args\n");

    opterr = 0 ;

     while ((c = getopt(argc, argv, "i:o:")) != -1) {
       switch (c) {
       case 'i':
         inFile   = strdup(optarg);
         break;
       case 'o':
          outFile = strdup(optarg);
          break;
       default:
         usage(argv[0]);
       }
     }

     printf("Received args\n") ;

    // open input file
    int fin = open(inFile, O_RDONLY);
    if (fin < 0) {
        perror("open");
        exit(1);
    }
    // open and create output file
    int fout = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
    if (fout < 0) {
        perror("open");
        exit(1);
    }

    // output the number of keys as a header for this file
    int recordsLeft;
    int rin;
    int rout;

    rin = read(fin, &recordsLeft, sizeof(recordsLeft));
    if (rin != sizeof(recordsLeft)) {
        perror("read");
        exit(1);
    }

    printf("Number of records: %d\n", recordsLeft);
    // output the number of keys as a header for this file
    rout = write(fout, &recordsLeft, sizeof(recordsLeft));
    if (rout != sizeof(recordsLeft)) {
        perror("write");
        exit(1);
    // should probably remove file here but ...
    }

printf("After reading first line\n");

    int numRecords = recordsLeft;
      rec_dataptr_t fullRecords[recordsLeft];
printf("After define fullrecords\n");
      rec_nodata_t tempRecord;
printf("After define temprecords\n");
//      unsigned int data[recordsLeft][MAX_DATA_INTS];
      unsigned int data[recordsLeft][15];

printf("Before entering read loop\n");

      int count;
      // read all data into array?
      while (recordsLeft) {
        // read fix part
          int data_size = sizeof(rec_nodata_t) ;
          rin = read(fin, &tempRecord,data_size );
          if (rin != data_size) {
              perror("read");
              exit(1);
          }
          //          assert(fixedHead.data_ints <= MAX_DATA_INTS);
          fullRecords[count].key = tempRecord.key;
          fullRecords[count].data_ints  = tempRecord.data_ints;
//////////////////////////////////////////////
// For the above, can try
//        rin = read(fin, &fullRecords[count], sizeof(rec_nodata_t));
//        if (rin != sizeof(rec_nodata_t)) {
//            perror("read");
//            exit(1);
//        }
// End try
//////////////////////////////////////////////

          // read data
          data_size = fullRecords[count].data_ints * sizeof(unsigned int);
          rin = read(fin, &data[count][0],data_size);
          if (rin != data_size) {
              perror("read");
              exit(1);
          }
          fullRecords[count].data_ptr = &data[count][0];

          --recordsLeft;
          ++count;
      }

//printf("After entering read loop\n");
//recordsLeft = numRecords;
//count=0;
//while (recordsLeft) {
//      printf("key %d: %u data_ints: %u rec: ", count, fullRecords[count].key, fullRecords[count].data_ints);
//      int j;
//      for (j = 0; j < fullRecords[count].data_ints; j++)
//        printf("%u ", data[count][j]);
//      printf("\n");
//        --recordsLeft;
//        ++count;
//}
//printf("\n");

      // sort the key
      qsort(fullRecords,numRecords,sizeof(rec_dataptr_t),CompareDataptr);

      // print current
//    recordsLeft = numRecords;
//    count=0;
//    while (recordsLeft) {
//          printf("key %d: %u data_ints: %u rec: ", count, fullRecords[count].key, fullRecords[count].data_ints);
//          int j;
//          for (j = 0; j < fullRecords[count].data_ints; j++)
//            printf("%u ", data[count][j]);
//          printf("\n");
//            --recordsLeft;
//            ++count;
//    }


printf("After sort array\n");

      // output data according to key
      recordsLeft = numRecords;
      count = 0;
      while (recordsLeft) {
          // output fixed part
          int data_size = sizeof(rec_nodata_t);
          rout = write(fout,&fullRecords[count],data_size);
            if (rout != data_size ) {
              perror("write");
              exit(1);
            }
           // output data
           data_size = fullRecords[count].data_ints * sizeof(unsigned int);
          rout = write(fout,(fullRecords[count].data_ptr),data_size);
            if (rout != data_size ) {
              perror("write");
              exit(1);
            }
              --recordsLeft;
              ++count;
      }
printf("After write data\n");

      (void) close(fin);
      (void) close(fout);
}


