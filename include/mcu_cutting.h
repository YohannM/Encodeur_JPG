#ifndef MCU_CUTTING_H
#define MCU_CUTTING_H

#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>
#include "image_reader.h"
#include "command_reader.h"

struct MCU {
    /* Pointeur 4D blocs : 
    Premiere dimension : 0 pour luminance, 1 pour chrominance bleu, 2 pour chrominance rouge
    Deuxieme dimension : k designant le k-ième bloc du mcu (si downsampling k différent pour les trois dimensions) 
    Troisème dimension : i (entre 0 et 7) désigne la i-ème ligne du bloc  
    Quatrieme dimension : j (entre 0 et 7) désigne la j-ème colonne du bloc  
    */
    uint8_t**** blocs;

    // liste chainée de MCUs donc pointeur suivant 
    struct MCU* suivant;

    // Assez sémantiques (taille des k des MCUs)
    int nb_blocs_mcu;
    int nb_blocs_mcu_Cb;
    int nb_blocs_mcu_Cr;
};


struct MCU* MCU_cutting_from_Image(Image* image, struct params* params);
void free_MCU(struct MCU* mcu);

#endif /* MCU_CUTTING_H */