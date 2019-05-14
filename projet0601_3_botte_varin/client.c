/**
 * Fichier client qui va gérer le client de jeu de l'utilisateur
 * @author BOTTE Florian & VARIN Alexandre
 * @version 16/03/2019
 **/
/*

 * MASTER SI OK :
    - argv[2] = adresse IP du serveur
    - argv[3] = numéro de port du serveur
    - argv[4] = numéro du port TCP sur lequel il va attendre une connexion d'un client
    - argv[5] = nom du fichier

 * SLAVE SI OK :
    - argv[2] = adresse IP du serveur
    - argv[3] = numéro de port du serveur

 * CARTE (fichier binaire) :
    - largeur  (unsigned  char)
    - hauteur  (unsigned  char)
    - cases (unsigned char x largeur x hauteur) 
    (0 pour vide 1 pour obstacle 2 pour lemmings allié 3 pour lemmings ennemis)

 Ajouter l'affichage du timer pour plus de compréhension 

 - placement lemmings OK
 - recharge de la carte par le slave envoyée par le master OK
 - envoyer la demande de placement d'un lemming slave sur la carte au master OK
 - la légende fait de la merde sur le slave (à voir sur le MASTER <- non) OK ? --> sinn methode majWindowJeu
 - gerer le fait de choisir le numéro du lemmings à placé OK
 - gérer le clique sur la légende dans master et slave (bouton mal placé a voir pour mieux gérer) OK
 - lemming placé : OK (normalement)
        - modification d'un tableau qui sera le tableau des lemming placé ou non (0 pour non placé 1 sinon)
        - modification de la fonction cliqueNumLemming qui affichera seulement les lemming non placé

 - faire le déplacement aléatoires des lemmings NOK
 - ajouter les fonctionnalités voulues (retirer plateau, bloquer, exploser) --> Utiliser des touches différentes (F1,F2 etc)
 - Gérer la mise en pause du jeu ?? (touche du clavier)

 - URGENT : envoie du lemming ajouter à la fin du timer du SLAVE !!
 */
#include <signal.h> /* Pour sigaction */
#include <stdio.h> /* Pour printf */
#include <string.h> /* pour strcmp */
#include <sys/time.h> /* pour itimer */
#include <time.h>
#include <errno.h> /* Pour errno */
#include <fcntl.h>  /* Pour fnctl */
#include <stdlib.h>     /* Pour EXIT_SUCCESS, malloc */
#include <unistd.h>     /* Pour sleep */
#include "structures.h" /* Pour les requetes */
#include <sys/socket.h>  /* Pour socket, bind */
#include <arpa/inet.h>   /* Pour sockaddr_in */
#include "carte.h"
#include "ncurses.h"

#define POSX    20      /* Position horizontale de la fenêtre */
#define POSY    5       /* Position verticale de la fenêtre */

int stop = 0; /* necessite d'être en global pour être utilisé et modifié partout */

WINDOW* fenPrincipale;      /* Plateau du jeu : necessite d'être en global pour l'ajout d'un lemmings à la fin du timer */
WINDOW* legende;            /* Legende du jeu : necessite d'être en global pour modification à la fin du timer */
int nbLemmings;             /* Nombre de lemmings : necessite d'être en global pour être modifié pour le handler lors d'ajout à la fin de 10s */
requete_t etatJeu;          /* Etat du jeu : necessite d'être en global pour être modifer par le handler à chaque fin de timer (et donc d'ajout de lemmings) */
int compteurLemmingEtat=0;  /* Compteur des états : necessite d'être en global pour être connu par le handler ET le main (pour que l'ajout par le clique ou la fin de timer incrément la même variable) */
int sockTCPSlave=0;           /* Socket TCP du slave : necessite d'être en global pour être connue par le handler pour l'envoie de l'état du jeu */
int sockTCP=0;              /* Socket TCP du master pour le salve : necessite d'être en global pour être connue par le handler */
int lesLemmings[NBLEMMINGS]={0}; /* Tableau des lemmings placé : necessite d'être en global pour être connue par le handler pour la modification lors d'ajout de lemmingt à la fin du timer */
int numLemmingAPlacer=0; /* Numéro du lemming à placé : nécessite d'être en global pour être connue par le handler pour l'utilisation et la modification d'ajout de lemming à la fin du timer */

