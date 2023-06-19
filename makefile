.PHONY: all
all : reversi

reversi : reversi.o
	gcc -g reversi.o -o reversi -lncurses

reversi.o : reversi.c reversi.h
	/usr/bin/gcc -g -c reversi.c -o reversi.o

.PHONY : clean
clean:
	rm -rf reversi.o
	rm -rf reversi
