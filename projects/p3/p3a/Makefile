#####################################################################
#
# Type "make" or "make all" to compile code
# 
# Type "make clean" to remove the executable (and any object files)
#
#####################################################################

CC=gcc
CFLAGS=-Wall -Werror -pthread -lrt
GDB=-g
# GDB=
RM = rm -f
LFLAGS = -lstats -L.

SOURCES=stats_server.c stats_client.c
EXECUTABLE = stats_server stats_client
LIBSOURCES = stats.c
LIBS = libstats.so
OBJECTS = stats_server.o stats_client.o stats.o

all:$(SOURCES) $(LIBSOURCES) $(LIBS) $(EXECUTABLE) stats.h

stats_server : stats_server.c defs.h stats.h
	$(CC) $(CFLAGS) $< -o $@ $(GDB)

stats_client : stats_client.c defs.h stats.h
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(GDB)

stats.o : stats.c stats.h defs.h
	$(CC) $(CFLAGS) -c -fpic $< 

libstats.so : stats.o 
	$(CC) -shared $< -o $@

clean:
	$(RM) $(EXECUTABLE) $(OBJECTS) $(LIBS)
