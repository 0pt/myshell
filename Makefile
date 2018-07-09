all :
	cc -c shell.c split.c
	cc -o shell shell.o split.o
	./shell
