#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>

#include "../include/map.h"

void verification(int condition, char * message) {
	if(!condition)
		perror(message);
}

int main (int argc, char** argv)
{
	//on vérifie la bonne utilisation de la commande
	if (argc < 3){
		fprintf(stderr,"Usage : %s <file> <option>",argv[0]);
		exit(2);
	}
	ssize_t r;//variable de vérification de la valeur de retour de read()
	//utilisation de l'option --getwidth
	if(strcmp(argv[2], "--getwidth") == 0) {
		//on ouvre le fichier passé en paramètre
		int fd = open(argv[1],O_RDONLY);
		int width;
		//on lit la valeur de la largeur de la carte
		if ((r = read(fd,&width,sizeof(int))) == 0) return 1;
		//on l'affiche
		printf("Largeur : %d\n", width);
		close(fd);
		return 0;
	}
	//utilisation de l'option --getheight
	else if (strcmp(argv[2], "--getheight") == 0) {
		//on ouvre le fichier passé en paramètre
		int fd = open(argv[1],O_RDONLY);
		int height;
		lseek(fd,sizeof(int),SEEK_SET); //On se deplace d'un entier dont on ne connait pas la valeur
		//on lit la valeur de la hauteur de la carte
		if ((r = read(fd,&height,sizeof(int))) == 0) return 1;
		//on l'affiche
		printf("Hauteur : %d\n",height);
		close(fd);
		return 0;
	}
	//utilisation de l'option --getobjects
	else if (strcmp(argv[2], "--getobjects") == 0) {
		//on ouvre le fichier passé en paramètre
		int fd = open(argv[1],O_RDONLY);
		int nb_objects;
		lseek(fd,2*sizeof(int),SEEK_SET); //On se déplace de deux entiers pour extraire le nombre d'objets
		//on lit le nombre d'objet sur la carte
		if ((r = read(fd,&nb_objects,sizeof(int))) == 0) return 1;
		//on l'affiche
		printf("Nombre d'objets : %d\n",nb_objects);
		close(fd);
		return 0;
	}
	//utilisation de l'option --getinfo
	else if (strcmp(argv[2], "--getinfo") == 0) {
		//on ouvre le fichier passé en paramètre
		int fd = open(argv[1],O_RDONLY);
		int width,height, nb_objetcs;
		//on lit la largeur, la hauteur et le nombre d'objet de la carte
		if ((r = read(fd,&width,sizeof(int))) == 0) return 1;
		if ((r = read(fd,&height,sizeof(int))) == 0) return 1;
		if ((r = read(fd,&nb_objetcs,sizeof(int))) == 0) return 1;
		//on affiche les informations récupérées
		printf("Informations sur %s : \nHauteur : %d\nLargeur : %d\nNombre d'objets : %d\n", argv[1],height,width, nb_objetcs);
		close(fd);
		return 0;
	}
	//utilisation de l'option --setwidth
	else if (strcmp(argv[2], "--setwidth") == 0) {
		//on vérifie la bonne utilisation de la commande
		if (argc != 4){
			fprintf(stderr, "Please enter a single number for width."); return 1;
		}
		int new_width = atoi(argv[3]);
		//on verifie que la nouvelle valeur de la largeur est possible
		if(new_width < MIN_WIDTH || new_width > MAX_WIDTH) {
			fprintf(stderr, "Minimum width : 16\n"
					"Maximum width : 1024\n"
					"Please enter a correct width.");
			return 1;
		}
		//on ouvre le fichier passé en paramètre
		int fd = open(argv[1],O_RDONLY);
		verification(fd != -1, "[--setwidth] open");

		int old_width;
		int height;
		int nb_objects;

		int verif_read;
		//on lit l'ancienne valeur de la largeur
		verif_read = read(fd, &old_width, sizeof(int));
		verification(verif_read != -1, "[--setwidth] read old width");
		//on lit la valeur de la hauteur
		verif_read = read(fd, &height, sizeof(int));
		verification(verif_read != -1, "[--setwidth] read height");
		//on lit le nombre d'objets
		verif_read = read(fd, &nb_objects, sizeof(int));
		verification(verif_read != -1, "[--setwidth] read nb objects");


		int new_matrix[new_width][height];
		int old_matrix[old_width][height];
		int type;
		//on initialise la nouvelle matrice à MAP_OBJECT_NONE (= -1)
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < new_width; ++x) {
				new_matrix[x][y] = -1;
			}
		}
		//on récupère toutes les valeurs de l'ancienne matrice
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < old_width; ++x) {
				verif_read = read(fd, &type, sizeof(int));
				verification(verif_read != -1, "[--setwidth] read matrix");
				old_matrix[x][y] = type;
			}
		}
		//si la nouvelle largeur est plus grande ou égale à l'ancienne
		//on recopie toutes les valeurs de l'ancienne matrice dans la nouvelle matrice
		if (old_width <= new_width) {
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < old_width - 1; ++x) {
					new_matrix[x][y] = old_matrix[x][y];
				}
				new_matrix[new_width - 1][y] = 1;
			}
			for (int x = old_width - 1; x < new_width; ++x) {
				new_matrix[x][height - 1] = 0;
			}
		} else { //si on a diminué la largeur, on récupère toutes les valeurs avec la nouvelle largeur
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < new_width; ++x) {
					new_matrix[x][y] = old_matrix[x][y];
				}
				new_matrix[new_width - 1][y] = 1;
			}
		}

		off_t offset = ((old_width * height) + 3) * sizeof(int);
		lseek(fd, offset, SEEK_SET);
		int path_length[nb_objects];
		char path[nb_objects][256];
		unsigned frames[nb_objects];
		int properties[nb_objects][4];
		//pour chaque objet
		for (int i = 0; i < nb_objects; ++i) {
			//on lit la longueur du chemin 
			verif_read = read(fd, &path_length[i], sizeof(int));
			verification(verif_read != -1, "[--setwidth] read path length");

			for (int j = 0; j <= path_length[i]; ++j) {
				//on lit le chemin 
				verif_read = read(fd, &path[i][j], sizeof(char));
				verification(verif_read != -1, "[--setwidth] read path");
			}
			//on lit son nombre de frames
			verif_read = read(fd, &frames[i], sizeof(unsigned));
			verification(verif_read != -1, "[--setwidth] read nb frames");
			//on lit toutes ses propriétés
			for (int k = 0; k < 4; ++k) {
				verif_read = read(fd, &properties[i][k], sizeof(int));
				verification(verif_read != -1, "[--setwidth] read properties");
			}
		}

		close(fd);
		//on ouvre le ficher en paramètre en écriture
		fd = open(argv[1], O_WRONLY|O_TRUNC, 0600);
		int verif_write;
		//on écrit la nouvelle largeur 
		verif_write = write(fd, &new_width, sizeof(int));
		verification(verif_write != -1, "[--setwidth] write new width");
		//on écrit la hauteur 
		verif_write = write(fd, &height, sizeof(int));
		verification(verif_write != -1, "[--setwidth] write height");
		//on écrit le nombre d'objets présents 
		verif_write = write(fd, &nb_objects, sizeof(int));
		verification(verif_write != -1, "[--setwidth] write nb objects");
		//on écrit pour chaque coordonées (x,y) de la carte son type
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < new_width; ++x) {
				verif_write = write(fd, &new_matrix[x][y], sizeof(int));
				verification(verif_write != -1, "[--setwidth] write new matrix");
			}
		}
		//pour chaque objet
		for (int i = 0; i < nb_objects; ++i) {
			//on écrit la longueur de son chemin
			verif_write = write(fd, &path_length[i], sizeof(int));
			verification(verif_write != -1, "[--setwidth] write path length");
			//on écrit son chemin
			for (int j = 0; j <= path_length[i]; ++j) {
				verif_write = write(fd, &path[i][j], sizeof(char));
				verification(verif_write != -1, "[--setwidth] write path");
			}
			//on écrit son nombre de frames
			verif_write = write(fd, &frames[i], sizeof(unsigned));
			verification(verif_write != -1, "[--setwidth] write frames");

			for (int k = 0; k < 4; ++k) {
				//on écrit ses propriétés
				verif_write = write(fd, &properties[i][k], sizeof(int));
				verification(verif_write != -1, "[--setwidth] write properties");
			}
		}
		//on ferme le fichier
		close(fd);
		return 0;
	}
	//utilisation de l'option --setheight
	else if (strcmp(argv[2], "--setheight") == 0) {
		//on vérifie la bonne utilisation de la commande
		if (argc != 4) {
			fprintf(stderr, "Please enter a single number for height."); return 1;
		}
		//on verifie que la nouvelle valeur de la largeur est possible
		int new_height = atoi(argv[3]);
		if(new_height < MIN_HEIGHT || new_height > MAX_HEIGHT) {
			fprintf(stderr, "Minimum height : 12\n"
					"Maximum height : 20\n"
					"Please enter a correct height.");
			return 1;
		}
		//on ouvre le fichier en paraamètre en lecture
		int fd = open(argv[1],O_RDONLY);
		verification(fd != -1, "[--setheight] open");

		int width;
		int old_height;
		int nb_objects;

		int verif_read;
		//on lit la largeur
		verif_read = read(fd, &width, sizeof(int));
		verification(verif_read != -1, "[--setheight] read old width");
		//on lit l'ancienne valeur de la hauteur
		verif_read = read(fd, &old_height, sizeof(int));
		verification(verif_read != -1, "[--setheight] read height");
		//on lit le nombre d'objet
		verif_read = read(fd, &nb_objects, sizeof(int));
		verification(verif_read != -1, "[--setheight] read nb objects");


		int new_matrix[width][new_height];
		int old_matrix[width][old_height];
		int type;
		//on initialise les valeurs de la nouvelle matrice
		for (int y = 0; y < new_height; ++y) {
			for (int x = 0; x < width; ++x) {
				new_matrix[x][y] = -1;
			}
		}
		//on récupère les valeurs de l'ancienne matrice
		for (int y = 0; y < old_height; ++y) {
			for (int x = 0; x < width; ++x) {
				verif_read = read(fd, &type, sizeof(int));
				verification(verif_read != -1, "[--setheight] read matrix");
				old_matrix[x][y] = type;
			}
		}
		//si la nouvelle hauteur est plus grande ou égale à l'ancienne
		//on recopie toutes les valeurs de l'ancienne matrice dans la nouvelle matrice
		if (old_height <= new_height) {
            int offset_height = new_height - old_height;
			for (int y = 0; y < old_height; ++y) {
				for (int x = 0; x < width; ++x) {
					new_matrix[x][y + offset_height] = old_matrix[x][y];
				}
			}
			for (int y = 0; y < offset_height; ++y) {
				new_matrix[0][y] = 1;
				new_matrix[width - 1][y] = 1;
			}
		} else {//si on a diminué la hauteur, on récupère toutes les valeurs avec la nouvelle hauteur
            int offset_height = old_height - new_height;
			for (int y = 0; y < new_height; ++y) {
				for (int x = 0; x < width; ++x) {
					new_matrix[x][y] = old_matrix[x][y+offset_height];
				}
			}
		}

		off_t offset = ((width * old_height) + 3) * sizeof(int);
		lseek(fd, offset, SEEK_SET);
		int path_length[nb_objects];
		char path[nb_objects][256];
		unsigned frames[nb_objects];
		int properties[nb_objects][4];
		//pour chaque les objets
		for (int i = 0; i < nb_objects; ++i) {
			//on lit la longueur de son chemin
			verif_read = read(fd, &path_length[i], sizeof(int));
			verification(verif_read != -1, "[--setheight] read path length");
			for (int j = 0; j <= path_length[i]; ++j) {
				//on lit son chemin
				verif_read = read(fd, &path[i][j], sizeof(char));
				verification(verif_read != -1, "[--setheight] read path");
			}
			//on lit son nombre de frames
			verif_read = read(fd, &frames[i], sizeof(unsigned));
			verification(verif_read != -1, "[--setheight] read nb frames");

			for (int k = 0; k < 4; ++k) {
				//on lit toutes ses propriétés
				verif_read = read(fd, &properties[i][k], sizeof(int));
				verification(verif_read != -1, "[--setheight] read properties");
			}
		}
		//on ferme le fichier en ouverture
		close(fd);
		//on ouvre le fichier en paramètre en écriture
		fd = open(argv[1], O_WRONLY|O_TRUNC, 0600);
        verification(fd != -1, "[--setheight] open write/trunc");

		int verif_write;
		//on écrit la largeur 
		verif_write = write(fd, &width, sizeof(int));
		verification(verif_write != -1, "[--setheight] write width");
		//on écrit la nouvelle hauteur
		verif_write = write(fd, &new_height, sizeof(int));
		verification(verif_write != -1, "[--setheight] write new height");
		//on écrit le nombre d'objet
		verif_write = write(fd, &nb_objects, sizeof(int));
		verification(verif_write != -1, "[--setheight] write nb objects");
		//on écrit pour chaque coordonées (x,y) de la carte son type
		for (int y = 0; y < new_height; ++y) {
			for (int x = 0; x < width; ++x) {
				verif_write = write(fd, &new_matrix[x][y], sizeof(int));
				verification(verif_write != -1, "[--setheight] write new matrix");
			}
		}
		//pour chaque objet 
		for (int i = 0; i < nb_objects; ++i) {
			//on écrit la longueur de son chemin
			verif_write = write(fd, &path_length[i], sizeof(int));
			verification(verif_write != -1, "[--setheight] write path length");
			//on écrit son chemin
			for (int j = 0; j <= path_length[i]; ++j) {
				verif_write = write(fd, &path[i][j], sizeof(char));
				verification(verif_write != -1, "[--setheight] write path");
			}
			//on écrit son nombre de frames
			verif_write = write(fd, &frames[i], sizeof(unsigned));
			verification(verif_write != -1, "[--setheight] write frames");

			for (int k = 0; k < 4; ++k) {	
				//on écrit toutes ses propriétés
				verif_write = write(fd, &properties[i][k], sizeof(int));
				verification(verif_write != -1, "[--setheight] write properties");
			}
		}
		//on ferme le fichier
		close(fd);
		return 0;
	}
	//utilisation de l'option --setobjects
	else if (strcmp(argv[2], "--setobjects") == 0){
		//on ouvre le fichier en paramètre 
        int fd = open(argv[1], O_RDONLY);
        verification(fd != -1, "[--setobjects] open read");

        int verif_read;
        int width;
        int height;
        int old_nb_objects;
        //on lit la largeur
        verif_read = read(fd, &width, sizeof(int));
        verification(verif_read != 1, "[--setobjects] read width");
        //on lit la hauteur
        verif_read = read(fd, &height, sizeof(int));
        verification(verif_read != 1, "[--setobjects] read height");
        //on lit l'ancien nombre d'objet 
        verif_read = read(fd, &old_nb_objects, sizeof(int));
        verification(verif_read != -1, "[--setobjects] read old nb objects");
        //on ferme le fichier
        close(fd);

		int nb_args = argc - 3;
        int new_nb_objects = nb_args/6;
        if (nb_args%6 != 0) {
            fprintf(stderr, "Nombre d'arguments incorrect.\n"
                    "Usage : %s <file> --setobjects { <filename> <frames> <solidity>\n"
                    "<destructible> <collectible> <generator> }", argv[0]);
            return 1;
        } else if (new_nb_objects < old_nb_objects) {
            fprintf(stderr, "Le nombre de nouveaux objets doit être égal ou supérieur au nombre d'objets initial.\n"
                    "Nombre d'objets initial : %d", old_nb_objects);
            return 1;
        }

        int filenames_length[new_nb_objects];
        char * filenames[new_nb_objects];
        unsigned frames[new_nb_objects];
        int properties[new_nb_objects][4];

        for (int i = 0; i < new_nb_objects; ++i) {
            int index = 6 * i + 3;

            filenames_length[i] = strlen(argv[index]);

            filenames[i] = argv[index];

            frames[i] = (unsigned)atoi(argv[index+1]);

            if (strcmp(argv[index+2], "solid") == 0){
                properties[i][0] = 2;
            } else if (strcmp(argv[index+2], "semi-solid") == 0){
                properties[i][0] = 1;
            } else if (strcmp(argv[index+2], "air") == 0){
                properties[i][0] = 0;
            } else {
                fprintf(stderr, "Arguments incorrects.");
                return 1;
            }

            if (strcmp(argv[index+3], "destructible") == 0){
                properties[i][1] = 1;
            } else if (strcmp(argv[index+3], "not-destructible") == 0){
                properties[i][1] = 0;
            } else {
                fprintf(stderr, "Arguments incorrects.");
                return 1;
            }

            if (strcmp(argv[index+4], "collectible") == 0){
                properties[i][2] = 1;
            } else if (strcmp(argv[index+4], "not-collectible") == 0){
                properties[i][2] = 0;
            } else {
                fprintf(stderr, "Arguments incorrects.");
                return 1;
            }

            if (strcmp(argv[index+5], "generator") == 0){
                properties[i][3] = 1;
            } else if (strcmp(argv[index+5], "not-generator") == 0){
                properties[i][3] = 0;
            } else {
                fprintf(stderr, "Arguments incorrects.");
                return 1;
            }
        }

        fd = open(argv[1], O_WRONLY, 0600);
        verification(fd != -1, "[--setobjects] open write");

        int verif_write;

        lseek(fd, 2 * sizeof(int), SEEK_SET);

        verif_write = write(fd, &new_nb_objects, sizeof(int));
        verification(verif_write != -1, "[--setobjects] write new nb objects");

        lseek(fd, (width * height) * sizeof(int), SEEK_CUR);

        for (int i = 0; i < new_nb_objects; ++i) {
            verif_write = write(fd, &filenames_length[i], sizeof(int));
            verification(verif_write != -1, "[--setobjects] write filename length");

            for (int j = 0; j <= filenames_length[i]; ++j) {
                verif_write = write(fd, &filenames[i][j], sizeof(char));
                verification(verif_write != -1, "[--setobjects] write filename");
            }

            verif_write = write(fd, &frames[i], sizeof(unsigned));
            verification(verif_write != -1, "[--setobjects] write nb frames");

            for (int k = 0; k < 4; ++k) {
                verif_write = write(fd, &properties[i][k], sizeof(int));
                verification(verif_write != -1, "[--setobjects] write properties");
            }
        }

        int filename_length_sum = 0;
        for (int i = 0; i < new_nb_objects; ++i) {
            filename_length_sum += filenames_length[i];
        }
        off_t max_size = (((width * height) + 3) * sizeof(int)) + ((filename_length_sum + new_nb_objects) * sizeof(char))
                         + (new_nb_objects * sizeof(int)) + (new_nb_objects * sizeof(unsigned))
                         + (4 * new_nb_objects * sizeof(int));
        ftruncate(fd, max_size);
        close(fd);
	}
    else if (strcmp("--pruneobjects", argv[2]) == 0){

        int verif_read;
        int width, height, nb_objects;

        int fd = open(argv[1], O_RDWR, 0600);
        verification(fd != -1, "[--pruneobjects] open read/write");

        verif_read = read(fd, &width, sizeof(int));
        verification(verif_read != -1, "[--pruneobjects] read width");

        verif_read = read(fd, &height, sizeof(int));
        verification(verif_read != -1, "[--pruneobjects] read heigth");

        verif_read = read(fd, &nb_objects, sizeof(int));
        verification(verif_read != -1, "[--pruneobjects] read nb objects");

        bool is_used[nb_objects];
        for (int i = 0; i < nb_objects; ++i) {
            is_used[i] = false;
        }

        int matrix[width][height];
        int new_nb_objects = 0;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                verif_read = read(fd, &matrix[x][y], sizeof(int));
                verification(verif_read != -1, "[--pruneobjects] read matrix");
                if (matrix[x][y] != -1 && !is_used[matrix[x][y]]){
                    is_used[matrix[x][y]] = true;
                    new_nb_objects++;
                }
            }
        }
        if (new_nb_objects == nb_objects)
            return 0;

        int new_type = 0;
        for (int i = 0; i < nb_objects; ++i) {
            if (is_used[i]){
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        if (matrix[x][y] == i){
                            matrix[x][y] = new_type;
                        }
                    }
                }
                new_type++;
            }
        }

        int filename_length[nb_objects];
        char filename[nb_objects][256];
        unsigned frames[nb_objects];
        int properties[nb_objects][4];

        for (int i = 0; i < nb_objects; ++i) {
            verif_read = read(fd, &filename_length[i], sizeof(int));
            verification(verif_read != -1, "[--pruneobjects] read filename length");

            for (int j = 0; j <= filename_length[i]; ++j) {
                verif_read = read(fd, &filename[i][j], sizeof(char));
                verification(verif_read != -1, "[--pruneobjects] read filejname");
            }

            verif_read = read(fd, &frames[i], sizeof(unsigned));
            verification(verif_read != -1, "[--pruneobjects] read nb frames");

            for (int k = 0; k < 4; ++k) {
                verif_read = read(fd, &properties[i][k], sizeof(int));
                verification(verif_read != -1, "[--pruneobjects] read properties");
            }
        }

        lseek(fd, 3 * sizeof(int), SEEK_SET);

        int verif_write;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                verif_write = write(fd, &matrix[x][y], sizeof(int));
                verification(verif_write != -1, "[--pruneobjects] write matrix");
            }
        }

        int filename_length_sum = 0;

        for (int i = 0; i < nb_objects; ++i) {
            if (is_used[i]){
                verif_write = write(fd, &filename_length[i], sizeof(int));
                verification(verif_write != -1, "[--pruneobjects] write filename length");
                filename_length_sum += filename_length[i];

                for (int j = 0; j <= filename_length[i]; ++j) {
                    verif_write = write(fd, &filename[i][j], sizeof(char));
                    verification(verif_write != -1, "[--pruneobjects] write filename");
                }

                verif_write = write(fd, &frames[i], sizeof(unsigned));
                verification(verif_write != -1, "[--pruneobjects] write nb frames");

                for (int k = 0; k < 4; ++k) {
                    verif_write = write(fd, &properties[i][k], sizeof(int));
                    verification(verif_write != -1, "[--pruneobjects] write properties");
                }
            }
        }

        lseek(fd, 2 * sizeof(int), SEEK_SET);

        verif_write = write(fd, &new_nb_objects, sizeof(int));
        verification(verif_write != -1, "[--pruneobjects] write new nb objects");

        off_t offset = (width * height + 3) * sizeof(int)
                       + new_nb_objects * sizeof(int)
                       + (filename_length_sum + new_nb_objects) * sizeof(char)
                       + new_nb_objects * sizeof(unsigned)
                       + 4 * new_nb_objects * sizeof(int);
        ftruncate(fd, offset);
        close(fd);
    }
	else {
		fprintf(stderr,"Usage: %s <file> <option : --get(width/height/objects/info) \n\
					--set(width/height) (<w>/<h>)  \n\
					--setobjects  <filename> <frames> <solidity> <destructible> <collectible> <generator> ",argv[0]);
		return 1;
	}
}
