#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "../include/map.h"

void verification(int condition, char * message) {
	if(!condition)
		perror(message);
}

int main (int argc, char** argv)
{
	if (argc < 3){
		fprintf(stderr,"Usage : %s <file> <option>",argv[0]);
		exit(2);
	}
	ssize_t r;
	if(strcmp(argv[2], "--getwidth") == 0){
		int fd = open(argv[1],O_RDONLY);
		int width;
		if ((r = read(fd,&width,sizeof(int))) == 0) return 1;
		printf("Largeur : %d\n", width);
		close(fd);
		return 0;
	}
	else if (strcmp(argv[2], "--getheight") == 0){
		int fd = open(argv[1],O_RDONLY);
		int height;
		lseek(fd,sizeof(int),SEEK_SET); /* On se deplace d'un entier dont on ne connait pas la valeur*/
		if ((r = read(fd,&height,sizeof(int))) == 0) return 1;
		printf("Hauteur : %d\n",height);
		close(fd);
		return 0;
	}
	else if (strcmp(argv[2], "--getobjects") == 0){
		int fd = open(argv[1],O_RDONLY);
		int nb_objects;
		lseek(fd,2*sizeof(int),SEEK_SET); //On se déplace de deux entiers pour extraire le nombre d'objets.
		if ((r = read(fd,&nb_objects,sizeof(int))) == 0) return 1;
		printf("Nombre d'objets : %d\n",nb_objects);
		close(fd);
		return 0;
	}
	else if (strcmp(argv[2], "--getinfo") == 0){
		int fd = open(argv[1],O_RDONLY);
		int width,height, nb_objetcs;
		if ((r = read(fd,&width,sizeof(int))) == 0) return 1;
		if ((r = read(fd,&height,sizeof(int))) == 0) return 1;
		if ((r = read(fd,&nb_objetcs,sizeof(int))) == 0) return 1;
		printf("Informations sur %s : \nHauteur : %d\nLargeur : %d\nNombre d'objets : %d\n", argv[1],height,width, nb_objetcs);
		close(fd);
		return 0;
	}
	else if (strcmp(argv[2], "--setwidth") == 0){ /*Incomplet, manque l'écriture de 1 sur la dernière colonne ! */
		if (argc != 4){
			fprintf(stderr, "Please enter a single number for width."); return 1;
		}
		int new_width = atoi(argv[3]);
		if(new_width < MIN_WIDTH || new_width > MAX_WIDTH){
			fprintf(stderr, "Minimum width : 16\n"
					"Maximum width : 1024\n"
					"Please enter a correct width.");
			return 1;
		}

		int fd = open(argv[1],O_RDONLY);
		verification(fd != -1, "[--setwidth] open");

		int old_width;
		int height;
		int nb_objects;

		int verif_read;

		verif_read = read(fd, &old_width, sizeof(int));
		verification(verif_read != -1, "[--setwidth] read old width");

		verif_read = read(fd, &height, sizeof(int));
		verification(verif_read != -1, "[--setwidth] read height");

		verif_read = read(fd, &nb_objects, sizeof(int));
		verification(verif_read != -1, "[--setwidth] read nb objects");


		int new_matrix[new_width][height];
		int old_matrix[old_width][height];
		int type;

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < new_width; ++x) {
				new_matrix[x][y] = -1;
			}
		}

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < old_width; ++x) {
				verif_read = read(fd, &type, sizeof(int));
				verification(verif_read != -1, "[--setwidth] read matrix");
				old_matrix[x][y] = type;
			}
		}

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
		} else {
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

		for (int i = 0; i < nb_objects; ++i) {
			verif_read = read(fd, &path_length[i], sizeof(int));
			verification(verif_read != -1, "[--setwidth] read path length");

			for (int j = 0; j <= path_length[i]; ++j) {
				verif_read = read(fd, &path[i][j], sizeof(char));
				verification(verif_read != -1, "[--setwidth] read path");
			}

			verif_read = read(fd, &frames[i], sizeof(unsigned));
			verification(verif_read != -1, "[--setwidth] read nb frames");

			for (int k = 0; k < 4; ++k) {
				verif_read = read(fd, &properties[i][k], sizeof(int));
				verification(verif_read != -1, "[--setwidth] read properties");
			}
		}

		close(fd);
		fd = open(argv[1], O_WRONLY|O_TRUNC, 0600);
		int verif_write;

		verif_write = write(fd, &new_width, sizeof(int));
		verification(verif_write != -1, "[--setwidth] write new width");

		verif_write = write(fd, &height, sizeof(int));
		verification(verif_write != -1, "[--setwidth] write height");

		verif_write = write(fd, &nb_objects, sizeof(int));
		verification(verif_write != -1, "[--setwidth] write nb objects");

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < new_width; ++x) {
				verif_write = write(fd, &new_matrix[x][y], sizeof(int));
				verification(verif_write != -1, "[--setwidth] write new matrix");
			}
		}

		for (int i = 0; i < nb_objects; ++i) {
			verif_write = write(fd, &path_length[i], sizeof(int));
			verification(verif_write != -1, "[--setwidth] write path length");

			for (int j = 0; j <= path_length[i]; ++j) {
				verif_write = write(fd, &path[i][j], sizeof(char));
				verification(verif_write != -1, "[--setwidth] write path");
			}

			verif_write = write(fd, &frames[i], sizeof(unsigned));
			verification(verif_write != -1, "[--setwidth] write frames");

			for (int k = 0; k < 4; ++k) {
				verif_write = write(fd, &properties[i][k], sizeof(int));
				verification(verif_write != -1, "[--setwidth] write properties");
			}
		}
		close(fd);
		return 0;
	}
	else if (strcmp(argv[2], "--setheight") == 0){
		//TODO
	}
	else if (strcmp(argv[2], "--setobjects") == 0){
		//TODO
	}
	else{
		fprintf(stderr,"Usage: %s <file> <option : --get(width/height/objects/info) \n\
					--set(width/height) (<w>/<h>)  \n\
					--setobjects  <filename> <frames> <solidity> <destructible> <collectible> <generator> ",argv[0]);
		return 1;
	}
}
