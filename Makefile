# Main suffixes for auto compile by dependencies
#.SUFFIXES : .cc .o

# Compiler and Compile options
CXX = g++
MAKEFLAGS += -j


# Pre-defined Macros 
CPPFLAGS += -Iinclude
CXXFLAGS += -std=c++11 -g -W -Wall -fPIC

CXX_SRCS := $(wildcard test/*cc)         \
            $(wildcard src/*.cc)

OBJS := $(CXX_SRCS:.cc=.o)
#SRCS = src/$(OBJS: .o=.cc)
BIN = bin/
INCLUDE = include
LIB = ./lib
LIBNAME = -lpthread -lrt

CFLAGS += -I$(INCLUDE) 

all: test

test: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(BIN)$@ $^ $(LIBNAME)

clean:
	rm $(BIN)test $(OBJS) 
	rm /opt/nvm1/NVM/*.txt
	rm /opt/nvm2/NVM/*.txt

again:
	clear
	rm $(BIN)test $(OBJS)	
	make

library:
	make
	g++ -shared -Wl,-soname,libnvm.so -o libnvm.so $(OBJS)
	mv libnvm.so $(LIB)
