all: test

test: test.o ncu.o
	g++ -o test test.o ncu.o -lncurses -lpanel

test.o: test.cpp ncu.h
	g++ -c test.cpp -lncurses -lpanel

ncu.o: ncu.cpp ncu.h
	g++ -c ncu.cpp -lncurses -lpanel
