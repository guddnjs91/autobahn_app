# Main suffixes for auto compile by dependencies
#.SUFFIXES : .cc .o

# Compiler and Compile options
CXX = g++

INCLUDE = -I../include/
LIBNAME = -lpthread -lrt

CPPFLAGS += $(INCLUDE)
CXXFLAGS = -std=c++11 -g -W -Wall

all: nvm0nvm nvm0lfqueue nvm0avltree ut0lflist

nvm0nvm: ./test_nvm0nvm.cc ../src/nvm0nvm.cc ../src/nvm0flush.cc ../src/nvm0balloon.cc ../src/nvm0sync.cc
	$(CXX) $(INCLUDE) $(CXXFLAGS) -o $@ $^ $(LIBNAME)

nvm0lfqueue: ./test_nvm0lfqueue.cc ../src/nvm0lfqueue.cc
	$(CXX) $(INCLUDE) $(CXXFLAGS) -o $@ $^ $(LIBNAME)

nvm0avltree: ./test_nvm0avltree.cc ../src/nvm0avltree.cc
	$(CXX) $(INCLUDE) $(CXXFLAGS) -o $@ $^ $(LIBNAME)

#nvm0write: ./test_nvm0write.cc ../src/nvm0nvm.cc ../src/nvm0write.cc ../src/nvm0avltree.cc ../src/nvm0flush.cc ../src/nvm0balloon.cc ../src/nvm0sync.cc
#	$(CXX) $(INCLUDE) $(CXXFLAGS) -o $@ $^ $(LIBNAME)

ut0lflist: ./test_ut0lflist.cc ../src/ut0lflist.cc
	$(CXX) $(INCLUDE) $(CXXFLAGS) -o $@ $^ $(LIBNAME)

clean:
	rm nvm0nvm nvm0lfqueue nvm0avltree nvm0write ut0lflist *.txt
