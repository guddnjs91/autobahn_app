CC = g++
CFLAGS = -std=c++11 -lpthread $(INCLUDE)
INCLUDE = -I../../include

test: *.cc
	$(CC) -o unit_test *.cc $(CFLAGS)

clean:
	rm unit_test
