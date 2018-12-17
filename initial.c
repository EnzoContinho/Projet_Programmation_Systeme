/*
 *
 *     initial <nb_archivistes> <nb_themes> :
 *     Création de nb_archivistes (entre 2 et 100)
 *     Création de nb_themes (entre 2 et 100)
 *
 */

#include "types.h"

int file_mess; /* ID de la file, necessairement global
		  pour pouvoir la supprimer a la terminaison */

/* Fonction d'usage pour une utilisation facilitée du programme */ 
void usage(char *nom){
    fprintf (stderr, "\nUsage: %s nb_archivistes nb_themes\n",nom);
    fprintf (stderr, "Création de nb_archivistes et nb_themes ");
    fprintf (stderr, "(Entre 2 et 100)\n\n");
    exit (-1);
}


/* Fonction principale */
int main(int argc, char *argv[], char **envp){
    int i;
    int j;
    int pid;
    int num_ordre;
    int nb_archivistes, nb_themes;
    nb_archivistes = atoi(argv[1]);
    nb_themes = atoi(argv[2]);
    int crea_alea;
    /* init pour fabrication de la clé */
    struct stat st;
    key_t cle;
    /* fin pour fabrication de la clé */
    /* init pour SMP */
    int *tab;
    int stock_SMP[nb_themes];
    /* fin init SMP */
    /* init pour SEM*/
    int semap,res_init;
    unsigned short val_init[1]={1};
    /* fin init pour sem*/

    if (argc < 3) {
        usage (argv[0]);
    }
    
    
    /* TEST si le nb_archivistes et nb_themes sont bien compris entre 2 et 100 */
    if(nb_archivistes < 2 || nb_archivistes > 100 || nb_themes < 2 || nb_themes > 100){
      printf("Le nombre d'archivistes et/ou de themes doit être compris entre 2 et 100...BYE !\n");
      exit(-1);
    }
    else{


      /* On va créer nb_themes SMP 
       * Avec une nouvelle clé à chaque fois */

      for(i=0;i<nb_themes;i++){
	/* ___________________________________________________________ */
	/* Création de la clé */      
	/* On teste si le fichier cle existe dans le repertoire courant */
	/* Fabrication du fichier cle s'il n'existe pas */
	if ((stat(FICHIER_CLE,&st) == -1) &&
	    (open(FICHIER_CLE,O_RDONLY|O_CREAT|O_EXCL,0660) == -1)){
	  fprintf(stderr,"Pb creation fichier cle\n");
	  exit(-1);
	}
	cle = ftok(FICHIER_CLE,i);
	if (cle==-1){
	  printf("Pb creation cle\n");
	  exit(-1);
	}
	/* FIN création de la clé */
	/* ___________________________________________________________ */
	/* Création SMP : */
	/* On crée le SMP et on teste s'il existe déjà */
	stock_SMP[i] = shmget(cle,sizeof(int),IPC_CREAT | IPC_EXCL | 0660);
	
	if (stock_SMP[i]==-1){
	  printf("Pb creation SMP ou il existe deja\n");
	  exit(-1);
	}
	/* Attachement de la memoire partagée */
	tab = shmat(stock_SMP[i],NULL,0);
	if (tab==(int *) -1){
	  printf("Problème d'attachement\n");
	  /* Nettoyage... */
	  shmctl(stock_SMP[i],IPC_RMID,NULL);
	  exit(-1);
	}
	//tab[0]=0;
	/* FIN création SMP : */
      }
      /* FIN créationS SMP : */

      /* ___________________________________________________________ */
      
      /* Création ensemble de sémaphores */

      
      /* On cree le semaphore (meme cle) */
      semap = semget(cle,1,IPC_CREAT | IPC_EXCL | 0660);
      if (semap==-1){
	printf("Pb creation semaphore ou il existe deja\n");
	/* Il faut detruire le SMP puisqu'on l'a cree : */
	shmctl(stock_SMP[i],IPC_RMID,NULL);
	/* Le detachement du SMP se fera a la terminaison */
	exit(-1);
      }

      /* On l'initialise */
      res_init = semctl(semap,1,SETALL,val_init);
      if (res_init==-1){
	printf("Pb initialisation semaphore\n");
	/* On detruit les IPC deja créées : */
	semctl(semap,1,IPC_RMID,NULL);
	shmctl(stock_SMP[i],IPC_RMID,NULL);
	exit(-1);
      }

      
      /* FIN création ensemble de sémaphores */

      /* ___________________________________________________________ */

      /* Création FM */

      file_mess = msgget(cle,IPC_CREAT | IPC_EXCL | 0660);
      if (file_mess==-1){
	fprintf(stderr,"Pb creation file de message\n");
	exit(-1);
      }

      /* Fin création FM */

      /* ___________________________________________________________ */
              

      /* Capture des signaux */

      
      
      
      /* Fin Capture des signaux */

      

      
      /* Création des archivistes */
      for(j=0;j<nb_archivistes;j++){
	printf("Nb archivistes DANS boucle : %d\n", j);

	
	srand(getpid());
	//num_ordre = rand()%(nb_archivistes)+1;
	  
	char* argv_stru[20];
	char tmp[50];
	
	argv_stru[0] = strdup("./Archivistes");	    	
	sprintf(tmp, "%d", j);
	argv_stru[1] = strdup(tmp);	    
	sprintf(tmp, "%d", nb_themes);
	argv_stru[2] = strdup(tmp);
	printf("Création de l'archiviste :  n°%d // Numéro de l'ordre : %s // Thème n° : %s\n",j,argv_stru[1],argv_stru[2]);
	pid = fork();
	printf("PID : %d\n",pid);
	if (pid==-1){      
	  break;
	}
	if (pid==0){
	  execve("./Archivistes", argv_stru, envp);
	  exit(-1);
	}
      }

      int nb_journalistes=10000;

      /* Création des journalistes */
      for(;;){
	if(nb_journalistes>9999 && nb_journalistes<10005){
	  pid = fork();                                                           
	  if (pid==-1){      
            break;
	  }
	  if (pid==0){
	    
	    srand(getpid());
	    crea_alea = rand()%10+1;
	    printf("Nombre aléatoire : %d\n", crea_alea);
	    char* argv_stru[20];
	    char* tmp={"0"};
	    int theme_alea = rand()%nb_themes;
	    char txt[4];
	    int numero_article = rand()%100; // On fixe a 100 le nb max d'articles
	    if(crea_alea == 1){
	      /*Création d'un journaliste avec une demande d'effacement d'archive */

	      sprintf(tmp, "%d", nb_journalistes);
	      argv_stru[0] = strdup(tmp);
				    
	      sprintf(tmp, "%d", nb_archivistes);
	      argv_stru[1] = strdup(tmp);

	      sprintf(tmp, "E");
	      argv_stru[2] = strdup(tmp);

	      sprintf(tmp, "%d", theme_alea);
	      argv_stru[3] = strdup(tmp);

	      sprintf(tmp, "%d", numero_article);
	      argv_stru[4] = strdup(tmp);
	    }
	    else if(crea_alea < 4 && crea_alea > 1){
	      /* Création d'un journaliste avec une demande de publication d'archive */

	      sprintf(tmp, "%d", nb_journalistes);
	      argv_stru[0] = strdup(tmp);
				    
	      sprintf(tmp, "%d", nb_archivistes);
	      argv_stru[1] = strdup(tmp);

	      sprintf(tmp, "P");
	      argv_stru[2] = strdup(tmp);

	      sprintf(tmp, "%d", theme_alea);
	      argv_stru[3] = strdup(tmp);

	      txt[0]=rand()%(90-65)+65;
	      txt[1]=rand()%(122-97)+97;
	      txt[2]=rand()%(122-97)+97;
	      txt[3]=rand()%(122-97)+97;
	      sprintf(tmp, "%s", txt);
	      argv_stru[4] = strdup(tmp);
	    }
	    else{
	      /* Création d'un journaliste avec une demande de consultation d'archive */

	      sprintf(tmp, "%d", nb_journalistes);
	      argv_stru[0] = strdup(tmp);
				    
	      sprintf(tmp, "%d", nb_archivistes);
	      argv_stru[1] = strdup(tmp);

	      sprintf(tmp, "C");
	      argv_stru[2] = strdup(tmp);

	      sprintf(tmp, "%d", stock_SMP[theme_alea]);
	      argv_stru[3] = strdup(tmp);

	      sprintf(tmp, "%d", numero_article);
	      argv_stru[4] = strdup(tmp);
	    }
	    execve("./journalistes", argv_stru, envp);
	    exit(-1);
	  }
	  nb_journalistes++;
	}
      }
	      


      /*
	à la réception de tout signal (sauf bien sûr SIGKILL et SIGCHLD), il doit terminer les archivistes (ainsi que les journalistes encore en attente), 
	puis supprimer les IPC avant de se terminer : ceci constitue l'unique façon d'arrêter l'application.
      */ 
   
      
      printf("Compilation parfaite !\n");
      /* Fermer SMP ??????? */

      /* On nettoie.............  */


      /* Un appel système de recouvrement doit toujours etre suivi par ... ? */
      
      exit(0);
    }
}

