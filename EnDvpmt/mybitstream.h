#ifndef _MY_BITSTREAM_H_
#define _MY_BITSTREAM_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*
    Type représentant le flux d'octets à écrire dans le fichier JPEG de
    sortie (appelé bitstream dans le sujet).
*/
struct mybitstream{
                      FILE *f; //fichier dans lequel on ecrit
                      uint32_t buffer; // octet buffer
                      uint8_t nb_bits; // nombre de bits déja ecrits dans buffer
};

/* Retourne un nouveau bitstream prêt à écrire dans le fichier filename. */
struct mybitstream *bitstream_create(const char *filename);


/*
    Ecrit nb_bits bits dans le bitstream. La valeur portée par cet ensemble de
    bits est value. Le paramètre is_marker permet d'indiquer qu'on est en train
    d'écrire un marqueur de section dans l'entête JPEG ou non (voir section
    2.10.4 du sujet).
*/
void bitstream_write_bits(struct mybitstream *stream,
                                 uint32_t value,
                                 uint8_t nb_b,
                                 bool is_marker);
/*
    Force l'exécution des écritures en attente sur le bitstream, s'il en
    existe.
*/
void bitstream_flush(struct mybitstream *stream);



/*
    Détruit le bitstream passé en paramètre, en libérant la mémoire qui lui est
    associée.
*/
void bitstream_destroy(struct mybitstream *stream);


#endif /* _MY_BITSTREAM_H_ */