void handler (int numSignal,siginfo_t*info,void*rien){
    requete_envCMS_t carte;
    if(numSignal==SIGINT) {
        stop = 1;
        ncurses_stopper();
        exit(EXIT_SUCCESS);
    }
    else if(numSignal==SIGALRM && compteurLemmingEtat<5) {
        int isOk=1;
        int y,x;
        requete_Etat_t etatUnLemming;
        requete_t reponseTCP;
        int numRandom = 0;
        y = x = 0;
        srand(time(NULL));
        carte = *(requete_envCMS_t*)info->si_value.sival_ptr;

        while(isOk){
            numRandom = rand()%((carte.largeur)*(carte.hauteur));
            if(carte.cases[numRandom] == '0'){
                isOk=0;
                y=(numRandom/LARGEURC)==0?1:(numRandom/LARGEURC);
                x=(numRandom%LARGEURC)==0?LARGEURC:(numRandom%LARGEURC);

                /* ne marche pas */
                if(sockTCPSlave==0) {
                    /* on est en mode slave */
                    reponseTCP.type = TYPE_AL; /* ajout lemming */
                    reponseTCP.req.r5.numLemming = numLemmingAPlacer;
                    reponseTCP.req.r5.posX = y;
                    reponseTCP.req.r5.posY = x;

                    if(write(sockTCP,&reponseTCP,sizeof(requete_t)) == -1) {
                        ncurses_stopper();
                        perror("Erreur lors de l'envoie du nouveau lemming");
                        exit(EXIT_FAILURE);
                    }
                    reponseTCP.type = 0;

                }else {
                    /* on envoie les infos du jeu au slave */
                    etatUnLemming.valeur = VAL_LEMMING_PLATEAU;
                    etatUnLemming.posY = y;
                    etatUnLemming.posX = x;

                    /* on est en mode master */
                    etatJeu.req.r2.etatLemmings[compteurLemmingEtat++] = etatUnLemming;

                    if(write(sockTCPSlave,&etatJeu,sizeof(requete_t)) == -1) {
                        perror("Erreur lors de l'écriture de l'état du jeu ");
                        exit(EXIT_FAILURE);
                    }
                }

                /* on modifie le tableau */
                lesLemmings[numLemmingAPlacer-1]=TYPE_LEMMING_PLACER;
                /* simulation du clique pour retirer l'affichage de l'indice numLemmingAPlacer */
                cliqueNumLemming(legende,&numLemmingAPlacer,lesLemmings,BOUTON_PLUS_LEG);

                /* on ajoute la case à la carte */
                cliqueCarte(fenPrincipale,&nbLemmings,y,x,0);
                setNbLemmings(legende,nbLemmings);
            }
        }
    }
}

