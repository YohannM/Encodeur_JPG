#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>
#include <math.h>
#include "command_reader.h"

typedef struct {
    int tailleX, tailleY; // longueur et largeur
    int nombre_colonnes_a_ajouter, nombre_lignes_a_ajouter;
    uint8_t** Y;
    uint8_t** Cb;
    uint8_t** Cr;
} Image;

Image *read_PPM(struct params* params)
{
    char buff[16];  
    Image *img;
    FILE *file;
    int c, rgb;

    file = fopen(params->nom_complet, "rb");
    if (!file) {
        fprintf(stderr, "Erreur lors de l'ouverture de '%s'\n", params->nom_complet);
        perror("Details ");
        exit(EXIT_FAILURE);
    }

    // lecture du format de l'image
    if (!fgets(buff, sizeof(buff), file)) {
        perror(params->nom_complet);
        exit(EXIT_FAILURE);
    }

    // vérification du format de l'image
    if (buff[0] != 'P' || buff[1] != '6') {
         fprintf(stderr, "L'image PPM doit etre au format P6\n");
         exit(EXIT_FAILURE);
    }

    img = (Image*) malloc(sizeof(Image));

    // on ignore les commentaires
    c = getc(file);
    while (c == '#') {
        while (getc(file) != '\n'){}
        c = getc(file);
    }

    ungetc(c, file);
    // on lit la taille
    if (fscanf(file, "%d %d", &img->tailleX, &img->tailleY) != 2) {
        fprintf(stderr, "La taille de l'image %s est illisible\n", params->nom_complet);
        exit(EXIT_FAILURE);
    }

    // on récupère le nombre de valeurs possibles d'une couleur
    if (fscanf(file, "%d", &rgb) != 1) {
        fprintf(stderr, "Les valeurs de l'image %s sont illisibles\n", params->nom_complet);
        exit(EXIT_FAILURE);
    }

    // on vérifie qu'elle vaut 255
    if (rgb != 255) {
        fprintf(stderr, "Les valeurs de couleurs de %s doivent etre entre 0 et 255\n", params->nom_complet);
        exit(EXIT_FAILURE);
    }

    // saut de ligne
    while (fgetc(file) != '\n');

    // si la longueur ou hauter ne sont pas divisibles par 8, on gere la troncature
    img->nombre_colonnes_a_ajouter = img->tailleX % 8 != 0 ? 8 - img->tailleX % 8 : 0;
    img->nombre_lignes_a_ajouter = img->tailleY % 8 != 0 ? 8 - img->tailleY % 8 : 0;

    // pour l'echantillonnage, si les MCUs font plus d'un bloc de large 
    while ((img->tailleX + img->nombre_colonnes_a_ajouter) % (params->sample1[0] * 8) != 0) {
        img->nombre_colonnes_a_ajouter += 8;
    }

    // pour l'echantillonnage, si les MCUs font plus d'un bloc de haut
    while ((img->tailleY + img->nombre_lignes_a_ajouter) % (params->sample1[1] * 8) != 0) {
        img->nombre_lignes_a_ajouter += 8;
    }

    // Allocation dynamique de tableau 2D pour les pixels (1ere dimension)
    img->Y = (uint8_t**)malloc((img->tailleY + img->nombre_lignes_a_ajouter) * sizeof(uint8_t*));
    img->Cb = (uint8_t**)malloc((img->tailleY + img->nombre_lignes_a_ajouter) * sizeof(uint8_t*));
    img->Cr = (uint8_t**)malloc((img->tailleY + img->nombre_lignes_a_ajouter) * sizeof(uint8_t*));


    for (int i = 0; i < img->tailleY; i++) {

        // Allocation dynamique de tableau 2D pour les pixels (2eme dimension)
        img->Y[i] = (uint8_t*) malloc((img->tailleX + img->nombre_colonnes_a_ajouter) * sizeof(uint8_t));
        img->Cb[i] = (uint8_t*) malloc((img->tailleX + img->nombre_colonnes_a_ajouter) * sizeof(uint8_t));
        img->Cr[i] = (uint8_t*) malloc((img->tailleX + img->nombre_colonnes_a_ajouter) * sizeof(uint8_t));

        for (int j = 0; j < img->tailleX; j++) {

            // tampon pour stocker les valeurs rgb (sur 8b chacune)
            uint8_t tampon[3];
            if (fread(tampon, 1, 3 * sizeof(uint8_t), file) != 3 * sizeof(uint8_t)) {
                fprintf(stderr, "Erreur de lecture de '%s'\n", params->nom_complet);
                exit(EXIT_FAILURE);
            }

            // passage de RGB à YCbCr
            img->Y[i][j] = (uint8_t)round(abs(0.299 * (float) tampon[0] + 0.587 * (float) tampon[1] + 0.114 * (float) tampon[2]));
            img->Cb[i][j] = (uint8_t)round(abs(-0.1687 * (float) tampon[0] - 0.3313 * (float) tampon[1] + 0.5 * (float) tampon[2] + 128));
            img->Cr[i][j] = (uint8_t)round(abs(0.5 * (float) tampon[0] - 0.4187 * (float) tampon[1] + 0.0813 * (float) tampon[2] + 128));
        }
    }


    // on malloc la deuxieme dimension des lignes à rajouter
    for (int i = img->tailleY; i < img->tailleY + img->nombre_lignes_a_ajouter; i++) {

        img->Y[i] = (uint8_t*) malloc((img->tailleX + img->nombre_colonnes_a_ajouter) * sizeof(uint8_t));
        img->Cb[i] = (uint8_t*) malloc((img->tailleX + img->nombre_colonnes_a_ajouter) * sizeof(uint8_t));
        img->Cr[i] = (uint8_t*) malloc((img->tailleX + img->nombre_colonnes_a_ajouter) * sizeof(uint8_t));

    }


    // On remplit les colonnes rajoutées
    for (int i = 0; i < img->tailleY; i++) {
        for (int j = 1; j < img->nombre_colonnes_a_ajouter + 1; j++) {
            img->Y[i][img->tailleX - 1 + j] = img->Y[i][img->tailleX - 1];
            img->Cb[i][img->tailleX - 1 + j] = img->Cb[i][img->tailleX - 1];
            img->Cr[i][img->tailleX - 1 + j] = img->Cr[i][img->tailleX - 1];
        }
    }

    // On remplit les lignes rajoutées
    for (int i = 1; i < img->nombre_lignes_a_ajouter + 1; i++) {
        for (int j = 0; j < img->tailleX + img->nombre_colonnes_a_ajouter; j++) {
            img->Y[img->tailleY + i - 1][j] = img->Y[img->tailleY - 1][j];
            img->Cb[img->tailleY + i - 1][j] = img->Cb[img->tailleY - 1][j];
            img->Cr[img->tailleY + i - 1][j] = img->Cr[img->tailleY - 1][j];
        }
    }
    

    fclose(file);

    return img;
}




