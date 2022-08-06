all:
	gcc main.c -ldl -o calc -g

libs: add.o sub.o mul.o div.o
	gcc -shared add.o -o libadd.so
	gcc -shared sub.o -o libsub.so
	gcc -shared mul.o -o libmul.so
	gcc -shared div.o -o libdiv.so

obj: add.c sub.c mul.c div.c
	gcc -fPIC -c $^