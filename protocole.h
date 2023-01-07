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

#define CLE 1234
#define SERVEUR       1

//      requete             sens      data1           data2         data3          data4          data5
#define CONNECT       1  // Cl -> S
#define DECONNECT     2  // Cl -> S
#define LOGIN         3  // Cl -> S   1 ou 0          nom           mot de passe
//                          S  -> Cl  1 ou 0                                       reponse en phrase pour l'utilisateur
#define LOGOUT        4  // Cl -> S 
#define UPDATE_PUB    5  // P  -> S
#define CONSULT       6  // Cl -> S   idArticle
//                          S  -> Ca  idArticle
//                          Ca -> BD  idArticle
//                          BD -> Ca  idArticle ou -1 intitule      stock          image          prix
//                          Ca -> Cl  idArticle       intitule      stock          image          prix
#define ACHAT         7  // Cl -> S   idArticle       quantite
//                          S  -> Ca  idArticle       quantite
//                          Ca -> BD  idArticle       quantite
//                          BD -> Ca  idArticle       intitule      quantite ou 0  image          prix
//                          Ca -> Cl  idArticle       intitule      quantite ou 0  image          prix
#define CADDIE        8  // Cl -> S   
//                          S  -> Ca
//                          Ca -> Cl  idArticle       intitule      quantite       image          prix
#define CANCEL        9  // Cl -> S   indiceArticle
//                          S  -> Ca  indiceArticle
//                          Ca -> BD  idArticle       quantite
#define CANCEL_ALL   10  // Cl -> S
//                          S  -> Ca
#define PAYER        11  // Cl -> S
//                       // S  -> Ca
#define TIME_OUT     12  // Ca -> Cl
#define BUSY         13  // S  -> Cl
#define NEW_PUB      14  // Ge -> S                                                publicite
//                          S  -> P                                                publicite
#define ISBUSY       15


typedef struct 
{
  long  type;
  int   expediteur;
  int   requete;
  int   data1;
  char  data2[20];
  char  data3[20];
  char  data4[100];
  float data5;
} MESSAGE;

typedef struct
{
  int   pidFenetre;
  char  nom[20];
  int   pidCaddie;
} CONNEXION;

typedef struct
{
  int   id;
  char  intitule[20];
  float prix;
  int   stock;  
  char  image[20];
} ARTICLE;

typedef struct
{
  int   pidServeur;
  int   pidPublicite;
  int   pidAccesBD;
  CONNEXION connexions[6];
} TAB_CONNEXIONS;


#define ERROR_PRINT(x) fprintf(stderr,"\e[91mERROR: %s \e[0m\n",x)
#define NORMAL_PRINT(x) fprintf(stderr,"\e[94m%s \e[0m\n",x)

/*
+----------+-------------+------+-----+---------+----------------+
| Field    | Type        | Null | Key | Default | Extra          |
+----------+-------------+------+-----+---------+----------------+
| id       | int         | NO   | PRI | NULL    | auto_increment |
| intitule | varchar(20) | YES  |     | NULL    |                |
| prix     | float       | YES  |     | NULL    |                |
| stock    | int         | YES  |     | NULL    |                |
| image    | varchar(20) | YES  |     | NULL    |                |
+----------+-------------+------+-----+---------+----------------+
*/
//****************************
//FILE DE MESSAGE
//****************************
int createMessageQueue(int Key, const char* msg = "Erreur de creation de la file de message.");
int deleteMessageQueue(int msgfd,const char* msg = "Erreur de suppression de la file de message.");
int getMessageQueue(int Key, const char* msg = "Erreur recuperation de la file de message.");
int sendMessageQueue(int msgfd, const MESSAGE message, const char* msg = "Erreur envoie message.");
int recieveMessageQueue(int msgfd, MESSAGE& message, pid_t pid, const char* msg = "Erreur reception message.");

