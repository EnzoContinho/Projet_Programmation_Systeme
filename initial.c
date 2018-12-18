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
int nb_themes;


void handler(int j){
    for(int i=0;i<nb_themes;i++){
        key_t cle;
        int stock_SMP[nb_themes];
        cle = ftok(FICHIER_CLE,i);
        if (cle==-1){
            printf("Pb creation cle\n");
            //supprimer les précédents
            for(int j=0;j<i;j++){
                shmctl(stock_SMP[j],IPC_RMID,NULL);
            }
            exit(-1);
        }
        /* FIN création de la clé */
        /* ___________________________________________________________ */
        /* Création SMP : */
        /* On crée le SMP et on teste s'il existe déjà */
        stock_SMP[i] = shmget(cle,sizeof(int),0);

        if (stock_SMP[i]==-1){
            printf("Pb creation SMP ou il existe deja\n");
            //supprimer les précédents
            exit(-1);
        }
    }
}

/*A la reception d'un signal la fonction arrete le programme et supprime les ipcs */
void arret(int s){
    couleur(ROUGE);
    fprintf(stdout,"Arret du programme !!\n");
    handler(s);
    couleur(REINIT);
    msgctl(file_mess,IPC_RMID,NULL);
    exit(-1);
}

/* Fonction d'usage pour une utilisation facilitée du programme */
void usage(char *nom){
    couleur(ROUGE);
    fprintf (stdout, "\nUsage: %s nb_archivistes nb_themes\n",nom);
    fprintf (stdout, "Création de nb_archivistes et nb_themes ");
    fprintf (stdout, "(Entre 2 et 100)\n\n");
    couleur(REINIT);
    exit (-1);
}


