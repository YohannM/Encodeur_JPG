#ifndef _MY_JPEG_WRITER_H_
#define _MY_JPEG_WRITER_H_


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/********************/
/* Types de données */
/********************/

/* Type énuméré représentant les composantes de couleur YCbCr. */
enum color_component
{
    Y,
    Cb,
    Cr,
    NB_COLOR_COMPONENTS
};

/*
    Type énuméré représentant les types de composantes fréquentielles (DC ou AC).
*/
enum sample_type
{
    DC,
    AC,
    NB_SAMPLE_TYPES
};

/*
    Type énuméré représentant la direction des facteurs d'échantillonnage (H
    pour horizontal, V pour vertical).
*/
enum direction
{
    H,
    V,
    NB_DIRECTIONS
};


/* Type opaque représentant un arbre de Huffman. */
struct huff_table;
/*
    Type contenant l'intégralité des informations
    nécessaires à l'écriture de l'en-tête JPEG.
*/
struct my_jpeg
{                     // en têtes
                      char *ppmname;
                      char *jpegname;
                      FILE* f;
                      uint16_t SOI;
                      uint8_t *APP0;
                      uint8_t *COM;
                      uint8_t *DQT;
                      uint8_t *SOF0;
                      uint8_t *DHT;
                      uint8_t *SOS;
                      uint16_t EOI;

};



/* Alloue et retourne une nouvelle structure jpeg. */
struct my_jpeg *my_jpeg_create(void);
/*
    Détruit une structure jpeg.
    Toute la mémoire qui lui est associée est libérée.
*/
void my_jpeg_destroy(struct my_jpeg *jpg);
/*
    Ecrit tout l'en-tête JPEG dans le fichier de sortie à partir des
    informations contenues dans la structure jpeg passée en paramètre.
    En sortie, le bitstream est positionné juste après l'écriture de
    l'en-tête SOS, à l'emplacement du premier octet de données brutes à écrire.
*/
void my_jpeg_write_header(struct my_jpeg *jpg);

/* Ecrit le footer JPEG (marqueur EOI) dans le fichier de sortie. */
void my_jpeg_write_footer(struct my_jpeg *jpg);

/****************************************************/
/* Gestion des paramètres de l'encodeur via le jpeg */
/****************************************************/

/* Ecrit le nom de fichier PPM ppm_filename dans la structure jpeg. */
void my_jpeg_set_ppm_filename(struct my_jpeg *jpg,
                                   char *ppm_filename);

/* Ecrit le nom du fichier de sortie jpeg_filename dans la structure jpeg. */
void my_jpeg_set_jpeg_filename(struct my_jpeg *jpg,
                                   char *jpeg_filename);


/*
    Ecrit la hauteur de l'image traitée, en nombre de pixels,
    dans la structure jpeg.
*/
void my_jpeg_set_image_height(struct my_jpeg *jpg,
                                  uint32_t image_height);

/*
    Ecrit la largeur de l'image traitée, en nombre de pixels,
    dans la structure jpeg.
*/
void my_jpeg_set_image_width(struct my_jpeg *jpg,
                                 uint32_t image_width);

/*
    Ecrit le nombre de composantes de couleur de l'image traitée
    dans la structure jpeg.
*/
void my_jpeg_set_nb_components(struct my_jpeg *jpg,
                                   uint8_t nb_components);

/*
    Ecrit dans la structure jpeg le facteur d'échantillonnage sampling_factor
    à utiliser pour la composante de couleur cc et la direction dir.
*/
void my_jpeg_set_sampling_factor(struct my_jpeg *jpg,
                                     enum color_component cc,
                                     enum direction dir,
                                     uint8_t sampling_factor);

/*
    Ecrit dans la structure jpeg la table de Huffman huff_table à utiliser
    pour encoder les données de la composante fréquentielle acdc, pour la
    composante de couleur cc.
*/
void my_jpeg_set_huffman_table(struct my_jpeg *jpg,
                                   enum sample_type acdc,
                                   enum color_component cc,
                                   struct huff_table *harbre);

/*
    Ecrit dans la structure jpeg la table de quantification à utiliser
    pour compresser les coefficients de la composante de couleur cc.
*/
void my_jpeg_set_quantization_table(struct my_jpeg *jpg,
                                        enum color_component cc,
                                        uint8_t *qtable);

struct mybitstream *my_jpeg_get_bitstream(struct my_jpeg *jpg);



#endif /* _MY_JPEG_WRITER_H_ */
