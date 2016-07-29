
# Main suffixes for auto compile by dependencies
#.SUFFIXES : .cc .o

# Compiler and Compile options
CXX = g++


# Pre-defined Macros 
CPPFLAGS += -Iinclude
CXXFLAGS += -std=c++11 -pg -g -W -Wall

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

##.SUFFIXES : .cc .o
#CXX = g++
#CPPFLAGS = -g -Wall -std=c++11
#
#OBJS = bin/nvm0nvm.o bin/nvm0avltree.o bin/nvm0flush.o bin/nvm0write.o bin/nvm0flush.o bin/nvm0balloon.o
#SRCS = src/$(OBJS:.o=.cc)
#BIN = ./bin/
#LIB = ./lib/
#INCLUDE = -Iinclude -Itest
#LIBNAME = -lpthread -lrt
#
#test: nvm0lfqueue.o nvm0avltree.o nvm0nvm.o nvm0write.o nvm0flush.o nvm0balloon.o test_write.o test_nvm_write.o test_all.o test_ver_0
#
#nvm0lfqueue.o: src/nvm0lfqueue.cc
#	$(CXX) $(INCLUDE) $(CPPFLAGS) -o $(BIN)$@ -c $^ $(LIBNAME)
#
#nvm0avltree.o: src/nvm0avltree.cc
#	$(CXX) $(INCLUDE) $(CPPFLAGS) -o $(BIN)$@ -c $^ $(LIBNAME)
#
#nvm0nvm.o: src/nvm0nvm.cc
#	$(CXX) $(INCLUDE) $(CPPFLAGS) -o $(BIN)$@ -c $^ $(LIBNAME)
#
#nvm0write.o: src/nvm0write.cc
#	$(CXX) $(INCLUDE) $(CPPFLAGS) -o $(BIN)$@ -c $^ $(LIBNAME)
#
#nvm0flush.o: src/nvm0flush.cc
#	$(CXX) $(INCLUDE) $(CPPFLAGS) -o $(BIN)$@ -c $^ $(LIBNAME)
#
#nvm0balloon.o: src/nvm0balloon.cc
#	$(CXX) $(INCLUDE) $(CPPFLAGS) -o $(BIN)$@ -c $^ $(LIBNAME)
#
#test_write.o: test/test_write.cc
#	$(CXX) $(INCLUDE) $(CPPFLAGS) -o $(BIN)$@ -c $^ $(LIBNAME)
#
#test_nvm_write.o: test/test_nvm_write.cc
#	$(CXX) $(INCLUDE) $(CPPFLAGS) -o $(BIN)$@ $^ $(LIBNAME)
#
#test_all.o: test/test_all.c
#	$(CXX) $(INCLUDE) $(CPPFLAGS) -o $(BIN)$@ $^ $(LIBNAME)
#
#test_ver_0: bin/test_all.o bin/test_write.o test/test_nvm_write.o $(OBJS)
#	$(CXX) $(INCLUDE) $(CPPFLAGS) -o $(BIN)$@ $^ $(LIBNAME)
#
#clean:
#	rm -rf $(BIN)*.o
#
#allclean:
#	rm -rf $(OBJS) $(BIN)test_ver_0
#
#
##g++ -std=c++11 -Iinclude -c src/nvm0nvm.cc src/nvm0write.cc src/nvm0avltree.cc util/nvm0lfqueue.cc -lpthread -lrt
