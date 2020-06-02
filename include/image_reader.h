#ifndef IMAGE_READER_H
#define IMAGE_READER_H

#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>
#include "command_reader.h"

typedef struct {
    int tailleX, tailleY;
    int nombre_colonnes_a_ajouter, nombre_lignes_a_ajouter;
    uint8_t** Y;
    uint8_t** Cb;
    uint8_t** Cr;
} Image;

Image *read_PPM(struct params* params);
Image *read_PGM(struct params* params);

void free_img(Image* img);

#endif /* IMAGE_READER_H */