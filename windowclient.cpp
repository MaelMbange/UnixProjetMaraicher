#include "windowclient.h"
#include "ui_windowclient.h"
#include <QMessageBox>
#include <string>
using namespace std;

#include "protocole.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>

extern WindowClient *w;

int idQ, idShm;
bool logged;
char* pShm;
ARTICLE articleEnCours;
float totalCaddie = 0.0;
char Curr_Name[100];

char print_msg[100];

void handlerSIGUSR1(int sig);
void handlerSIGUSR2(int sig);

void handlerSIGINT(int sig);

#define REPERTOIRE_IMAGES "images/"

void handlerCONNEXION(int);

void handlerSIGINT(int sig)
{
  w->closeEvent(NULL);
}

void handlerSIGUSR2(int sig)
{
  w->setPublicite(pShm);
}



void handlerCONNEXION(int)
{
  MESSAGE msg;
  if(msgrcv(idQ,&msg,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
  {
    sprintf(print_msg,"(CLIENT) pid=%d : ERREUR MSGRCV",getpid());
    ERROR_PRINT(print_msg);
    exit(1);
  }
  if(msg.requete == CONNECT)
  {
    sprintf(print_msg,"(CLIENT) pid=%d : CONNEXION SUCCEED...",getpid());
    NORMAL_PRINT(print_msg);
    return;
  }
  if(msg.requete == DECONNECT)
  {
    sprintf(print_msg,"(CLIENT) pid=%d : CONNEXION REFUSED...",getpid());
    ERROR_PRINT(print_msg);    

    sprintf(print_msg,"(CLIENT) pid=%d : PROGRAM ENDING IN ...",getpid());
    for(int i = 5;i >= 0; i--)
    {
      sprintf(print_msg,"%d",i);
      ERROR_PRINT(print_msg);
      sleep(1);
    }
    
    exit(1);
  }
}

WindowClient::WindowClient(QWidget *parent) : QMainWindow(parent), ui(new Ui::WindowClient)
{
    ui->setupUi(this);

    // Configuration de la table du panier (ne pas modifer)
    ui->tableWidgetPanier->setColumnCount(3);
    ui->tableWidgetPanier->setRowCount(0);
    QStringList labelsTablePanier;
    labelsTablePanier << "Article" << "Prix à l'unité" << "Quantité";
    ui->tableWidgetPanier->setHorizontalHeaderLabels(labelsTablePanier);
    ui->tableWidgetPanier->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetPanier->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetPanier->horizontalHeader()->setVisible(true);
    ui->tableWidgetPanier->horizontalHeader()->setDefaultSectionSize(160);
    ui->tableWidgetPanier->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetPanier->verticalHeader()->setVisible(false);
    ui->tableWidgetPanier->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    // Recuperation de l'identifiant de la file de messages
    //fprintf(stderr,"(CLIENT %d) Recuperation de l'id de la file de messages\n",getpid());
    // TO DO
    idQ = msgget(CLE,0);
    if(idQ == -1)
    {
      sprintf(print_msg,"(CLIENT) pid=%d : Could not acces to the server...",getpid());
      ERROR_PRINT(print_msg);
      exit(1);
    }

    // Attachement à la mémoire partagée
    // TO DO
    fprintf(stderr,"(PUBLICITE %d) Recuperation de l'id de la memoire partagee\n",getpid());
    if((idShm = shmget(CLE,0,0)) == -1)
    {
      perror("(CLIENT) Erreur de shmget");
      exit(1);
    }

    fprintf(stderr,"(PUBLICITE %d) Attachement a la memoire partagee\n",getpid());
    if((pShm = (char*)shmat(idShm,NULL,SHM_RDONLY)) == (char*)-1)
    {
      perror("(CLIENT) Erreur de shmat");
      exit(1);
    }


    // Armement des signaux
    // TO DO
    //SIGUSR1 RECEPETION CONNEXION
    struct sigaction sig;
    sigemptyset(&sig.sa_mask);
    sig.sa_handler = handlerCONNEXION;
    sig.sa_flags = 0;
    sigaction(SIGHUP,&sig,NULL);

    sigemptyset(&sig.sa_mask);
    sig.sa_handler = handlerSIGUSR1;
    sig.sa_flags = 0;
    sigaction(SIGUSR1,&sig,NULL);

    sigemptyset(&sig.sa_mask);
    sig.sa_handler = handlerSIGUSR2;
    sig.sa_flags = 0;
    sigaction(SIGUSR2,&sig,NULL);

    sigemptyset(&sig.sa_mask);
    sig.sa_handler = handlerSIGINT;
    sig.sa_flags = 0;
    sigaction(SIGINT,&sig,NULL);

    /*******************************************
    Envoie d'un message de connexion au serveur
    *******************************************/
    MESSAGE msg; int isMsgSend;
    
    msg.type = 1;
    msg.requete = CONNECT;
    msg.expediteur = getpid();

    isMsgSend = msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0);

    if(isMsgSend == -1)
    {
      sprintf(print_msg,"(CLIENT) pid=%d : Message Connexion Failed...",getpid());
      ERROR_PRINT(print_msg);
      exit(1);
    }

    // pause();

    // NORMAL_PRINT("CLIENT CREE...");
    // Exemples à supprimer
    /* setPublicite("Promotions sur les concombres !!!");
    setArticle("pommes",5.53,18,"pommes.jpg");
    ajouteArticleTablePanier("cerises",8.96,2); */
}

WindowClient::~WindowClient()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setNom(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditNom->clear();
    return;
  }
  ui->lineEditNom->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getNom()
{
  strcpy(nom,ui->lineEditNom->text().toStdString().c_str());
  return nom;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setMotDePasse(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditMotDePasse->clear();
    return;
  }
  ui->lineEditMotDePasse->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getMotDePasse()
{
  strcpy(motDePasse,ui->lineEditMotDePasse->text().toStdString().c_str());
  return motDePasse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setPublicite(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditPublicite->clear();
    return;
  }
  ui->lineEditPublicite->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setImage(const char* image)
{
  // Met à jour l'image
  char cheminComplet[80];
  sprintf(cheminComplet,"%s%s",REPERTOIRE_IMAGES,image);
  QLabel* label = new QLabel();
  label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  label->setScaledContents(true);
  QPixmap *pixmap_img = new QPixmap(cheminComplet);
  label->setPixmap(*pixmap_img);
  label->resize(label->pixmap()->size());
  ui->scrollArea->setWidget(label);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::isNouveauClientChecked()
{
  if (ui->checkBoxNouveauClient->isChecked()) return 1;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setArticle(const char* intitule,float prix,int stock,const char* image)
{
  ui->lineEditArticle->setText(intitule);
  if (prix >= 0.0)
  {
    char Prix[20];
    sprintf(Prix,"%.2f",prix);
    ui->lineEditPrixUnitaire->setText(Prix);
  }
  else ui->lineEditPrixUnitaire->clear();
  if (stock >= 0)
  {
    char Stock[20];
    sprintf(Stock,"%d",stock);
    ui->lineEditStock->setText(Stock);
  }
  else ui->lineEditStock->clear();
  setImage(image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getQuantite()
{
  return ui->spinBoxQuantite->value();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setTotal(float total)
{
  if (total >= 0.0)
  {
    char Total[20];
    sprintf(Total,"%.2f",total);
    ui->lineEditTotal->setText(Total);
  }
  else ui->lineEditTotal->clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::loginOK()
{
  ui->pushButtonLogin->setEnabled(false);
  ui->pushButtonLogout->setEnabled(true);
  ui->lineEditNom->setReadOnly(true);
  ui->lineEditMotDePasse->setReadOnly(true);
  ui->checkBoxNouveauClient->setEnabled(false);

  ui->spinBoxQuantite->setEnabled(true);
  ui->pushButtonPrecedent->setEnabled(true);
  ui->pushButtonSuivant->setEnabled(true);
  ui->pushButtonAcheter->setEnabled(true);
  ui->pushButtonSupprimer->setEnabled(true);
  ui->pushButtonViderPanier->setEnabled(true);
  ui->pushButtonPayer->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::logoutOK()
{
  ui->pushButtonLogin->setEnabled(true);
  ui->pushButtonLogout->setEnabled(false);
  ui->lineEditNom->setReadOnly(false);
  ui->lineEditMotDePasse->setReadOnly(false);
  ui->checkBoxNouveauClient->setEnabled(true);

  ui->spinBoxQuantite->setEnabled(false);
  ui->pushButtonPrecedent->setEnabled(false);
  ui->pushButtonSuivant->setEnabled(false);
  ui->pushButtonAcheter->setEnabled(false);
  ui->pushButtonSupprimer->setEnabled(false);
  ui->pushButtonViderPanier->setEnabled(false);
  ui->pushButtonPayer->setEnabled(false);

  setNom("");
  setMotDePasse("");
  ui->checkBoxNouveauClient->setCheckState(Qt::CheckState::Unchecked);

  setArticle("",-1.0,-1,"");

  w->videTablePanier();
  totalCaddie = 0.0;
  w->setTotal(-1.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table du panier (ne pas modifier) /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::ajouteArticleTablePanier(const char* article,float prix,int quantite)
{
    char Prix[20],Quantite[20];

    sprintf(Prix,"%.2f",prix);
    sprintf(Quantite,"%d",quantite);

    // Ajout possible
    int nbLignes = ui->tableWidgetPanier->rowCount();
    nbLignes++;
    ui->tableWidgetPanier->setRowCount(nbLignes);
    ui->tableWidgetPanier->setRowHeight(nbLignes-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(article);
    ui->tableWidgetPanier->setItem(nbLignes-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Prix);
    ui->tableWidgetPanier->setItem(nbLignes-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Quantite);
    ui->tableWidgetPanier->setItem(nbLignes-1,2,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::videTablePanier()
{
    ui->tableWidgetPanier->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getIndiceArticleSelectionne()
{
    QModelIndexList liste = ui->tableWidgetPanier->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int indice = index.row();
    return indice;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue (ne pas modifier ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueMessage(const char* titre,const char* message)
{
   QMessageBox::information(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueErreur(const char* titre,const char* message)
{
   QMessageBox::critical(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CLIC SUR LA CROIX DE LA FENETRE /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::closeEvent(QCloseEvent *event)
{
  // TO DO (étape 1)
  // Envoi d'une requete DECONNECT au serveur

  MESSAGE msg; int isMsgSend;

  // envoi d'un logout si logged
  if(logged)
  {
    msg.type = 1;
    msg.requete = LOGOUT;
    msg.expediteur = getpid();
    sprintf(msg.data2,"%s",Curr_Name);

    isMsgSend = msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0);

    if(isMsgSend == -1)
    {
      sprintf(print_msg,"(CLIENT) pid=%d : Message Logout Failed...",getpid());
      ERROR_PRINT(print_msg);
      exit(1);
    }

    sprintf(print_msg,"(CLIENT) pid=%d : LOGOUT COMPLETED...",getpid());
    NORMAL_PRINT(print_msg);
  }
    
  msg.type = 1;
  msg.requete = DECONNECT;
  msg.expediteur = getpid();

  isMsgSend = msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0);

  if(isMsgSend == -1)
  {
    sprintf(print_msg,"(CLIENT) pid=%d : Message Deconnexion Failed...",getpid());
    ERROR_PRINT(print_msg);
    exit(1);
  }

  sprintf(print_msg,"(CLIENT) pid=%d : DECONNEXION COMPLETED...",getpid());
  NORMAL_PRINT(print_msg);  

  // Envoi d'une requete de deconnexion au serveur

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogin_clicked()
{
    // Envoi d'une requete de login au serveur
    // TO DO

  MESSAGE m;

  sprintf(m.data2,getNom());
  sprintf(m.data3,getMotDePasse());
  m.data1 = isNouveauClientChecked();
  m.requete = LOGIN;
  m.expediteur = getpid();
  m.type = 1;

  int isMsgSend = msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0);

  if(isMsgSend == -1)
  {
    sprintf(print_msg,"(CLIENT) pid=%d : Message Login Failed...",getpid());
    ERROR_PRINT(print_msg);
    exit(1);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogout_clicked()
{
    // Envoi d'une requete CANCEL_ALL au serveur (au cas où le panier n'est pas vide)
    // TO DO

    // Envoi d'une requete de logout au serveur
    // TO DO
    MESSAGE msg; int isMsgSend;

    msg.type = 1;
    msg.requete = LOGOUT;
    msg.expediteur = getpid();
    sprintf(msg.data2,"%s",Curr_Name);

    isMsgSend = msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0);

    if(isMsgSend == -1)
    {
      sprintf(print_msg,"(CLIENT) pid=%d : Message Logout Failed...",getpid());
      ERROR_PRINT(print_msg);
      exit(1);
    }

    sprintf(print_msg,"(CLIENT) pid=%d : LOGOUT COMPLETED...",getpid());
    NORMAL_PRINT(print_msg);

    logoutOK();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSuivant_clicked()
{
    // TO DO (étape 3)
    // Envoi d'une requete CONSULT au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPrecedent_clicked()
{
    // TO DO (étape 3)
    // Envoi d'une requete CONSULT au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonAcheter_clicked()
{
    // TO DO (étape 5)
    // Envoi d'une requete ACHAT au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSupprimer_clicked()
{
    // TO DO (étape 6)
    // Envoi d'une requete CANCEL au serveur

    // Mise à jour du caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);

    // Envoi requete CADDIE au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonViderPanier_clicked()
{
    // TO DO (étape 6)
    // Envoi d'une requete CANCEL_ALL au serveur

    // Mise à jour du caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);

    // Envoi requete CADDIE au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPayer_clicked()
{
    // TO DO (étape 7)
    // Envoi d'une requete PAYER au serveur

    char tmp[100];
    sprintf(tmp,"Merci pour votre paiement de %.2f ! Votre commande sera livrée tout prochainement.",totalCaddie);
    dialogueMessage("Payer...",tmp);

    // Mise à jour du caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);

    // Envoi requete CADDIE au serveur
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Handlers de signaux ////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handlerSIGUSR1(int sig)
{
    MESSAGE m;
    MESSAGE rep;
  
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) != -1)  // !!! a modifier en temps voulu !!!
    {
      switch(m.requete)
      {
        case LOGIN :
                    if(m.data1 == 1)
                    {
                      w->dialogueMessage("Login",m.data4);
                      sprintf(print_msg,"(CLIENT) pid=%d : LOGIN SUCCEED...",getpid());
                      NORMAL_PRINT(print_msg);
                      logged = true;
                      w->loginOK();
                      sprintf(Curr_Name,"%s",w->getNom());
                      
                      rep.type        = 1;
                      rep.expediteur  = getpid();
                      rep.requete     = CONSULT;
                      rep.data1       = 1;

                      int isMsgSend = msgsnd(idQ,&rep,sizeof(MESSAGE)-sizeof(long),0);
                      //NORMAL_PRINT("Message envoyé !!!!!!!");
                      if(isMsgSend == -1)
                      {
                        sprintf(print_msg,"(CLIENT) pid=%d : Message CONSULT send Failed...",getpid());
                        ERROR_PRINT(print_msg);
                        exit(1);
                      }
                    }
                    else if(m.data1 == 0)
                    {
                      logged = false;
                      w->dialogueErreur("Login",m.data4);
                      sprintf(print_msg,"(CLIENT) pid=%d : LOGIN FAILED...",getpid());
                      ERROR_PRINT(print_msg);
                    }
                    break;

        case CONSULT : // TO DO (étape 3)
                    break;

        case ACHAT : // TO DO (étape 5)
                    break;

         case CADDIE : // TO DO (étape 5)
                    break;

         case TIME_OUT : // TO DO (étape 6)
                    break;

         case BUSY : // TO DO (étape 7)
                    break;

         default :
                    break;
      }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////