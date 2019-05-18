/**
 * @author Florian Botte & Alexandre Varin
 * @version 17/01/2019
 **/
#ifndef CARTE_H
#define CARTE_H

#include "ncurses.h"
#include <ncurses.h>    /* Pour printw, attron, attroff, COLOR_PAIR, getch */
#include <errno.h> /* Pour errno */


#define BOUTON_PLUS_LEG 0
#define BOUTON_MOINS_LEG 1


/*renvoie la largeur de la carte*/
int getLargeurCarte(int);

/*renvoie la hauteur de la carte*/
int getHauteurCarte(int);

/* renvoie la struct carte de la carte fd */
requete_envCMS_t getStructByFd(int);

/* crée une carte aléatoire */
void creerCarteAleatoire();

/* crée un fichier carte */
void createCarte(int);

/* ouvre/créer une carte */
int editCarte(const char *);

/* connaitre le nombre de lemming de la carte passé en paramètre */
int nbLemmingEtatJeu(requete_EtatJeu_t carte);

/* crée la fenètre de la carte */
WINDOW * createWindowJeu(WINDOW *,int,int);

/* supprime tous les lemmings de la carte pour MAJ de l'affichage */
void suppAllLemmings(WINDOW * fen,requete_envCMS_t *carte);

/* ajouter un lemmings sur la fenêtre passé en paramètre */
void addLemings(WINDOW*,requete_envCMS_t *,int,int,int);

/* supprime un lemmings de la fenêtre passé en paramètre */
void suppLemmings(WINDOW * fen,requete_envCMS_t *carte,int y,int x);

/* met à jour l'affichage du plateau */
void majWindowJeu(WINDOW *,requete_envCMS_t *,requete_t,requete_t*);

/* renvoie une nouvelle carte correspondant à la copie champ à champ */
requete_envCMS_t copyCarte(requete_envCMS_t carte);

/* renvoie un nouvel etat de jeu correspondant à la copie champ à champ */
requete_EtatJeu_t copyEtatJeu(requete_EtatJeu_t etatJeu);

/* initialisation de l'état du jeu */
void initEtatJeu(requete_EtatJeu_t *etatJeu);

/*affichage de la légende*/
WINDOW* afficherLegende(WINDOW *,WINDOW *,int);

/* gère le clique sur les boutons + et - du numéro du lemming */
void cliqueNumLemming(WINDOW * ,int * ,int *,int );

/* modifier l'affichage du nbr de lemmings */
void setNbLemmings(WINDOW*, int);

/* gére le clique sur la carte */
void cliqueCarte(WINDOW*,requete_envCMS_t *,int*,int,int,int);

/*modification des valeurs des cases pour le déplacement des lemmings*/
void deplacementLemmings(WINDOW * ,requete_envCMS_t*, requete_t*,int);

/* remplie la fenètre de la carte avec la structure de la carte*/
void remplireFenCarteStruct(WINDOW *,requete_envCMS_t*);

/* trouve l'indice minimal du lemming du type par rapport au tableau passé en paramètre */
int indiceMinLemming(int *,int);

/* trouve l'indice maximal du lemming du type par rapport au tableau passé en paramètre */
int indiceMaxLemming(int *,int);

/* renvoie un entier vrai ou faux pour savoir s'il existe aucun lemming du type dans le tableau passé en param */
int aucunLemming(int *, int );

/* deplace le tête de lecture dans le fichier à l'endroit voulue (pour les cases) */
void deplaceInFileTo(int,int,int);

/*On met à jour la legende avec la valeur du timer*/
void majTimerLegende(WINDOW *, timer_t);

/* ferme le fichier de la carte */
void closeCarte(int);

#endif