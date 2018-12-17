/*
 *
 *     initial <nb_archivistes> <nb_themes> :
 *     Création de nb_archivistes (entre 2 et 100)
 *     Création de nb_themes (entre 2 et 100)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>       
#include <dirent.h>         

#define MAX_LGR_NOM 1024   /* Longueur maximale d'un nom de fichier absolu  */

/* Fonction d'usage pour une utilisation facilitée du programme */ 
void usage(char *nom){
    fprintf (stderr, "\nUsage: %s nb_archivistes nb_themes\n",nom);
    fprintf (stderr, "Création de nb_archivistes et nb_themes ");
    fprintf (stderr, "(Entre 2 et 100)\n\n");
    exit (-1);
}


/* Fonction principale */
int main(int argc, char *argv[]){
 
    int nb_archivistes, nb_themes;

    if (argc < 3) {
        usage (argv[0]);
    }
    
    nb_archivistes = atoi(argv[1]);
    nb_themes = atoi(argv[2]);

    /* tester si le nb_archivistes et nb_themes sont bien compris entre 2 et 100 */
    if(nb_archivistes < 2 || nb_archivistes > 100 || nb_themes < 2 || nb_themes > 100){
      printf("Le nombre d'archivistes et/ou de themes doit être compris entre 2 et 100...BYE !\n");
      exit(-1);
    }
    else{
      
      /* il doit créer et initialiser les IPC nécessaires à l'application (ensembles de sémaphores, segments de mémoire, file de message);
il doit créer et lancer les archivistes;
c'est lui qui crée les journalistes : indéfiniment, il crée un journaliste avec une requête. On a observé que, sur 10 demandes aux archives,

    7 sont des demandes de consultation d'archives,
    2 sont des demandes de publication d'articles,
    1 est une demande d'effacement d'articles.

Il y a donc plus de demandes de consultation que de demande de modification des archives : le processus initial doit tenir compte de ces statistiques lors de la création aléatoire des journalistes ainsi que des opérations qu'ils demandent aux archivistes;
à la réception de tout signal (sauf bien sûr SIGKILL et SIGCHLD), il doit terminer les archivistes (ainsi que les journalistes encore en attente), puis supprimer les IPC avant de se terminer : ceci constitue l'unique façon d'arrêter l'application. */      

      
      printf("Compilation parfaite !");   
      exit(0);
    }
}

