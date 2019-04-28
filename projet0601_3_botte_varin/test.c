#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LARGEURC 50 /* Correspond à la largeur max de la carte <--- Arbitraire */
#define HAUTEURC 25 /* Correspond à la hauteur max de la carte <--- Arbitraire */

typedef struct {
    unsigned char largeur;
    unsigned char hauteur;
    unsigned char *cases; /* malloc à faire avec largeur x hauteur */
}requete_envCMS_t;

void creerCarteAleatoire(requete_envCMS_t*carte) {
  int tmp=0,i=0;
  carte->largeur = LARGEURC;
  carte->hauteur = HAUTEURC;
  carte->cases = (unsigned char*)malloc(LARGEURC*HAUTEURC);
  srand(time(NULL));
  for(i=0;i<(LARGEURC*HAUTEURC);i++) {
      tmp=rand()%100;
      if(tmp>=90) carte->cases[i]='1';
      else carte->cases[i]='0';
  }
}

void main() {
    requete_envCMS_t carte;
    int i;
    creerCarteAleatoire(&carte);
    for(i=0;i<(carte.largeur*carte.hauteur);i++) {
        if(i%carte.largeur==0) printf("\n");
        printf("%d;%d   ",i,carte.cases[i]);
    }

    printf("\n\n\n");
    for(i=0;i<(carte.largeur*carte.hauteur);i++) {
        if(i%carte.largeur==0) printf("\n");
        printf("%c  ",carte.cases[i]);
    }
}
