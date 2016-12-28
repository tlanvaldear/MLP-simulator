# MLP-simulator

J'ai modifié la structure du fichier de sauvegarde, il ressemble à ça:
- Largeur (int),
- Hauteur (int),
- Nombre d'objets disponibles (int),
- Matrice de la map (largeur x hauteur d'ints),
- Longueur du chemin vers le PNG. (int),
- Chemin vers le PNG ((longueur du chemin +1) chars)(Ne pas oublier le caractère de fin de chaîne, merci le C :trollface:),
- Nombre de frames (unsigned),
- Les propriétés du type d'objet (4 ints)

TO DO LIST:
- [x] Map_save()
- [x] Map_load()
- Je vais rajouter les autres :ok_hand: :wink:
  
