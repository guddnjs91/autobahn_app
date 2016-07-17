.SUFFIXES : .cc .o
CC = g++

CFLAGS = -g -Wall -std=c++11
 all : hw

OBJS = ./src/nvm0avltree.o ./src/nvm0flush.o ./src/nvm0write.o ./src/nvm0nvm.o ./src/test/test_nvm_write.o
SRCS = ./src/$(OBJS:.o=.cc)
BIN = ./bin/
LIB = ./lib
INCLUDE = include
LIBNAME = -lpthread -lrt

CFLAGS += -I$(INCLUDE)

hw: $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN)test $(OBJS) $(LIBNAME) -L$(LIB) -O3

clean:
	rm -rf $(OBJS)

allclean:
	rm -rf $(OBJS) $(BIN)test


#g++ -std=c++11 -Iinclude -c src/nvm0nvm.cc src/nvm0write.cc src/nvm0avltree.cc util/nvm0lfqueue.cc -lpthread -lrt

