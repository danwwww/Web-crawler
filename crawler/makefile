crawler: function.o main.o
	gcc -o crawler function.o main.o -Wall

function.o : function.c header.h
	gcc -c -Wall function.c

main.o : main.c header.h
	gcc -c -Wall main.c
