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

 Si un slave et un master sont connecté, qu'un autre master se connecte
 que juste ensuite un slave se connecte, quel master va être choisis pour le
 slave ? A méditer
 */
#define MAX_FILE 50

#include <stdlib.h>     /* Pour EXIT_SUCCESS, malloc */
#include <stdio.h>      /* Pour fprintf */
#include <signal.h>     /* Pour sigaction */ 
#include <string.h>
#include <sys/socket.h>  /* Pour socket, bind */
#include <arpa/inet.h>   /* Pour sockaddr_in */
#include <unistd.h>      /* Pour close */
#include <errno.h>       /* Pour errno */
#include "structures.h" /* Pour les requetes */

int stop = 0;

void handler (int numSignal){
    stop = 1;
}

int main(int argc, char * argv[])
    {
    int sockfd;
    struct sockaddr_in adresseServeur;

    struct sockaddr_in adresseMaster;
    struct sockaddr_in adresseSlave;
    socklen_t tailleAddr = sizeof(struct sockaddr_in);

    struct sigaction action;
    int portTCPMaster=0;

    struct sockaddr_in fileAttente[MAX_FILE]; /* contient toutes les adresses de tous les slaves ayant tenté une connexion avant un MASTER */
    int cpt=0;      /* compteur du nombre de requete en attente */
    int cptSend=0;  /* indice du tableau sur lequel faire l'envoie : premier arrivé premier servis */

    requete_t requete;
    requete_envAMS_t reqToSlave;

    if(argc!=2) {
        fprintf(stderr,"Utilisation : %d paramètre(s)\n",argc);
        fprintf(stderr,"\tOu : \n");
        fprintf(stderr,"\t\t- port : port d'écoute du serveur\n");
        exit(EXIT_FAILURE);
    }
    /* arguments OK */
    /* Creation de la socket */
    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("Erreur lors de la creation de la socket ");
        exit(EXIT_FAILURE);
    }

    memset(&adresseServeur, 0, sizeof(struct sockaddr_in));
    adresseServeur.sin_family = AF_INET;
    adresseServeur.sin_port = htons(atoi(argv[1]));
    adresseServeur.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Nommage de la socket */
    if(bind(sockfd, (struct sockaddr*)&adresseServeur, sizeof(struct sockaddr_in)) == -1) {
        perror("Erreur lors du nommage de la socket ");
        exit(EXIT_FAILURE);
    }

    listen(sockfd,10);
    /*On positionne un gestionnaire sur SIGINT*/
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if(sigaction(SIGINT, &action, NULL) == -1){
        perror("Erreur lors du postionnement !");
        exit(EXIT_FAILURE);
    }

    /* boucle principale */
    while(!stop) {
        printf("En attente de requete...\n");
        /* attente de requête(s) */
        if(portTCPMaster==0) {
            if(recvfrom(sockfd, &requete, sizeof(requete_t), 0, (struct sockaddr*)&adresseMaster, &tailleAddr) == -1) {
                if(errno!=EINTR){
                    perror("Erreur lors de la reception des infos du SLAVE ");
                    exit(EXIT_FAILURE);
                }
            }
        }else {
            if(recvfrom(sockfd, &requete, sizeof(requete_t), 0, (struct sockaddr*)&adresseSlave,&tailleAddr) == -1) {
                perror("Erreur lors de la reception des infos du SLAVE ");
                exit(EXIT_FAILURE);
            }
        }
        switch(requete.type) {
            case 1:
                /* Connexion MASTER Serveur */
                portTCPMaster = requete.req.r1.portTCP;
                printf("Le MASTER se connecte et utilise le portTCP %d.\n",portTCPMaster);
                /* test si des slaves ont essayé de se co avant lui */
                if(inet_ntop(AF_INET, &adresseMaster.sin_addr,reqToSlave.adresse, INET_ADDRSTRLEN) == NULL) {
                    fprintf(stderr,"Conversion de l'adresse impossible\n");
                }
                reqToSlave.numPort = portTCPMaster;
                if(cpt!=0) {
                    printf("Envoie des infos du MASTER au SLAVE en attente...\n");
                    /* envoie des réponses au slaves avec les infos du MASTER */
                    if(sendto(sockfd, &reqToSlave, sizeof(requete_envAMS_t), 0, (struct sockaddr*)&fileAttente[cptSend], sizeof(struct sockaddr_in)) == -1) {
                        perror("Erreur lors de l'envoi des infos du MASTER ");
                        exit(EXIT_FAILURE);
                    }
                    cptSend++;
                }
                break;
            case 2:
                if(portTCPMaster!=0) {
                    /* master est co */
                    printf("Demande de connexion d'un SLAVE : envoie des infos du MASTER.\n");
                    if(inet_ntop(AF_INET, &adresseMaster.sin_addr,reqToSlave.adresse, INET_ADDRSTRLEN) == NULL) {
                        fprintf(stderr,"Conversion de l'adresse impossible\n");
                    }
                    reqToSlave.numPort = portTCPMaster;
                    if(sendto(sockfd, &reqToSlave, sizeof(requete_envAMS_t), 0, (struct sockaddr*)&adresseSlave, sizeof(struct sockaddr_in)) == -1) {
                        perror("Erreur lors de l'envoi des infos du MASTER ");
                        exit(EXIT_FAILURE);
                    }
                }else {
                    /* master pas co  : pas bien*/
                    portTCPMaster=0;
                    printf("Demande de connexion d'un SLAVE : aucun MASTER connecté. Stockage dans la file d'attente.\n");
                    fileAttente[cpt++]=adresseMaster; /* adresseMaster car ici adresseMaster == adresseSlave */
                }

                break;
            default:
                /* type non gérer : renvoie erreur ou rien ?*/
                break;
        }
    }
    printf("Fermeture du serveur et de la socket.\n");
    close(sockfd);
    return EXIT_SUCCESS;
    }
