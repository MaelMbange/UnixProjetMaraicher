#ifndef FICHIER_CLIENT_H
#define FICHIER_CLIENT_H

#define FICHIER_CLIENTS "clients.dat"

// Pour le fichier des clients
typedef struct
{
  char  nom[20];
  int   hash;
} CLIENT;

int estPresent(const char* nom);
// retourne -1 en cas d'erreur
//           0 si pas trouve
//           la position (1,2,3, ...) dans le fichier si trouve

int hash(const char* motDePasse);
// calcul le hash du mot de passe = (somme ponderee des codes ASCII) % 97

void ajouteClient(const char* nom, const char* motDePasse);
// ajoute un nouveau client à la fin du fichier
// crée le fichier si celui-ci n'existe pas

int verifieMotDePasse(int pos, const char* motDePasse);
// reçoit la position du client obligatoirement présent dans le fichier et un mot de passe
// retourne 1 si le mot de passe est correct
//          0 si le mot de passe est incorrect
//         -1 en cas d'erreur 

int listeClients(CLIENT *vecteur);
// reçoit l'adresse d'un vecteur de clients suffisament grand pour recevoir le contenu du fichier
// retourne le nombre de clients présents dans le fichier
//          -1 si le fichier n'existe pas


int signin(const char* name, const char* password);
//  recoit le nom et le mdp
//  si 1 alors la personne à été rajoutée au fichier.
//  si -1 alors la personne est deja présente dans le fichier
//  donc pas d'ajout.
int logIn(const char* name,const char* password);
// Verifie si une personne peut se log ou non.
// si -1 => la personne n'existe pas.
// si -2 => mdp incorrecte.
// si 1 => mdp correcte.

#endif
