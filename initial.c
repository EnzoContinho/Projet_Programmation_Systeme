/*
*
*     initial <nb_archivistes> <nb_themes> :
*     Création de nb_archivistes (entre 2 et 100)
*     Création de nb_themes (entre 2 et 100)
*
*/

#include "types.h"

int nb_themes; /* nb_themes, necessairement global pour pouvoir la supprimer a la terminaison */

int fils[NB_ARCHIVISTES_MAX];/* tableau des pid des archivistes pour suppression*/
int cmpt; /* compteur du tableau des fils */
/*
* Fonction appelée à la fin du programme ou lors d'une interruption du programme  pour nettoyer les IPC
*
*/
void handler(int j){
  key_t cle;
  int tmp;
  couleur(ROUGE);
  fprintf(stdout,"Effacement des IPC... \n");
  couleur(REINIT);
  /* Nettoyage des SMP */
  for(int i=0;i<nb_themes;i++){

    cle = ftok(FICHIER_CLE,LETTRE_CODE+i);
    if (cle==-1){
      couleur(ROUGE);
      fprintf(stdout,"Pb création clé\n");
      couleur(REINIT);
      exit(-1);
    }
    tmp = shmget(cle,0,0);
    shmctl(tmp,IPC_RMID,NULL);
  }
  /* Nettoyage FM et SEM */
  cle = ftok(FICHIER_CLE,LETTRE_CODE);
  if (cle==-1){
    couleur(ROUGE);
    fprintf(stdout,"Pb creation cle");
    couleur(REINIT);
    exit(-1);
  }
  tmp = msgget(cle,0);
  msgctl(tmp,IPC_RMID,NULL);
  tmp = semget(cle,0,0);
  semctl(tmp,0,IPC_RMID,NULL);

  /* Nettoyage SMP file */

  cle = ftok(FICHIER_CLE,LETTRE_CODE-1);
  if (cle==-1){
    couleur(ROUGE);
    fprintf(stdout,"Problème création clé");
    couleur(REINIT);
    exit(-1);
  }
  int get;
  get = shmget(cle,0,0);
  shmctl(get,IPC_RMID,NULL);

  /* on tue les fils */
  for (int i = 0; i < cmpt; i++) {
    kill(fils[i],SIGUSR1);
  }

  /* on attends la mort des fils */
  int encore_fils=1;
  int cause_fin=0;
  int res_wait=0;
  while(encore_fils)
  {
    res_wait=waitpid(-1,&cause_fin,WUNTRACED|WCONTINUED);
    if((res_wait==-1) && (errno==ECHILD))
    encore_fils=0;
  }
  printf("BYE !\n");
  exit(-1);
}


/*
* A la reception d'un signal la fonction arrete le programme et appelle handler qui supprime les ipcs
*
*/
void arret(int s){
  couleur(ROUGE);
  fprintf(stdout,"Arret du programme !\n");
  couleur(REINIT);
  handler(s);
  exit(-1);
}

/*
* Fonction d'usage pour une utilisation facilitée du programme
*
*/
void usage(char *nom){
  couleur(ROUGE);
  fprintf (stdout, "\nUsage: %s nb_archivistes nb_themes\n",nom);
  fprintf (stdout, "Création de nb_archivistes et nb_themes ");
  fprintf (stdout, "(Entre 2 et 100)\n\n");
  couleur(REINIT);
  exit (-1);
}


