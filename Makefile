cc = gcc
opt = -Wall


all : clean initial

initial: initial.c
	$(cc) $(opt) initial.c -o initial

clear: clean
clean :
	rm -f initial
	rm -f fichier_SMP

prog: all
	sh ./script.sh