char * testArgs(int nbArg, char * valArg[]) {
    if(nbArg<=1) {
        fprintf(stderr,"Utilisation : %d paramètre(s)\n",nbArg);
        fprintf(stderr,"\tOu : \n");
        fprintf(stderr,"\tmode : \n");
        fprintf(stderr,"\t\tMASTER : nécessite :\n");
        fprintf(stderr,"\t\t\t- adresse IP du serveur\n");
        fprintf(stderr,"\t\t\t- numéro de port du serveur\n");
        fprintf(stderr,"\t\t\t- numéro de port TCP sur lequel attendre la connexion de l'adversaire\n");
        fprintf(stderr,"\t\t\t- nom du fichier de la carte\n");
        fprintf(stderr,"\t\tSLAVE : nécessite :\n");
        fprintf(stderr,"\t\t\t- adresse IP du serveur\n");
        fprintf(stderr,"\t\t\t- numéro de port du serveur\n");
        exit(EXIT_FAILURE);
    }

    if(strcmp(valArg[1],"MASTER")==0 && nbArg!=6) {
        fprintf(stderr,"Utilisation : %d paramètre(s)\n",nbArg);
        fprintf(stderr,"\tOu : \n");
        fprintf(stderr,"\tmode : \n");
        fprintf(stderr,"\t\tMASTER : nécessite :\n");
        fprintf(stderr,"\t\t\t- adresse IP du serveur\n");
        fprintf(stderr,"\t\t\t- numéro de port du serveur\n");
        fprintf(stderr,"\t\t\t- numéro de port TCP sur lequel attendre la connexion de l'adversaire\n");
        fprintf(stderr,"\t\t\t- nom du fichier de la carte\n");
        exit(EXIT_FAILURE);
    }
    else if(strcmp(valArg[1],"SLAVE")==0 && nbArg!=4) {
        fprintf(stderr,"Utilisation : %d paramètre(s)\n",nbArg);
        fprintf(stderr,"\tOu : \n");
        fprintf(stderr,"\tmode : \n");
        fprintf(stderr,"\t\tSLAVE : nécessite :\n");
        fprintf(stderr,"\t\t\t- adresse IP du serveur\n");
        fprintf(stderr,"\t\t\t- numéro de port du serveur\n");
        exit(EXIT_FAILURE);
    }
    return valArg[1];
}

void resetTimer(timer_t timerid, struct itimerspec its) {
    its.it_value.tv_sec = 10;
    its.it_interval.tv_sec = 10;
    if ((timer_settime(timerid, 0, &its, NULL)) == -1){
        perror("Erreur lors de l'armement du timer ");
        exit(EXIT_FAILURE);
    }
}
void setSockFlags(int fdSock, int myFlag) {
    int flags=0;

    /* récup des flags déjà présent sur la socket */
    if((flags = fcntl(fdSock, F_GETFL)) == -1) {
        perror("Erreur lors de la récupération des flags de la socket TCP ");
        exit(EXIT_FAILURE);
    }

    /* ajout du Flag à la socket */
    if(fcntl(fdSock,F_SETFL,flags | myFlag) == -1) {
        perror("Erreur lors du positionnement des flags sur la socket TCP ");
        exit(EXIT_FAILURE);
    }

}

