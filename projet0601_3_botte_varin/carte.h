/**
 * Classe carte qui créer un labyrinthe de 30 cases par 15 cases
 * L'entré est situé sur la case (0,8) 
 * La sortie est situé sur la case (29,8)
 * Premier clique = ajout mur invisible
 * Deuxième clique = ajout mur visible
 * Troisième clique = suppression mur
 * Choisir nombre de vies avec "+" et "-"
 *
 * @author Alexandre Varin & Florian Botte
 * @version 17/01/2019
 **/
#ifndef CARTE_H
#define CARTE_H

#include "ncurses.h"
#include <ncurses.h>    /* Pour printw, attron, attroff, COLOR_PAIR, getch */

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

/* crée la fenètre de la carte */
WINDOW * createWindowJeu(WINDOW *,int,int);

/* met à jour l'affichage du plateau */
void majWindowJeu(WINDOW *,requete_t);

/*affichage de la légende*/
WINDOW* afficherLegende(int);

/* gère le clique sur les boutons + et - du numéro du lemming */
void cliqueNumLemming(WINDOW * ,int * ,int *,int );

/* modifier l'affichage du nbr de lemmings */
void setNbLemmings(WINDOW*, int);

/* ajouter un lemmings sur la fenêtre passé en paramètre */
void addLemings(WINDOW*,int,int,int);

/* gére le clique sur la carte */
void cliqueCarte(WINDOW*,int*,int,int,int);

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