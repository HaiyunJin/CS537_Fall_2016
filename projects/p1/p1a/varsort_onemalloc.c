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

int CompareDataptr(const void *r1, const void *r2) {
    const rec_dataptr_t *r1l = (const rec_dataptr_t *) r1;
    const rec_dataptr_t *r2l = (const rec_dataptr_t *) r2;
    return ((int) r1l->key) - ((int) r2l->key);
}

int main(int argc, char *argv[] ) {
    // arguments
    char *inFile, *outFile;
    int c;

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

    int numRecords;
    int rin, rout;
    int intSize = sizeof(int);
    int rec_nodata_t_size = sizeof(rec_nodata_t);
    int rec_dataptr_t_size = sizeof(rec_dataptr_t);

    // get the total size of infile
    struct stat fileStat;
    if ( stat(inFile, &fileStat) < 0 ) {
        perror("read");
        exit(1);
    }

    int fileSize = (int) fileStat.st_size;

    // read out the header
    rin = read(fin, &numRecords, intSize);
    if (rin != intSize) {
        perror("read");
        exit(1);
    }
    // mem       file       header        keys and num
    int dataSize = fileSize - intSize - 2*numRecords*intSize;
    rec_nodata_t tempRecord;
    rec_dataptr_t *fullRecords = malloc(numRecords * rec_dataptr_t_size); //sizeof(rec_dataptr_t));
    int *data = malloc(dataSize * intSize);
    int recordsLeft = numRecords;
    int count = 0;
    int data_size;
    int pos = 0;

    while (recordsLeft) {
      // read fix part key and num
        data_size = rec_nodata_t_size; //sizeof(rec_nodata_t);
        rin = read(fin, &fullRecords[count], rec_nodata_t_size); //sizeof(rec_nodata_t));
        if (rin != rec_nodata_t_size ) { //sizeof(rec_nodata_t)) {
            perror("read");
            exit(1);
        }
        // read data
        data_size = fullRecords[count].data_ints * intSize;
        fullRecords[count].data_ptr = data + pos;
        rin = read(fin, fullRecords[count].data_ptr, data_size);
        if (rin != data_size) {
            perror("read");
            exit(1);
        }

        --recordsLeft;
        ++count;
        pos = pos + data_size;
        // pos = pos + data_size/sizeof(int);
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
      // qsort(fullRecords, numRecords, sizeof(rec_dataptr_t), CompareDataptr);
      qsort(fullRecords, numRecords, rec_dataptr_t_size, CompareDataptr);

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



      // output the number of keys as a header for this file
      rout = write(fout, &numRecords, intSize);
      if (rout != intSize) {
          perror("write");
          exit(1);
      }
      // output data according to key
      recordsLeft = numRecords;
      count = 0;
      while (recordsLeft) {
          // output fixed part
          int data_size = rec_nodata_t_size; //sizeof(rec_nodata_t);
          rout = write(fout, &fullRecords[count], data_size);
          if ( rout != data_size ) {
              perror("write");
              exit(1);
          }
          // output data
          data_size = fullRecords[count].data_ints * intSize;
          rout = write(fout, (fullRecords[count].data_ptr), data_size);
          if ( rout != data_size ) {
              perror("write");
              exit(1);
          }
          --recordsLeft;
          ++count;
      }

      // free memory
      free(fullRecords);
      free(data);
      free(inFile);
      free(outFile);
      (void) close(fin);
      (void) close(fout);

      return 0;
}

