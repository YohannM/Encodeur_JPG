#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "htables.h"
#include "bitstream.h"
#include "huffman.h"
#include "qtables.h"
#include "image_reader.h"
#include "command_reader.h"
#include "mcu_cutting.h"
#include "bloc_encoder.h"

struct jpeg* encode_MCUs(Image* image, struct MCU* MCUs, struct params* params) {

    struct huff_table *huffman_Y_DC = huffman_table_build(htables_nb_symb_per_lengths[DC][Y], htables_symbols[DC][Y], htables_nb_symbols[DC][Y]);
    struct huff_table *huffman_Y_AC = huffman_table_build(htables_nb_symb_per_lengths[AC][Y], htables_symbols[AC][Y], htables_nb_symbols[AC][Y]);
    struct huff_table *huffman_Cb_DC = huffman_table_build(htables_nb_symb_per_lengths[DC][Cb], htables_symbols[DC][Cb], htables_nb_symbols[DC][Cb]);
    struct huff_table *huffman_Cb_AC = huffman_table_build(htables_nb_symb_per_lengths[AC][Cb], htables_symbols[AC][Cb], htables_nb_symbols[AC][Cb]);
    struct huff_table *huffman_Cr_DC = huffman_table_build(htables_nb_symb_per_lengths[DC][Cr], htables_symbols[DC][Cr], htables_nb_symbols[DC][Cr]);
    struct huff_table *huffman_Cr_AC = huffman_table_build(htables_nb_symb_per_lengths[AC][Cr], htables_symbols[AC][Cr], htables_nb_symbols[AC][Cr]);

    // création du jpeg
    struct jpeg* jpg = jpeg_create();

    if(params->gotOutfile) {
        jpeg_set_jpeg_filename(jpg, params->outfile);
    } else {
        int taille = strlen(params->nom_complet);
        char jpg_filename[taille];
        strcpy(jpg_filename, params->nom_complet);
        jpg_filename[taille - 3] = 'j';
        jpg_filename[taille - 2] = 'p';
        jpg_filename[taille - 1] = 'g';
        jpeg_set_jpeg_filename(jpg, jpg_filename);
    }


    // On met tout ce qu'il faut dedans
    jpeg_set_ppm_filename(jpg, params->nom_complet);
    jpeg_set_image_width(jpg, image->tailleX);
    jpeg_set_image_height(jpg, image->tailleY);

    // selon si l'image est n&b ou en couleurs, nombre de composantes :
    jpeg_set_nb_components(jpg, strcasecmp(params->extension, "ppm") == 0 ? 3 : 1);

    //Facteurs d'échantillonage
    jpeg_set_sampling_factor(jpg, Y, H, params->sample1[0]); //horizontal : h1
    jpeg_set_sampling_factor(jpg, Y, V, params->sample1[1]); //vertical : v1


    jpeg_set_huffman_table(jpg, DC, Y, huffman_Y_DC); //ecriture table huffman utilisée en DC pour Y
    jpeg_set_huffman_table(jpg, AC, Y, huffman_Y_AC); //ecriture table huffman utilisée en AC pour Y


    jpeg_set_quantization_table(jpg, Y, quantification_table_Y);


    if(strcasecmp(params->extension, "ppm") == 0) {

        jpeg_set_sampling_factor(jpg, Cb, H, params->sample2[0]); //horizontal : h1
        jpeg_set_sampling_factor(jpg, Cb, V, params->sample2[1]); //vertical : v1
        jpeg_set_sampling_factor(jpg, Cr, H, params->sample3[0]); //horizontal : h1
        jpeg_set_sampling_factor(jpg, Cr, V, params->sample3[1]); //vertical : v1

        jpeg_set_huffman_table(jpg, DC, Cb, huffman_Cb_DC); //ecriture table huffman utilisée en DC pour Cb
        jpeg_set_huffman_table(jpg, AC, Cb, huffman_Cb_AC); //ecriture table huffman utilisée en AC pour Cb
        jpeg_set_huffman_table(jpg, DC, Cr, huffman_Cr_DC); //ecriture table huffman utilisée en DC pour Cr
        jpeg_set_huffman_table(jpg, AC, Cr, huffman_Cr_AC); //ecriture table huffman utilisée en AC pour Cr

        jpeg_set_quantization_table(jpg, Cb, quantification_table_CbCr);
        jpeg_set_quantization_table(jpg, Cr, quantification_table_CbCr);

    }


    // Plus qu'à écrire l'en-tête jpeg dans le fichier de sortie.
    jpeg_write_header(jpg);

    struct bitstream *my_bitstream = jpeg_get_bitstream(jpg);


    /*le prédicateur, utilisé pour le calcul du DPCM (c'est le premier élément de
    la MCU précédente) */
    int16_t *predicateur = NULL;
    predicateur = malloc(sizeof(int16_t));
    *predicateur = 0;

    int16_t *predicateur_Cb = NULL;
    predicateur_Cb = malloc(sizeof(int16_t));
    *predicateur_Cb = 0;

    int16_t *predicateur_Cr = NULL;
    predicateur_Cr = malloc(sizeof(int16_t));
    *predicateur_Cr = 0;


    // parcours des MCUs
    // appel du module bloc_encoder

    while(MCUs != NULL) {

        // Bloc de luminance
        for (int k = 0; k < MCUs->nb_blocs_mcu; k++) {
            encode(MCUs->blocs[0][k], quantification_table_Y, my_bitstream,
                                    huffman_Y_DC, huffman_Y_AC, predicateur);
        }

        if(strcasecmp(params->extension, "ppm") == 0) {

            // Bloc de chrominance bleu
            for (int k = 0; k < MCUs->nb_blocs_mcu_Cb; k++) {
                encode(MCUs->blocs[1][k], quantification_table_CbCr, my_bitstream,
                                        huffman_Cb_DC, huffman_Cb_AC, predicateur_Cb);
            }

            // Bloc de chrominance rouge
            for (int k = 0; k < MCUs->nb_blocs_mcu_Cr; k++) {
                encode(MCUs->blocs[2][k], quantification_table_CbCr, my_bitstream,
                                        huffman_Cr_DC, huffman_Cr_AC, predicateur_Cr);
            }

        }


        MCUs = MCUs->suivant;

    }



    /* Voilà, on finit proprement notre belle image! */
    jpeg_write_footer(jpg);
    jpeg_destroy(jpg);

    return jpg;

}
