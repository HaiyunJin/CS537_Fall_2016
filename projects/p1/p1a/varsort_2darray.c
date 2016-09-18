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


void usage(char *prog) {
  fprintf(stderr, "Usage: varsort -i inputfile -o outputfile\n");
  exit(1);
}

// int CompareDataptr(const rec_dataptr_t *r1, const rec_dataptr_t *r2) {
int CompareDataptr(const void *r1, const void *r2) {
    const rec_dataptr_t *r1l = (const rec_dataptr_t *) r1;
    const rec_dataptr_t *r2l = (const rec_dataptr_t *) r2;
    return ((int) r1l->key) - ((int) r2l->key);
}


int main(int argc, char *argv[] ) {
    // arguments
    char *inFile, *outFile;
    int c;


// /    if ( argc == 1 ) {
// /        fprintf(stderr, "No argument\n");
// /        usage(argv[0]);
// /    }

  // printf("# of argv: %d\n", argc);
  // c = getopt(argc, argv, "io:");
  // printf("c value: %d\n", c);


    int validflag = 0;
     if ( argc == 5 ) {
        while ((c = getopt(argc, argv, "i:o:")) != -1) {
            validflag = 1;
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
     } else {
         usage(argv[0]);
     }

     if ( validflag == 0 ) {
         usage(argv[0]);
     }




    // open input file
    int fin = open(inFile, O_RDONLY);
    if (fin < 0) {
        fprintf(stderr, "Error: Cannot open file %s\n", inFile);
        exit(1);
    }
    // open and create output file
    int fout = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
    if (fout < 0) {
        fprintf(stderr, "Error: Cannot open file %s\n", outFile);
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

    // output the number of keys as a header for this file
    rout = write(fout, &recordsLeft, sizeof(recordsLeft));
    if (rout != sizeof(recordsLeft)) {
        perror("write");
        exit(1);
    // should probably remove file here but ...
    }

// printf("After reading first line\n");

    int numRecords = recordsLeft;
// /      rec_dataptr_t fullRecords[recordsLeft];
      rec_dataptr_t *fullRecords = malloc( sizeof(rec_dataptr_t)*recordsLeft);

// printf("After define fullrecords\n");
      rec_nodata_t tempRecord;
// printf("After define temprecords\n");

      unsigned int **data = malloc( sizeof(*data) * recordsLeft);
// /      int i;

// printf("Before entering read loop\n");

      int count = 0;
      // read all data into array?
      while (recordsLeft) {
        // read fix part
          int data_size = sizeof(rec_nodata_t);
          rin = read(fin, &tempRecord, data_size);
          if (rin != data_size) {
              perror("read");
              exit(1);
          }
          fullRecords[count].key = tempRecord.key;
          fullRecords[count].data_ints  = tempRecord.data_ints;
          data[count] =
              malloc(sizeof(*data[count])*fullRecords[count].data_ints);
//////////////////////////////////////////////
// For the above, can try
// 1       rin = read(fin, &fullRecords[count], sizeof(rec_nodata_t));
// 1       if (rin != sizeof(rec_nodata_t)) {
// 1           perror("read");
// 1           exit(1);
// 1       }
// End try
//////////////////////////////////////////////

          // read data
          data_size = fullRecords[count].data_ints * sizeof(unsigned int);
          // rin = read(fin, &data[count][0],data_size);
          rin = read(fin, data[count], data_size);
          if (rin != data_size) {
              perror("read");
              exit(1);
          }
          fullRecords[count].data_ptr = &data[count][0];

          --recordsLeft;
          ++count;
      }

// printf("After entering read loop\n");
// recordsLeft = numRecords;
// count=0;
// while (recordsLeft) {
// /     printf("key %d: %u data_ints: %u rec: ",
// count, fullRecords[count].key, fullRecords[count].data_ints);
// /    int j;
// /    for (j = 0; j < fullRecords[count].data_ints; j++)
// /      printf("%u ", data[count][j]);
// /    printf("\n");
// /      --recordsLeft;
// /      ++count;
// }
// printf("\n");

      // sort the key
      qsort(fullRecords, numRecords, sizeof(rec_dataptr_t), CompareDataptr);

      // print current
// /  recordsLeft = numRecords;
// /  count=0;
// /  while (recordsLeft) {
// /        printf("key %d: %u data_ints: %u rec: ",
// count, fullRecords[count].key, fullRecords[count].data_ints);
// /        int j;
// /        for (j = 0; j < fullRecords[count].data_ints; j++)
// /          printf("%u ", data[count][j]);
// /        printf("\n");
// /          --recordsLeft;
// /          ++count;
// /  }


// printf("After sort array\n");

      // output data according to key
      recordsLeft = numRecords;
      count = 0;
      while (recordsLeft) {
          // output fixed part
          int data_size = sizeof(rec_nodata_t);
          rout = write(fout, &fullRecords[count], data_size);
            if ( rout != data_size ) {
              perror("write");
              exit(1);
            }
           // output data
           data_size = fullRecords[count].data_ints * sizeof(unsigned int);
          rout = write(fout, (fullRecords[count].data_ptr), data_size);
            if ( rout != data_size ) {
              perror("write");
              exit(1);
            }
              --recordsLeft;
              ++count;
      }
// printf("After write data\n");

      // free memory
      recordsLeft = numRecords;
      count = 0;
      while (recordsLeft) {
          free(data[count]);
          --recordsLeft;
          ++count;
      }
      free(fullRecords);
      free(data);
      free(inFile);
      free(outFile);
      (void) close(fin);
      (void) close(fout);

      return 0;
}


