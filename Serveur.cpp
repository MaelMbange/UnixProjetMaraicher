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
#include <time.h>
#include <setjmp.h>
#include "protocole.h" // contient la cle et la structure d'un message
#include "FichierClient.h"

int idQ,idShm,idSem;
int fdPipe[2];
TAB_CONNEXIONS *tab;

void afficheTab();
void handlerSIGINT(int);
void handlerSIGCHLD(int);

char txt[100];
int Publicite;
int idCaddie;

sigjmp_buf back;

int main()
{
  // Armement des signaux
  // TO DO
  struct sigaction sig;
  sigemptyset(&sig.sa_mask);
  sig.sa_handler = handlerSIGINT;
  sig.sa_flags = 0;
  sigaction(SIGINT,&sig,NULL);

  sigemptyset(&sig.sa_mask);
  sig.sa_handler = handlerSIGCHLD;
  sig.sa_flags = 0;
  sigaction(SIGCHLD,&sig,NULL);

  // Creation des ressources
  // Creation de la file de message
  NORMAL_PRINT("\033[38;5;75m(SERVEUR) CREATION DE LA FILE DE MESSAGE...\033[0m");
  if((idQ = msgget(CLE,IPC_CREAT | IPC_EXCL | 0600)) == -1)  // CLE definie dans protocole.h
  {
    perror("(SERVEUR) Erreur de msgget");
    exit(1);
  }

  //Creation de la memoire partagée
  NORMAL_PRINT("\033[92m(SERVEUR) CREATION DE LA MEMOIRE PARTAGEE...\033[0m");
  if((idShm = shmget(CLE,51,IPC_CREAT | IPC_EXCL | 0777)) == -1)
  {
    perror("(SERVEUR) Erreur de shmget");
    exit(1);
  }
  //CREATION PUBLICITE
  /*******************
  ********************
  ********************
  *******************/
  {
    char* Pshm;
    Pshm = (char*)shmat(idShm,NULL,0);
    sprintf(Pshm,"Hello world!");
  }
  /*******************
  ********************
  ********************
  *******************/

  if((Publicite = fork()) == 0)
  {
    execl("./Publicite",NULL);
    exit(0);
  }

  // TO BE CONTINUED

  // Creation du pipe
  // TO DO

  // Initialisation du tableau de connexions
  tab = (TAB_CONNEXIONS*) malloc(sizeof(TAB_CONNEXIONS)); 

  for (int i=0 ; i<6 ; i++)
  {
    tab->connexions[i].pidFenetre = 0;
    strcpy(tab->connexions[i].nom,"");
    tab->connexions[i].pidCaddie = 0;
  }
  tab->pidServeur = getpid();
  tab->pidPublicite = Publicite;

  afficheTab();

  // Creation du processus Publicite (étape 2)
  // TO DO

  // Creation du processus AccesBD (étape 4)
  // TO DO
  bool verif;
  MESSAGE m;
  MESSAGE reponse;

  sigsetjmp(back,1);

  while(1)
  {
  	fprintf(stderr,"(SERVEUR) pid = %d : Attente d'une requete...\n",getpid());
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),1,0) == -1)
    {
      perror("(SERVEUR) Erreur de msgrcv");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
    }

    fprintf(stderr,"\033[H\033[J");
    switch(m.requete)
    {
      case CONNECT :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CONNECT reçue de %d\n",getpid(),m.expediteur);
                      verif = false;
                      for(auto& this_connexion: tab->connexions)
                      {
                        if(this_connexion.pidFenetre == 0)
                        {
                          this_connexion.pidFenetre = m.expediteur;
                          verif = true;
                          // Envoie d'un message de confirmation
                          reponse.type = m.expediteur;
                          reponse.requete = CONNECT;
                          reponse.expediteur = 1;
                          msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0);
                          kill(m.expediteur,SIGHUP);
                          break;
                        }
                      }
                      if(verif == false)
                      {
                        reponse.type = m.expediteur;
                        reponse.requete = DECONNECT;
                        reponse.expediteur = 1;
                        msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0);
                        kill(m.expediteur,SIGHUP);
                      }

                      break;

      case DECONNECT : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete DECONNECT reçue de %d\n",getpid(),m.expediteur);
                      for(auto& this_connexion: tab->connexions)
                      {
                        if(this_connexion.pidFenetre == m.expediteur)
                        {
                          this_connexion.pidFenetre = 0;
                          break;
                        }
                      }
                      break;
      case LOGIN :    // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete LOGIN reçue de %d : --%d--%s--%s--\n",getpid(),m.expediteur,m.data1,m.data2,m.data3);
                      if(m.data1 == 0)
                      {
                        NORMAL_PRINT("(SERVER) TENTATIVE DE LOGIN...");
                        int position = estPresent(m.data2);
                        
                        /* Si position > 0, alors on obtient la position du client
                        Si position = 0, alors on a pas trouvé le client
                        Si position = -1,alors une erreur es apparue */

                        if(position > 0)
                        {
                          int getHash = verifieMotDePasse(position,m.data3);
                          // retourne 1 si le mot de passe est correct
                          //          0 si le mot de passe est incorrect
                          //         -1 en cas d'erreur 
                          if(getHash == 1)
                          {
                            for(auto& this_connexion: tab->connexions)
                            {
                              if(this_connexion.pidFenetre == m.expediteur)
                              {
                                strcpy(this_connexion.nom,m.data2);
                                reponse.type = m.expediteur;
                                reponse.requete = LOGIN;
                                reponse.expediteur = 1;
                                reponse.data1 = 1;
                                sprintf(reponse.data4,"BONJOUR %s !",m.data2);

                                msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0);
                                kill(m.expediteur,SIGUSR1);
                                break;
                              }
                            }
                          }
                          else
                          {
                            reponse.type = m.expediteur;
                            reponse.requete = LOGIN;
                            reponse.expediteur = 1;
                            reponse.data1 = 0;
                            sprintf(reponse.data4,"MOT DE PASSE INCORRECT!");

                            msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0);
                            kill(m.expediteur,SIGUSR1);
                          }
                        }
                        else
                        {
                          reponse.type = m.expediteur;
                          reponse.requete = LOGIN;
                          reponse.expediteur = 1;
                          reponse.data1 = 0;
                          sprintf(reponse.data4,"NOM INCORRECT!");

                          msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0);
                          kill(m.expediteur,SIGUSR1);
                        }

                      }
                      else
                      {
                        NORMAL_PRINT("(SERVER) ENREGISTREMENT DU COMPTE...");

                        int position = estPresent(m.data2);
                        if(position > 0)
                        {
                          reponse.type = m.expediteur;
                          reponse.requete = LOGIN;
                          reponse.expediteur = 1;
                          reponse.data1 = 0;
                          sprintf(reponse.data4,"CLIENT ALREADY EXISTS!");

                          msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0);
                          kill(m.expediteur,SIGUSR1);
                        }
                        else
                        {
                          for(auto& this_connexion: tab->connexions)
                          {
                            if(this_connexion.pidFenetre == m.expediteur)
                            {                              
                              strcpy(this_connexion.nom,m.data2);
                              ajouteClient(m.data2,m.data3);

                              reponse.type = m.expediteur;
                              reponse.requete = LOGIN;
                              reponse.expediteur = 1;
                              reponse.data1 = 1;
                              sprintf(reponse.data4,"CLIENT CREATED!\nWELCOME %s",m.data2);

                              msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0);
                              kill(m.expediteur,SIGUSR1);
                              break;
                            }
                          }
                        }
                      }
                      if(reponse.data1 == 1)
                      {
                        for(auto& this_connexion: tab->connexions)
                        {
                          if(this_connexion.pidFenetre == m.expediteur)
                          {
                            if((this_connexion.pidCaddie = fork()) == 0)
                            {
                              execl("./Caddie",NULL);
                              exit(0);
                            }
                            break;
                          }
                        }
                      }
                      break; 

      case LOGOUT :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      for(auto& this_connexion: tab->connexions)
                      {
                        if(this_connexion.pidFenetre == m.expediteur)
                        {
                          if(strcmp(this_connexion.nom,m.data2) == 0)
                          {
                            sprintf(this_connexion.nom,"");
                            //kill(this_connexion.pidCaddie,SIGKILL);
                            reponse.type = this_connexion.pidCaddie;
                            reponse.requete = LOGOUT;
                            reponse.expediteur = 1;

                            msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0);
                            // this_connexion.pidCaddie = 0;
                            break;
                          }
                        }
                      }
                      break;

      case UPDATE_PUB :  // TO DO
                      struct tm* timeinfo;
                      time_t timer;
                      time(&timer);
                      timeinfo = localtime(&timer);
                      
                      sprintf(txt,"\033[38;5;11m(PUBLICITE) MISE A JOUR: \033[0m%dh:%dm:%ds",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
                      NORMAL_PRINT(txt);

                      for(auto& x: tab->connexions)
                      {
                        if(x.pidFenetre != 0)
                          kill(x.pidFenetre,SIGUSR2);
                      }

                      break;

      case CONSULT :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);

                      for(auto& i : tab->connexions)
                        if(i.pidFenetre == m.expediteur)
                        {
                          reponse.type = i.pidCaddie;
                          break;
                        }

                      reponse.data1 = m.data1;
                      reponse.requete = m.requete;

                      int isMsgSend = msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0);

                      if(isMsgSend == -1)
                      {
                        sprintf(print_msg,"(SERVEUR): Message CONSULT send Failed...");
                        ERROR_PRINT(print_msg);
                        exit(1);
                      }
                      
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

void handlerSIGINT(int)
{
  if(msgctl(idQ,IPC_RMID,NULL) == -1)
  {    
    ERROR_PRINT("IMPOSSIBLE DE SUPPRIMER LA FILE DE MESSAGES...");
    exit(1);
  }
  NORMAL_PRINT("\n\033[38;5;75m(SERVEUR) SUPPRESSION DE LA FILE DE MESSAGE...\033[0m");


  if(shmctl(idShm,IPC_RMID,NULL) == -1)
  {    
    ERROR_PRINT("IMPOSSIBLE DE SUPPRIMER LA MEMOIRE PARTAGEE...");
    exit(1);
  }
  NORMAL_PRINT("\033[92m(SERVEUR) SUPPRESSION DE LA MEMOIRE PARTAGEE...\033[0m");

  kill(Publicite,SIGKILL);
  NORMAL_PRINT("\033[38;5;11m(SERVEUR) SUPPRESSION DU PROCESSUS (PUBLICITE)...\033[0m");

  exit(0);
}

void handlerSIGCHLD(int)
{
  pid_t pidCaddie = wait(NULL);
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

