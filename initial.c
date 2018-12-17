/*
 *
 *     initial <nb_archivistes> <nb_themes> :
 *     Création de nb_archivistes (entre 2 et 100)
 *     Création de nb_themes (entre 2 et 100)
 *
 */

#include "types.h"

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
    int crea_alea;
    /* init pour SMP */
    struct stat st;
    key_t cle;
    int mem_part;
    /* fin init SMP */
    /* init pour SEM*/
    int semap,res_init;
    unsigned short val_init[1]={1};
    /* fin init pour sem*/

    if (argc < 3) {
        usage (argv[0]);
    }
    
    nb_archivistes = atoi(argv[1]);
    nb_themes = atoi(argv[2]);

    /* TEST si le nb_archivistes et nb_themes sont bien compris entre 2 et 100 */
    if(nb_archivistes < 2 || nb_archivistes > 100 || nb_themes < 2 || nb_themes > 100){
      printf("Le nombre d'archivistes et/ou de themes doit être compris entre 2 et 100...BYE !\n");
      exit(-1);
    }
    else{

      
      /* Création SMP : */

      /* Creation de la cle */
      /* On teste si le fichier cle existe dans le repertoire courant */
      /* Fabrication du fichier cle s'il n'existe pas */
      if ((stat(FICHIER_CLE,&st) == -1) &&
	  (open(FICHIER_CLE,O_RDONLY|O_CREAT|O_EXCL,0660) == -1)){
	fprintf(stderr,"Pb creation fichier cle\n");
	exit(-1);
      }

      cle = ftok(FICHIER_CLE,LETTRE_CODE);
      if (cle==-1){
	printf("Pb creation cle\n");
	exit(-1);
      }
      
      int *tab;
      
      /* On crée le SMP et on teste s'il existe déjà */
      mem_part = shmget(cle,sizeof(int),IPC_CREAT | IPC_EXCL | 0660);
      if (mem_part==-1){
	printf("Pb creation SMP ou il existe deja\n");
	exit(-1);
      }
    
      /* Attachement de la memoire partagée */
      tab = shmat(mem_part,NULL,0);
      if (tab==(int *) -1){
	printf("Problème d'attachement\n");
	/* Nettoyage... */
	shmctl(mem_part,IPC_RMID,NULL);
	exit(-1);
      }

      tab[0]=0;
      

      /* FIN création SMP : */




      

      
      /* Création ensemble de sémaphores */

      
      /* On cree le semaphore (meme cle) */
      semap = semget(cle,1,IPC_CREAT | IPC_EXCL | 0660);
      if (semap==-1){
	printf("Pb creation semaphore ou il existe deja\n");
	/* Il faut detruire le SMP puisqu'on l'a cree : */
	shmctl(mem_part,IPC_RMID,NULL);
	/* Le detachement du SMP se fera a la terminaison */
	exit(-1);
      }

      /* On l'initialise */
      res_init = semctl(semap,1,SETALL,val_init);
      if (res_init==-1){
	printf("Pb initialisation semaphore\n");
	/* On detruit les IPC deje crees : */
	semctl(semap,1,IPC_RMID,NULL);
	shmctl(mem_part,IPC_RMID,NULL);
	exit(-1);
      }

      
      /* FIN création ensemble de sémaphores */






      




      /* Création FM */

      

      /* Fin création FM */



      
      /* Création du type de demande
       * Génération d'un nb aléatoire entre 1 et 10
       * Avec les statistiques données dans le sujet : 
       * S'il est égal à 1, alors on va créer une demande d'effacement 
       * S'il est compris entre 2 et 3 alors on va créer une demande de publication
       * S'il est compris entre 4 et 10 alors va créer une demande de consultation
       */
      srand(time(NULL));
      crea_alea = rand()%10+1;
      printf("Nombre aléatoire : %d\n", crea_alea);

      if(crea_alea == 1){
	printf("Création d'un journaliste avec une demande d'effacement d'archive\n");
      }
      if(crea_alea < 4 && crea_alea > 1){
	printf("Création d'un journaliste avec une demande de publication d'archive\n");
      }
      if(crea_alea < 11 && crea_alea > 3){
	printf("Création d'un journaliste avec une demande de consultation d'archive\n");
      }
  
      
      
      
      /*

Il y a donc plus de demandes de consultation que de demande de modification des archives : le processus initial doit tenir compte de ces statistiques lors de la création aléatoire des journalistes ainsi que des opérations qu'ils demandent aux archivistes;
à la réception de tout signal (sauf bien sûr SIGKILL et SIGCHLD), il doit terminer les archivistes (ainsi que les journalistes encore en attente), puis supprimer les IPC avant de se terminer : ceci constitue l'unique façon d'arrêter l'application. */      

      
      printf("Compilation parfaite !");
      /* Fermer SMP ??????? */

      /* On nettoie */
      shmctl(mem_part,IPC_RMID,NULL);
      semctl(semap,1,IPC_RMID,NULL);

      
      exit(0);
    }
}

