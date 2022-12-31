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
bool found;
MYSQL* connexion;

int main(int argc,char* argv[])
{
  // Masquage de SIGINT
  sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);
 

  //********************************************************
  // Recuperation de l'identifiant de la file de messages
  //********************************************************
  fprintf(stderr,"(ACCESBD %d) Recuperation de l'id de la file de messages\n",getpid());
  idQ = getMessageQueue(CLE);

  //********************************************************
  // Récupération descripteur écriture du pipe
  //********************************************************
  int fdRpipe = atoi(argv[1]);

  //********************************************************
  // Connexion à la base de donnée
  //********************************************************
  connexion = mysql_init(NULL);
  if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(SERVEUR) Erreur de connexion à la base de données...\n");
    exit(1);  
  }

  MESSAGE m;
  MESSAGE reponse;

  char requete[200];
  char newUser[20];
  MYSQL_RES  *resultat;
  MYSQL_ROW  Tuple;

  while(1)
  {
    
    //********************************************************
    // Lecture d'une requete sur le pipe
    //********************************************************
    if(read(fdRpipe,&m,sizeof(m)) != sizeof(m))
    {
      ERROR_PRINT("Erreur de lecture dans le pipe.");
      mysql_close(connexion);
      exit(1);
    }
    // NORMAL_PRINT("(ACCESBD) READ ENTREE LUE.");

    clearMessage(reponse);
    // ERROR_PRINT("(ACCESBD) print MESSAGE.");
    // printMessage(m,true);

    switch(m.requete)
    {
      case CONSULT :  // TO DO
                      // Acces BD
                      fprintf(stderr,"(ACCESBD %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      
                      sprintf(requete,"select * from  UNIX_FINAL;");
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
                      found = false;
                      while((Tuple = mysql_fetch_row(resultat)) != NULL)
                      {
                        // fprintf(stderr, "NO ID = %d\n",atoi(Tuple[0])); 
                        if(atoi(Tuple[0]) == m.data1)
                        {                                       
                          found = true;
                          break;
                        } 
                      }
                      //*****************************
                      // Preparation de la reponse
                      //*****************************
                      clearMessage(reponse);
                      if(found)
                      {
                        makeMessageBasic(reponse,m.expediteur,getpid(),CONSULT);
                        makeMessageData(reponse,atoi(Tuple[0]),Tuple[1],Tuple[3],Tuple[4],atoi(Tuple[2]));
                      }
                      else
                      {
                        makeMessageBasic(reponse,m.expediteur,getpid(),CONSULT);
                        makeMessageData(reponse,-1);
                      }
                      //*************************************
                      // Envoi de la reponse au bon caddie
                      //*************************************
                      // NORMAL_PRINT("(ACCESBD) ENVOIE MESSAGE");
                      // printMessage(reponse,true);
                      sendMessageQueue(idQ,reponse,"(ACCESBD) Erreur envoie message.");
                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD

                      // Finalisation et envoi de la reponse
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD

                      // Mise à jour du stock en BD
                      break;

      case DECONNECT:
                    NORMAL_PRINT("(ACCESBD) Fin du programme.");
                    exit(1);
                    break;

    }
  }
}
