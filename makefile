all: test

tmp: tmp.o ncu.o
	g++ -o tmp tmp.o ncu.o -lncurses -lpanel

test2: test2.o ncu.o
	g++ -o test2 test2.o ncu.o -lncurses -lpanel

test: test.o ncu.o
	g++ -o test test.o ncu.o -lncurses -lpanel

tmp.o: tmp.cpp ncu.h
	g++ -c tmp.cpp -lncurses -lpanel

test.o: test.cpp ncu.h
	g++ -c test.cpp -lncurses -lpanel

test2.o: test2.cpp ncu.h
	g++ -c test2.cpp -lncurses -lpanel

ncu.o: ncu.cpp ncu.h
	g++ -c ncu.cpp -lncurses -lpanel

clean:
	rm  *.o
