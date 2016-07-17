# Main suffixes for auto compile by dependencies
.SUFFIXES : .cc .o

# Compiler and Compile options
CXX = g++
CXXFLAGS = -std=c++11 -g -W -Wall

#OBJS = ../src/nvm0nvm.o ../src/nvm0lfqueue.o
#SRCS = ../src/$(OBJS: .o=.cc)
INCLUDE = ../../include/
LIBNAME = -lpthread
BIN = ./bin/
CXXFLAGS += -I$(INCLUDE)

all: nvm0nvm nvm0lfqueue nvm0avltree nvm0write

nvm0nvm: ./test_nvm0nvm.cc ../../src/nvm0nvm.cc
	$(CXX) -o $(BIN)$@ $^ $(CXXFLAGS) $(LIBNAME)

nvm0lfqueue: ./test_nvm0lfqueue.cc ../../util/nvm0lfqueue.cc
	$(CXX) -o $(BIN)$@ $^ $(CXXFLAGS) $(LIBNAME)

nvm0avltree: ./test_nvm0avltree.cc ../../src/nvm0avltree.cc
	$(CXX) -o $(BIN)$@ $^ $(CXXFLAGS) $(LIBNAME)

nvm0write: ./test_nvm0write.cc ../../src/nvm0nvm.cc ../../src/nvm0write.cc ../../src/nvm0avltree.cc
	$(CXX) -o $(BIN)$@ $^ $(CXXFLAGS) $(LIBNAME)

clean:
	rm $(BIN)*
