#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "error.h"

#ifdef PADAWAN

void verification(int condition, char * message) {
  if(!condition)
    perror(message);
}

void map_new (unsigned width, unsigned height)
{
  map_allocate (width, height);

  for (int x = 0; x < width; x++)
    map_set (x, height - 1, 0); // Ground

  for (int y = 0; y < height - 1; y++) {
    map_set (0, y, 1); // Wall
    map_set (width - 1, y, 1); // Wall
  }

  map_object_begin (6);

  // Texture pour le sol
  map_object_add ("images/ground.png", 1, MAP_OBJECT_SOLID);
  // Mur
  map_object_add ("images/wall.png", 1, MAP_OBJECT_SOLID);
  // Gazon
  map_object_add ("images/grass.png", 1, MAP_OBJECT_SEMI_SOLID);
  // Marbre
  map_object_add ("images/marble.png", 1, MAP_OBJECT_SOLID | MAP_OBJECT_DESTRUCTIBLE);
  //Fleur
  map_object_add("images/flower.png", 1, MAP_OBJECT_AIR);
  //Piece
  map_object_add("images/coin.png", 20, MAP_OBJECT_AIR | MAP_OBJECT_COLLECTIBLE);

  map_object_end ();

}

void map_save (char *filename)
{
  //Récupération de la largeur.
  int width = map_width();
  printf("Largeur : %d\n", width);

  //Récupération de la hauteur.
  int height = map_height();
  printf("Hauteur : %d\n", height);

  //Récupération du nombre d'objets disponibles.
  int number_of_different_objects = map_objects();
  printf("Nombre d'objets : %d\n", number_of_different_objects);

  //Ouverture du fichier de sauvegarde en écriture.
  int fileDescriptor = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0600);
  verification(fileDescriptor != -1, "[map_save]open");

  //Variable de retour des appels systèmes write().
  int verif_write;

  //Inscription dans le fichier de sauvegarde de la largeur de la map.
  verif_write = write(fileDescriptor, &width, sizeof(int));
  verification(verif_write != -1, "[map_save]write_width");

  //Inscription dans le fichier de sauvegarde de la hauteur de la map.
  verif_write = write(fileDescriptor, &height, sizeof(int));
  verification(verif_write != -1, "[map_save]write_height");

  //Inscription dans le fichier de sauvegarde du nombre d'objets disponibles.
  verif_write = write(fileDescriptor, &number_of_different_objects, sizeof(int));
  verification(verif_write != -1, "[map_save]write_number_of_different_objects");

  //On parcourt la matrice correspondant à la map.
  for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
      //On récupère le type de l'objet en (x,y)
      int type = map_get(x,y);
      //Affichage de la matrice
      if(type == -1){
        printf("%d ", type);
      } else {
        printf(" %d ", type);
      }
      //On inscrit la matrice dans le fichier de sauvegarde.
      verif_write = write(fileDescriptor, &type, sizeof(int));
      verification(verif_write != -1, "[map_save]write_matrice");
    }
    printf("\n");
  }

  //Variables contenant les différentes données nécessaires.
  char * pathPngFile;
  int pathPngFileLength;
  unsigned frames;
  int properties[4];

  //On effectue les manipulations suivantes pour chaque type d'objet.
  for(int i = 0; i < number_of_different_objects; i++) {

      //On récupère le chemin du fichier en PNG.
      pathPngFile = map_get_name(i);
      printf("Chemin vers PNG : %s\n", pathPngFile);

      //On récupère la longueur du chemin vers le fichier PNG.
      pathPngFileLength = (int)strlen(pathPngFile);
      printf("Longueur du chemin : %d\n", pathPngFileLength);

      //Et le nombre de frames.
      frames = map_get_frames(i);
      printf("Number of frames : %d\n", frames);

      //Et les propriétés
      properties[0] = map_get_solidity(i);
      printf("Solidity : %d\n", properties[0]);
      properties[1] = map_is_destructible(i);
      printf("Is destructible : %d\n", properties[1]);
      properties[2] = map_is_collectible(i);
      printf("is collectible : %d\n", properties[2]);
      properties[3] = map_is_generator(i);
      printf("is generator : %d\n", properties[3]);

      //On inscrit la longueur du chemin.
      verif_write = write(fileDescriptor, &pathPngFileLength, sizeof(int));

      //Pour chaque caractère du chemin...
      for(int k = 0; k < pathPngFileLength; k++){
        //On l'inscrit dans le fichier de sauvegarde.
        verif_write = write(fileDescriptor, &pathPngFile[k],sizeof(char));
        verification(verif_write != -1, "[map_save]write_pathPngFile");
        printf("%c", pathPngFile[k]);
      }
      // !! On oublie pas le caractère de fin de chaîne !!
      verif_write = write(fileDescriptor, &pathPngFile[pathPngFileLength], sizeof(char));
      verification(verif_write != -1, "[map_save]write_pathPngFile");
      printf("\n");

      //On inscrit le nombre de frames du fichier PNG.
      verif_write = write(fileDescriptor, &frames, sizeof(unsigned));
      verification(verif_write != -1, "[map_save]write_frames");

      //On inscrit chaque propriété.
      for(int j = 0; j < 4; j++){
        verif_write = write(fileDescriptor, &properties[j], sizeof(int));
        verification(verif_write != -1, "[map_save]write_properties");
      }
  }
  //On ferme le fichier.
  close(fileDescriptor);
}

