/**
 * Classe carte
 * @author Alexandre Varin & Florian Botte
 * @version 17/01/2019


  Y = hauteur
  X = largeur

 **/

#include <string.h>
#include <stdio.h>     /* Pour printf, scanf */
#include <fcntl.h>     /* Pour open */
#include <unistd.h>    /* Pour write */
#include <sys/stat.h>  /* Pour O_WRONLY, O_CREAT, S_IRUSR, S_IWUSR */
#include <stdlib.h>    /* Pour exit, EXIT_SUCCESS, EXIT_FAILURE */
#include <ncurses.h>    /* Pour printw, attron, attroff, COLOR_PAIR, getch */
#include <string.h>
#include <time.h> 

#include "structures.h"
#include "carte.h"



int getLargeurCarte(int fdCarte){
    unsigned char largeur;

    if(lseek(fdCarte, 0, SEEK_SET) == (off_t)-1) /* retour au debut du fichier */{
      perror("Erreur lors du deplacement dans le fichier ");
      exit(EXIT_FAILURE);
    }

    if(read(fdCarte, &largeur, sizeof(unsigned char))==-1){
      perror("Erreur lors de la lecture de la largeur de la carte \n");
      exit(EXIT_FAILURE);
    }
    return largeur;
}

int getHauteurCarte(int fdCarte){
    unsigned char hauteur;

    if(lseek(fdCarte, sizeof(unsigned char), SEEK_SET) == (off_t)-1) /* retour au debut du fichier */{
      perror("Erreur lors du deplacement dans le fichier ");
      exit(EXIT_FAILURE);
    }

    if(read(fdCarte, &hauteur, sizeof(unsigned char))==-1){
      perror("Erreur lors de la lecture de la hauteur de la carte \n");
      exit(EXIT_FAILURE);
    }
    return hauteur;
}
/* renvoie une structure qui correspond à la carte correspondant au descripteur de fichier passé en paramètre */
requete_envCMS_t getStructByFd(int fd) {

  requete_envCMS_t carte;
  
  if(lseek(fd, 0, SEEK_SET) == (off_t)-1) {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
  }
  if(read(fd, &carte.largeur, sizeof(unsigned char)) == -1) {
    perror("Erreur lors de la lecture de la larguer de la carte ");
    exit(EXIT_FAILURE);
  }
  if(read(fd, &carte.hauteur, sizeof(unsigned char)) == -1) {
    perror("Erreur lors de la lecture de la hauteur de la carte ");
    exit(EXIT_FAILURE);
  }

  carte.cases = (unsigned char*)malloc(sizeof(unsigned char)*(carte.largeur*carte.hauteur));

  if(read(fd, carte.cases, sizeof(unsigned char)*(carte.largeur*carte.hauteur)) == -1) {
    perror("Erreur lors de la lecture des cases de la carte ");
    exit(EXIT_FAILURE);
  }
  printf("Lecture de la carte réussi.\n");
  return carte;
}

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

/**
  * Créer une carte par defaut (0 sur toutes les cases, 1 vie et version 0)
  * @param fdCarte le descripteur de fichier de la carte
  **/
void createDefaultCarte(int fdCarte)
	{
	requete_envCMS_t carte;
  creerCarteAleatoire(&carte);
  /* on écrit les cases de la carte*/
  if(write(fdCarte, &carte.largeur, sizeof(unsigned char)) == -1) {
    perror("Erreur write ");
    exit(EXIT_FAILURE);
  }
  if(write(fdCarte, &carte.hauteur, sizeof(unsigned char)) == -1) {
    perror("Erreur write ");
    exit(EXIT_FAILURE);
  }
  if(write(fdCarte, carte.cases, sizeof(unsigned char)*(carte.largeur*carte.hauteur)) == -1) {
    perror("Erreur write ");
    exit(EXIT_FAILURE);
  }
}


/**
  * Créer/Charge la carte dont le nom est passer en paramètre pour édition/jeu
  * @param nomCarte le nom de la carte à créer/charger
  * @return l'entier du descripteur du fichier
  **/
