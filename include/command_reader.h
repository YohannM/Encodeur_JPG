#ifndef _COMMAND_READER_H_
#define _COMMAND_READER_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/*
    Type opaque représentant une commande grâce à ses arguments
*/
struct params {
    char* nom_complet;
    char* extension;
    bool gotOutfile;
    bool gotSample;
    bool verbose;
    char* outfile;
    int sample1[2];
    int sample2[2];
    int sample3[2];
};

/* Retourne une struct décrivant les arguments */
extern struct params *read_params(int argc, char *argv[]);


#endif /* _COMMAND_READER_H_ */
