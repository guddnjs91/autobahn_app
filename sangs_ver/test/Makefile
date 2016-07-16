CC = g++
CFLAGS = -std=c++11 -lpthread $(INCLUDE)
INCLUDE = -I../include
SRC = ../src/nvm0nvm.cc ../src/nvm0write.cc 
#test: *.cc
#	$(CC) -o test *.cc test.h $(SRC) $(CFLAGS)

test_write: test_write.cc
	$(CC) -o test_write test_all.cc test_write.cc test.h $(CFLAGS)

clean:
	rm test_write *.txt
