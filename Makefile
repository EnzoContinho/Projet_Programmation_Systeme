cc = gcc
opt = -Wall


all : clean initial archivistes journalistes

initial: initial.c types.h
	$(cc) $(opt) initial.c -o initial

archivistes : Archivistes.c types.h
	$(CC) $(opt) Archivistes.c -o archivistes

journalistes : Journalistes.c types.h
	$(CC) $(opt) Journalistes.c -o journalistes

clean :
	rm -f initial archivistes journalistes
	rm -f fichier_serv
	sh ./script.sh