int editCarte(const char * nomCarte) {
	int fdCarte=0;
  char pathVanilla[30] = "./cartes/";
  char pathCpy[30] = "./cartes/";
  char * fullpath;
  strcat(pathCpy,nomCarte);
  fullpath = realpath(pathCpy, NULL);

	/* Ouverture et/ou création du fichier de la carte */
	if((fdCarte = open(fullpath, O_RDWR, S_IRUSR|S_IWUSR)) == -1){
		/* si on est ici la carte n'existe pas */
    /* création du lien de la futur carte */
    fullpath = realpath(pathVanilla, NULL);
    strcat(fullpath,"/");
    strcat(fullpath,nomCarte);

  	if((fdCarte = open(fullpath, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR)) == -1) {
      fprintf(stderr,"Probleme d'ouverture du fichier '%s' au chemin %s.",nomCarte,fullpath);
  		perror("Erreur ");
  		exit(EXIT_FAILURE);
  		}
  	createDefaultCarte(fdCarte);
		}
	return fdCarte;
	}


int nbLemmingEtatJeu(requete_EtatJeu_t etatJeu) {
  int i;
  int nbLemming=0;
  for(i=0;i<NBLEMMINGS*2;i++) {
    if(etatJeu.etatLemmings[i].valeur==VAL_LEMMING_PLATEAU) nbLemming++;
    }
  return nbLemming;
}


void initEtatJeu(requete_EtatJeu_t *etatJeu) {
  int i;
  for(i=0;i<NBLEMMINGS*2;i++) {
    etatJeu->etatLemmings[i].valeur = '0';
    etatJeu->etatLemmings[i].posY = 0;
    etatJeu->etatLemmings[i].posX = 0;
  }
}

/**
  * Créer la fenètre de la carte
  * @param fenMere la fenetre mère de la carte
  * @param fdCarte le descripteur de fichier
  * @param y position Y par rapport à la fenetre mère
  * @param x position X par rapport à la fenetre mère
  * @return la fenetre de la carte créer
  **/
WINDOW * createWindowJeu(WINDOW *fenMere,int y,int x){
	WINDOW * carte;
	carte = derwin(fenMere,HAUTEURC+1, LARGEURC+1, y, x); /* +1 pour le décalage avec la fenetre mere (bien avoir 15 lignes et 30 colonnes) */
	return carte;
}

/**
  * Remplie la fenetre passer en paramètre par les couleurs des cases
  * @param fen la fenetre de la carte à modifier
  * @param carte la structure de la carte
  **/
void remplireFenCarteStruct(WINDOW * fen,requete_envCMS_t *carte) {
  /* Lecture de tout le fichier */
  char val;
  int i,j;
  for(i=0;i<carte->hauteur;i++) {
    for(j=0;j<carte->largeur;j++) {
      val = carte->cases[(i*carte->largeur)+j];
      wattron(fen,COLOR_PAIR(atoi(&val)+10));
      mvwaddch(fen,i,j,val);
      wattroff(fen,COLOR_PAIR(atoi(&val)+10));
    }
  }
}

requete_envCMS_t copyCarte(requete_envCMS_t carte) {
  requete_envCMS_t newCarte;
  int i,j;
  newCarte.largeur = carte.largeur;
  newCarte.hauteur = carte.hauteur;
  newCarte.cases = (unsigned char*)malloc(carte.largeur*carte.hauteur);
  for(i=0;i<carte.hauteur;i++) {
    for(j=0;j<carte.largeur;j++) {
      newCarte.cases[(i*carte.largeur)+j] = carte.cases[(i*carte.largeur)+j];
    }
  }
  return newCarte;
}

requete_EtatJeu_t copyEtatJeu(requete_EtatJeu_t etatJeu) {
  requete_EtatJeu_t newEtatJeu;
  int i;
  for(i=0;i<NBLEMMINGS*2;i++) {
    newEtatJeu.etatLemmings[i].valeur = etatJeu.etatLemmings[i].valeur;
    newEtatJeu.etatLemmings[i].posY = etatJeu.etatLemmings[i].posY;
    newEtatJeu.etatLemmings[i].posX = etatJeu.etatLemmings[i].posX;
  }
  return newEtatJeu;
}

