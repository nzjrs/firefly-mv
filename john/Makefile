#####################################################################
# Copyright (c) 2005 Point Grey Research Inc.
#
# This Makefile is free software; Point Grey Research Inc. 
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.
#
#####################################################################

# compilation flags
CFLAGS += -I. 
CFLAGS += -Wall -g
CFLAGS += -DLINUX
CFLAGS += `pkg-config --cflags libdc1394-2`

LDFLAGS	+= -L. 
LIBS += `pkg-config --libs libdc1394-2` 

BIN := camls color f7record f7playback

all: $(BIN)

f7record: format7-record-simple.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

f7playback: format7-playback-simple.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

camls: camls.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

color: color.c
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o:%.c
	$(CC) -c $(CFLAGS) $*.c

clean:
	rm -f *~ *.o $(BIN)

