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

  // int intSize = sizeof(int);
  // int rec_nodata_t_size = sizeof(rec_nodata_t);
  // int rec_dataptr_t_size = sizeof(rec_dataptr_t);

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
    // data        file       header        keys and num
    int dataSize = fileSize - intSize - 2*numRecords*intSize;
    rec_dataptr_t *fullRecords = malloc(numRecords * rec_dataptr_t_size);
    int *data = malloc(dataSize * intSize);
    int data_size = 0;
    int recordsLeft = numRecords;
    int count = 0;
    int pos = 0;

    while (recordsLeft) {
      // read fix part key and num
        data_size = rec_nodata_t_size;
        rin = read(fin, &fullRecords[count], data_size);
        if (rin != data_size) {
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
        // pos = pos + data_size;
        pos = pos + data_size/intSize;
    }


      // sort the key
      // qsort(fullRecords, numRecords, sizeof(rec_dataptr_t), CompareDataptr);
      qsort(fullRecords, numRecords, rec_dataptr_t_size, CompareDataptr);

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
          int data_size = rec_nodata_t_size;
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

