#include "FichierClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>

int estPresent(const char* nom)
{
  // TO DO
  fprintf(stderr,"\nSTART %s\n",__func__);

  CLIENT cli;
  int compteur = 0;
  int nbr_elem;
  int file = open(FICHIER_CLIENTS, O_RDONLY );

  if(file == -1)
  {
    //perror("Erreur ouverture fichier");
    fprintf(stderr,"=>file opening failed\n");
    return -1;
  }
  read(file,&nbr_elem,sizeof(int));
  fprintf(stderr,"nbr_elem = \e[96m%d\e[0m\n",nbr_elem);

  do
  {
    read(file,&cli,sizeof(CLIENT));
    compteur++;

    fprintf(stderr,"client: \e[96m%s\e[0m-\e[91m%d\e[0m\n",cli.nom,cli.hash);
    if(strcmp(nom,cli.nom) == 0)
    {
      fprintf(stderr,"Client=\e[96m%s \e[0m: \e[32mTROUVE\e[0m\n",nom);
      return compteur;
    }
    fprintf(stderr,"compteur(\e[96m%d\e[0m) < nbr_element(\e[96m%d\e[0m)\n",compteur,nbr_elem);
  }while(compteur < nbr_elem);


  fprintf(stderr,"Client=\e[96m%s \e[0m: \e[31mNON TROUVE\e[0m\n",nom);
  close(file);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////
int hash(const char* motDePasse)
{
  // TO DO
  //fprintf(stderr,"START %s\n",__func__);

  std::string mdp = motDePasse;
  int i = 1;
  int hashed_password = 0;
  for(char x : mdp)
  {
    hashed_password = i*(int)x + hashed_password;
    i++;
  }  

  return hashed_password%97;
}

////////////////////////////////////////////////////////////////////////////////////
void ajouteClient(const char* nom, const char* motDePasse)
{
  // TO DO
  fprintf(stderr,"\nSTART %s\n",__func__);

  CLIENT cli ;
  strcpy(cli.nom,nom);
  cli.hash = hash(motDePasse);
  int file;
  int nbr = 1;

  file = open(FICHIER_CLIENTS, O_RDWR|O_CREAT|O_EXCL,0777);
  if(file == -1)
  {
    file = open(FICHIER_CLIENTS, O_RDWR );
    if(file == -1)
    {
      perror("Erreur Ouverture fichier");
      fprintf(stderr,"=> opening failed+creation failed\n");
      return;
    }
  }
  else
  {

    fprintf(stderr,"AJOUT= \e[96m%s-\e[91m%d\e[0m\n",cli.nom,cli.hash);
    write(file,&nbr,sizeof(int));
    write(file,&cli,sizeof(CLIENT));
    close(file);
    return;
  }

  read(file,&nbr,sizeof(int));
  lseek(file,0,SEEK_SET);
  printf("AVANT: valeur de \e[96mnbr=%d\e[0m\n",nbr);
  nbr++;
  printf("APRES: valeur de \e[96mnbr=%d\e[0m\n",nbr);
  write(file,&nbr,sizeof(int));
  lseek(file,0,SEEK_END);

  fprintf(stderr,"AJOUT= \e[96m%s \e[91%d\e[0m\n",cli.nom,cli.hash);
  int i = write(file,&cli,sizeof(CLIENT));
  if(i != sizeof(CLIENT))
  {
    //perror("Erreur ecriture");
    fprintf(stderr,"=> Erreur ecriture\n");
    close(file);
    exit(1);
  }

  close(file);

}

////////////////////////////////////////////////////////////////////////////////////
int verifieMotDePasse(int pos, const char* motDePasse)
{
  // TO DO
  fprintf(stderr,"\nSTART %s\n",__func__);

  int file;
  CLIENT cli;
  pos--;

  file = open(FICHIER_CLIENTS, O_RDONLY);
  if(file == -1) 
  {
    fprintf(stderr,"=> ERREUR OPEN\n");
    return -1;
  }

  lseek(file,sizeof(CLIENT)*pos+sizeof(int),SEEK_SET);
  int i = read(file,&cli,sizeof(CLIENT));
  close(file);

  if(cli.hash != hash(motDePasse))
  {
    //perror("Erreur de correspondance des hash");
    fprintf(stderr,"=> Erreur de correspondance des hash\n");
    return 0;
  }


  fprintf(stderr,"MOTDEPASSE=\e[32mTRUE\e[0m\n");
  return 1;
}

////////////////////////////////////////////////////////////////////////////////////
int listeClients(CLIENT *vecteur) // le vecteur doit etre suffisamment grand
{
  // TO DO
  fprintf(stderr,"\nSTART %s\n",__func__);

  int file = open(FICHIER_CLIENTS, O_RDONLY);
  if(file == -1)
  {
    //perror("liste client=ERREUR OUVERTURE");
    fprintf(stderr,"=>liste client=ERREUR OUVERTURE\n");
    return -1;
  }

  int nbr ;

  read(file,&nbr,sizeof(int));
  fprintf(stderr,"Read nbr_element= %d \n",nbr);
  if(vecteur == NULL)
  {
    close(file);
    return nbr;
  }

  read(file,vecteur,sizeof(CLIENT)*nbr);

  close(file);
  return nbr;
}
