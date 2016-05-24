.SUFFIXES : .c .o
CC = g++

CFLAGS = -g -Wall
 all : hw

OBJS = ./src/test1.o
SRCS = ./src/$(OBJS:.o=.c)
BIN = ./bin/
LIB = ./lib
INCLUDE = ./include
LIBNAME = -lpthread -lrt

CFLAGS += -I$(INCLUDE)

hw: $(OBJS)
	$(CC) -std=c++11 -g -o $(BIN)test $(OBJS) $(LIBNAME) -L$(LIB) -O3

clean:
	rm -rf $(OBJS)

allclean:
	rm -rf $(OBJS) $(BIN)*
