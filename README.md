# MLP-simulator

Pour que l'on soit tous d'accord sur la sauvegarde (et le chargement) des maps:
Ce qui me semble bien : (fichier type)
-Largeur
-Hauteur
-Nombre d'objets différents sur la map
-Matrice d'entier = contenu de chaque case de la map (exemple)
0 0 0 0 0 0 0
0 0 0 0 0 0 0
1 1 1 1 1 1 1 
-Type d'objet
NomFichierImage NbSprites Propriétés(type 1)
NomFichierImage NBSprites Propriétés(type 2)
...(pour tous les types présents)

Pour le nom comme le nb de caractères est variable, on prend la taille max et si c'est plus court on comble avec du blanc (ou des zéros)
Le nombre de sprites assez simple, c'est juste un entier
Les propriétès sont variables suivant l'objet, c'est des chaînes de caractères
