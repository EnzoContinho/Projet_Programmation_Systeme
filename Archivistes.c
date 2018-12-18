/*
* Archiviste : reçoit 2 paramètres, son numéro d'ordre (un entier compris entre 1 et nb_archivistes)
* et le nombre de thèmes différents (c'est-à-dire le nb_themes du processus initial).
* Attend indéfiniment des requetes venant des journalistes et effectue l'action associée.
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <string.h>
/*Pour les differentes structures*/
#include "types.h"

int file_mess; /* ID de la file, necessairement global
pour pouvoir la supprimer a la terminaison */

void arret(int s){
  couleur(ROUGE);
  fprintf(stdout,"archiviste s'arrete\n");
  couleur(REINIT);
  exit(0);
}

void usage(char *s){
  couleur(ROUGE);
  fprintf(stdout,"Usage : %s Numero_archiviste Nb_thèmes\n\tNumero_archiviste : entiers compris entre 1 et nb_archivistes\n\top Nb_thèmes : nb_themes initial\n",s);
  couleur(REINIT);
  exit(-1);
}




int main (int argc, char *argv[]){

  key_t cle;     /* cle de la file     */
  FILE *fich_cle;
  requete_t requete;
  reponse_t reponse;
  int mem_part; /* ID du SMP    */
  int semap;    /* ID du semaphore    */
  int nb_lus;
  int num_archiv,nb_theme; /*numero d'archiviste & nombre de thème*/
  /*Exclusion mutuelle*/
  theme_t* tab_theme; /*Tableau de thème*/
  /*
  struct sembuf P = {0,-1,SEM_UNDO};
  struct sembuf V = {0,1,SEM_UNDO};
  */
  int i;  /*pour la boucle*/
  char article[5]; /*pour la recuperation de l'article*/


  /* Recuperation des arguments                   */
  if(argc<3)
  {
    usage(argv[0]);
    exit(-1);
  }

  num_archiv=atoi(argv[1]);
  nb_theme=atoi(argv[2]);
  /*Verification du numero d'Archivistes*/
  if(num_archiv<1 || num_archiv>NB_ARCHIVISTES_MAX)
  {
    usage(argv[0]);
    exit(-1);
  }


  /* 1 - On teste si le fichier cle existe dans le repertoire courant : */
  fich_cle = fopen(FICHIER_CLE,"r");
  if (fich_cle==NULL)
  {
    couleur(ROUGE);
    fprintf(stdout,"Lancement client impossible\n");
    couleur(REINIT);
    exit(-1);
  }

  cle = ftok(FICHIER_CLE,LETTRE_CODE);
  if (cle==-1)
  {
    couleur(ROUGE);
    fprintf(stdout,"Pb creation cle\n");
    couleur(REINIT);
    exit(-1);
  }

  /* On recupere les IPC           */

  /* Recuperation semaphore :         */
  semap = semget(cle,1,0);
  if (semap==-1)
  {
    couleur(ROUGE);
    printf("(%d) Pb recuperation semaphore\n",num_archiv);
    couleur(REINIT);
    exit(-1);
  }

  /* Recuperation file de message :               */
  file_mess=msgget(cle,0);

  if (file_mess==-1)
  {
    couleur(ROUGE);
    printf("(%d) Pb creation FM ou il existe deja\n", num_archiv);
    couleur(REINIT);
    exit(-1);
  }

  for(i=0;i<nb_theme;i++)
  {
    cle = ftok(FICHIER_CLE,LETTRE_CODE+i);

    /* Recuperation SMP :            */
    mem_part=shmget(cle, sizeof(int), 0);

    if(mem_part==-1)
    {
      couleur(ROUGE);
      fprintf(stdout,"(%d) Pb recuperation SMP\n",num_archiv);
      couleur(REINIT);
    }

    /* Attachement SMP :      */
    tab_theme = shmat(mem_part,NULL,0);
    if (tab_theme==NULL)
    {
      couleur(ROUGE);
      printf("(%d) Pb attachement SMP\n",num_archiv);
      couleur(REINIT);
      exit(-1);
    }
  }


  mon_sigaction(SIGUSR1,arret);
  mon_sigaction(SIGUSR2,arret);
  for(;;) /* Indefiniment : archiviste attend des requetes      */
  {
    /*Recuperation de la requete*/
    if  ((nb_lus=msgrcv(file_mess,&requete,sizeof(requete_t),num_archiv,0))==-1)
    {
      couleur(ROUGE);
      fprintf(stdout,"Erreur de lecture, erreur %d\n",errno);
      couleur(REINIT);
      raise(SIGUSR1);
    }

    /* traitement de la requete :                      */
    /*  couleur(BLEU);*/
    /*CONSULTATION*/
    fprintf(stderr, "%d\n", requete.mode);

    switch (requete.mode)
    {
      /*CONSULTATION*/
      case 0 :
      fprintf(stdout,"(Archiviste) Requete reçue de %d\n\tConsultation du thème numéro : %d de l'article numero : %d \n",
      requete.journaliste,
      requete.theme,
      requete.num_article);
      /*Recherche*/
      for(i=0;i<4;i++)
      {
        article[i]=tab_theme[requete.theme].articles[(4*requete.num_article)+i];
        fprintf(stderr, "%c\n",article[i]);
      }
      article[4]='\0';

      fprintf(stderr, "TEST !!\n");

      /*Type de reponse judicieusement choisi*/
      reponse.mode=0;
      reponse.archiviste=num_archiv;
      //sprintf(reponse.resu,"La consultation a bien été faite ! L'article n°%d du thème n°%d est : %s",requete.num_article,requete.theme,article);
      fprintf(stderr, "TEST2 %s !!\n", article);
      break;
      /*PUBLICATION*/
      /*AJOUT*/
      case 1:
      fprintf(stdout,"(Archiviste) Requete recue de %d\n\tAjout dans le thème numéro : %d et du texte : %s \n",
      requete.journaliste,
      requete.theme,
      requete.texte);

      /*A MODIFIER*/
      /*
      semop(semap,&P,1);
      */

      /*Ajout*/
      fprintf(stderr, "TESTE AJOUT\n");
      i=0;
      while(i<400)
      {
        if(tab_theme[requete.theme].articles[i]!='\0')
        {
          break;
        }
        else
        {
          i++;
        }
      }

      tab_theme[requete.theme].articles[i]=requete.texte[1];
      tab_theme[requete.theme].articles[i+1]=requete.texte[2];
      tab_theme[requete.theme].articles[i+2]=requete.texte[3];
      tab_theme[requete.theme].articles[i+3]=requete.texte[4];


      /*
      semop(semap,&V,1);
      */

      /*Type de reponse judicieusement choisi*/
      reponse.mode=1;
      reponse.archiviste=num_archiv;
      sprintf(reponse.resu,"La publication du texte %s dans le theme n°%d a bien été ajouté !\n",requete.texte,requete.theme);
      break;
      /*PUBLICATION*/
      /*SUPRESSION*/
      case 2:
      fprintf(stdout,"(Archiviste) Requete recue de %d\n\tSuppression dans le thème numéro : %d et précisement de l'article : %d\n",
      requete.journaliste,
      requete.theme,
      requete.num_article);

      /*
      semop(semap,&P,1);
      */

      /*Supression*/
      i=requete.num_article+4;
      while(i<400 && tab_theme[requete.theme].articles[i]!='\0')
      {
        tab_theme[requete.theme].articles[i-4]=tab_theme[requete.theme].articles[i];
        i++;
      }

      /*
      semop(semap,&V,1);
      */

      /*Type de reponse judicieusement choisi*/
      reponse.mode=2;
      reponse.archiviste=num_archiv;
      sprintf(reponse.resu,"La publication de l'article n°%d dans le theme n°%d a bien été suprimé !\n",requete.num_article,requete.theme);
      break;
    }

    /*  couleur(REINIT); */
    couleur(BLEU);
    fprintf(stdout, "Je fais le travail demandé ...\n");
    /* Attente aleatoire */
    sleep(rand()%3);
    fprintf(stdout,"Travail effectué !\n");
    couleur(REINIT);
    /* envoi de la reponse :                           */
    reponse.type=requete.journaliste;
    msgsnd(file_mess,&reponse,sizeof(reponse_t),0);
  } /*Fin de la boucle FOR*/

  /*pour gcc*/
  exit(0);
}
