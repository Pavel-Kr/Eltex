all: lib
	gcc -o calc main.o -L. -lcalc

lib: objects
	ar rc libcalc.a add.o sub.o mul.o div.o

objects: main.c add.c sub.c mul.c div.c
	gcc -c $^

dyn: dynamic_lib
	gcc main.c -o calc_dyn -L. -ldynamic_calc

dynamic_lib: dynamic_objects
	gcc -shared add.o sub.o mul.o div.o -o libdynamic_calc.so

dynamic_objects: add.c sub.c mul.c div.c
	gcc -fPIC -c $^