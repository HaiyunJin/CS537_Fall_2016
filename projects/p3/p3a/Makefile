#####################################################################
#
# Type "make" or "make all" to compile code
# 
# Type "make clean" to remove the executable (and any object files)
#
#####################################################################

CC=gcc
CFLAGS=-Wall -Werror
GDB=-g
RM = rm -f
LFLAGS = -lstats -L.

SOURCES=stat_server.c stats_client.c
EXECUTABLE = stat_server stats_client
LIBSOURCES = stats.c
LIBS = libstats.so
OBJECTS = stat_server.o stats_client.o stats.o

all:$(SOURCES) $(LIBSOURCES) $(EXECUTABLE) $(LIBS)

stat_server : stat_server.c
	$(CC) $(CFLAGS) $< -o $@

stats_client : stats_client.c
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS)

stats.o : stats.c
	$(CC) $(CFLAGS) -c -fpic $< 

libstats.so : stats.o
	$(CC) -shared $< -o $@

clean:
	$(RM) $(EXECUTABLE) $(OBJECTS) 
