
# compliler
CC = gcc
# flags
# CFLAGS = -I. -Wall -c  -Werror
CFLAGS = -Wall -c  -Werror
#-Werror
LDFLAGS =
# source files
SOURCES = varsort.c
OBJECTS ?= $(SOURCES:.c=.o)
# executable file
EXECUTABLE = varsort

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm  -f $(EXECUTABLE) $(OBJECTS)

varone : 
	$(CC)  varsort_onemalloc.c -o varsort_onemalloc
