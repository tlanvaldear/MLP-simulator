# MLP-simulator

J'ai modifié la structure du fichier de sauvegarde, il ressemble à ça:
- Largeur (int),
- Hauteur (int),
- Nombre d'objets disponibles (int),
- Matrice de la map (largeur x hauteur d'ints),
- Pour chaque type d'objet disponible:
  - Longueur du chemin vers le PNG. (int),
  - Chemin vers le PNG ((longueur du chemin +1) chars)(Ne pas oublier le caractère de fin de chaîne, merci le C :trollface:),
  - Nombre de frames (unsigned),
  - Les propriétés du type d'objet (4 ints)

TO DO LIST:
- Sauvegarde et chargement des cartes :
  - [x] Sauvegarde
  - [x] Chargement
  - [x] Informations élémentaires (e.g. maputil –getinfo)
  - [x] Modification de la taille de la carte (debug à faire dans --setheight)
  - [x] Remplacement des objets d’une carte
  - [x] Suppression des objets inutilisés (modification de la matrice à faire, je l'ai oubliée :grimacing:)
- Gestion des temporisateurs :
  - [ ] Réception des signaux par un thread démon
  - [ ] Implémentation simple (un temporisateur à la fois)
  - [ ] Implémentation complète (et protocole de test)
  - [ ] Mise en service dans le jeu
  
