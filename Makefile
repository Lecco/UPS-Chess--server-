ups: main.o
	gcc main.o -o ups

main.o: main.c constants.h
	gcc main.c -c
