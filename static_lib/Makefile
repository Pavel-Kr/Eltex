all: lib
	gcc -o calc main.o -L. -lcalc

lib: objects
	ar rc libcalc.a add.o sub.o mul.o div.o

objects: main.c add.c sub.c mul.c div.c
	gcc -c $^