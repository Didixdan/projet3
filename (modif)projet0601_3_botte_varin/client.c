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
 *
 *
 Si slave se déconnecte, le master se remet en attente
 Si master se déconnecte, slave se déconnecte

 Gérer la taille de la fenêtre en fonction de LARGEURC et HAUTEURC
 */
#include <signal.h> /* Pour sigaction */
#include <stdio.h> /* Pour printf */
#include <string.h> /* pour strcmp */
#include <sys/time.h> /* pour itimer */
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

WINDOW* fenPrincipale;  /* Plateau du jeu : necessite d'être en global pour l'ajout d'un lemmings à la fin du timer */
int carteTaille=0;      /* Nombre de cases du plateau : necessite d'être en global pour être connu par le handler */

void handler (int numSignal,siginfo_t*info,void*rien){
    if(numSignal==SIGINT) {
        stop = 1;
        ncurses_stopper();
    }
    else if(numSignal==SIGVTALRM) {
        /* on pose un lemmings allié sur la carte */
        //srand(time(NULL));
        printf("Signal SIGVALRM reçu ...\n");
    }

    else if(numSignal==SIGALRM) {
        /* on pose un lemmings allié sur la carte */
        //srand(time(NULL));
        printf("Signal SIGVALRM reçu ...\n");
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

int main(int argc, char * argv[])
    {
    requete_envCMS_t carte;
    char * mode;
    int sockUDP,sockTCP,fdCarte;
    int sockClient;/*socket du slave*/
    int ch=0;

    requete_t requeteUDP;
    requete_t reponseTCP;

    int sourisX,sourisY; /* valeur connu lors d'un clique de souris */
    int limitX,limitY; /* limite du clique en X et Y */

    struct sockaddr_in adresseServeurUDP;
    struct sockaddr_in adresseMasterTCP;

    requete_envAMS_t infoMaster;

    struct sigaction action;
    struct itimerval timer;

    WINDOW* legende;

    /*  0 = mode ajout de lemmings allié
        1 = mode ajout de lemmings ennmie
        2 = mode retirer du plateau
        3 = mode explosion
        4 = mode stopper le lemmings
    */
    int modeJeu = 0;

    int nbLemmings = NBLEMMINGS;

    /* phase d'initialisation */
    if(true) {
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
        if(sigaction(SIGINT, &action, NULL) == -1 || sigaction(SIGVTALRM, &action, NULL) == -1){
            perror("Erreur lors du positionnement !");
            exit(EXIT_FAILURE);
        }

        /* Configuré le timer pour s'arrêter après 10sec... */
        timer.it_value.tv_sec = 10;
        timer.it_value.tv_usec = 0;
        /* ... et toutes les 10 secondes il recommence */
        timer.it_interval.tv_sec = 10;
        timer.it_interval.tv_usec = 0;

        memset(&adresseServeurUDP, 0, sizeof(struct sockaddr_in));
        adresseServeurUDP.sin_family = AF_INET;
        adresseServeurUDP.sin_port = htons(atoi(argv[3]));
        if(inet_pton(AF_INET, argv[2], &adresseServeurUDP.sin_addr) != 1) {
            perror("Erreur lors de la conversion de l'adresse ");
            exit(EXIT_FAILURE);
        }

        memset(&adresseMasterTCP, 0, sizeof(struct sockaddr_in));
        adresseMasterTCP.sin_family = AF_INET;


        /* Commencement du timer virtuel qui comptera peu importe la position dans le processus */
        setitimer (ITIMER_VIRTUAL, &timer, NULL);
    }

    /* On est en mode Master */
    if(strcmp(mode,"MASTER")==0) {
        fdCarte = editCarte(argv[5]);
        carte = getStructByFd(fdCarte);

        carteTaille = carte.hauteur*carte.largeur; /* MAJ de la taille pour le handler*/

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
        if((sockClient = accept(sockTCP, NULL, NULL)) == -1) {
            perror("Erreur lors de la demande de connexion ");
            exit(EXIT_FAILURE);
        }else {
            printf("Connexion d'un SLAVE ! Envoie des infos de la carte.\n");
            if(write(sockClient, &carte.largeur, sizeof(unsigned char)) == -1) {
                perror("Erreur lors de l'envoi de la largeur de la carte ");
                exit(EXIT_FAILURE);
            }
            if(write(sockClient, &carte.hauteur, sizeof(unsigned char)) == -1) {
                perror("Erreur lors de l'envoi de la hauteur de la carte ");
                exit(EXIT_FAILURE);
            }
            printf("hauter : %d\n",carte.hauteur);
            if(write(sockClient, carte.cases, sizeof(unsigned char)*(carte.largeur*carte.hauteur)) == -1) {
                perror("Erreur lors de l'envoi des cases de la carte ");
                exit(EXIT_FAILURE);
            }
            printf("Carte envoyé ! Attente de la réponse.\n");

            if(read(sockClient,&reponseTCP,sizeof(requete_t)) == -1) {
                perror("Erreur lors de la lecture de la réponse du SLAVE ");
                exit(EXIT_FAILURE);
            }
            if(reponseTCP.type==TYPE_OKSM) printf("Réponse reçu ! Mise en place du jeu.\n");

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
            limitX = carte.largeur;
            limitY = carte.hauteur;

            remplireFenCarteStruct(fenPrincipale,&carte);
            box(fenPrincipale, 0, 0);
            wrefresh(fenPrincipale);

            legende = afficherLegende(nbLemmings);
            wrefresh(legende);
    
            while(!stop) {
                ch = getch();
                if((ch == KEY_MOUSE) && (souris_getpos(&sourisX, &sourisY, NULL) == OK)){
                    /* clique sur carte (+ reset timer) */
                    modeJeu=0;
                    if((sourisX >= 1) && (sourisX <= limitX) && (sourisY >= 1) && (sourisY <= limitY))   {
                        cliqueCarte(fenPrincipale,&nbLemmings,sourisY,sourisX,modeJeu);
                        setNbLemmings(legende,nbLemmings);
                    }
                }
                wrefresh(fenPrincipale);
                wrefresh(legende);
            }

            delwin(fenPrincipale);
            delwin(legende);
            close(sockTCP);
            close(sockUDP);
            closeCarte(fdCarte);
            close(sockClient);
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

        fenPrincipale = newwin(carte.hauteur+1,carte.largeur+1,0,0);
        remplireFenCarteStruct(fenPrincipale,&carte);
        box(fenPrincipale, 0, 0);
        wrefresh(fenPrincipale);

        legende = afficherLegende(5);
        wrefresh(legende);

        while(!stop) {
            /* mettre la possibilité de cliquer ici */

            if(read(sockTCP, &reponseTCP, sizeof(requete_t)) == -1) {
                perror("Erreur lors de la lecture des cases de la carte ");
                exit(EXIT_FAILURE);
            }

            switch(reponseTCP.type) {
                case 5:
                    majWindowJeu(fenPrincipale,reponseTCP.req.r2);
                    break;
            }
        }

        delwin(fenPrincipale);
        delwin(legende);
        close(sockTCP);
        close(sockUDP);

    }

        /* on envoie le OK de la bonne réception de la carte */

    ncurses_stopper();
    return EXIT_SUCCESS;
    }
