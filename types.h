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
#include <signal.h>

#define MAX_LGR_NOM 1024   /* Longueur maximale d'un nom de fichier absolu  */
#define FICHIER_CLE "fichier_serv"
#define LETTRE_CODE 'a'
#define NB_THEME_MIN 2
#define NB_ARCHIVISTES_MIN 2
#define NB_THEME_MAX 100
#define NB_ARCHIVISTES_MAX 100


typedef struct
{
    long type;       
    int journaliste; /*ID du journaliste entre 10 000 & 20 000*/
    int mode;        /* si mode=0 => consultation
                        si mode=1 => ajout
                        sinon mode=2 => suppression*/
    int theme;       /*numéro du thème de l'article*/
    int num_article; /*numéro de l'article; 
                       Dans le cas d'un consultation 
                       ou d'une suppresion */
    char* texte;    /*Dans le cas de l'ajout*/
} 
requete_t;

typedef struct
{
    long type;
    int archiviste; /*ID de l'archiviste*/
    int mode;        /* si mode=0 => consultation
                        si mode=1 => ajout
                        sinon mode=2 => suppression*/
    char* resu;
} 
reponse_t;

typedef struct
{
    int nb_lecteurs;
    int nb_ecrivains;
    char articles[4*NB_THEME_MAX] ;
}
theme_t;

/* Couleurs dans xterm                                     */
#define couleur(param) printf("\033[%sm",param)

#define NOIR  "30"
#define ROUGE "31"
#define VERT  "32"
#define JAUNE "33"
#define BLEU  "34"
#define CYAN  "36"
#define BLANC "37"
#define REINIT "0"


/* 
   Utilisation (pour ecrire en rouge) :
   
   couleur(ROUGE); fprintf(stdout,"Hello\n"); couleur(REINIT);
 
*/

void mon_sigaction(int signal, void (*f)(int)){
    struct sigaction action;
 
    action.sa_handler = f;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(signal,&action,NULL);
}
