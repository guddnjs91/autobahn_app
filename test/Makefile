# Main suffixes for auto compile by dependencies
.SUFFIXES : .cc .o

# Compiler and Compile options
CC = g++
CFLAGS = -std=c++11 -g -W -Wall

#OBJS = ../src/nvm0nvm.o ../src/nvm0lfqueue.o
#SRCS = ../src/$(OBJS: .o=.cc)
BIN = ./unit_test/
INCLUDE = ../include
LIBNAME = -lpthread

CFLAGS += -I$(INCLUDE)

all: nvm0nvm nvm0lfqueue

nvm0nvm: ./test_nvm0nvm.cc ../src/nvm0nvm.cc
	$(CC) -o $(BIN)$@ $^ $(CFLAGS) $(LIBNAME)

nvm0lfqueue: ./test_nvm0lfqueue.cc ../src/nvm0lfqueue.cc
	$(CC) -o $(BIN)$@ $^ $(CFLAGS) $(LIBNAME)

clean:
	rm $(BIN)*
