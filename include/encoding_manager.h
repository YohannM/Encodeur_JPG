
#ifndef _ENCODING_MANAGER_H
#define _ENCODING_MANAGER_H

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

struct jpeg* encode_MCUs(Image* image, struct MCU* MCUs, struct params* params);

#endif /* _ENCODING_MANAGER_H */