/**
  * Remplie la fenetre passer en paramètre avec les nouvelles informations
  * @param fen la fenetre de la carte à modifier
  * @param req la structure de l'état des lemmings
  * @param nbLemmingPlateau savoir combien il y a de lemming sur le plateau (que ce soit allié ou ennemi)
  **/
void majWindowJeu(WINDOW *fen,requete_envCMS_t *carte, requete_t requete,requete_t* oldEtatJeu) {
  int k;
  char valCaseVoulue = 0;
  char valeurLemming = 0; /* allié ou ennemi (2 ou 3) */
  requete_envCMS_t oldCarte;
  switch(requete.type) {
    case TYPE_ETAT_JEU:
      oldCarte = copyCarte((*carte));
      /* on supp tous les lemmings pour les remettre */
      suppAllLemmings(fen,carte);
      for(k=0;k<nbLemmingEtatJeu(requete.req.r2);k++) {
        if(requete.req.r2.etatLemmings[k].valeur==VAL_LEMMING_PLATEAU) {
          valeurLemming = oldCarte.cases[(oldEtatJeu->req.r2.etatLemmings[k].posY*oldCarte.largeur)+oldEtatJeu->req.r2.etatLemmings[k].posX];
          
          /* si ça vaut 0 c'est qu'on a recu l'état de jeu pour ajouter un lemming ennemi */
          if(valeurLemming=='0')valeurLemming=VAL_LEMMING_ENNEMI;
          else valeurLemming = valeurLemming-'2';

          valCaseVoulue = mvwinch(fen,requete.req.r2.etatLemmings[k].posY,requete.req.r2.etatLemmings[k].posX) & A_CHARTEXT;
          if(valCaseVoulue=='0') {
            addLemings(fen,carte,valeurLemming,requete.req.r2.etatLemmings[k].posY,requete.req.r2.etatLemmings[k].posX);  
          }
        }
      }
      break;
    case TYPE_AL:
      addLemings(fen,carte,VAL_LEMMING_ENNEMI,requete.req.r5.posY,requete.req.r5.posX);
      break;
  }
  wrefresh(fen);
}

/**
  * Ajouter un lemmings sur le fenêtre fen
  * @param fen la fenetre de la carte à modifier
  * @param type lemmings allié ou ennemi (0 ou 1)
  * @param y position Y par rapport à la fenetre
  * @param x position X par rapport à la fenetre
  **/
void addLemings(WINDOW* fen,requete_envCMS_t *carte,int type,int y,int x) {
  /* on modifie la carte en ajoutant le lemming */
  carte->cases[(y*carte->largeur)+x] = '2'+type;
  

  wattron(fen,COLOR_PAIR(8+type));
  mvwaddch(fen,y,x,carte->cases[(y*carte->largeur)+x]);
  wattroff(fen,COLOR_PAIR(8+type));
  wrefresh(fen);
}

void suppLemmings(WINDOW * fen,requete_envCMS_t *carte,int y,int x) {
  carte->cases[(y*carte->largeur)+x] = '0';
  
  wattron(fen,COLOR_PAIR(10));
  mvwaddch(fen,y,x,'0');
  wattroff(fen,COLOR_PAIR(10));

  wrefresh(fen);
}

/**
 * Supprime tous les lemmings de la carte (pour maj de la fenetre) 
 */
void suppAllLemmings(WINDOW * fen,requete_envCMS_t *carte) {
  int i,j;
  for(i=0;i<carte->hauteur;i++) {
    for(j=0;j<carte->largeur;j++) {
      if(carte->cases[(i*carte->largeur)+j]>='2') {
        carte->cases[(i*carte->largeur)+j] = '0';

        wattron(fen,COLOR_PAIR(10));
        mvwaddch(fen,i,j,'0');
        wattroff(fen,COLOR_PAIR(10));
      }
    }
  }
}