int main(int argc, char * argv[])
    {

    /* phase de déclaration */
    requete_envCMS_t carte;
    char * mode;
    int sockUDP,fdCarte;
    int ch=0;

    requete_t requeteUDP;
    requete_t reponseTCP;

    int sourisX,sourisY; /* valeur connu lors d'un clique de souris */
    int limitX,limitY; /* limite du clique en X et Y */

    /*  0 = mode ajout de lemmings allié
        1 = mode ajout de lemmings ennmie
        2 = mode retirer du plateau
        3 = mode explosion
        4 = mode stopper le lemmings
    */
    int modeJeu;

    struct sockaddr_in adresseServeurUDP;
    struct sockaddr_in adresseMasterTCP;

    requete_envAMS_t infoMaster;

    requete_Etat_t etatUnLemming;

    struct sigaction action;
    struct itimerspec its;/*structure pour intervalle de temps entre chaque appel au signal*/

    timer_t timerid;
    union sigval val;
    struct sigevent event;


    /* phase d'initialisation */
    if(1) {
        etatJeu.type = TYPE_ETAT_JEU;

        val.sival_ptr=&carte;
    
        event.sigev_notify = SIGEV_SIGNAL; /*SIGEV_SIGNAL va notifier le process avec le signal renseigné dans signo*/
        event.sigev_signo = SIGALRM;
        event.sigev_value = val;
        

        /*  0 = mode ajout de lemmings allié */
        modeJeu = 0;

        nbLemmings = NBLEMMINGS;

        mode = testArgs(argc,argv);

    
        /* arguments OK */
        if((sockUDP = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
            perror("Erreur lors de la creation de la socket ");
            exit(EXIT_FAILURE);
        }

        if((sockTCP = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
            perror("Erreur lors de la creation de la socket ");
            exit(EXIT_FAILURE);
        }

        /*On positionne un gestionnaire sur SIGINT*/
        action.sa_sigaction = handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = SA_SIGINFO;
        if(sigaction(SIGINT, &action, NULL) == -1 || sigaction(SIGALRM, &action, NULL) == -1){
            perror("Erreur lors du positionnement ");
            exit(EXIT_FAILURE);
        }

        if ((timer_create(CLOCK_REALTIME, &event, &timerid)) == -1){
            perror("Erreur lors de la création du timer ");
            exit(EXIT_FAILURE);
        }


        memset(&adresseServeurUDP, 0, sizeof(struct sockaddr_in));
        adresseServeurUDP.sin_family = AF_INET;
        adresseServeurUDP.sin_port = htons(atoi(argv[3]));
        if(inet_pton(AF_INET, argv[2], &adresseServeurUDP.sin_addr) != 1) {
            perror("Erreur lors de la conversion de l'adresse ");
            exit(EXIT_FAILURE);
        }

        memset(&adresseMasterTCP, 0, sizeof(struct sockaddr_in));
        adresseMasterTCP.sin_family = AF_INET;
    }

    /* On est en mode Master */
    if(strcmp(mode,"MASTER")==0) {
        fdCarte = editCarte(argv[5]);
        carte = getStructByFd(fdCarte);

        printf("Mode MASTER activé : envoie des données au serveur.\n");

        adresseMasterTCP.sin_addr.s_addr = htonl(INADDR_ANY);
        adresseMasterTCP.sin_port = htons(atoi(argv[4]));

        /* envoie de l'info au serveur que le master se co */
        requeteUDP.type = TYPE_CMS;
        requeteUDP.req.r1.portTCP = atoi(argv[4]);
        if(sendto(sockUDP, &requeteUDP, sizeof(requete_t), 0, (struct sockaddr*)&adresseServeurUDP, sizeof(struct sockaddr_in)) == -1) {
            perror("Erreur lors de l'envoi de la requete de connexion ");
            exit(EXIT_FAILURE);
        }

        if(bind(sockTCP, (struct sockaddr*)&adresseMasterTCP, sizeof(struct sockaddr_in)) == -1) {
            perror("Erreur lors du nommage de la socket ");
            exit(EXIT_FAILURE);
        }
        /* Mise en mode passif de la socket */
        if(listen(sockTCP, 1) == -1) {
            perror("Erreur lors de la mise en modie passif ");
            exit(EXIT_FAILURE);
        }

        /* Attente d'une connexion */
        printf("Attente de connexion TCP...\n");
        if((sockTCPSlave = accept(sockTCP, NULL, NULL)) == -1) {
            perror("Erreur lors de la demande de connexion ");
            exit(EXIT_FAILURE);
        }else {
            printf("Connexion d'un SLAVE ! Envoie des infos de la carte.\n");
            if(write(sockTCPSlave, &carte.largeur, sizeof(unsigned char)) == -1) {
                perror("Erreur lors de l'envoi de la largeur de la carte ");
                exit(EXIT_FAILURE);
            }
            if(write(sockTCPSlave, &carte.hauteur, sizeof(unsigned char)) == -1) {
                perror("Erreur lors de l'envoi de la hauteur de la carte ");
                exit(EXIT_FAILURE);
            }
            printf("hauter : %d\n",carte.hauteur);
            if(write(sockTCPSlave, carte.cases, sizeof(unsigned char)*(carte.largeur*carte.hauteur)) == -1) {
                perror("Erreur lors de l'envoi des cases de la carte ");
                exit(EXIT_FAILURE);
            }
            printf("Carte envoyé ! Attente de la réponse.\n");

            if(read(sockTCPSlave,&reponseTCP,sizeof(requete_t)) == -1) {
                perror("Erreur lors de la lecture de la réponse du SLAVE ");
                exit(EXIT_FAILURE);
            }
            if(reponseTCP.type==TYPE_OKSM) printf("Réponse reçu ! Mise en place du jeu.\n");

            /* on set le timer */
            its.it_value.tv_sec = 10;
            its.it_value.tv_nsec = 0;
            its.it_interval.tv_sec = 10;
            its.it_interval.tv_nsec = 0;
            if ((timer_settime(timerid, 0, &its, NULL)) == -1){
                perror("Erreur lors de l'armement du timer ");
                exit(EXIT_FAILURE);
            }

            ncurses_initialiser();
            ncurses_souris();
            ncurses_couleurs();
            /* Vérification de la taille du terminale */
            if((COLS < POSX + LARGEURC) || (LINES < POSY + HAUTEURC)) {
                ncurses_stopper();
                fprintf(stderr, "Les dimensions du terminal sont insufisantes : l=%d,h=%d au lieu de l=%d,h=%d\n", COLS, LINES, POSX + LARGEURC, POSY + HAUTEURC);
                exit(EXIT_FAILURE);
            }

            fenPrincipale = newwin(carte.hauteur+1,carte.largeur+1,0,0);
            /* calcule des limites à ne pas dépasser */
            limitX = carte.largeur-1;
            limitY = carte.hauteur-1;

            remplireFenCarteStruct(fenPrincipale,&carte);
            box(fenPrincipale, 0, 0);
            wrefresh(fenPrincipale);

            legende = afficherLegende(nbLemmings);
            wrefresh(legende);
            cliqueNumLemming(legende,&numLemmingAPlacer,lesLemmings,0);

            setSockFlags(sockTCPSlave,SOCK_NONBLOCK);

            while(!stop) {
                if(nbLemmings!=0) {
                    if((ch = getch() == KEY_MOUSE) && (souris_getpos(&sourisX, &sourisY, NULL) == OK)){
                        /* clique sur carte (+ reset timer) */
                        modeJeu=0;
                        if(carte.cases[(sourisY*carte.largeur)+sourisX]=='1') continue;
                        if((sourisX >= 1) && (sourisX <= limitX) && (sourisY >= 1) && (sourisY <= limitY))   {
                            
                            etatUnLemming.valeur = VAL_LEMMING_PLATEAU;
                            etatUnLemming.posY = sourisY;
                            etatUnLemming.posX = sourisX;

                            etatJeu.req.r2.etatLemmings[compteurLemmingEtat++] = etatUnLemming;
                        
                            if(write(sockTCPSlave, &etatJeu,sizeof(requete_t)) == -1) {
                                ncurses_stopper();
                                perror("Erreur lors de l'écriture de l'état du jeu ");
                                exit(EXIT_FAILURE);
                            }
                            
                            /* on modifie le tableau */
                            lesLemmings[numLemmingAPlacer-1]=TYPE_LEMMING_PLACER;
                            /* simulation du clique pour retirer l'affichage de l'indice numLemmingAPlacer */
                            cliqueNumLemming(legende,&numLemmingAPlacer,lesLemmings,BOUTON_PLUS_LEG);

                            cliqueCarte(fenPrincipale,&nbLemmings,sourisY,sourisX,modeJeu);
                            resetTimer(timerid,its);
                            setNbLemmings(legende,nbLemmings);
                        }
                        if((sourisX >= LARGEURC+2) && (sourisX <= (LARGEURC+2)+8) && (sourisY >= 3) && (sourisY <= 6))   {
                            if(sourisX >= LARGEURC+2 && sourisX <= (LARGEURC+2)+4) cliqueNumLemming(legende,&numLemmingAPlacer,lesLemmings,BOUTON_PLUS_LEG);
                            else if(sourisX >= (LARGEURC+2)+4 && sourisX <= (LARGEURC+2)+8) cliqueNumLemming(legende,&numLemmingAPlacer,lesLemmings,BOUTON_MOINS_LEG);
                            }
                        /* bougerLemming(WINDOW * fen,requete_t etatJeu,requete_envCMS_t carte) */
                    }
                }

                /* on lit la case envoyé par le slave s'il y a lieu */
                if(read(sockTCPSlave,&reponseTCP,sizeof(requete_t)) == -1) {
                    if(errno!=EAGAIN && errno!=EWOULDBLOCK) {
                        ncurses_stopper();
                        perror("Erreur lors de la lecture des infos envoyé par le MASTER ");
                        exit(EXIT_FAILURE);
                    }
                }

                if(reponseTCP.type!=0) {
                    majWindowJeu(fenPrincipale,reponseTCP);
                    reponseTCP.type = 0;
                }
            
                /*majTimerLegende(legende,timerid);*/
                wrefresh(fenPrincipale);
                wrefresh(legende);
            }
            closeCarte(fdCarte);
            close(sockTCPSlave);
        }
        

    }
    /* On est en mode Slave */
    else if(strcmp(mode,"SLAVE")==0) {
        /* Le client est un SLAVE*/
        printf("Mode SLAVE activé.\n");
        requeteUDP.type = TYPE_CSS;
        /* Attente de la réponse pour avoir l'adresse d'un client MASTER */
        if(sendto(sockUDP, &requeteUDP, sizeof(requete_t), 0, (struct sockaddr*)&adresseServeurUDP, sizeof(struct sockaddr_in)) == -1) {
            perror("Erreur lors de l'envoi de la requete de connexion ");
            exit(EXIT_FAILURE);
        }

        printf("Attente de la récéption des infos du MASTER...\n");
        if(recvfrom(sockUDP, &infoMaster, sizeof(requete_envAMS_t), 0, NULL, NULL) == -1) {
            perror("Erreur lors de la reception des infos du MASTER ");
            exit(EXIT_FAILURE);
        }
        printf("Infos reçu ! Connexion en TCP au MASTER...\n");
        /* Connexion en mode connectés au MASTER */
        /* on crée l'adresse */
        adresseMasterTCP.sin_port = htons(infoMaster.numPort);
        if(inet_pton(AF_INET, infoMaster.adresse, &adresseMasterTCP.sin_addr) != 1) {
            perror("Erreur lors de la conversion de l'adresse ");
            exit(EXIT_FAILURE);
        }
        /* on se connecte au MASTER */
        if(connect(sockTCP, (struct sockaddr*)&adresseMasterTCP, sizeof(adresseMasterTCP)) == -1) {
            perror("Erreur lors de la connexion ");
            exit(EXIT_FAILURE);
        }
        printf("Connexion réussis, attente de la réception de la carte...\n");
        /* on attend la carte */
        if(read(sockTCP, &carte.largeur, sizeof(unsigned char)) == -1) {
            perror("Erreur lors de la lecture de la largeur de la carte ");
            exit(EXIT_FAILURE);
        }
        if(read(sockTCP, &carte.hauteur, sizeof(unsigned char)) == -1) {
            perror("Erreur lors de la lecture de la hauteur de la carte ");
            exit(EXIT_FAILURE);
        }
        carte.cases = (unsigned char *)malloc(sizeof(unsigned char)*(carte.largeur*carte.hauteur));

        if(read(sockTCP, carte.cases, sizeof(unsigned char)*(carte.largeur*carte.hauteur)) == -1) {
            perror("Erreur lors de la lecture des cases de la carte ");
            exit(EXIT_FAILURE);
        }
        reponseTCP.type = TYPE_OKSM;
        if(write(sockTCP, &reponseTCP, sizeof(requete_t)) == -1) {
            perror("Erreur lors de l'envoie du ACK pour la carte ");
            exit(EXIT_FAILURE);
        }
    
        printf("Carte reçu en intégralité.\n");
        ncurses_initialiser();
        ncurses_souris();
        ncurses_couleurs();

        /* Vérification de la taille du terminale */
        if((COLS < POSX + LARGEURC) || (LINES < POSY + HAUTEURC)) {
            ncurses_stopper();
            fprintf(stderr, "Les dimensions du terminal sont insufisantes : l=%d,h=%d au lieu de l=%d,h=%d\n", COLS, LINES, POSX + LARGEURC, POSY + HAUTEURC);
            exit(EXIT_FAILURE);
        }

        /* on set le timer */
        its.it_value.tv_sec = 10;
        its.it_value.tv_nsec = 0;
        its.it_interval.tv_sec = 10;
        its.it_interval.tv_nsec = 0;
        if ((timer_settime(timerid, 0, &its, NULL)) == -1){
            perror("Erreur lors de l'armement du timer ");
            exit(EXIT_FAILURE);
        }

        fenPrincipale = newwin(carte.hauteur+1,carte.largeur+1,0,0);

        /* calcule des limites à ne pas dépasser */
        limitX = carte.largeur-1; /* car on compte le 0 */
        limitY = carte.hauteur-1; /* car on compte le 0 */

        remplireFenCarteStruct(fenPrincipale,&carte);
        box(fenPrincipale, 0, 0);
        wrefresh(fenPrincipale);

        legende = afficherLegende(nbLemmings);
        wrefresh(legende);

        /* positionnement de la socketTCP vers le master en non bloquante (pour éviter l'arrêt du prog */
        setSockFlags(sockTCP,SOCK_NONBLOCK);

        cliqueNumLemming(legende,&numLemmingAPlacer,lesLemmings,0);
        while(!stop) {
            if(nbLemmings!=0) {
                if((ch = getch() == KEY_MOUSE) && (souris_getpos(&sourisX, &sourisY, NULL) == OK)){
                    /*clique sur carte (+ reset timer) */
                    modeJeu=0;
                    if(carte.cases[(sourisY*carte.largeur)+sourisX]=='1') continue;
                    if((sourisX >= 1) && (sourisX <= limitX) && (sourisY >= 1) && (sourisY <= limitY))   {

                        reponseTCP.type = TYPE_AL; /* ajout lemming */
                        reponseTCP.req.r5.numLemming = numLemmingAPlacer;
                        reponseTCP.req.r5.posX = sourisX;
                        reponseTCP.req.r5.posY = sourisY;

                        if(write(sockTCP,&reponseTCP,sizeof(requete_t)) == -1) {
                            ncurses_stopper();
                            perror("Erreur lors de l'envoie du nouveau lemming");
                            exit(EXIT_FAILURE);
                        }
                        reponseTCP.type = 0;

                        /* on modifie le tableau */
                        lesLemmings[numLemmingAPlacer-1]=TYPE_LEMMING_PLACER;
                        /* simulation du clique pour retirer l'affichage de l'indice numLemmingAPlacer */
                        cliqueNumLemming(legende,&numLemmingAPlacer,lesLemmings,BOUTON_PLUS_LEG);

                        /* envoyé le nouveau lemming au master */
                        cliqueCarte(fenPrincipale,&nbLemmings,sourisY,sourisX,modeJeu);
                        resetTimer(timerid,its);
                        setNbLemmings(legende,nbLemmings);
                    }
                if((sourisX >= LARGEURC+2) && (sourisX <= (LARGEURC+2)+8) && (sourisY >= 3) && (sourisY <= 6))   {
                    if(sourisX >= LARGEURC+2 && sourisX <= (LARGEURC+2)+4) cliqueNumLemming(legende,&numLemmingAPlacer,lesLemmings,0);
                    else if(sourisX >= (LARGEURC+2)+4 && sourisX <= (LARGEURC+2)+8) cliqueNumLemming(legende,&numLemmingAPlacer,lesLemmings,1);
                    }
                }
            }

            if(read(sockTCP, &reponseTCP, sizeof(requete_t)) == -1) {
                if(errno==EAGAIN || errno==EWOULDBLOCK) {
                    continue;
                }
                ncurses_stopper();
                perror("Erreur lors de la lecture des infos envoyé par le MASTER ");
                exit(EXIT_FAILURE);
            }


            if(reponseTCP.type!=0) {
                majWindowJeu(fenPrincipale,reponseTCP);
                reponseTCP.type = 0;
            }

            /*majTimerLegende(legende,timerid);*/
            wrefresh(fenPrincipale);
            wrefresh(legende);
        }


    }


    delwin(fenPrincipale);
    delwin(legende);
    close(sockTCP);
    close(sockUDP);

    ncurses_stopper();
    return EXIT_SUCCESS;
    }
