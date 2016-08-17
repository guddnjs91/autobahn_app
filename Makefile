
# Main suffixes for auto compile by dependencies
#.SUFFIXES : .cc .o

# Compiler and Compile options
CXX = g++


# Pre-defined Macros 
CPPFLAGS += -Iinclude
CXXFLAGS += -std=c++11 -g -W -Wall

CXX_SRCS := $(wildcard test/*cc)         \
            $(wildcard src/*.cc)

OBJS := $(CXX_SRCS:.cc=.o)
#SRCS = src/$(OBJS: .o=.cc)
BIN = bin/
INCLUDE = include
LIBNAME = -lpthread -lrt

CFLAGS += -I$(INCLUDE) 

all: test

test: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(BIN)$@ $^ $(LIBNAME)

clean:
	rm $(BIN)test $(OBJS) *.txt *.txt.swp
