all: main.o list.o error_exit.o
	gcc main.o list.o error_exit.o -std=c99
main.o: main.c
	gcc -c main.c
list.o: list.c
	gcc -c list.c
error_exit.o: error_exit.c
	gcc -c error_exit.c
