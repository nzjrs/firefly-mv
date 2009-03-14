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
CPPFLAGS += -I. 
CPPFLAGS += -Wall -g
CPPFLAGS += -DLINUX
CPPFLAGS += `pkg-config --cflags libdc1394-2`

LDFLAGS	+= -L. 
LIBS += `pkg-config --libs libdc1394-2` 

# executable name and contents
EXEC1		= grabgrey
EXEC1SRC	= $(EXEC1).cpp	

EXEC2		= grabbayer
EXEC2SRC	= $(EXEC2).cpp	\
		  pgrutils.cpp

EXEC3		= grabcolor
EXEC3SRC	= $(EXEC3).cpp	

EXEC4		= grabdx
EXEC4SRC	= $(EXEC4).cpp	

EXEC5		= grabform7
EXEC5SRC	= $(EXEC5).cpp	


EXECS		= $(EXEC1) $(EXEC2) $(EXEC3) $(EXEC4) $(EXEC5)


all:	bin

bin: $(EXECS)

$(EXEC1): $(EXEC1SRC:%.cpp=%.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(EXEC2): $(EXEC2SRC:%.cpp=%.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(EXEC3): $(EXEC3SRC:%.cpp=%.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(EXEC4): $(EXEC4SRC:%.cpp=%.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(EXEC5): $(EXEC5SRC:%.cpp=%.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o:%.cpp
	g++ -c $(CXXFLAGS) $(CPPFLAGS) $*.cpp

clean:
	rm -f *~ *.o *.d $(EXECS) 

#####################################################################
#
# $Id: Makefile,v 1.2 2006/11/10 00:28:25 donm Exp $
# $Author: donm $
# $Revision: 1.2 $
# $Date: 2006/11/10 00:28:25 $
#
#####################################################################