void clearMessage(MESSAGE& msg);
void makeMessage(MESSAGE& msg, long type, int expediteur, int requete, int data1,const char* data2,const char* data3,const char* data4, float prix);
void makeMessageBasic(MESSAGE& msg, long type, int expediteur, int requete);
void makeMessageData(MESSAGE& msg,int data1 =0, const char* data2=NULL,const char* data3=NULL,const char* data4=NULL, float prix=0);
void printMessage(MESSAGE& msg,bool red = false);

int createMessageQueue(int Key, const char* msg)
{
  int msgfd = msgget(Key,IPC_CREAT | IPC_EXCL | 0777);
  if(msgfd == -1)
  {
    ERROR_PRINT(msg);
    exit(1);
  }
  return msgfd;
}
int deleteMessageQueue(int msgfd,const char* msg)
{
  int resultat = msgctl(msgfd, IPC_RMID,NULL);
  if(resultat == -1)
  {  
    ERROR_PRINT(msg);
    exit(1);
  }
  return resultat;
}
int getMessageQueue(int Key, const char* msg)
{
  int msgfd = msgget(Key,0);
  if(msgfd == -1)
  {
    ERROR_PRINT(msg);
    exit(1);
  }
  return msgfd;
}
int sendMessageQueue(int msgfd, const MESSAGE message, const char* msg)
{
  int send;
  if((send=msgsnd(msgfd,&message,sizeof(message)-sizeof(long),0))==-1)
  {
    ERROR_PRINT(msg);
    exit(1);
  }
  return send;
}
int recieveMessageQueue(int msgfd, MESSAGE& message, pid_t pid,const char* msg)
{
  int send = msgrcv(msgfd,&message,sizeof(message)-sizeof(long),pid,0);
  if(send == -1)
  {
    ERROR_PRINT(msg);
    exit(1);
  }
  return send;
}

void clearMessage(MESSAGE& msg)
{
  bzero(&msg,sizeof(msg));
}
void makeMessage(MESSAGE& msg, long type, int expediteur, int requete, int data1 =0,const char* data2=NULL,const char* data3=NULL,const char* data4=NULL, float prix=0)
{
  bzero(&msg,sizeof(msg));
  //NORMAL_PRINT("NETTOYAGE REUSSI");
  msg.type        = type;
  msg.expediteur  = expediteur;
  msg.requete     = requete;
  msg.data1       = data1;
  msg.data5       = prix;
  if(data2 != NULL)
    strcpy(msg.data2,data2);
  if(data3 != NULL)
    strcpy(msg.data3,data3);
  if(data4 != NULL)
    strcpy(msg.data4,data4);
}
void makeMessageBasic(MESSAGE& msg, long type, int expediteur, int requete)
{
  //bzero(&msg,sizeof(msg));
  //NORMAL_PRINT("NETTOYAGE REUSSI");
  msg.type        = type;
  msg.expediteur  = expediteur;
  msg.requete     = requete;
}
void makeMessageData(MESSAGE& msg,int data1 , const char* data2,const char* data3,const char* data4, float prix)
{
  msg.data1       = data1;
  msg.data5       = prix;
  if(data2 != NULL)
    strcpy(msg.data2,data2);
  if(data3 != NULL)
    strcpy(msg.data3,data3);
  if(data4 != NULL)
    strcpy(msg.data4,data4);
}

void printMessage(MESSAGE& msg,bool red)
{
  char txt[300];

  snprintf(txt,300,"MESSAGE :\n"
          "type       : %ld\n"
          "expediteur : %d\n"
          "requete    : %d\n"
          "data1      : %d\n"
          "data2      : %s\n"
          "data3      : %s\n"
          "data4      : %s\n"
          "data5      : %.2f\n"
          ,msg.type
          ,msg.expediteur
          ,msg.requete
          ,msg.data1
          ,msg.data2
          ,msg.data3
          ,msg.data4
          ,msg.data5
          );
  if(!red)
    NORMAL_PRINT(txt);
  else
    ERROR_PRINT(txt);
}


//****************************
//MEMOIRE PARTAGÉE
//****************************

