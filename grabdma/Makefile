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
#CPPFLAGS += -Wall -O3

LDFLAGS	+= -L. 
LIBS    += -ldc1394_control -lraw1394 -pthread 

# executable name and contents
EXEC1		= grabdma
EXEC1SRC	= $(EXEC1).cpp	

EXEC2		= grabcolor
EXEC2SRC	= $(EXEC2).cpp	\
		  conversions.cpp

EXEC3		= grabform7
EXEC3SRC	= $(EXEC3).cpp	\
		  conversions.cpp


EXECS		= $(EXEC1) $(EXEC2) $(EXEC3)


all:	bin

bin: $(EXECS)

$(EXEC1): $(EXEC1SRC:%.cpp=%.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(EXEC2): $(EXEC2SRC:%.cpp=%.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(EXEC3): $(EXEC3SRC:%.cpp=%.o)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o:%.cpp
	g++ -c $(CXXFLAGS) $(CPPFLAGS) $*.cpp

clean:
	rm -f *~ *.o *.d $(EXECS) 

#####################################################################
#
# $Id: Makefile,v 1.4 2005/03/15 19:50:24 donm Exp $
# $Author: donm $
# $Revision: 1.4 $
# $Date: 2005/03/15 19:50:24 $
#
#####################################################################