/* Fonction principale */
int main(int argc, char *argv[], char **envp){
    int i;
    int j;
    int pid;
    int num_ordre;
    int nb_archivistes;
    char nom[7];
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

    int crea_alea;
    /* init pour fabrication de la clé */
    struct stat st;
    key_t cle;
    /* fin pour fabrication de la clé */
    /* init pour SMP */
    char** tab;
    int stock_SMP[nb_themes];
    /* fin init SMP */
    /* init pour SEM*/
    int semap,res_init;
    unsigned short val_init = 1;
    /* fin init pour sem*/

    /* TEST si le nb_archivistes et nb_themes sont bien compris entre 2 et 100 */
    if(nb_archivistes < 2 || nb_archivistes > 100 || nb_themes < 2 || nb_themes > 100){
        couleur(ROUGE);
        printf("Le nombre d'archivistes et/ou de themes doit être compris entre 2 et 100...BYE !\n");
        couleur(REINIT);
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
            sprintf(nom,"%d.serv",i);
            if ((stat(nom,&st) == -1) &&
                (open(nom,O_RDONLY|O_CREAT|O_EXCL,0660) == -1)){
                couleur(ROUGE);
                fprintf(stdout,"Pb creation fichier cle\n");
                couleur(REINIT);
                exit(-1);
            }
            printf("création de %s\n", nom);
            
            cle = ftok(nom,LETTRE_CODE);
            if (cle==-1){
                couleur(ROUGE);
                perror("Pb creation cle");
                couleur(REINIT);
                //supprimer les précédents
                for(int j=0;j<i;j++){
                    shmctl(stock_SMP[j],IPC_RMID,NULL);
                }
                exit(-1);
            }
            /* FIN création de la clé */
            /* ___________________________________________________________ */
            /* Création SMP : */
            /* On crée le SMP et on teste s'il existe déjà */
            stock_SMP[i] = shmget(cle,sizeof(int),IPC_CREAT | IPC_EXCL | 0660);

            if (stock_SMP[i]==-1){
                couleur(ROUGE);
                printf("Pb creation SMP ou il existe deja\n");
                couleur(REINIT);
                //supprimer les précédents
                for(int j=0;j<i;j++){
                    shmctl(stock_SMP[j],IPC_RMID,NULL);
                }
                exit(-1);
            }
            /* Attachement SMP :      */
            tab = shmat(stock_SMP[i],NULL,0);
            if (tab==(char **)-1)
            {
                couleur(ROUGE);
                printf("Pb attachement SMP\n");
                couleur(REINIT);
                exit(-1);
            }
            tab[i]="\0";
            /* FIN création SMP : */
        }
        /* FIN créationS SMP : */



        /* ___________________________________________________________ */

        /* Création ensemble de sémaphores */


        /* On cree le semaphore (meme cle) */
        semap = semget(cle,sizeof(char),IPC_CREAT | IPC_EXCL | 0660);
        if (semap==-1){
            couleur(ROUGE);
            printf("Prob creation semaphore ou il existe deja\n");
            couleur(REINIT);
            /* Il faut detruire le SMP puisqu'on l'a cree : */
            for (int i = 0; i < nb_themes; i++) {
                shmctl(stock_SMP[i],IPC_RMID,NULL);
            }
            /* Le detachement du SMP se fera a la terminaison */
            exit(-1);
        }

        /* On l'initialise */
        res_init = semctl(semap,1,SETALL,val_init);
        if (res_init==-1){
            couleur(ROUGE);
            printf("Pb initialisation semaphore\n");
            couleur(REINIT);
            /* On detruit les IPC deja créées : */
            semctl(semap,1,IPC_RMID,NULL);
            for (int i = 0; i < nb_themes; i++) {
                shmctl(stock_SMP[i],IPC_RMID,NULL);
            }
            exit(-1);
        }


        /* FIN création ensemble de sémaphores */

        /* ___________________________________________________________ */

        /* Création FM */

        file_mess = msgget(cle,IPC_CREAT | IPC_EXCL | 0660);
        if (file_mess==-1){
            couleur(ROUGE);
            fprintf(stderr,"Pb creation file de message\n");
            couleur(REINIT);
            exit(-1);
        }

        /* Fin création FM */

        /* ___________________________________________________________ */


        /* Capture des signaux */
 
        mon_sigaction(SIGINT,arret);
        mon_sigaction(SIGQUIT,arret);
        mon_sigaction(SIGUSR1,arret);
        mon_sigaction(SIGUSR2,arret);
        mon_sigaction(SIGTERM,arret);

        /* Fin Capture des signaux */




        /* Création des archivistes */
        for(j=0;j<nb_archivistes;j++){
            couleur(ROUGE);
            printf("Nb archivistes DANS boucle : %d\n", j);
            couleur(REINIT);
          

            srand(getpid());
            //num_ordre = rand()%(nb_archivistes)+1;

            char* argv_stru[6];
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
                // supprimer tout
                exit(-1);
            }
        }

        int nb_journalistes=10000;

        /* Création des journalistes */
        for(;;){
            if(nb_journalistes>9999 && nb_journalistes<10005){
                pid = fork();
                if (pid==-1){
                    // clean
                    break;
                }
                if (pid==0){

                    srand(getpid());
                    crea_alea = rand()%10+1;
                    couleur(BLEU);
                    printf("Nombre aléatoire : %d\n", crea_alea);
                    couleur(REINIT);
                    char* argv_stru[20];
                    char* tmp={"0"};
                    int theme_alea = rand()%nb_themes;
                    char txt[4];
                    int numero_article = rand()%100; // On fixe a 100 le nb max d'articles
                    if(crea_alea == 1){
                        /*Création d'un journaliste avec une demande d'effacement d'archive */
                        // manque argv[0] ./journaliste
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

        couleur(BLEU);
        printf("Compilation parfaite !\n");
        couleur(REINIT);
        /* Fermer SMP ??????? */

        /* On nettoie.............  */


        /* Un appel système de recouvrement doit toujours etre suivi par ... ? */

        exit(0);
    }
}
