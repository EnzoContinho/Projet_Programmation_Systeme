#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/stat.h>  
#include <time.h>     
#include <dirent.h>         

#define MAX_LGR_NOM 1024   /* Longueur maximale d'un nom de fichier absolu  */
#define FICHIER_CLE "cle.serv"
#define LETTRE_CODE 'a'
