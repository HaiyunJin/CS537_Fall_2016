#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "stats.h"
#include "defs.h"

stats_t* stat_init(key_t key); // return NULL on failure
int stat_unlink(key_t key); // return 0 on succsess, -1 on failure

