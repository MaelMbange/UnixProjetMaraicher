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

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(PUBLICITE %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(PUBLICITE) Erreur de msgget");
    exit(1);
  }

  // Recuperation de l'identifiant de la mémoire partagée  
  fprintf(stderr,"(PUBLICITE %d) Recuperation de l'id de la memoire partagee\n",getpid());
  if((idShm = shmget(CLE,0,0)) == -1)
  {
    perror("(PUBLICITE) Erreur de shmget");
    exit(1);
  }

  // Attachement à la mémoire partagée
  //pShm = (char*)malloc(52); // a supprimer et remplacer par ce qu'il faut
  NORMAL_PRINT("(PUBLICITE) Connexion à la file de message...");
  if((pShm = (char*)shmat(idShm,NULL,0)) == (char*)-1)
  {
    perror("(SERVEUR) Erreur de shmat");
    exit(1);
  }

  //Copier dans un (char[51]) le contenu de la memoire partagée
  char Pub[51];
  sprintf(Pub,"%51s","");

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

  MESSAGE m;int isMsgSend;
  m.type = 1;
  m.expediteur = getpid();
  m.requete = UPDATE_PUB;

  char left_char;
  while(1)
  {
    // Envoi d'une requete UPDATE_PUB au serveur
    isMsgSend = msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0);
    if(isMsgSend == -1)
    {
      sprintf(txt,"(PUBLICITE) pid=%d : Message Update Failed...",getpid());
      ERROR_PRINT(txt);
      exit(1);
    }

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
    /* int j = 0;
    for(auto& x: Pub)
      fprintf(stderr,"%d=%d\n",j++,x);
      usleep(1'000'000'000); */
  } 

  return 0;
}

void handlerSIGUSR1(int sig)
{
  fprintf(stderr,"(PUBLICITE %d) Nouvelle publicite !\n",getpid());

  // Lecture message NEW_PUB

  // Mise en place de la publicité en mémoire partagée
}

