#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "protocole.h" // contient la cle et la structure d'un message
#include "FichierClient.h"

int idQ,idShm,idSem;
int fdPipe[2];
TAB_CONNEXIONS *tab;

void afficheTab();

void handlerSIGINT(int);
void handlerSIGINT(int sig)
{
  deleteMessageQueue(idQ);
  deleteSharedMemory(idShm);
  exit(0);
}

int main()
{
  MESSAGE m;
  MESSAGE reponse;
  // Armement des signaux
  // TO DO
  struct sigaction sig;

  sigemptyset(&sig.sa_mask);
  sig.sa_handler = handlerSIGINT;
  sig.sa_flags = 0;
  sigaction(SIGINT,&sig,NULL);

  //********************************************************
  // Creation de la file de message
  //********************************************************
  fprintf(stderr,"(SERVEUR %d) Creation de la file de messages\n",getpid());  
  idQ = createMessageQueue(CLE);
  if(idQ == -1)
    exit(1);

  //********************************************************
  // Creation de la memoire partagée
  //********************************************************
  fprintf(stderr,"(SERVEUR) Creation de la memoire partagée\n");  
  idShm = createSharedMemory(CLE,sizeof(char)*52);

  //********************************************************
  //********************************************************
  //********************************************************

  // Creation du pipe
  // TO DO

  //********************************************************
  // Initialisation du tableau de connexions
  //********************************************************
  tab = (TAB_CONNEXIONS*) malloc(sizeof(TAB_CONNEXIONS)); 

  for (int i=0 ; i<6 ; i++)
  {
    tab->connexions[i].pidFenetre = 0;
    strcpy(tab->connexions[i].nom,"");
    tab->connexions[i].pidCaddie = 0;
  }
  tab->pidServeur = getpid();
  tab->pidPublicite = 0;

  // afficheTab();

  //********************************************************
  // Creation du processus Publicite (étape 2)
  //********************************************************
  fprintf(stderr,"(SERVEUR) Creation du processus Publicite\n");
  if((tab->pidPublicite = fork()) == 0)
  {
    execl("./Publicite",NULL);
    exit(0);
  }

  // Creation du processus AccesBD (étape 4)
  // TO DO

  //*****************************************
  //SUPPRESSION DU FLUX STDOUT/STDERR
  //*****************************************

  fprintf(stderr,"\033[H\033[J");
  afficheTab();

  while(1)
  {
    if(recieveMessageQueue(idQ,m,SERVEUR)==-1)
      exit(1);
    

    clearMessage(reponse);
    fprintf(stderr,"\033[H\033[J");
    switch(m.requete)
    {
      case CONNECT :  // TO DO
                      //******************************************************
                      //RECHERCHE D'UNE PLACE DANS LA TABLE DE CONNEXION
                      // si isConnected est faux alors on doit retourner
                      // une reponse au client.
                      //******************************************************

                      bool isConnected;
                      isConnected = false;
                      for(auto& this_c : tab->connexions)
                      {
                        if(this_c.pidFenetre == 0)
                        {
                          // NORMAL_PRINT("RECEPTION MESSAGE");
                          this_c.pidFenetre = m.expediteur;
                          isConnected = true;
                          break;
                        }
                      }
                      if(!isConnected)
                      {
                        makeMessageBasic(reponse,m.expediteur,1,CONNECT);
                        makeMessageData(reponse,0);
                        if(sendMessageQueue(idQ,reponse)==-1)
                          handlerSIGINT(0);
                        break;
                      }
                      makeMessageBasic(reponse,m.expediteur,1,CONNECT);
                      makeMessageData(reponse,1);
                      if(sendMessageQueue(idQ,reponse)==-1)
                        handlerSIGINT(0);
                      
                      break;

      case DECONNECT : // TO DO
                      //NORMAL_PRINT("DECONNECT RCV");
                      for(auto& thisc : tab->connexions)
                      {
                        // gfprintf(stderr,"pid fenetre:%d\npid expe:%d",thisc.pidFenetre,m.expediteur);
                        if(thisc.pidFenetre == m.expediteur)
                        {
                          thisc.pidFenetre = 0;
                          break;
                        }
                      }
                      break;
      case LOGIN :    // TO DO
                      //data = 0 si la case nvC n'est pas cochée
                      //     = 1 si la case nvC est cochée
                      // clearMessage(reponse);
                      if(m.data1 == 0)
                      {
                        int ret = logIn(m.data2,m.data3);
                        //1   = correct
                        //2  = incorrect
                        //3  = mv mdp
                        if(ret == 1)
                        {
                          makeMessageData(reponse,1);
                          for(auto& i : tab->connexions)
                          {
                            if(i.pidFenetre == m.expediteur)
                            {
                              strcpy(i.nom,m.data2);
                              break;
                            }
                          }
                        }
                        if(ret == -1)
                        {
                          makeMessageData(reponse,2);
                        }
                        if(ret == 2)
                        {
                          makeMessageData(reponse,3);
                        }                        
                      }
                      if(m.data1 == 1)
                      {
                        int ret = signin(m.data2,m.data3);
                        if(ret == 1)
                        {
                          makeMessageData(reponse,4);
                          for(auto& i : tab->connexions)
                          {
                            if(i.pidFenetre == m.expediteur)
                            {
                              strcpy(i.nom,m.data2);
                              break;
                            }
                          }
                        }
                        if(ret == -1)
                        {                          
                          makeMessageData(reponse,5);
                        }
                      }

                      makeMessageBasic(reponse,m.expediteur,SERVEUR,LOGIN);
                      sendMessageQueue(idQ,reponse);      
                      kill(m.expediteur,SIGUSR1);                

                      break; 

      case LOGOUT :   // TO DO
                      for(auto& i : tab->connexions)
                      {
                        if(i.pidFenetre == m.expediteur)
                        {
                          strcpy(i.nom,"");
                          break;
                        }
                      }
                      
                      break;

      case UPDATE_PUB :  // TO DO
                      break;

      case CONSULT :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CADDIE :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CANCEL_ALL : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);
                      break;

      case PAYER : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);
                      break;

      case NEW_PUB :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete NEW_PUB reçue de %d\n",getpid(),m.expediteur);
                      break;
    }
    // system("clear");
    afficheTab();
  }
}


void afficheTab()
{
  fprintf(stderr,"Pid Serveur   : \033[38;5;213m%d\033[0m\n",tab->pidServeur);
  fprintf(stderr,"Pid Publicite : \033[92m%d\033[0m\n",tab->pidPublicite);
  fprintf(stderr,"Pid AccesBD   : \033[93m%d\033[0m\n",tab->pidAccesBD);
  for (int i=0 ; i<6 ; i++)
  {  
    if(tab->connexions[i].pidFenetre == 0)
    {
      fprintf(stderr,"%6d -%20s- %6d\n",tab->connexions[i].pidFenetre,
                                                      tab->connexions[i].nom,
                                                      tab->connexions[i].pidCaddie);
    }
    else if(tab->connexions[i].pidCaddie == 0)
    {
      fprintf(stderr,"\e[91m%6d\e[0m -\e[94m%20s\e[0m- %6d\n",tab->connexions[i].pidFenetre,
                                                      tab->connexions[i].nom,
                                                      tab->connexions[i].pidCaddie);
    }
    else
    {
      fprintf(stderr,"\e[91m%6d\e[0m -\e[94m%20s\e[0m- \e[92m%6d\e[0m\n",tab->connexions[i].pidFenetre,
                                                      tab->connexions[i].nom,
                                                      tab->connexions[i].pidCaddie);
    }    
  
  }
  fprintf(stderr,"\n");
}

