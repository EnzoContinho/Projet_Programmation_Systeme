/*
 * Journaliste : Envoie des requetes aux archivistes et affiche ensuite la reponse de celui-ci.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

#include "types.h"

void usage(char *s){
    couleur(ROUGE);
    fprintf(stdout,"Usage : %s id_journaliste nb_archivistes action option_action1 option_action2\n\tid_journaliste : entier compris entre [10 000 et 20 000]\n\tnb_archivistes : entier nb_archivistes du processus initial\n\taction : char : -C : Consultation => option_action1 : entier theme concerne & option_action2 : entier numero de l'article\n\t                -P : Publication => option_action1 : entier theme concerne & option_action2 : char* texte de l'article\n\t                -E : Effacement => option_action1 : entier theme concerne & option_action2 : entier numero de l'article\n",s);
    couleur(REINIT);
    exit(-1);
}

/*renvoie 1 si le numero de journaliste est correct (compris entre [10 000 et 20 000])*/
int num_journaliste_correct(int num)
{
    if(num < 10000 || num > 20000)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int main (int argc, char *argv[]){
    key_t cle; /* cle de la file     */
    int file_mess; /* ID de la file */
    requete_t requete;
    reponse_t reponse;
    int id_journaliste; /*ID du journaliste*/
    int nb_archivistes; /*nombre d'archivistes*/
    char action;        /*Action à effectuer*/
    int mode;           /*Traduit l'action en int pour la requete*/
    int theme;          /*numero du theme*/
    int num_article;    /*numero de l'article*/ 
    char* texte;        /*texte à inserer*/
    int res_rcv;
    char* type_reponse;
    int i;
    int * tab_archiviste_court;
    int id_nb_message;
    int sauv_i;
    

    /* Recuperation des arguments */
    if(argc<6)
    {
        usage(argv[0]);
        exit(-1);
    }
    id_journaliste=atoi(argv[1]);
    nb_archivistes=atoi(argv[2]);
    action=argv[3][1];

    if(num_journaliste_correct(id_journaliste)==0)
    {
        usage(argv[0]);
        exit(-1);
    }
    if(action!='c' || action!='C'|| action!='p' || action!='P' || action!='e' || action!='E')
    {
        usage(argv[0]);
        exit(-1);
    }

    switch(action)
    {
    /*Cas d'une consultation*/
    case 'c':
    case 'C':
        /*le mode 0 veut maintenant dire que c'est une consultation*/
        mode=0;
        theme=atoi(argv[4]);
        num_article=atoi(argv[5]);
        break;
    /*Cas d'une publication*/
    case 'p':
    case 'P':
        /*le mode 1 veut maintenant dire que c'est un ajout*/
        mode=1;
        theme=atoi(argv[4]);
        texte=argv[5];
        break;
    /*Cas d'un effacement*/
    case 'e':
    case 'E':
        /*le mode 2 veut maintenant dire que c'est une suppression*/
        mode=2;
        theme=atoi(argv[4]);
        num_article=atoi(argv[5]);
        break;
        
    }
    

    /* Creation de la cle de la file de message:          */
    cle = ftok(FICHIER_CLE,LETTRE_CODE);
    if (cle==-1){
	fprintf(stderr,"Pb creation cle\n");
	exit(-1);
    }

    /* On recupere les IPC           */

    /* Recuperation file de message :               */
    file_mess=msgget(cle,0);

    /* Recuperation du SMP */
    id_nb_message=shmget(cle,0,0);

    tab_archiviste_court=shmat(id_nb_message,NULL,0);

    /*Test du meilleur archivistes*/
    sauv_i=0;
    for(i=0;i<nb_archivistes;i++)
    {
        /*Recuperation de l'archiviste dont la file d'attente est la plus courte*/
	if(tab_archiviste_court[i]==0)
	    {
		sauv_i=i;
		break;
	    }
	if(tab_archiviste_court[sauv_i]>tab_archiviste_court[i])
	    {
		sauv_i=i;
	    }
    }
    
     /* creation de la requete :                      */
    requete.type = sauv_i;
    requete.journaliste=id_journaliste;
    requete.mode=mode;
    requete.theme=theme;
    requete.num_article=num_article;
    requete.texte=texte;

    tab_archiviste_court[sauv_i]++;
    /* envoi de la requete :                            */
    msgsnd(file_mess,&requete,sizeof(requete_t),0);

    /* attente de la reponse :                        */
    res_rcv = msgrcv(file_mess,&reponse,sizeof(reponse_t),requete.journaliste,0);
    tab_archiviste_court[sauv_i]--;
    if (res_rcv ==-1){
	fprintf(stderr,"Erreur, numero %d\n",errno);
	exit(-1);
    }

    if(mode==0)
    {
        type_reponse="- Une consultation à une publication";
    }
    else if(mode==1)
    {
        type_reponse="- Ajout d'une publication";
    }
    else if(mode == 2)
    {
        type_reponse="- Supression d'une publication";
    }
    
    couleur(BLEU);
    /*Affichage de la requete*/
    fprintf(stdout,"Le journaliste %d envoie comme requete : %s de thème numéro  %d à l'article numéro %d\n", id_journaliste,type_reponse,theme, num_article);
    /*Affichage de la reponse*/
    fprintf(stdout, "\t\tLe journaliste %d recoit comme réponse : %s\n", id_journaliste, reponse.resu);
    couleur(REINIT);

    


    /* pour gcc */
    exit(0);
}



