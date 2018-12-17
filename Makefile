cc = gcc
opt = -Wall


all : initial

initial: initial.c
	$(cc) $(opt) initial.c -o initial

clean :
	rm -f initial
