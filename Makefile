# Makefile for semestral work KIV/UPS (Chess)
#
# Author: Oldřich Pulkrt <O.Pulkrt@gmail.com>
# Date: 25.1.2014

nazev=ups

${nazev}: main.o communication.o chess.o
	gcc main.o -o ups

main.o: main.c constants.h chess_game.h
	gcc main.c -c

communication.o: communication.c
	gcc communication.c -c

chess.o: chess.c
	gcc chess.c -c

.PHONY: build
.PHONY: install
.PHONY: uninstall
.PHONY: clean
.PHONY: distrib

clean: 
	rm -f *.o ${nazev}
