#
# This file is part of the course materials for AMATH483/583 at the University of Washington,
# Spring 2017
#
# Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
# https://creativecommons.org/licenses/by-nc-sa/4.0/
#
# Author: Andrew Lumsdaine
# 

LANG		= -std=c++17 -Wc++17-extensions
PICKY		= -Wall 

CXX		= c++
MPICXX	        = mpic++
#OPTS		= -Ofast -march=native -pthread -DNDEBUG #OPTS		= -g -fsanitize=address
OPTS		= -g
DEFS		= 
INCLUDE		= -I../../src 
LIB		= 
VPATH		= ../../src

REPORTING       = -Rpass=.*

CXXFLAGS	= $(OPTS) $(LANG) $(PICKY) $(DEFS) $(INCLUDE) -MD -MP

SOURCES		= 
TESTS		= ch1_toposort.cpp ch3_toposort.cpp ch4_kevin_bacon.cpp
OBJECTS		= $(SOURCES:.cpp=.o) $(TESTS:.cpp=.o)
TARGETS		= $(TESTS:.cpp=.exe)
HEADERS		= $(wildcard *.hpp)
PCHS		= $(HEADERS:=.gch)

.PHONY		: all clean squeaky defreport optreport format

all		: $(TARGETS)

%.s 		: %.cpp
		  $(CXX) -S $(CXXFLAGS) $<

%.o  		: %.cpp
		  $(CXX) -c $(CXXFLAGS) $<

%.exe  		: %.o
		  $(CXX) $(CXXFLAGS) $^ -o $@ $(LIB)

ch1_toposort.exe	: ch1_toposort.o
ch3_toposort.exe	: ch3_toposort.o
ch4_kevin_bacon.exe	: ch4_kevin_bacon.o

defreport	:
		  $(CXX) -dM -E -x c++ /dev/null

optreport	:
		  echo 'int;' | $(CXX) -xc++ $(CXXFLAGS) - -o /dev/null -\#\#\#

format		:
		  clang-format -i $(SOURCES) $(HEADERS) $(TESTS)

clean		:
		  /bin/rm -f a.out $(TARGETS) $(OBJECTS) $(PCHS) Matrix.s *~


squeaky		: clean
		  /bin/rm -f $(SOURCES:%.cpp=%.d) $(TESTS:%.cpp=%.d)


-include $(SOURCES:%.cpp=%.d) $(TESTS:%.cpp=%.d)