int createSharedMemory(int Key,size_t size ,const char* msg = "Erreur de creation de la memoire partagee.");
void deleteSharedMemory(int shmfd,const char* msg = "Erreur de suppression de la memoire partagee.");
int getSharedMemory(int Key,const char* msg = "Erreur de recuperation de la memoire partagee.");
char* connectSharedMemory(int shmfd,const char* flag = "RD",const char* msg = "Erreur de connexion de la memoire partagee.");

int createSharedMemory(int Key, size_t size,const char* msg)
{
  int shmfd = shmget(Key,size,IPC_CREAT | IPC_EXCL | 0777);
  if(shmfd == -1)
  {
    ERROR_PRINT(msg);
    exit(1);
  }
  return shmfd;
}
void deleteSharedMemory(int shmfd,const char* msg)
{
  int res = shmctl(shmfd,IPC_RMID,NULL);
  if(res == -1)
  {
    ERROR_PRINT(msg);
    exit(1);
  }
}
int getSharedMemory(int Key,const char* msg)
{
  int shmfd = shmget(Key,0,0);
  if(shmfd == -1)
  {
    ERROR_PRINT(msg);
    exit(1);
  }
  return shmfd;
}

char* connectSharedMemory(int shmfd,const char* flag,const char* msg)
{
  char* pshm;
  if(strcmp(flag,"RD") == 0)
  {
    pshm = (char*)shmat(shmfd,NULL,SHM_RDONLY);
    if(pshm == (char*)-1)
    {
      ERROR_PRINT(msg);
      exit(1);
    }
    return pshm;
  }
  else if(strcmp(flag,"RW") == 0 || flag == NULL)
  {
    pshm = (char*)shmat(shmfd,NULL,0);
    if(pshm == (char*)-1)
    {
      ERROR_PRINT(msg);
      exit(1);
    }
    return pshm;
  }
  else
  {
    ERROR_PRINT("Erreur de connexion de la memoire partagee : bad flag");
    exit(1);
  }
  return NULL;
}

//****************************
// PIPE
//****************************
void closePipe(int* vec,const char* msg = "Erreur fermeture du pipe.");

void closePipe(int* vec,const char* msg)
{
  if(close(vec[1]) == -1 || close(vec[0]) == -1)
  {
    ERROR_PRINT(msg);
    exit(1);
  }
}

//****************************
// SEMAPHORE
//****************************

typedef union semun
{
int val;
struct semid_ds *buf;
unsigned short *array;
} semun;


int createSemaphore(int Key,int nbr);
int setValueSemaphore(int idSem, int num = 0,int arg = 1);
int getSemaphore(int Key);
int deleteSemaphore(int idSem);
int sem_wait(int idSem,int num,int flag = SEM_UNDO);
int sem_signal(int idSem,int num);

int createSemaphore(int Key,int nbr)
{
  int idSem = semget(Key,nbr,IPC_CREAT | IPC_EXCL | 0777);
  if(idSem == -1)
  {
    ERROR_PRINT("Erreur creation semaphore");
    exit(1);
  }
  return idSem;
}
int setValueSemaphore(int idSem, int num, int arg)
{
  int res = semctl(idSem,num,SETVAL,arg);
  if(res == -1)
  {
    ERROR_PRINT("Erreur Set valeur semaphore");
    exit(1);
  }
  return res;
}
int getSemaphore(int Key)
{
  int idSem = semget(Key,0,0);
  if(idSem == -1)
  {
    ERROR_PRINT("Erreur Recuperation semaphore");
    exit(1);
  }
  return idSem;
}
int deleteSemaphore(int idSem)
{
  int res = semctl(idSem,0,IPC_RMID,NULL);
  if(res == -1)
  {
    ERROR_PRINT("Erreur Suppression semaphore");
    exit(1);
  }
  return res;
}
int sem_wait(int idSem,int num,int flag)
{
  struct sembuf action;
  action.sem_num = num;
  action.sem_op = -1;
  action.sem_flg = flag;
  return semop(idSem,&action,1);
}
int sem_signal(int idSem,int num)
{
  struct sembuf action;
  action.sem_num = num;
  action.sem_op = +1;
  action.sem_flg = SEM_UNDO;
  return semop(idSem,&action,1);
}
