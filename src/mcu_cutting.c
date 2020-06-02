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


struct MCU* MCU_cutting_from_Image(Image* image, struct params* params) {
    
    //Intuitif
    int nb_blocs_par_mcu = params->sample1[0] * params->sample1[1];
    int nb_blocs_par_mcu_Cb = params->sample2[0] * params->sample2[1];
    int nb_blocs_par_mcu_Cr = params->sample3[0] * params->sample3[1];
    int nb_MCU = (image->tailleX + image->nombre_colonnes_a_ajouter) * (image->tailleY + image->nombre_lignes_a_ajouter) / (nb_blocs_par_mcu * 64);

    // on instancie la premiere MCU
    struct MCU* tete = malloc(sizeof(struct MCU));

    struct MCU* courant = tete;

    // instanciation des MCUs 
    for(int i = 1; i < nb_MCU; i++) {
        courant->suivant = (struct MCU*) malloc(sizeof(struct MCU));
        courant = courant->suivant;
    }

    courant->suivant = NULL;
    courant = tete;

    int numero_du_mcu = 0;

    // On parcourt les MCUs pour malloc blocs et pour le remplir
    
    while(courant != NULL) {

        // logique
        courant->nb_blocs_mcu = nb_blocs_par_mcu;
        courant->nb_blocs_mcu_Cb = nb_blocs_par_mcu_Cb;
        courant->nb_blocs_mcu_Cr = nb_blocs_par_mcu_Cr;

        // allocation de la premire dimension 
        courant->blocs = malloc(3* sizeof(uint8_t***));

        // allocation de la deuxieme dimension 
        courant->blocs[0] = (uint8_t***) malloc(sizeof(uint8_t**) * nb_blocs_par_mcu); // les blocs de Y
        courant->blocs[1] = (uint8_t***) malloc(sizeof(uint8_t**) * nb_blocs_par_mcu_Cb); // les blocs de Cb
        courant->blocs[2] = (uint8_t***) malloc(sizeof(uint8_t**) * nb_blocs_par_mcu_Cr); // les blocs de Cr
        
        // allocation de la troisieme et quatrieme dimension pour Y 
        for (int i = 0; i < nb_blocs_par_mcu; i++) {
            courant->blocs[0][i] = (uint8_t**) malloc(8 * sizeof(uint8_t*));
            for(int j = 0; j < 8; j++) {
                courant->blocs[0][i][j] = (uint8_t*) malloc(8 * sizeof(uint8_t));
            }
        }


        // allocation de la troisieme et quatrieme dimension pour Cb
        for (int i = 0; i < nb_blocs_par_mcu_Cb; i++) {
            courant->blocs[1][i] = (uint8_t**) malloc(8 * sizeof(uint8_t*));
            for(int j = 0; j < 8; j++) {
                courant->blocs[1][i][j] = (uint8_t*) malloc(8 * sizeof(uint8_t));
            }
        }


        // allocation de la troisieme et quatrieme dimension pour Cr
        for (int i = 0; i < nb_blocs_par_mcu_Cr; i++) {
            courant->blocs[2][i] = (uint8_t**) malloc(8 * sizeof(uint8_t*));
            for(int j = 0; j < 8; j++) {
                courant->blocs[2][i][j] = (uint8_t*) malloc(8 * sizeof(uint8_t));
            }
        }


        // sémantique
        int nb_mcu_par_ligne = (image->tailleX + image->nombre_colonnes_a_ajouter) / (8 * params->sample1[0]);

        // calcul des indice lignes et colonnes du debut du MCU en cours
        int indice_colonne = (numero_du_mcu * params->sample1[0] * 8) % (image->tailleX + image->nombre_colonnes_a_ajouter);
        int indice_ligne = ((numero_du_mcu) * params->sample1[0] * 8 - indice_colonne) / nb_mcu_par_ligne;

        // on remplit chaque bloc de luminance
        for (int k = 0; k < nb_blocs_par_mcu; k++) {

            // calcul des indices du bloc en cours dans le MCU
            int supp_colonne = (k * 8) % (params->sample1[0] * 8);
            int supp_ligne = (k * 8 - supp_colonne) / params->sample1[0];

            for (int i = 0; i < 8; i++) { 
                for (int j = 0; j < 8; j++) {

                    courant->blocs[0][k][i][j] = image->Y[indice_ligne + supp_ligne + i][indice_colonne + supp_colonne + j];
                    
                }
            }
        }


        // on remplit chaque bloc de Cb
        for (int k = 0; k < nb_blocs_par_mcu_Cb ; k++) {

            int un_Cb_pour_X_blocs_Y = nb_blocs_par_mcu / nb_blocs_par_mcu_Cb;

            // on calcule la position du bloc sur lequel notre bloc de chorminance prendra ses valeurs
            int supp_colonne = (k * un_Cb_pour_X_blocs_Y * 8) % (params->sample1[0] * 8);
            int supp_ligne = (k * un_Cb_pour_X_blocs_Y * 8 - supp_colonne) / params->sample1[0];

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {

                    courant->blocs[1][k][i][j] = image->Cb[indice_ligne + supp_ligne + i][indice_colonne + supp_colonne + j];
                }
            }
        }



        // on remplit chaque bloc de Cr
        for (int k = 0; k < nb_blocs_par_mcu_Cr; k++) {

            int un_Cr_pour_X_blocs_Y = nb_blocs_par_mcu / nb_blocs_par_mcu_Cr;

            // on calcule la position du bloc sur lequel notre bloc de chorminance prendra ses valeurs
            int supp_colonne = (k * un_Cr_pour_X_blocs_Y * 8) % (params->sample1[0] * 8);
            int supp_ligne = (k * un_Cr_pour_X_blocs_Y * 8 - supp_colonne) / params->sample1[0];

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {

                    courant->blocs[2][k][i][j] = image->Cr[indice_ligne + supp_ligne + i][indice_colonne + supp_colonne + j];
                }
            }
        }



        numero_du_mcu++;
        courant = courant->suivant;
        
    }

    return tete;
}


void free_MCU(struct MCU* mcu) {

    while(mcu != NULL) {

        // on free chaque bloc de Y
        for (int i = 0; i < mcu->nb_blocs_mcu; i++) {
            for(int j = 0; j < 8; j++) {
                free(mcu->blocs[0][i][j]);
            }
            free(mcu->blocs[0][i]);
        }

        // on free chaque bloc de Cb
        for (int i = 0; i < mcu->nb_blocs_mcu_Cb; i++) {
            for(int j = 0; j < 8; j++) {
               free(mcu->blocs[1][i][j]);
            }
            free(mcu->blocs[1][i]);
        }

        // on free chaque bloc de Cr
        for (int i = 0; i < mcu->nb_blocs_mcu_Cr; i++) {
            for(int j = 0; j < 8; j++) {
                free(mcu->blocs[2][i][j]);
            }
            free(mcu->blocs[2][i]);
        }

        // on free la premiere dimension
        free(mcu->blocs[0]);
        free(mcu->blocs[1]);
        free(mcu->blocs[2]);

        // on free blocs
        free(mcu->blocs);
        
        struct MCU* prec = mcu;
        mcu = mcu->suivant;

        // on free le MCU
        free(prec); 
    }

}