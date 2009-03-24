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


BIN := dc1394-camls dc1394-show dc1394-view dc1394-record dc1394-play

all: $(BIN)

dc1394-camls: camls.o utils.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

dc1394-record: record.o utils.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

dc1394-play: play.o utils.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) $(GTKLIBS)

play.o: play.c
	$(CC) -c $(CFLAGS) $(GTKCFLAGS) -o $@ $^

dc1394-show: show.o utils.o gtkutils.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) $(GTKLIBS)

show.o: show.c
	$(CC) -c $(CFLAGS) $(GTKCFLAGS) -o $@ $^

gtkutils.o: gtkutils.c
	$(CC) -c $(CFLAGS) $(GTKCFLAGS) -o $@ $^

dc1394-view: view.o utils.o gtkutils.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) $(GTKLIBS)

view.o: view.c
	$(CC) -c $(CFLAGS) $(GTKCFLAGS) -o $@ $^

%.o:%.c
	$(CC) -c $(CFLAGS) $*.c

clean:
	rm -f *~ *.o $(BIN)

