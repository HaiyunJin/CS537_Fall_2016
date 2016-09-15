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
// fprintf(stderr, "usage: %s <-i inFile> <-o outFile>\n", prog);
  fprintf(stderr, "usage: varsort <-i inFile> <-o outFile>\n");
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


     if ( argc == 5 ) {
    while ((c = getopt(argc, argv, "i:o:")) != -1) {
        printf(" I came here. \n");
      switch (c) {
      case 'i':
        inFile   = strdup(optarg);
        printf("Infile: %s\n", inFile);
        break;
      case 'o':
        outFile = strdup(optarg);
        printf("outfile: %s\n", outFile);
        break;
      default:
        printf("Here comes the default\n");
        usage(argv[0]);
      }
    }
     } else if ( argc == 1 ) {
         fprintf(stderr, "No argument\n");
         usage(argv[0]);
     } else {
         usage(argv[0]);
     }




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

    int numRecords;
    int rin;
    int rout;
    int intSize = sizeof(int);


    // get the total size of infile
    struct stat fileStat;
    if ( stat(inFile, &fileStat) < 0 ) {
        perror("read");
        exit(1);
    }
    //  fileStat.st_size

    printf("file size: %d\n", (int)fileStat.st_size);
    // exit(1);
    int fileSize = (int) fileStat.st_size;

    // read out the header
    rin = read(fin, &numRecords, sizeof(numRecords));
    if (rin != sizeof(numRecords)) {
        perror("read");
        exit(1);
    }
    // mem       file       header        keys and num
    int dataSize = fileSize - intSize - 2*numRecords*intSize;
    printf("data size: %d\n", dataSize/4 );




// printf("After reading first line\n");

// /      rec_dataptr_t fullRecords[numRecords];
      rec_dataptr_t *fullRecords = malloc( sizeof(rec_dataptr_t)*numRecords);

// printf("After define fullrecords\n");
      rec_nodata_t tempRecord;
// printf("After define temprecords\n");

// /      unsigned int **data = malloc( sizeof(*data) * numRecords);
// /      int i;

// printf("Before entering read loop\n");

      int *data = malloc(dataSize*sizeof(int));


        
      int recordsLeft = numRecords;
      int count = 0;
      int data_size;
      int pos = 0;
      // read all data into array?
      while (recordsLeft) {
        // read fix part
          data_size = sizeof(rec_nodata_t);
          rin = read(fin, &fullRecords[count], sizeof(rec_nodata_t));
          if (rin != sizeof(rec_nodata_t)) {
              perror("read");
              exit(1);
          }

          // read data
          data_size = fullRecords[count].data_ints * sizeof(unsigned int);
          fullRecords[count].data_ptr = data + pos ;
          rin = read(fin, fullRecords[count].data_ptr, data_size);
          if (rin != data_size) {
              perror("read");
              exit(1);
          }

          --recordsLeft;
          ++count;
          pos = pos + data_size;
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

      // output the number of keys as a header for this file
      rout = write(fout, &numRecords, sizeof(numRecords));
      if (rout != sizeof(numRecords)) {
          perror("write");
          exit(1);
          // should probably remove file here but ...
      }
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
      free(data);
      (void) close(fin);
      (void) close(fout);

      return 0;
}