/*
* Fonction principale
*
*/
int main(int argc, char *argv[], char **envp){
  /* Déclarations globales du programme */
  int i;
  int j;
  int y;
  int pid;
  int nb_archivistes;
  int nb_article = NB_ARTICLE; // On fixe a 100 le nb max d'articles
  int crea_alea;
  /* init pour fabrication de la clé */
  struct stat st;
  key_t cle;
  /* init pour SMP */
  int stock_SMP[nb_themes];
  int stock_SEM;
  char *stock_tab_SMP[nb_themes];
  /* init pour SEM*/
  int resultat;
  unsigned short test[nb_themes];
  int t;
  int file_mess;
  int nb_journalistes=10000; // On fixe a 10000 le nb de journalistes
  char argv_stru[20][40];
  char tmp1[50];
  int SMP_FILE;
  char tmp[200];
  int theme_alea;
  char txt[5];
  int numero_article;
  cmpt = 0;
  /* TEST si il manque des arguments, ajouts éventuels*/
  switch (argc) {
    case 1:
    nb_themes=2;
    nb_archivistes=2;
    break;
    case 2:
    nb_themes=2;
    nb_archivistes = atoi(argv[1]);
    break;
    case 3:
    nb_archivistes = atoi(argv[1]);
    nb_themes = atoi(argv[2]);
    break;
    default:
    usage(argv[0]);
  }

  /* TEST si nb_archivistes et nb_themes sont bien compris entre 2 et 100 */
  if(nb_archivistes < 2 || nb_archivistes > 100 || nb_themes < 2 || nb_themes > 100){
    couleur(ROUGE);
    fprintf(stdout,"Le nombre d'archivistes et/ou de themes doit être compris entre 2 et 100...BYE !\n");
    couleur(REINIT);
    exit(-1);
  }
  else{

    /* On va créer nb_themes SMP
    * Avec une nouvelle clé à chaque fois */

    for(i=0;i<nb_themes;i++){

      /* Création de la clé */
      /* On teste si le fichier cle existe dans le repertoire courant */
      /* Fabrication du fichier cle s'il n'existe pas */
      if ((stat(FICHIER_CLE,&st) == -1) &&
      (open(FICHIER_CLE,O_RDONLY|O_CREAT|O_EXCL,0660) == -1)){
        couleur(ROUGE);
        fprintf(stdout,"Pb creation fichier cle\n");
        couleur(REINIT);
        exit(-1);
      }

      cle = ftok(FICHIER_CLE,LETTRE_CODE+i);
      if (cle==-1){
        couleur(ROUGE);
        pfprintf(stdout,"Problème création clé");
        couleur(REINIT);
        //supprimer les précédents
        for(j=0;j<i;j++){
          shmctl(stock_SMP[j],IPC_RMID,NULL);
        }
        exit(-1);
      }

      /* Création SMP */
      /* On crée le SMP et on teste s'il existe déjà */
      stock_SMP[i] = shmget(cle,400*sizeof(char),IPC_CREAT | IPC_EXCL | 0660);

      if (stock_SMP[i]==-1){
        couleur(ROUGE);
        fprintf(stdout,"Pb creation SMP ou il existe déjà\n");
        couleur(REINIT);
        //supprimer les précédents
        for(j=0;j<i;j++){
          shmctl(stock_SMP[j],IPC_RMID,NULL);
        }
        exit(-1);
      }

      /* Attachement SMP */
      stock_tab_SMP[i] = shmat(stock_SMP[i],NULL,0);
      if (stock_tab_SMP[i]==(char *)-1){
        couleur(ROUGE);
        fprintf(stdout,"Pb attachement SMP\n");
        couleur(REINIT);
        exit(-1);
      }

      /* FIN création SMP */

      /* init de chaque thème */
      for(y=0;y<400;y++){
        stock_tab_SMP[i][y]='a';
      }

    }
    /* FIN créationS SMP (fin boucle for) */



    /* Création AUTRE SMP pour le nombre de messages dans la file de chaque archivistes */

    cle = ftok(FICHIER_CLE,LETTRE_CODE-1);
    if (cle==-1){
      couleur(ROUGE);
      fprintf(stdout,"Problème création clé");
      couleur(REINIT);
      exit(-1);
    }

    /* Création SMP */
    /* On crée le SMP et on teste s'il existe déjà */
    SMP_FILE = shmget(cle,nb_archivistes*sizeof(int),IPC_CREAT | IPC_EXCL | 0660);

    if (SMP_FILE==-1){
      couleur(ROUGE);
      fprintf(stdout,"Pb creation SMP ou il existe déjà\n");
      couleur(REINIT);
      handler(0);
      exit(-1);
    }

    /* Attachement SMP */
    int *attache;
    attache = shmat(SMP_FILE,NULL,0);
    if (attache==NULL){
      couleur(ROUGE);
      fprintf(stdout,"Pb attachement SMP\n");
      couleur(REINIT);
      exit(-1);
    }

    for(i=0;i<nb_archivistes;i++){
      attache[i]=0;
    }

    /* FIN AUTRE SMP */

    /* Création ensemble de sémaphores */

    cle = ftok(FICHIER_CLE,LETTRE_CODE);

    stock_SEM = semget(cle,sizeof(char),IPC_CREAT | IPC_EXCL | 0660);
    if (stock_SEM==-1){
      couleur(ROUGE);
      fprintf(stdout,"Prob création semaphore ou il existe deja\n");
      couleur(REINIT);
      /* Il faut detruire le SMP puisqu'on l'a cree : */
      for (i = 0; i < nb_themes; i++) {
        shmctl(stock_SMP[i],IPC_RMID,NULL);
      }
      exit(-1);
    }


    /* On l'initialise */

    for(t=0;t<nb_themes;t++){
      test[t]=0;
    }
    resultat = semctl(stock_SEM,1,SETALL,test);
    if (resultat==-1){
      couleur(ROUGE);
      fprintf(stdout,"Pb initialisation sémaphore");
      couleur(REINIT);
      /* On detruit les IPC déjà créées : */
      semctl(stock_SEM,1,IPC_RMID,NULL);
      for (i = 0; i < nb_themes; i++) {
        shmctl(stock_SMP[i],IPC_RMID,NULL);
      }
      exit(-1);
    }

    /* FIN création ensemble de sémaphores */

    /* Création FM */

    /* nouvelle clé pour la FM */
    cle = ftok(FICHIER_CLE,LETTRE_CODE);
    if (cle==-1){
      couleur(ROUGE);
      fprintf(stdout,"Pb creation cle");
      couleur(REINIT);
      exit(-1);
    }

    file_mess = msgget(cle,IPC_CREAT | IPC_EXCL | 0660);
    if (file_mess==-1){
      couleur(ROUGE);
      fprintf(stdout,"Pb création file de message\n");
      couleur(REINIT);
      exit(-1);
    }

    /* Fin création FM */


    /* TEST DES AFFICHAGES */


    /* Fin test */


    /* Capture des signaux */

    mon_sigaction(SIGINT,arret);
    mon_sigaction(SIGQUIT,arret);
    mon_sigaction(SIGUSR1,arret);
    mon_sigaction(SIGUSR2,arret);
    mon_sigaction(SIGTERM,arret);

    /* Fin Capture des signaux */



    char *args[20] = {argv_stru[0],argv_stru[1],argv_stru[2],argv_stru[3],argv_stru[4],argv_stru[5],NULL};
    /* Création des archivistes */
    for(j=1;j<=nb_archivistes;j++){

      strcpy(argv_stru[0],"./Archivistes");
      sprintf(tmp1, "%d", j);
      strcpy(argv_stru[1],tmp1);
      sprintf(tmp1, "%d", nb_themes);
      strcpy(argv_stru[2],tmp1);
      pid = fork();
      if (pid==-1){
        break;
      }
      if (pid==0){
        couleur(VERT)
        fprintf(stdout,"Archiviste : %s %s %s\n", argv_stru[0],argv_stru[1],argv_stru[2]);
        couleur(REINIT);
        execve("./Archivistes", args, envp);
        // supprimer tout
        exit(-1);
      }
      sleep(1);
      fils[cmpt] = pid;
      cmpt++;
    }


    /* Création des journalistes */
    strcpy(argv_stru[0],"./Journalistes");

    srand(getpid());

    for(;;){
      if(nb_journalistes>9999 && nb_journalistes<20001){

        crea_alea = rand()%10+1;
        theme_alea = rand()%nb_themes;
        numero_article = rand()%nb_article; // On fixe a 100 le nb max d'articles
        if(crea_alea == 1){
          /*Création d'un journaliste avec une demande d'effacement d'archive */

          sprintf(tmp, "%d", nb_journalistes);
          strcpy(argv_stru[1],tmp);

          sprintf(tmp, "%d", nb_archivistes);
          strcpy(argv_stru[2],tmp);

          sprintf(tmp, "E");
          strcpy(argv_stru[3],tmp);

          sprintf(tmp, "%d", theme_alea);
          strcpy(argv_stru[4],tmp);

          sprintf(tmp, "%d", numero_article);
          strcpy(argv_stru[5],tmp);

        }
        else if(crea_alea < 4 && crea_alea > 1){
          /* Création d'un journaliste avec une demande de publication d'archive */

          sprintf(tmp, "%d", nb_journalistes);
          strcpy(argv_stru[1],tmp);

          sprintf(tmp, "%d", nb_archivistes);
          strcpy(argv_stru[2],tmp);

          sprintf(tmp, "P");
          strcpy(argv_stru[3],tmp);

          sprintf(tmp, "%d", theme_alea);
          strcpy(argv_stru[4],tmp);

          txt[0]=rand()%(90-65)+65;
          txt[1]=rand()%(122-97)+97;
          txt[2]=rand()%(122-97)+97;
          txt[3]=rand()%(122-97)+97;
          txt[4]='\0';
          sprintf(tmp, "%s", txt);
          strcpy(argv_stru[5],tmp);
        }
        else{
          /* Création d'un journaliste avec une demande de consultation d'archive */
          sprintf(tmp, "%d", nb_journalistes);
          strcpy(argv_stru[1],tmp);

          sprintf(tmp, "%d", nb_archivistes);
          strcpy(argv_stru[2],tmp);

          sprintf(tmp, "C");
          strcpy(argv_stru[3],tmp);

          sprintf(tmp, "%d", theme_alea);
          strcpy(argv_stru[4],tmp);

          sprintf(tmp, "%d", numero_article);
          strcpy(argv_stru[5],tmp);
        }
      }
      else{
        break;
      }
      couleur(VERT);
      fprintf(stdout,"Journaliste : %s %s %s %s %s %s\n", argv_stru[0],argv_stru[1],argv_stru[2],argv_stru[3],argv_stru[4],argv_stru[5]);
      couleur(REINIT);
      nb_journalistes++;
      pid = fork();
      if (pid==-1){
        // clean
        exit(-1);
      }
      else if (pid==0){
        execve("./Journalistes", args, envp);
        exit(-1);
      }
      sleep(1);
    }
    /* En principe jamais atteint, mais au cas ou je nettoie avec handler*/
    handler(0);
    exit(0);
  }
}
