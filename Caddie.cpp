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
#include <mysql.h>
#include "protocole.h" // contient la cle et la structure d'un message

int idQ;

ARTICLE articles[10] = {0};
int nbArticles = 0;

int fdWpipe;
int pidClient;

MYSQL* connexion;

void handlerSIGALRM(int sig);

int main(int argc,char* argv[])
{
  // Masquage de SIGINT
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  // Armement des signaux
  // TO DO
  struct sigaction sig;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;
  sig.sa_handler = handlerSIGALRM;
  sigaction(SIGALRM,&sig,NULL);

  
  //********************************************************
  // Recuperation de l'identifiant de la file de messages
  //********************************************************
  fprintf(stderr,"(CADDIE %d) Recuperation de l'id de la file de messages\n",getpid());
  idQ = getMessageQueue(CLE);

  
  //********************************************************
  // Connexion à la base de donnée
  //********************************************************
  // connexion = mysql_init(NULL);
  // if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  // {
  //   fprintf(stderr,"(SERVEUR) Erreur de connexion à la base de données...\n");
  //   exit(1);  
  // }



  //********************************************************
  // Récupération descripteur écriture du pipe
  //********************************************************
  fdWpipe = atoi(argv[1]);


  MESSAGE m;
  MESSAGE reponse;
  
  char requete[200];
  char newUser[20];
  MYSQL_RES  *resultat;
  MYSQL_ROW  Tuple;
  int tempsRestant;

  //********************************************************
  // Debut du pc
  //********************************************************

  while(1)
  {

    //****************************
    // Mise en place du timer
    //****************************
    ERROR_PRINT("DEBUT ALARM");
    alarm(10);

    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
    {
      perror("(CADDIE) Erreur de msgrcv");
      exit(1);
    }

    ERROR_PRINT("FIN ALARM");
    tempsRestant = alarm(0);

    switch(m.requete)
    {
      case LOGIN :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete LOGIN reçue de %d\n",getpid(),m.expediteur);
                      pidClient = m.expediteur;
                      break;

      case LOGOUT :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      // mysql_close(connexion);
                      exit(0);
                      break;

      case CONSULT :  // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      
                    /*   sprintf(requete,"select * from  UNIX_FINAL;");
                      if (mysql_query(connexion,requete) != 0)
                      {
                        fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
                        exit(1);
                      }

                     if((resultat = mysql_store_result(connexion)) == NULL)
                     {
                       fprintf(stderr, "Erreur de mysql_store_result: %s\n",mysql_error(connexion));
                       mysql_close(connexion);
                       exit(1);
                     }

                      while((Tuple = mysql_fetch_row(resultat)) != NULL)
                      {
                        if(atoi(Tuple[0]) == m.data1)
                        {
                          clearMessage(reponse);
                          makeMessageBasic(reponse,m.expediteur,getpid(),CONSULT);
                          makeMessageData(reponse,atoi(Tuple[0]),Tuple[1],Tuple[3],Tuple[4],atoi(Tuple[2]));
                          printMessage(reponse);
                          sendMessageQueue(idQ,reponse);
                          kill(m.expediteur,SIGUSR1);
                        } 
                      }  */
                      m.expediteur = getpid();
                      m.type       = 1;  

                      // NORMAL_PRINT("(CADDIE) Ecriture sur le pipe.");
                      write(fdWpipe,&m,sizeof(m));

                      // NORMAL_PRINT("(CADDIE) Attente reception message.");
                      recieveMessageQueue(idQ,reponse,getpid(),"(Caddie) Erreur rcv : L31");

                      // NORMAL_PRINT("(CADDIE) Reception du message de ACCESBD");
                      if(reponse.data1 != -1)
                      {
                        reponse.type = pidClient;
                        reponse.expediteur = getpid();
                        // NORMAL_PRINT("(CADDIE) Envoie message vers client");
                        sendMessageQueue(idQ,reponse,"(CADDIE) Erreur envoie message");
                        // printMessage(reponse);
                        kill(pidClient,SIGUSR1);
                      }
                      // NORMAL_PRINT("(CADDIE) fin consult");
                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);

                      //********************************************************
                      // on transfert la requete à AccesBD
                      //********************************************************
                      m.expediteur = getpid();
                      printMessage(m,true);

                      write(fdWpipe,&m,sizeof(m));

                      // on attend la réponse venant de AccesBD
                      clearMessage(reponse);
                      recieveMessageQueue(idQ,reponse,getpid(),"(Caddie) erreur message recu L159");
                                              
                      // Envoi de la reponse au client
                      if(strcmp(reponse.data3,"0") != 0)
                      {

                        for(auto& i : articles)
                        {
                          if(i.id == reponse.data1)
                          {
                            i.stock += atoi(reponse.data3);
                            break;
                          }
                          else if(i.id == 0)
                          {
                            i.id = reponse.data1;
                            strncpy(i.intitule,reponse.data2,20);
                            i.stock = atoi(reponse.data3);
                            strncpy(i.image,reponse.data4,20);
                            i.prix = reponse.data5;
                            ++nbArticles;
                            break;
                          }
                        }
                      }
                      
                      reponse.type = pidClient;
                      reponse.expediteur = getpid();
                      sendMessageQueue(idQ,reponse);
                      kill(pidClient,SIGUSR1);

                      break;

      case CADDIE :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                      
                      for(const auto& i : articles)
                      {
                        if(i.id != 0)
                        {
                          NORMAL_PRINT("ENVOIE REQUETE CADDIE");
                          char stock[20];
                          sprintf(stock,"%d",i.stock);
                          clearMessage(reponse);
                          makeMessage(reponse,pidClient,getpid(),CADDIE,
                                      i.id,
                                      i.intitule,
                                      stock,
                                      i.image,
                                      i.prix);
                          sendMessageQueue(idQ,reponse);
                          kill(pidClient,SIGUSR1); 
                          printMessage(reponse); 
                          usleep(10'000);
                        }                      
                      }

                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);

                      // on transmet la requete à AccesBD
                      clearMessage(reponse);
                      makeMessageBasic(reponse,getpid(),getpid(),CANCEL);                      
                      reponse.data1 = articles[m.data1].id;
                      sprintf(reponse.data3,"%d",articles[m.data1].stock);
                      write(fdWpipe,&reponse,sizeof(reponse));

                      clearMessage(reponse);

                      articles[m.data1] = {0};
                      nbArticles--;
                      // Suppression de l'aricle du panier
                      break;

      case CANCEL_ALL : // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);
                      // On envoie a AccesBD autant de requeres CANCEL qu'il y a d'articles dans le panier
                                            
                      for(auto& i : articles)
                      {
                        if(i.id != 0)
                        {
                          clearMessage(reponse);
                          makeMessageBasic(reponse,getpid(),getpid(),CANCEL);
                          reponse.data1 = i.id;
                          sprintf(reponse.data3,"%d",i.stock);
                          write(fdWpipe,&reponse,sizeof(reponse));
                          // On vide le panier
                          i = {0};
                        }
                      }                      
                      nbArticles = 0;
                      break;

      case PAYER :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);

                      // On vide le panier
                      for(auto& i : articles)
                      {
                        i = {0};
                      }
                      nbArticles = 0;
                      break;
    }
  }
}

void handlerSIGALRM(int sig)
{
  fprintf(stderr,"(CADDIE %d) Time Out !!!\n",getpid());
  
  // Annulation du caddie et mise à jour de la BD
  // On envoie a AccesBD autant de requetes CANCEL qu'il y a d'articles dans le panier
  MESSAGE reponse;
  for(auto& i : articles)
  {
    if(i.id != 0)
    {
      clearMessage(reponse);
      makeMessageBasic(reponse,getpid(),getpid(),CANCEL);
      reponse.data1 = i.id;
      sprintf(reponse.data3,"%d",i.stock);
      write(fdWpipe,&reponse,sizeof(reponse));
      // On vide le panier
      i = {0};
    }
  }                      
  nbArticles = 0;
  // Envoi d'un Time Out au client (s'il existe toujours)

  if(kill(pidClient,0) != 0)
  {
    NORMAL_PRINT("LE CLIENT N'EXISTE PLUS!");
    exit(0);
  }

  clearMessage(reponse);
  makeMessageBasic(reponse,pidClient,getpid(),TIME_OUT);
  sendMessageQueue(idQ,reponse);
  kill(pidClient,SIGUSR1);

  exit(0);
}