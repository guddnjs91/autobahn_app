# Main suffixes for auto compile by dependencies
.SUFFIXES : .cc .o

# Compiler and Compile options
CC = g++
CFLAGS = -std=c++11 -g -W -Wall

#OBJS = ../src/nvm0nvm.o ../src/nvm0lfqueue.o
#SRCS = ../src/$(OBJS: .o=.cc)
INCLUDE = ../include/
LIBNAME = -lpthread

CFLAGS += -I$(INCLUDE)

all: write-test

write-test: ./test_all.cc ./test_write.cc ./test.h
	$(CC) -o $@ $^ $(CFLAGS) $(LIBNAME)

clean:
	rm write-test *.txt
