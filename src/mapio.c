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
  int width = map_width();
  int height = map_height();
  int number_of_different_objects = map_objects();

  int fileDescriptor = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0600);
  verification(fileDescriptor != -1, "[map_save]open");
  //écriture de la largeur, de la hauteur et du nombre d'objets différent de la carte
  int verif_write;
  verif_write = write(fileDescriptor, &width, sizeof(int));
  verification(verif_write != -1, "[map_save]write_width");
  verif_write = write(fileDescriptor, &height, sizeof(int));
  verification(verif_write != -1, "[map_save]write_height");
  verif_write = write(fileDescriptor, &number_of_different_objects, sizeof(int));
  verification(verif_write != -1, "[map_save]write_number_of_different_objects");
  //TODO augmenter de 1 si la bombe est un type 
  int tab[7] = {-2, -2, -2, -2, -2, -2, -2};

  for(int x = 0; x < width; x++) {
    for(int y = 0; y < height; y++) {
      //on récupère le type de l'objet en (x,y)
      int type = map_get(x,y);
      //stocke les différents types d'objet de la map
      for(int i = 0; i < 7; i++) {
        if(!(tab[i] == type)) {
         if(type == -1)
            tab[6] = -1;
         else
            tab[type] = type;
        }
      }
      //on écrit la matrice
      verif_write = write(fileDescriptor, &type, sizeof(int));
      verification(verif_write != -1, "[map_save]write_matrice");
    }
  }

  //max caractères = 17 (exemple: images/ground.png)
  char * pathPngFile;
  unsigned frames;
  int properties[4];

  //pour chaque type d'objet (de notre tableau)...
  for(int i = 0; i < 7; i++) {
    //existant.. (-1)
    if(tab[i] != -2 && tab[i] != -1) {
      //on récupère le chemin du fichier en png
      pathPngFile = map_get_name(tab[i]);
      
      //pour faciliter le chargement
      for(int j = strlen(pathPngFile); j < 17; j++) {
        strcat(pathPngFile, "0");
      }
      //et le nombre de frames
      frames = map_get_frames(tab[i]);
      //et les propriétés
      properties[0] = map_get_solidity(tab[i]);
      properties[1] = map_is_destructible(tab[i]);
      properties[2] = map_is_collectible(tab[i]);
      properties[3] = map_is_generator(tab[i]);

      //on écrit toutes les informations récupérée
      verif_write = write(fileDescriptor, &pathPngFile, sizeof(char*));
      verification(verif_write != -1, "[map_save]write_pathPngFile");
      verif_write = write(fileDescriptor, &frames, sizeof(unsigned));
      verification(verif_write != -1, "[map_save]write_frames");
      verif_write = write(fileDescriptor, &properties, sizeof(int *));
      verification(verif_write != -1, "[map_save]write_properties");
    }
  }
}

void map_load (char *filename)
{
  // TODO
  exit_with_error ("Map load is not yet implemented\n");
}

#endif
