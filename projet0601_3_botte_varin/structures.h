#ifndef _STRUCTURES_H
#define _STRUCTURES_H

/****************** Constante à modifier pour modifier le jeu ******************/

#define LARGEURC    50 /* Correspond àcases la largeur max de la carte <--- Arbitraire */
#define HAUTEURC    25 /* Correspond à la hauteur max de la carte <--- Arbitraire */
#define NBLEMMINGS  5 /* Correspond au nombre de lemmings max <--- Arbitraire */

/********************************************************************************/

#define TYPE_CMS        1 /*Connexion client en mode MASTER*/
#define TYPE_CSS        2 /*Connexion client en mode SLAVE*/
#define TYPE_ENVCMS     3 /*MASTER envoie la carte au SLAVE*/
#define TYPE_OKSM       4 /*Réponse OK du SLAVE au MASTER*/
#define TYPE_ETAT_JEU   5 /*MASTER envoie etat jeu au SLAVE*/
#define TYPE_MAMUS      6 /*MASTER annonce mort user au SLAVE*/
#define TYPE_MAMPS      7 /*MASTER annonce mise en pause au SLAVE*/
#define TYPE_MASPS      8 /*Master annonce sortie pause au SLAVE*/
#define TYPE_AL         9 /*Ajout lemming*/
#define TYPE_RL         10 /*Retrait lemming*/
#define TYPE_EL         11 /*Explosion lemming*/
#define TYPE_BL         12 /*Blocage lemming*/
#define TYPE_DMP        13 /*Demande mise en pause*/
#define TYPE_DSP        14 /*Demande sortie pause*/

#define VAL_LEMMING_RETIRE 0
#define VAL_LEMMING_PLATEAU 1
#define VAL_LEMMING_MORT 2
#define VAL_LEMMING_BLOQUE 3

#define VAL_LEMMING_ALLIE 0
#define VAL_LEMMING_ENNEMI 1

#define TYPE_LEMMING_NON_PLACER 0
#define TYPE_LEMMING_PLACER 1

typedef struct {
    unsigned short portTCP;
}requete_CMS_t;

typedef struct {
    char adresse[16];
    unsigned short numPort;
}requete_envAMS_t;/*Envoi de l'adresse du client MASTER au client SLAVE*/

typedef struct {
    unsigned char largeur;
    unsigned char hauteur;
    unsigned char *cases; /* malloc à faire avec largeur x hauteur */
}requete_envCMS_t;


typedef struct {
    unsigned char valeur; /* 0(retiré), valeur = 1 (sur le plateau), valeur = 2 (mort), valeur = 3 (bloqué) */
    unsigned char posX;
    unsigned char posY;
}requete_Etat_t;

typedef struct {
    requete_Etat_t etatLemmings[NBLEMMINGS*2];
}requete_EtatJeu_t;

typedef struct {
    unsigned char joueur;
}requete_MAMUS_t;

typedef struct {
    unsigned char joueur;
}requete_MAMPS_t;

typedef struct {
    unsigned char numLemming;
    unsigned char posX;
    unsigned char posY;
}requete_AL_t;

typedef struct {
    unsigned char numLemming;
}requete_RL_t;

typedef struct {
    unsigned char numLemming;
}requete_EL_t;

typedef struct {
    unsigned char numLemming;
}requete_BL_t;

typedef struct {
    unsigned char type;
    union {
        requete_CMS_t r1;       /* Connexion MASTER Serveur */
        requete_EtatJeu_t r2;   /*Envoi de l'état du jeu au SLAVE*/
        requete_MAMUS_t r3;     /*Le MASTER annonce la mort d'un user au SLAVE*/
        requete_MAMPS_t r4;     /*Le MASTER annonce la mise en pause du client SLAVE*/
        requete_AL_t r5;        /*Ajout d'un lemming par l'utilisateur*/
        requete_RL_t r6;        /*Retrait d'un lemming par l'utilisateur*/
        requete_EL_t r7;        /*Explosion d'un lemming*/
        requete_BL_t r8;       /*Blocage d'un lemming*/
    }req;
}requete_t;



#endif