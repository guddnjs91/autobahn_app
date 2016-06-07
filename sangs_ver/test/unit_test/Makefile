CC = g++
CFLAGS = -std=c++11 -lpthread $(INCLUDE)
INCLUDE = -I../../include
SRC = ../../src/nvm0nvm.cc
test: *.cc
	$(CC) -o unit_test *.cc $(SRC) $(CFLAGS)

clean:
	rm unit_test
