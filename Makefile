cc = gcc
opt = -Wall


all : clean initial archivistes journalistes

initial: initial.c
	$(cc) $(opt) initial.c -o initial

archivistes : Archivistes.c Types.h
	$(CC) $(opt) Archivistes.c -o archivistes

journalistes : Journalistes.c Types.h
	$(CC) $(opt) Journalistes.c -o journalistes

clean :
	rm -f initial archivistes journalistes
	rm -f fichier_SMP
	sh ./script.sh
