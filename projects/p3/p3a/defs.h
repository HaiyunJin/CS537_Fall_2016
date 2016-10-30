#define NUMBER_OF_BUFFERS 8 // need to change to 16 before submit
#define NAME_LENGTH 15
#define NS_PER_SEC  1000000000L;


stats_t* stats_init(key_t key); // return NULL on failure
int stats_unlink(key_t key); // return 0 on succsess, -1 on failure

int debug = 0;


