#include "ncurses.h"

#include <ncurses.h>   /* Pour toutes les fonctions/constantes ncurses */
#include <stdlib.h>    /* Pour exit, EXIT_FAILURE */

/**
 * Initialisation de ncurses.
 */
void ncurses_initialiser() {
  initscr();	        /* Demarre le mode ncurses */
  cbreak();	        /* Pour les saisies clavier (desac. mise en buffer) */
  noecho();             /* Desactive l'affichage des caracteres saisis */
  keypad(stdscr, TRUE);	/* Active les touches specifiques */
  refresh();            /* Met a jour l'affichage */
  timeout(1000);    /*mettre le getch non bloquant */
  curs_set(FALSE);      /* Masque le curseur */
}

/**
 * Fin de ncurses.
 */
void ncurses_stopper() {
  endwin();
}

/**
 * Initialisation des couleurs.
 */
void ncurses_couleurs() {
  /* Verification du support de la couleur */
  if(has_colors() == FALSE) {
    ncurses_stopper();
    fprintf(stderr, "Le terminal ne supporte pas les couleurs.\n");
    exit(EXIT_FAILURE);
  }

  /* Activation des couleurs */
  start_color();

  /* Definition de la palette */
  init_pair(1, COLOR_BLUE, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLUE);
  init_pair(3, COLOR_GREEN, COLOR_WHITE);
  init_pair(4, COLOR_CYAN, COLOR_BLACK);
  init_pair(8, COLOR_GREEN, COLOR_GREEN);    /* lemmings allié */
  init_pair(9, COLOR_RED, COLOR_RED);    /* lemmings ennemis */
  init_pair(10, COLOR_WHITE, COLOR_WHITE);    /* rien */
  init_pair(11, COLOR_BLUE, COLOR_BLUE);    /* obstacle */
  init_pair(15, COLOR_BLACK, COLOR_WHITE);    /* fond fenetre mère carte */
}

/**
 * Initialisation de la souris.
 */
void ncurses_souris() {
  if(!mousemask(ALL_MOUSE_EVENTS, NULL)) {
    ncurses_stopper();
    fprintf(stderr, "Erreur lors de l'initialisation de la souris.\n");
    exit(EXIT_FAILURE);
  }
 
  if(has_mouse() != TRUE) {
    ncurses_stopper();
    fprintf(stderr, "Aucune souris n'est détectée.\n");
    exit(EXIT_FAILURE);
  }
}
 
/**
 * Recupere la position x et y de la souris.
 * @param[out] x la position en x
 * @param[out] y la position en y
 * @param[out] bouton l'évenement associé au clic (ou NULL)
 * @return OK si reussite
 */
int souris_getpos(int *x, int *y, int *bouton) {
  MEVENT event;
  int resultat = getmouse(&event);
 
  if(resultat == OK) {
    *x = event.x;
    *y = event.y;
    if(bouton != NULL) *bouton = event.bstate;
  }
  return resultat;
}