void map_load (char *filename)
{
   //Variables qui vont contenir les données extraites.
   int width;
   int height;
   int nb_objects;

   //Variable de retour des appels systèmes read().
   int verif_read;

   //On ouvre le fichier en lecture.
   int fd = open(filename, O_RDONLY);
   verification(fd != -1, "[map_load]open");

   //On lit la largeur de la map.
   verif_read = read(fd, &width, sizeof(int));
   verification(verif_read != -1, "[map_load]load width");
   printf("Largeur : %d\n", width);

   //On extrait la hauteur de la map.
   verif_read = read(fd, &height, sizeof(int));
   verification(verif_read != -1, "[map_load]load height");
   printf("Hauteur : %d\n", height);

   //On lit le nombre d'objets disponibles.
   verif_read = read(fd, &nb_objects, sizeof(int));
   verification(verif_read != -1, "[map_load]load nb objects");
   printf("Number of objects : %d\n", nb_objects);

   //On appelle la méthode permettant d'allouer un map.
   map_allocate (width, height);

   //Variable qui va contenir le type d'objet d'une case donnée.
   int type;

   //On parcourt la map...
   for(int y = 0; y < height; y++){
    for(int x = 0; x < width; x++){
        //On lit le type de la case (x, y).
        verif_read = read(fd, &type, sizeof(int));
        verification(verif_read != -1, "[map_load]load matrix");
        //Affichage de la matrice.
        if(type == -1){
            printf("%d ", type);
        } else {
            printf(" %d ", type);
        }
        //On appelle la méthode qui applique le type donné à la case (x, y).
        map_set(x, y, type);
    }
    printf("\n");
   }

  //Appel similaire dans la fonction map_new()
  map_object_begin (nb_objects);

  //Variables qui vont contenir les données de chaque type d'objet.
  char path_to_png[256];
  int path_length;
  unsigned frames;
  int properties[4] = {-1, -1, -1, -1};
  int flags;

  //Pour chaque type d'objet...
  for(int i = 0; i < nb_objects; i++){
      //On lit la longueur de la chaîne de caractères.
      verif_read = read(fd, &path_length, sizeof(int));
      verification(verif_read != -1, "[map_load]load path length");
      printf("Path length : %d\n", path_length);

      //Pour chaque caractère de la chaîne.
      for(int k = 0; k < path_length; k++){
        //On récupère le caractère.
        verif_read = read(fd, &path_to_png[k],sizeof(char));
        verification(verif_read != -1, "[map_load]load path to png");
        printf("%c", path_to_png[k]);
      }
      // !! On oublie toujours pas le caractère de fin de chaîne. !!
      verif_read = read(fd, &path_to_png[path_length],sizeof(char));
      verification(verif_read != -1, "[map_load]load path to png");
      printf("\n");

      //Récupération du nombre de frames dans le fichier PNG.
      verif_read = read(fd, &frames, sizeof(unsigned));
      verification(verif_read != -1, "[map_load]load nb frames");
      printf("Number of frames : %u\n", frames);

      //Récupération des propriétés
      for(int j = 0; j < 4; j++){
        verif_read = read(fd, &properties[j], sizeof(int));
        verification(verif_read != -1, "[map_load]load property");
      }

      //La fonction map_get_solidity renvoie la constante souhaitée.
      flags = properties[0];

      //Les autres fonctions de propriétés renvoient seulement 0 ou 1.
      //Donc on convertit les valeurs de retour en constantes.
      if(properties[1] == 1){
        flags = flags | MAP_OBJECT_DESTRUCTIBLE;
      }
      if(properties[2] == 1){
        flags = flags | MAP_OBJECT_COLLECTIBLE;
      }
      if(properties[3] == 1){
        flags = flags | MAP_OBJECT_GENERATOR;
      }

      //On appelle la méthode permettant d'ajouter un type d'objet avec les données ci-dessus.
      map_object_add(path_to_png, frames, flags);
  }
  //Appel similaire dans la fonction map_new()
  map_object_end ();

  //On ferme le fichier.
  close(fd);
}

#endif
