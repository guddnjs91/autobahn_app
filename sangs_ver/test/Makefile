CC = g++
CFLAGS = -std=c++11 -lpthread $(INCLUDE)
INCLUDE = -I../include
SRC = ../src/nvm0nvm.cc ../src/nvm0write.cc
test: *.cc
	$(CC) -o test *.cc test.h $(SRC) $(CFLAGS)

clean:
	rm test
