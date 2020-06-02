#include <stdlib.h>
#include <stdio.h>
#include "command_reader.h"
#include "image_reader.h"
#include "mcu_cutting.h"
#include "encoding_manager.h"


void infos_commande(struct params* params);
void infos_image(Image *image);
int cmparr(uint8_t array1[], uint8_t arra2[], int size);
void infos_MCU(struct MCU* mcu);


int main (int argc, char *argv[]) 
{  

    // module command_reader
    struct params* params = read_params(argc, argv);

    if(params->verbose) {
        infos_commande(params);
    }
    
    Image *image;

    if(params->verbose)
        printf("Lecture de l'image de depart.\n");

    // module image_reader
    if(strcasecmp(params->extension, "ppm") == 0) {
        image = read_PPM(params);
    } else {
        image = read_PGM(params);
    }

    if(params->verbose)
        printf("Lecture de l'image de depart OK.\n");
    

    if(params->verbose) {
        infos_image(image);
        printf("\nDecoupage de l'image en MCU.\n");
    }
    
    // module mcu_cutting
    struct MCU* MCUs = MCU_cutting_from_Image(image, params);

    if(params->verbose) {
        printf("Decoupage de l'image en MCU OK.\n");
        infos_MCU(MCUs);
        printf("\nEncodage des MCUs.\n");
    }

    // module encoding_manager
    encode_MCUs(image, MCUs, params);

    if(params->verbose) {
        printf("Encodage des MCUs. OK.\n");
        printf("\nFin du programme\n");
    }

    free_MCU(MCUs);
    free(params);
    free_img(image);

    return EXIT_SUCCESS; 
} 


void infos_commande(struct params* params) {

    printf("\nNom : %s\n", params->nom_complet);
    printf("Extension : %s\n", params->extension);
    printf("A un fichier en sortie ? : %s\n", params->gotOutfile ? "true" : "false");
    printf("Nom du fichier en sortie : %s\n", params->outfile);
    printf("A un sous-echantillonnage ? : %s\n", params->gotSample ? "true" : "false");
    printf("Sous-ech, premiere coord : %d et %d\n", params->sample1[0], params->sample1[1]);
    printf("Sous-ech, deuxieme coord : %d et %d\n", params->sample2[0], params->sample2[1]);
    printf("Sous-ech, troisieme coord : %d et %d\n\n", params->sample3[0], params->sample3[1]);

}

void infos_image(Image *image) {

    printf("\nTaille du fichier d'entree : %d lignes x %d colonnes\n", image->tailleY, image->tailleX);

    printf("Il y a %d lignes a rajouter et %d colonnes a rajouter\n", image->nombre_lignes_a_ajouter, image->nombre_colonnes_a_ajouter);

    printf("Taille totale du fichier d'entree, avec les troncatures : %d lignes x %d colonnes\n", image->tailleY + image->nombre_lignes_a_ajouter, image->tailleX + image->nombre_colonnes_a_ajouter);
}


void infos_MCU(struct MCU* mcu) { 

    int nb_mcu = 0;

    while(mcu->suivant != NULL) {
        nb_mcu++;
        mcu = mcu->suivant;
    }

    nb_mcu++;

    printf("\nNombre de MCUs generes : %d\n", nb_mcu);
    printf("Nombre de blocs de luminance par MCU : %d\n", mcu->nb_blocs_mcu);
    printf("Nombre de blocs de chrominance de bleu par MCU : %d\n", mcu->nb_blocs_mcu_Cb);
    printf("Nombre de blocs de chrominance de rouge par MCU : %d\n", mcu->nb_blocs_mcu_Cr);
}

