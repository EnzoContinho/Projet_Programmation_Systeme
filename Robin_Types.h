#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

/*Les fichiers d'enzo peuvent par exemple tous commencer par cle.archiv? avec ? allant de 2 à 100 et dans le fichier archivistes je dois boucler sur l'ouverture des 
fichier cle.*/

#define FICHIER_CLE_FILE "cle.file"
#define LETTRE_CODE 'a'
#define NB_THEME_MIN 2
#define NB_ARCHIVISTES_MIN 2
#define NB_THEME_MAX 100
#define NB_ARCHIVISTES_MAX 100


typedef struct
{
    long type;       /*Type 1 => consultation ; 
                       Type 2 => publication*/
    int journaliste; /*Id du journaliste entre 10 000 & 20 000*/
    int mode;        /*Seulement pour le type 2 (si mode=1 => ajout)
                                       sinon mode=2 => suppression)*/
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
    int archiviste; /*Id de l'archiviste*/
    char* resu;
} 
reponse_t;


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