/**
  * Gére le fait qu'on clique sur la carte
  * @param fen la fenetre de la carte à modifier
  * @param nbLemmings nombre de lemmings nécessaire afin de construire la légende 
  * @param posY position Y de la case
  * @param posX position X de la case
  * @param modeJeu connaitre le mode de jeu (explosion,ajout plateau,blocage,retirer plateau)
  **/
void cliqueCarte(WINDOW*fen,requete_envCMS_t *carte,int*nbLemmings,int posY,int posX,int modeJeu) {
  switch(modeJeu) {
    case 0:
      if((*nbLemmings)>0) {
        addLemings(fen,carte,VAL_LEMMING_ALLIE,posY,posX);
        (*nbLemmings)--;
      }
      break;
  }
  wrefresh(fen);
}
/**
  * Fait se déplacer les lemmings de manière aléatoire
  *
  *
  *
void deplacementLemmings(requete_envCMS_t carte, WINDOW* fen){
  int i = 0;
  int x = 0;
  int y = 0;

  ncurses_stopper();

  for(i = 0; i < (LARGEURC*HAUTEURC); i++){
    printf("i %d : cases%d\n",i,carte.cases[i]);
    
    printf("case[%d] : %u\n",i,carte.cases[i]);
  
    if(carte.cases[i] > '0'){
      printf("dans condition i : %d\n",i);
      y=(i/LARGEURC)==0?1:(i/LARGEURC);
      x=(i%LARGEURC)==0?LARGEURC:(i%LARGEURC);
      modifValeurCaseLemming(fen,carte, x, y);
    }
  }
 
}*/

/**
  * Fait se déplacer les lemmings de manière aléatoire
  * @param fen la fenêtre du jeu pour modifier l'affichage
  * @param carte la carte pour savoir les valeurs des cases (savoir allié ou ennemi)
  * @param etatJeu connaître les position de chacun des cases et modification pour envoie au slave
  * @param sockTCPSlave socket TCP pour envoyer l'état du jeu à la fin du déplacement de tous les lemmings
  * @param nbLemmingPlateau savoir combien il y a de lemming sur le plateau (que ce soit allié ou ennemi)
  **/
void deplacementLemmings(WINDOW * fen,requete_envCMS_t* carte, requete_t *etatJeu, int nbLemmingPlateau) {
  int i;
  unsigned char val;
  unsigned char valLemming;
  int dir=0;
  int pos=0;
  int oldPos=0;
  int ok=0;
  int y,x=y=0;
	srand(time(NULL));
  /*
    * Direction choisi aléatoirement
    * 0 pour haut, 1 pour bas, 2 pour gauche, 3 pour droite
    */  
  /*printf("Je bouge ! %d;%d;%d\n",*position/30,*position%30,*position);*/
  for(i=0;i<nbLemmingPlateau;i++) {
    y=etatJeu->req.r2.etatLemmings[i].posY;
    x=etatJeu->req.r2.etatLemmings[i].posX;
    pos=y==0?x:((y*carte->largeur)+x);
    do {
      dir=rand()%4;
      oldPos=pos;
      switch(dir) {
        case 0:
          pos-=carte->largeur; /*haut*/ 
          break;
        case 1:
          pos+=carte->largeur; /*bas*/ 
          break;
        case 2:
          pos-=1; /*gauche*/ 
          break;
        case 3:
          pos+=1; /*droite*/ 
          break;
      }
      if(pos<0 || pos>(carte->largeur*carte->hauteur)) {pos=oldPos;continue;}
      val = carte->cases[pos];
      if(val>'0') {pos=oldPos;continue;}
      if(val=='0') {
        if(etatJeu->req.r2.etatLemmings[i].valeur==VAL_LEMMING_PLATEAU) {
          /* on retire le lemming de son ancienne position */
          valLemming = mvwinch(fen,y,x) & A_CHARTEXT;
          suppLemmings(fen,carte,y,x);


          /*printw("1=%d;%d;%d;%d;%d        ",y,x,oldPos,i,nbLemmingPlateau);*/

          /* on calcule la nouvelle position */
          y=(pos/(carte->largeur))==0?1:(pos/(carte->largeur));
          x=(pos%(carte->largeur))==0?(carte->largeur):(pos%(carte->largeur));
          /*printw("2=%d;%d;%d;%c\n",y,x,pos,valLemming);*/
          addLemings(fen,carte,valLemming-'2',y,x);

          etatJeu->req.r2.etatLemmings[i].posY = y;
          etatJeu->req.r2.etatLemmings[i].posX = x;

          ok=1;
        }
      }
      /*printf("(Type;CaseTypeNew;pos) -> (%d;%d;%d)\n",valLemming,carte.cases[pos],pos);*/
    }while(!ok);
  }

  /*position=pos;*/
  wrefresh(fen);
}

