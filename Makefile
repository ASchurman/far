#-------------------------------------------------------------------------------

#	Makefile for Far
#
#	Alexander Schurman
#	alexander.schurman@gmail.com

# target executable name
TARGET	:=Far

# source files with extensions, separated by spaces
SOURCES	:=main.c far.c charBuffer.c fileList.c

# define DEBUG=1 in command line for debug

#-------------------------------------------------------------------------------

CC		:= gcc

# flags------------------------------------
ALLFLAGS	:= -Wall -pedantic -Werror
CFLAGSBASE	:= -std=c99

DEBUGFLAGS	:= -g3
RELEASEFLAGS	:= -O3

VALGRIND	:= valgrind -q

ifeq ($(DEBUG),1)
	CFLAGS	:= $(ALLFLAGS) $(CFLAGSBASE) $(DEBUGFLAGS)
else
	CFLAGS	:= $(ALLFLAGS) $(CFLAGSBASE) $(RELEASEFLAGS)
endif

# building---------------------------------

OBJ	:= $(SOURCES:.c=.o)

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $^

Far: all

main.o: far.h
far.o: fileList.h charBuffer.h

# cleaning---------------------------------

clean:
	rm -f $(TARGET) *.o
