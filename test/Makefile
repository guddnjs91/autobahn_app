# Main suffixes for auto compile by dependencies
#.SUFFIXES : .cc .o

# Compiler and Compile options
CXX = g++


# Pre-defined Macros 
CPPFLAGS += -I../include
CXXFLAGS += -std=c++11 -g -W -Wall

CXX_SRCS := $(wildcard ./*cc)         \
            $(wildcard ../src/*.cc)

OBJS := $(CXX_SRCS:.cc=.o)
SRCS = ../src/$(OBJS: .o=.cc)
INCLUDE = ../include
LIBNAME = -lpthread -lrt

CFLAGS += -I$(INCLUDE)

all: test

test: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBNAME)

clean:
	rm $(OBJS) test
