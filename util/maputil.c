#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>




int main (int argc, char** argv)
{
	if (argc < 3){
		fprintf(stderr,"Usage : %s <file> <option>",argv[0]);
		exit(2);
	}
	ssize_t r,w;
	if(strcmp(argv[2], "--getwidth") == 0){
		int fd = open(argv[1],O_RDONLY);
		int width;
		if (r = read(fd,&width,sizeof(int)) == 0) return 1;
		printf("Largeur : %d\n", width);
		close(fd);
		return 0;
	}
	else if (strcmp(argv[2], "--getheight") == 0){
		int fd = open(argv[1],O_RDONLY);
		int height;
		lseek(fd,sizeof(int),SEEK_SET); /* On se deplace d'un entier dont on ne connait pas la valeur*/
		if (r = read(fd,&height,sizeof(int)) == 0) return 1;
		printf("Hauteur : %d\n",height);
		close(fd);
		return 0;
	}
	else if (strcmp(argv[2], "--getinfo") == 0){
		int fd = open(argv[1],O_RDONLY);
		int width,height;
		if (r = read(fd,&width,sizeof(int)) == 0) return 1;
		if (r = read(fd,&height,sizeof(int)) == 0) return 1;
		printf("Informations sur %s : \nHauteur : %d\nLargeur : %d\n", argv[1],height,width);
		close(fd);
		return 0;
	}
	else if (strcmp(argv[2], "--setwidth") == 0){ /*Incomplet, manque l'écriture de 1 sur la dernière colonne ! */
		int fd = open(argv[1],O_RDWR);
		int height;
		char newline = "\n";
		if (argc != 4) {fprintf(stderr, "Please enter a single number for width"); return 1;}
		int width = atoi(argv[3]);
		if (w = write(fd,&width,sizeof(int)) == -1) return 1;
		lseek(fd,sizeof(int),SEEK_SET);
		if (r = read(fd,&height,sizeof(int)) == 0) return 1;
		lseek(fd,sizeof(int),SEEK_CUR);
		printf("%d %d", width,height);
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
