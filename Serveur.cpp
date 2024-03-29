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
#include <setjmp.h>
#include "protocole.h" // contient la cle et la structure d'un message
#include "FichierClient.h"

int idQ,idShm,idSem;
int fdPipe[2];
TAB_CONNEXIONS *tab;
sigjmp_buf back;

void afficheTab();

void handlerSIGINT(int);
void handlerSIGCHLD(int);
void exitfunc()
{
  handlerSIGINT(0);
}


int main()
{
  MESSAGE m;
  MESSAGE reponse;
  MESSAGE log;
  // Armement des signaux
  // TO DO
  struct sigaction sig;

  sigemptyset(&sig.sa_mask);
  sig.sa_handler = handlerSIGINT;
  sig.sa_flags = 0;
  sigaction(SIGINT,&sig,NULL);

  sig.sa_handler = handlerSIGCHLD;
  sig.sa_flags = 0;
  sigaction(SIGCHLD,&sig,NULL);

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
  idShm = createSharedMemory(CLE,sizeof(char)*51);

  atexit(exitfunc);

  //********************************************************
  // Creation du pipe
  //********************************************************
  if(pipe(fdPipe)!= 0 )
  {
    ERROR_PRINT("Erreur ouverture pipe");
    exit(1);
  } 
  //********************************************************
  // Creation du semaphore
  //********************************************************
  idSem = createSemaphore(CLE,1);
  setValueSemaphore(idSem);

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

  //afficheTab();

  //********************************************************
  // Creation du processus Publicite (étape 2)
  //********************************************************

  char* connexion = connectSharedMemory(idShm,"RW");
  sprintf(connexion,"Hello world!");

  fprintf(stderr,"(SERVEUR) Creation du processus Publicite\n");
  if((tab->pidPublicite = fork()) == 0)
  {
    execl("./Publicite","./Publicite",NULL);
    exit(0);
  }
  
  //********************************************************
  // Creation du processus AccesBD (étape 4)
  //********************************************************
  if((tab->pidAccesBD = fork()) == 0)
  {
    char entry[10];
    sprintf(entry,"%d",fdPipe[0]);
    execl("./AccesBD","./AccesBD",entry,NULL);
    ERROR_PRINT("Fin AccesBD SEVREUR");
    exit(0);
  }

  //*****************************************
  //  SUPPRESSION DU FLUX STDOUT/STDERR
  //*****************************************

  fprintf(stderr,"\033[H\033[J");
  afficheTab();

  sigsetjmp(back,1);

  while(1)
  {
    if(recieveMessageQueue(idQ,m,SERVEUR,"Erreur rcv Serveur")==-1)
      exit(1);    

    clearMessage(reponse);
    // fprintf(stderr,"\033[H\033[J");
    int semVal = sem_wait(idSem,0,IPC_NOWAIT);
    if(semVal != -1) sem_signal(idSem,0);

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
                        if(sendMessageQueue(idQ,reponse,"(SERVEUR) CONNECT")==-1)
                          handlerSIGINT(0);
                        break;
                      }
                      makeMessageBasic(reponse,m.expediteur,1,CONNECT);
                      makeMessageData(reponse,1);
                      if(sendMessageQueue(idQ,reponse,"(SERVEUR) CONNECT")==-1)
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
                      if(semVal == -1)
                      {
                        clearMessage(reponse);
                        makeMessageBasic(reponse,m.expediteur,getpid(),BUSY);
                        sendMessageQueue(idQ,reponse);
                        kill(m.expediteur,SIGUSR1);
                        break;
                      }

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
                              if((i.pidCaddie = fork()) == 0)
                              {
                                char entry[10];
                                sprintf(entry,"%d",fdPipe[1]);
                                execl("./Caddie","./Caddie",entry,NULL);
                              }
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
                              if((i.pidCaddie = fork()) == 0)
                              {
                                char entry[10];
                                sprintf(entry,"%d",fdPipe[1]);
                                execl("./Caddie","./Caddie",entry,NULL);
                              }
                              break;
                            }
                          }
                        }
                        if(ret == -1)
                        {                          
                          makeMessageData(reponse,5);
                        }
                      }

                      for(const auto& i : tab->connexions)
                      {
                        if(i.pidFenetre == m.expediteur)
                        {
                          clearMessage(log);
                          makeMessageBasic(log,i.pidCaddie,m.expediteur,LOGIN);
                          sendMessageQueue(idQ,log);
                          break;
                        }
                      } 

                      makeMessageBasic(reponse,m.expediteur,SERVEUR,LOGIN);
                      sendMessageQueue(idQ,reponse,"(SERVEUR) LOGIN");     
                      kill(m.expediteur,SIGUSR1);                                  

                      break; 

      case LOGOUT :   // TO DO
                      if(semVal == -1)
                      {
                        clearMessage(reponse);
                        makeMessageBasic(reponse,m.expediteur,getpid(),BUSY);
                        sendMessageQueue(idQ,reponse);
                        kill(m.expediteur,SIGUSR1);
                        break;
                      }

                      for(auto& i : tab->connexions)
                      {
                        if(i.pidFenetre == m.expediteur)
                        {
                          strcpy(i.nom,"");
                          //**************************************
                          // Tuer le pcs Caddie avec un message
                          //**************************************
                          if(i.pidCaddie != 0)
                          {
                            clearMessage(reponse);
                            makeMessageBasic(reponse,i.pidCaddie,SERVEUR,LOGOUT);
                            sendMessageQueue(idQ,reponse,"(SERVEUR) LOGOUT");
                          }
                          break;
                        }
                      }
                      
                      break;

      case UPDATE_PUB :  // TO DO

                      for(const auto& i : tab->connexions)
                      {
                        if(i.pidFenetre != 0)
                          kill(i.pidFenetre,SIGUSR2);
                      }
                      break;

      case CONSULT :  // TO DO
                      if(semVal == -1)
                      {
                        clearMessage(reponse);
                        makeMessageBasic(reponse,m.expediteur,getpid(),BUSY);
                        sendMessageQueue(idQ,reponse);
                        kill(m.expediteur,SIGUSR1);
                        break;
                      }

                      fprintf(stderr,"(SERVEUR %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      
                      for(const auto& [pidF,nom,pidC] : tab->connexions)
                      {
                        if(pidF == m.expediteur)
                        {
                          makeMessageBasic(reponse,pidC,pidF,CONSULT);
                          makeMessageData(reponse,m.data1);
                          sendMessageQueue(idQ,reponse,"(SERVEUR) CONSULT");

                          break;
                        }
                      }
                      
                      break;

      case ACHAT :    // TO DO
                      if(semVal == -1)
                      {
                        clearMessage(reponse);
                        makeMessageBasic(reponse,m.expediteur,getpid(),BUSY);
                        sendMessageQueue(idQ,reponse);
                        kill(m.expediteur,SIGUSR1);
                        break;
                      }

                      fprintf(stderr,"(SERVEUR %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);
                      for(const auto& [pidF,nom,pidC] : tab->connexions)
                      {
                        if(pidF == m.expediteur)
                        {
                          clearMessage(reponse);
                          makeMessageBasic(reponse,pidC,m.expediteur,ACHAT);
                          makeMessageData(reponse,m.data1,m.data2,m.data3,m.data4,m.data5);
                          sendMessageQueue(idQ,reponse);
                          break;
                        }
                      }
                      break;

      case CADDIE :   // TO DO
                      if(semVal == -1)
                      {
                        clearMessage(reponse);
                        makeMessageBasic(reponse,m.expediteur,getpid(),BUSY);
                        sendMessageQueue(idQ,reponse);
                        kill(m.expediteur,SIGUSR1);
                        break;
                      }

                      fprintf(stderr,"(SERVEUR %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                      for(const auto& [pidF,nom,pidC] : tab->connexions)
                      {
                        if(pidF == m.expediteur)
                        {
                          clearMessage(reponse);
                          makeMessageBasic(reponse,pidC,m.expediteur,CADDIE);
                          sendMessageQueue(idQ,reponse);
                          break;
                        }
                      }
                      break;

      case CANCEL :   // TO DO
                      if(semVal == -1)
                      {
                        clearMessage(reponse);
                        makeMessageBasic(reponse,m.expediteur,getpid(),BUSY);
                        sendMessageQueue(idQ,reponse);
                        kill(m.expediteur,SIGUSR1);
                        break;
                      }

                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                      for(const auto& [pidF,nom,pidC] : tab->connexions)
                      {
                        if(pidF == m.expediteur)
                        {
                          clearMessage(reponse);
                          makeMessageBasic(reponse,pidC,m.expediteur,CANCEL);
                          reponse.data1 = m.data1;
                          sendMessageQueue(idQ,reponse);
                          break;
                        }
                      }
                      break;

      case CANCEL_ALL : // TO DO
                      if(semVal == -1)
                      {
                        clearMessage(reponse);
                        makeMessageBasic(reponse,m.expediteur,getpid(),BUSY);
                        sendMessageQueue(idQ,reponse);
                        kill(m.expediteur,SIGUSR1);
                        break;
                      }

                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);
                      for(const auto& [pidF,nom,pidC] : tab->connexions)
                      {
                        if(pidF == m.expediteur)
                        {
                          clearMessage(reponse);
                          makeMessageBasic(reponse,pidC,m.expediteur,CANCEL_ALL);
                          sendMessageQueue(idQ,reponse);
                          break;
                        }
                      }
                      break;

      case PAYER : // TO DO
                      if(semVal == -1)
                      {
                        clearMessage(reponse);
                        makeMessageBasic(reponse,m.expediteur,getpid(),BUSY);
                        sendMessageQueue(idQ,reponse);
                        kill(m.expediteur,SIGUSR1);
                        break;
                      }

                      fprintf(stderr,"(SERVEUR %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);
                      for(const auto& [pidF,nom,pidC] : tab->connexions)
                      {
                        if(pidF == m.expediteur)
                        {
                          clearMessage(reponse);
                          makeMessageBasic(reponse,pidC,m.expediteur,PAYER);
                          sendMessageQueue(idQ,reponse);
                          break;
                        }
                      }
                      break;

      case NEW_PUB :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete NEW_PUB reçue de %d\n",getpid(),m.expediteur);
                      clearMessage(reponse);
                      makeMessageBasic(reponse,tab->pidPublicite,getpid(),NEW_PUB);
                      strcpy(reponse.data4,m.data4);
                      sendMessageQueue(idQ,reponse);
                      kill(tab->pidPublicite,SIGUSR1);
                      break;
      case ISBUSY :
                    if(semVal == -1)
                    {
                      clearMessage(reponse);
                      makeMessageBasic(reponse,m.expediteur,getpid(),BUSY);
                      makeMessageData(reponse,1);
                      sendMessageQueue(idQ,reponse);
                      break;
                    }

                    clearMessage(reponse);
                    makeMessageBasic(reponse,m.expediteur,getpid(),BUSY);
                    makeMessageData(reponse,0);
                    sendMessageQueue(idQ,reponse);

                    break;
    }
    // system("clear");
    afficheTab();
  }
}


void handlerSIGINT(int sig)
{
  if(tab->pidAccesBD != 0)
  {
    MESSAGE m;
    clearMessage(m);
    makeMessageBasic(m,tab->pidAccesBD,SERVEUR,DECONNECT);
    write(fdPipe[1],&m,sizeof(m));
  }

  deleteMessageQueue(idQ);
  deleteSharedMemory(idShm);
  closePipe(fdPipe);
  deleteSemaphore(idSem);

  exit(0);
}
void handlerSIGCHLD(int)
{
  pid_t pidCaddie = wait(NULL);
  fprintf(stderr,"(SERVEUR) Recuperation du fils %d\n",pidCaddie);
  if(pidCaddie != -1)
  {
    for(auto& x: tab->connexions)
    {
      if(x.pidCaddie == pidCaddie)
      {
        x.pidCaddie = 0;
        NORMAL_PRINT("RECEPTION SIGNAL SIGCHLD...");
        siglongjmp(back,0);
      }
    }
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

