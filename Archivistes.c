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
/*Pour strcpy*/
#include <string.h>
/*Pour les differentes structures*/
#include "Types.h"

int file_mess; /* ID de la file, necessairement global
		  pour pouvoir la supprimer a la terminaison */

void arret(int s){
    couleur(ROUGE);
    fprintf(stdout,"archiviste s'arrete (sigusr1 recu)\n");
    couleur(REINIT);
    msgctl(file_mess,IPC_RMID,NULL);
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
    char** tab;     /* l'entier de la SMP */
    int nb_lus;
    int num_archiv,nb_theme; /*numero d'archiviste & nombre de thème*/
    pid_t pid;
    int theme;
    /*Exclusion mutuelle*/
    struct sembuf P = {0,-1,SEM_UNDO};
    struct sembuf V = {0,1,SEM_UNDO};
    /*booléen de type int qui renvoie 1 s'il y a de la place, 0 sinon*/
    int place = 0;
    /*pour la boucle*/
    int i;
    

/* Recuperation des arguments                   */
    if(argc<3)
    {
        usage(argv[0]);
        exit(-1);
    }
    
    pid=getpid();
    num_archiv=atoi(argv[0]);
    nb_theme=atoi(argv[1]);
    /*Verification du nombre d'Archivistes*/
    if(num_archiv<1 || num_archiv>NB_ARCHIVISTES_MAX)
    {
        usage(argv[0]);
        exit(-1);
    }
    
    
   
    /* 1 - On teste si le fichier cle existe dans le repertoire courant : */
    fich_cle = fopen(FICHIER_CLE_FILE,"r");
    if (fich_cle==NULL)
    {
	fprintf(stderr,"Lancement client impossible\n");
	exit(-1);
    }
    
    cle = ftok(FICHIER_CLE_FILE,LETTRE_CODE);
    if (cle==-1)
    {
	fprintf(stderr,"Pb creation cle\n");
	exit(-1);
    }

/*Recuperation des differentes clés lié au theme*/ 
   
    /* On recupere les IPC           */
    
    /* Recuperation SMP :            */
    mem_part=shmget(cle, sizeof(int), 0);

    if(mem_part==-1)
    {
        fprintf(stderr,"(%d) Pb recuperation SMP\n",pid);
    }


    /* Attachement SMP :      */
    tab = shmat(mem_part,NULL,0);
    if (tab==(char **)-1)
    {
	printf("(%d) Pb attachement SMP\n",pid);
	exit(-1);
    }

    /* Recuperation semaphore :         */
    semap = semget(cle,1,0);
    if (semap==-1)
    {
	printf("(%d) Pb recuperation semaphore\n",pid);
	exit(-1);
    }
    
    /* Recuperation file de message :               */
    file_mess=msgget(cle,0);
    
    if (file_mess==-1)
    {
	printf("Pb creation FM ou il existe deja\n");
	exit(-1);
    }

    mon_sigaction(SIGUSR1,arret);
    mon_sigaction(SIGUSR2,arret);
    for(;;) /* Indefiniment :*/
        /* archiviste attend des requetes, de type (1 pour la consultation, 2 pour la publication) :        */
    {
        if  ((nb_lus=msgrcv(file_mess,&requete,sizeof(requete_t),1,0))==-1
             || (nb_lus=msgrcv(file_mess,&requete,sizeof(requete_t),2,0))==-1)
        {
	    fprintf(stderr,"Erreur de lecture, erreur %d\n",errno);
	    raise(SIGUSR1);
	}
        
	/* traitement de la requete :                      */
        couleur(ROUGE);
        /*CONSULTATION*/
        if(requete.type==1)
        {
           
            fprintf(stdout,"(Archiviste) Requete recue de %d\n\top Consultation du thème numéro : %d de l'article numero : %d \n",
                    requete.journaliste,
                    requete.theme,
                    requete.num_article);
            
            /*Type de reponse judicieusement choisi*/
            reponse.type=1;
            reponse.archiviste=num_archiv;
            reponse.resu="La consultation a bien été faite !\n";

        }
        /*PUBLICATION*/
        /*AJOUT*/
        else if(requete.type==2 && requete.mode==1)
        {
                     
            fprintf(stdout,"(Archiviste) Requete recue de %d\n\top Ajout dans le thème numéro : %d et du texte : %s \n",
                    requete.journaliste,
                    requete.theme,
                    requete.texte);

            semop(semap,&P,1);
            while(place==0)
            {
                if(tab[i]==NULL)
                {
                    place = 1;
                }
                else
                {
                    i++;
                }
            }
            strcpy(tab[i],requete.texte);
            semop(semap,&V,1);
            
            /*Type de reponse judicieusement choisi*/
            reponse.type=2;
            reponse.archiviste=num_archiv;
            reponse.resu="La publication a bien été ajouté !\n";
        }
        /*SUPPRESION*/
        else if(requete.type==2 && requete.mode==2)
        {
            
            fprintf(stdout,"(Archiviste) Requete recue de %d\n\top Suppression dans le thème numéro : %d et précisement de l'article : %d\n",
                    requete.journaliste,
                    requete.theme,
                    requete.num_article);

            semop(semap,&P,1);

            tab[requete.num_article]=NULL;           
            
            semop(semap,&V,1);

            /*Type de reponse judicieusement choisi*/
            reponse.type=2;
            reponse.archiviste=num_archiv;
            reponse.resu="La publication a bien été supprimé !\n";
        }
        couleur(REINIT);

        fprintf(stdout, "Je fais le travail demandé ...\n");
	/* Attente aleatoire */
	sleep(rand()%3);
        fprintf(stdout,"Travail effectué !\n");
	
        /* envoi de la reponse :                           */
        msgsnd(file_mess,&reponse,sizeof(reponse_t),0);
    }

    /*pour gcc*/
    exit(0);
}


