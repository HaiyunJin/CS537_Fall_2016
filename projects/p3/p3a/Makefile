#####################################################################
#
# mysh.c is the name of your source code; you may change this.
# However, you must keep the name of the executable as "mysh".
#
# Type "make" or "make mysh" to compile your code
# 
# Type "make clean" to remove the executable (and any object files)
#
#####################################################################

CC=gcc
CFLAGS=-Wall -g
#CFLAGS=-g
EXECUTABLE = stat_server stats_client


all:$(EXECUTABLE)

stat_server: stat_server.c
	$(CC) -o stat_server $(CFLAGS) stat_server.c

stats_client: stats_client.c
	$(CC) -o stats_client $(CFLAGS) stats_client.c

clean:
	$(RM) stats_client stat_server
