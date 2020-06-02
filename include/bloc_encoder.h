
#ifndef _BLOC_ENCODER_H
#define _BLOC_ENCODER_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "htables.h"
#include "bitstream.h"
#include "huffman.h"
#include "qtables.h"
#include "image_reader.h"
#include "command_reader.h"
#define PI 3.141592653589793

void encode(uint8_t** bloc, uint8_t quantification_table[64], struct bitstream* bitsream,
                                    struct huff_table* huffman_DC, struct huff_table* huffman_AC, int16_t *predicateur) ;

#endif /* _BLOC_ENCODER_H */
