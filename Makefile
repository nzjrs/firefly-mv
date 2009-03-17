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

CFLAGS += `pkg-config --cflags gtk+-2.0`
LIBS += `pkg-config --libs gtk+-2.0` 


BIN := camls grey show-gray show-color record-gray

all: $(BIN)

f7record: format7-record-simple.o utils.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

f7playback: format7-playback-simple.o utils.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

camls: camls.o utils.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

grey: grey.o utils.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

record-gray: record-gray.o utils.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

show-gray: show-gray.o utils.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) $(GTKLIBS)

show-color: show-color.o utils.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) $(GTKLIBS)

show-gray.o: show-gray.c
	$(CC) -c $(CFLAGS) $(GTKCFLAGS) -o $@ $^

show-color.o: show-color.c
	$(CC) -c $(CFLAGS) $(GTKCFLAGS) -o $@ $^

%.o:%.c
	$(CC) -c $(CFLAGS) $*.c

clean:
	rm -f *~ *.o $(BIN)

