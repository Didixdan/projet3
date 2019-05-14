#include <stdio.h>
/*#include <stdlib.h>

#define LARGEURC 50  Correspond à la largeur max de la carte <--- Arbitraire 
#define HAUTEURC 25  Correspond à la hauteur max de la carte <--- Arbitraire 

typedef struct {
    unsigned char largeur;
    unsigned char hauteur;
    unsigned char *cases;  malloc à faire avec largeur x hauteur 
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
*/
int indiceMaxLemmingNonPlace(int * tabLemming) {
  int i;
  int indice=0;
  for(i=0;i<5;i++) {
    if(tabLemming[i]==0) {
      if(i>indice) indice=i;
    }
  }
  return indice;
}
int indiceMinLemmingNonPlace(int * tabLemming) {
  int i;
  int indice=5;
  for(i=0;i<5;i++) {
    if(tabLemming[i]==0) {
      if(i<indice) indice=i;
    }
  }
  return indice;
}

int aucunLemmingNonPlacer(int * tabLemming) {
  int i;
  int ok=1;
  for(i=0;i<5;i++) {
    if(tabLemming[i]==0) {
      /* on quitte la boucle si on trouve un lemming non placé */
      ok=0;
      i=5;
    }
  }
  return ok;
}

void main() {
    /*requete_envCMS_t carte;
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
    */
    int tab[5]={0,0,1,0,0};
    printf("%d;%d\n",indiceMinLemmingNonPlace(tab),indiceMaxLemmingNonPlace(tab));
    int tab2[5]={1,1,1,1,1};
    printf("%d;%d;%d\n",indiceMinLemmingNonPlace(tab2),indiceMaxLemmingNonPlace(tab2),aucunLemmingNonPlacer(tab2));
}
