#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "protocole.h" // contient la cle et la structure d'un message

int idQ, idShm;
char *pShm;
void handlerSIGUSR1(int sig);
int fd;

char txt[100];

int main()
{
  // Armement des signaux
  // TO DO
  struct sigaction sig;

  sig.sa_flags = 0;
  sigemptyset(&sig.sa_mask);
  sig.sa_handler = handlerSIGUSR1;
  sigaction(SIGUSR1,&sig,NULL);

  // Masquage des signaux
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask,SIGUSR1);
  sigdelset(&mask,SIGKILL);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  //*********************************************************
  // Recuperation de l'identifiant de la file de messages
  //*********************************************************
  NORMAL_PRINT("(Publicite) Recuperation de la file de message.");
  idQ = getMessageQueue(CLE);

  //*********************************************************
  // Recuperation de l'identifiant de la mémoire partagée
  //*********************************************************
  NORMAL_PRINT("(Publicite) Recuperation de la memoire partagee.");
  idShm = getSharedMemory(CLE);

  //*********************************************************
  // Attachement a la mémoire partagée
  //*********************************************************
  NORMAL_PRINT("(Publicite) Connexion a la memoire partagee.");
  pShm = connectSharedMemory(idShm,"RW");

  //Copier dans un (char[51]) le contenu de la memoire partagée
  char Pub[51] = {0};
  sprintf(Pub,"%50s","");
  //fprintf(stderr,"ICI : %d\n",Pub[50]);

  /* sprintf(txt,"(PUBLICITE) CONTENU DE PUB(taille %d)=>",strlen(Pub));
  NORMAL_PRINT(txt);
  NORMAL_PRINT(Pub); */

  //*******************************
  // *******************************
  // *******************************

  //for (int i=0 ; i<=50 ; i++) Pub[i] = ' ';
  //Pub[51] = '\0';
  int ind = 25 - strlen(pShm)/2;
  for (int i=0 ; i<strlen(pShm) ; i++) Pub[ind + i] = pShm[i];// NORMAL_PRINT(Pub);

  MESSAGE m;

  clearMessage(m);
  makeMessageBasic(m,SERVEUR,getpid(),UPDATE_PUB);

  char left_char;
  while(1)
  {
    // Envoi d'une requete UPDATE_PUB au serveur
    sendMessageQueue(idQ,m);

    //sleep(1); 
    usleep(1'000'000);

    // Decallage vers la gauche
    left_char = Pub[0];
    for (int j = 0; j < 49; j++)
    {
      Pub[j] = Pub[j+1];
    }
    Pub[49] = left_char;
    Pub[50] = '\0';

    sprintf(pShm,"%s",Pub);

    // NORMAL_PRINT(Pub);
  } 

  return 0;
}

void handlerSIGUSR1(int sig)
{
  fprintf(stderr,"(PUBLICITE %d) Nouvelle publicite !\n",getpid());

  // Lecture message NEW_PUB

  // Mise en place de la publicité en mémoire partagée
}