/**
  * Deplace le curseur du fichier vers la case y*x pour lecture/ecriture
  * @param fdCarte le descripteur de fichier de la carte
  * @param y position Y de la case
  * @param x position X de la case
  **/
void deplaceInFileTo(int fdCarte,int y,int x)
  {
  int pos;
  pos=y==0?x:y*30+x;
  if(lseek(fdCarte, 0L, SEEK_SET) == (off_t)-1) /* retour au debut du fichier */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }
  if(lseek(fdCarte, sizeof(unsigned char)*2, SEEK_CUR) == (off_t)-1) /* retour au début des cases */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }
  if(lseek(fdCarte, sizeof(unsigned char)*pos, SEEK_CUR) == (off_t)-1) /* déplacement sur la case */
    {
    perror("Erreur lors du deplacement dans le fichier ");
    exit(EXIT_FAILURE);
    }
  }

/**
  * Permet l'affichage de la légende une fois le jeu lancé
  * @param nbLemmings nombre de dlemmings nécessaire afin de construire la légende  
  **/
WINDOW* afficherLegende(WINDOW * boutonPlus,WINDOW * boutonMoins,int nbLemmings) {
    WINDOW * legende;

    legende = newwin(12,27,0,LARGEURC+1);
    wbkgd(legende,COLOR_PAIR(15));
    box(legende,0,0);
    
    wmove(legende,1,1);
    wprintw(legende,"Lemmings : %d/%d",nbLemmings,nbLemmings);

    boutonPlus = derwin(legende,3,4,3,1);
    boutonMoins = derwin(legende,3,4,3,6);

    mvwaddch(boutonPlus,1,1,'+');
    mvwaddch(boutonMoins,1,1,'-');

    wbkgd(boutonPlus,COLOR_PAIR(15));
    wbkgd(boutonMoins,COLOR_PAIR(15));

    box(boutonPlus,0,0);
    box(boutonMoins,0,0);

    wrefresh(boutonPlus);
    wrefresh(boutonMoins);


    wattron(legende,COLOR_PAIR(8));
    mvwaddch(legende,7,1,' ');
    wattroff(legende,COLOR_PAIR(8));
    wmove(legende,7,3);
    wprintw(legende,"Lemmings Allié");

    
    wattron(legende,COLOR_PAIR(9));
    mvwaddch(legende,8,1,' ');
    wattroff(legende,COLOR_PAIR(9));
    wmove(legende,8,3);
    wprintw(legende,"Lemmings Ennemis");

    wattron(legende,COLOR_PAIR(11));
    mvwaddch(legende,9,1,' ');
    wattroff(legende,COLOR_PAIR(11));
    wmove(legende,9,3);
    wprintw(legende,"Mur");

    
    wmove(legende,10,1);
    wprintw(legende,"Apparition lemmings : 10");

    return legende;
}

int indiceMaxLemming(int * tabLemming,int type) {
  int i;
  int indice=0;
  for(i=0;i<NBLEMMINGS;i++) {
    if(tabLemming[i]==type) {
      if(i>indice) indice=i;
    }
  }
  return indice;
}
int indiceMinLemming(int * tabLemming, int type) {
  int i;
  int indice=5;
  for(i=0;i<NBLEMMINGS;i++) {
    if(tabLemming[i]==type) {
      if(i<indice) indice=i;
    }
  }
  return indice;
}
int aucunLemming(int * tabLemming, int type) {
  int i;
  int ok=1;
  for(i=0;i<NBLEMMINGS;i++) {
    if(tabLemming[i]==type) {
      /* on quitte la boucle si on trouve un lemming du type passé */
      ok=0;
      i=NBLEMMINGS;
    }
  }
  return ok;
}