Image *read_PGM(struct params* params)
{
    char buff[16];  
    Image *img;
    FILE *file;
    int c, rgb;

    file = fopen(params->nom_complet, "rb");
    if (!file) {
        fprintf(stderr, "Erreur lors de l'ouverture de '%s'\n", params->nom_complet);
        perror("Details ");
        exit(EXIT_FAILURE);
    }

    // lecture du format de l'image
    if (!fgets(buff, sizeof(buff), file)) {
        perror(params->nom_complet);
        exit(EXIT_FAILURE);
    }

    // vérification du format de l'image
    if (buff[0] != 'P' || buff[1] != '5') {
         fprintf(stderr, "L'image PPM doit etre au format P5\n");
         exit(EXIT_FAILURE);
    }

    img = (Image*) malloc(sizeof(Image));

    // on ignore les commentaires
    c = getc(file);
    while (c == '#') {
        while (getc(file) != '\n'){}
        c = getc(file);
    }

    ungetc(c, file);
    // on lit la taille
    if (fscanf(file, "%d %d", &img->tailleX, &img->tailleY) != 2) {
        fprintf(stderr, "La taille de l'image %s est illisible\n", params->nom_complet);
        exit(EXIT_FAILURE);
    }

    // on récupère le nombre de valeurs possibles d'une couleur
    if (fscanf(file, "%d", &rgb) != 1) {
        fprintf(stderr, "Les valeurs de l'image %s sont illisibles\n", params->nom_complet);
        exit(EXIT_FAILURE);
    }

    // on vérifie qu'elle vaut 255
    if (rgb != 255) {
        fprintf(stderr, "Les valeurs de couleurs de %s doivent etre entre 0 et 255\n", params->nom_complet);
        exit(EXIT_FAILURE);
    }

    while (fgetc(file) != '\n');

    // si la longueur ou hauter ne sont pas divisibles par 8, on gere la troncature
    img->nombre_colonnes_a_ajouter = img->tailleX % 8 != 0 ? 8 - img->tailleX % 8 : 0;
    img->nombre_lignes_a_ajouter = img->tailleY % 8 != 0 ? 8 - img->tailleY % 8 : 0;

    // pour l'echantillonnage, si les MCUs font plus d'un bloc de large 
    while ((img->tailleX + img->nombre_colonnes_a_ajouter) % (params->sample1[0] * 8) != 0) {
        img->nombre_colonnes_a_ajouter += 8;
    }

    // pour l'echantillonnage, si les MCUs font plus d'un bloc de haut
    while ((img->tailleY + img->nombre_lignes_a_ajouter) % (params->sample1[1] * 8) != 0) {
        img->nombre_lignes_a_ajouter += 8;
    }


    // Allocation dynamique de tableau 2D pour les pixels (1ere dimension)
    img->Y = (uint8_t**)malloc((img->tailleY + img->nombre_lignes_a_ajouter) * sizeof(uint8_t*));
    img->Cb = (uint8_t**)malloc((img->tailleY + img->nombre_lignes_a_ajouter) * sizeof(uint8_t*));
    img->Cr = (uint8_t**)malloc((img->tailleY + img->nombre_lignes_a_ajouter) * sizeof(uint8_t*));

    for (int i = 0; i < img->tailleY; i++) {

        // Allocation dynamique de tableau 2D pour les pixels (2eme dimension)
        img->Y[i] = (uint8_t*) malloc((img->tailleX + img->nombre_colonnes_a_ajouter) * sizeof(uint8_t));
        img->Cb[i] = (uint8_t*) malloc((img->tailleX + img->nombre_colonnes_a_ajouter) * sizeof(uint8_t));
        img->Cr[i] = (uint8_t*) malloc((img->tailleX + img->nombre_colonnes_a_ajouter) * sizeof(uint8_t));

        for (int j = 0; j < img->tailleX; j++) {
            // tampon pour stocker les valeurs rgb (sur 8b chacune)
            uint8_t tampon;
            if (fread(&tampon, sizeof(uint8_t), sizeof(uint8_t), file) != sizeof(uint8_t)) {
                fprintf(stderr, "Erreur de lecture de '%s'\n", params->nom_complet);
                exit(EXIT_FAILURE);
            }

            // passage de RGB à YCbCr
            img->Y[i][j] = (int)(tampon);
            img->Cb[i][j] = 0;
            img->Cr[i][j] = 0;
        }
    }


    // on malloc la deuxieme dimension des lignes à rajouter
    for (int i = img->tailleY; i < img->tailleY + img->nombre_lignes_a_ajouter; i++) {

        img->Y[i] = (uint8_t*) malloc((img->tailleX + img->nombre_colonnes_a_ajouter) * sizeof(uint8_t));
        img->Cb[i] = (uint8_t*) malloc((img->tailleX + img->nombre_colonnes_a_ajouter) * sizeof(uint8_t));
        img->Cr[i] = (uint8_t*) malloc((img->tailleX + img->nombre_colonnes_a_ajouter) * sizeof(uint8_t));

    }

    // On remplit les colonnes rajoutées
    for (int i = 0; i < img->tailleY; i++) {
        for (int j = 1; j < img->nombre_colonnes_a_ajouter + 1; j++) {
            img->Y[i][img->tailleX - 1 + j] = img->Y[i][img->tailleX - 1];
            img->Cb[i][img->tailleX - 1 + j] = img->Cb[i][img->tailleX - 1];
            img->Cr[i][img->tailleX - 1 + j] = img->Cr[i][img->tailleX - 1];
        }
    }

    // On remplit les lignes rajoutées
    for (int i = 1; i < img->nombre_lignes_a_ajouter + 1; i++) {
        for (int j = 0; j < img->tailleX + img->nombre_colonnes_a_ajouter; j++) {
            img->Y[img->tailleY + i - 1][j] = img->Y[img->tailleY - 1][j];
            img->Cb[img->tailleY + i - 1][j] = img->Cb[img->tailleY - 1][j];
            img->Cr[img->tailleY + i - 1][j] = img->Cr[img->tailleY - 1][j];
        }
    }

    fclose(file);

    return img;
}


void free_img(Image* img) {

    for (int i = 0; i < img->tailleY + img->nombre_lignes_a_ajouter; i++) 
    {
        free(img->Y[i]);
        free(img->Cb[i]);
        free(img->Cr[i]);
    }

    free(img->Y);
    free(img->Cb);
    free(img->Cr);
}