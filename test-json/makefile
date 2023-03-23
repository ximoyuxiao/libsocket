CXXFLAGS= -I../include  -g -Wall -std=c++11
all:main
main:test.o json.o
	g++ $^ -o $@

clean:
	rm *.o main