/**
 * Fonction qui gère l'affichage du numéro du lemming choisis pour ajout
 * @param legende la fenètre de la légende pour modif
 * @param numLemming le numéro du lemming avant le clique
 * @param tabLemming le tableau des lemmings (pour afficher seulement ceux non posés)
 * @param type type du clique (0 appuie sur le '+' et 1 appuie sur le '-')
 */ 
void cliqueNumLemming(WINDOW * legende,int * numLemming,int * tabLemming,int type) {

  /* on test si il reste encore des lemmings à placer */
  if(aucunLemming(tabLemming,TYPE_LEMMING_NON_PLACER)==0) {
    if(tabLemming[(*numLemming)-1]==0) {
      if(type==BOUTON_PLUS_LEG && (*numLemming)<indiceMaxLemming(tabLemming,TYPE_LEMMING_NON_PLACER)+1) (*numLemming)++;
      else if(type==BOUTON_MOINS_LEG && (*numLemming)>indiceMinLemming(tabLemming,TYPE_LEMMING_NON_PLACER)+1) (*numLemming)--;
    }
    
    if(tabLemming[(*numLemming)-1]==1) {
      if(type==BOUTON_PLUS_LEG && (*numLemming)==NBLEMMINGS) (*numLemming)=indiceMinLemming(tabLemming,TYPE_LEMMING_NON_PLACER)+1;
      /*else if(type==BOUTON_MOINS_LEG && (*numLemming)==indiceMinLemmingNonPlace(tabLemming)+1) (*numLemming)=indiceMaxLemmingNonPlace(tabLemming)+1;*/
      while(tabLemming[(*numLemming)-1]==1) {
        if(type==BOUTON_PLUS_LEG && (*numLemming)<indiceMaxLemming(tabLemming,TYPE_LEMMING_NON_PLACER)+1) (*numLemming)++;
        else if(type==BOUTON_MOINS_LEG && (*numLemming)>indiceMinLemming(tabLemming,TYPE_LEMMING_NON_PLACER)+1) (*numLemming)--;
      }
    }
  }else (*numLemming)=0;

  wmove(legende,2,1);
  wclrtoeol(legende);
  box(legende,0,0);
  wprintw(legende,"Num lemming a placer : %d",(*numLemming));
  wrefresh(legende);
}

void majTimerLegende(WINDOW * legende, timer_t timerid) {

    struct itimerspec its;/*structure pour intervalle de temps entre chaque appel au signal*/
    printf("MAJ ; ");
    /* récupération du temps du timer */
    if(timer_gettime(timerid,&its) == -1) {
      perror("Erreur lors de la récupération du temps du timer ");
      exit(EXIT_FAILURE);
    }

    wmove(legende,5,1);
    wclrtoeol(legende);
    wprintw(legende,"Apparition lemmings : %d",its.it_value.tv_sec);

    wrefresh(legende);
}
/*
 * Modifier le nbr de lemmings de la légende
 * @param nbLemmings nombre de lemmings nécessaire afin de construire la légende 
 */
void setNbLemmings(WINDOW* legende, int NewNbLemmings) {
    wmove(legende,1,1);
    wclrtoeol(legende);
    box(legende,0,0);
    wprintw(legende,"Lemmings : %d/%d",NewNbLemmings,NBLEMMINGS);

    wrefresh(legende);
}

/**
  * Ferme le fichier de la carte
  * @param fdCarte le descripteur de fichier de la carte
  **/
void closeCarte(int fdCarte)
  {
  if(close(fdCarte) == -1) {
      perror("Erreur close ");
      exit(EXIT_FAILURE);
    }
  }
