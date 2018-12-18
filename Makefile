cc = gcc
opt = -Wall


all : clean initial Archivistes Journalistes

initial: initial.c types.h
	$(cc) $(opt) initial.c -o initial

Archivistes : Archivistes.c types.h
	$(CC) $(opt) Archivistes.c -o Archivistes

Journalistes : Journalistes.c types.h
	$(CC) $(opt) Journalistes.c -o Journalistes

clean :
	rm -f initial Archivistes Journalistes
	rm -f fichier_serv
	sh ./script.sh
