#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

/*
    Type contenant les paramètres d'une commande 
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

bool helpDisplayed = false;

void displayHelp() {
    if(!helpDisplayed) {
        printf("\n ------ Ceci est l'aide ------ \n\n");

        printf("Cet executable ppm2jpeg convertit une image au format ppm ou pgm en une image jpeg.\n");
        printf("Il prend comme seul parametre obligatoire le nom de l'image PPM a convertir, d'extension .ppm ou .pgm. Seules les images au format PPM P6 ou PGM P5 sont acceptees.\n");
        printf("En sortie une image au format JPEG sera generee, encodee en mode JFIF, baseline sequentiel, DCT, Huffman, de meme nom que l'image d'entree et d'extension .jpg.\n");
        printf("\nppm2jpeg prend aussi les parametres optionnels suivants .\n");
        printf("\n--help pour afficher la liste des options acceptees ;\n");
        printf("--outfile=sortie.jpg pour redefinir le nom du fichier de sortie ;\n");
        printf("--sample=h1xv1,h2xv2,h3xv3 pour definir les facteurs d'echantillonnage hxv des trois composantes de couleur.\n");
        printf("--verbose pour avoir un retour sur la generation du fichier.\n");

        helpDisplayed = true;
    }
}


/* Instancie et remplit une struct params */
struct params *read_params(int argc, char *argv[]){

    struct params* params = (struct params*) malloc(sizeof(struct params));

    params->gotOutfile = false;
    params->gotSample = false;
    params->verbose = false;

    params->outfile = "";
    params->sample1[0] = 1;
    params->sample1[1] = 1;
    params->sample2[0] = 1;
    params->sample2[1] = 1;
    params->sample3[0] = 1;
    params->sample3[1] = 1;
    
    // si il n'y a aucun paramètre
    if (argc == 1) {
        fprintf(stderr, "Il manque le nom de l'image a convertir\n");
        exit(EXIT_FAILURE);
    }
    
    // le dernier argument est le nom de l'image
    params->nom_complet = argv[argc-1];

    if (strcmp(params->nom_complet, "--help") == 0) {
        displayHelp();
        exit(EXIT_SUCCESS);
    }

    int taille_nom = strlen(params->nom_complet);
    params->extension = malloc(4 * sizeof(char));
    params->extension[0] = params->nom_complet[taille_nom - 3];
    params->extension[1] = params->nom_complet[taille_nom - 2];
    params->extension[2] = params->nom_complet[taille_nom - 1];
    params->extension[3] = '\0';


    // si l'extension ne fait pas partie de celle traitées
    if(strcasecmp(params->extension, "ppm") != 0 && strcmp(params->extension, "pgm") != 0) {
        fprintf(stderr, "L'image a convertir doit etre au format ppm ou bien pgm\n");
        exit(EXIT_FAILURE);
    }

    

    // si il y a des options supplémentaires
    if (argc > 2) {
        
        int i;  
        for (i = 1; i < argc-1; i++) {  

            if(strcmp(argv[i], "--help") == 0) {
                displayHelp();
                continue;
            }

            // nom de l'option
            char* option = strtok(argv[i], "=");

            if (strcasecmp(option, "--outfile") == 0) {

                if (params->gotOutfile) {

                    fprintf(stderr, "Une des options est specifiee plusieurs fois\n");
                    exit(EXIT_FAILURE);

                }

                params->gotOutfile = true;
                params->outfile = strtok(NULL, "=");

                if (strcasecmp(params->outfile, "") == 0 || params->outfile == NULL) {

                    fprintf(stderr, "Essayez un autre chemin pour le chemin de sortie\n");
                    exit(EXIT_FAILURE);

                }
            
            } else if (strcasecmp(option, "--sample") == 0) {

                if (params->gotSample) {
                    fprintf(stderr, "Une des options est specifiee plusieurs fois\n");
                    exit(EXIT_FAILURE);
                }

                params->gotSample = true;
                
                char* valeur = strtok(NULL, "=");

                char* c1 = strtok(valeur, ",");
                char* c2 = strtok(NULL, ",");
                char* c3 = strtok(NULL, ",");

                params->sample1[0] = atoi(strtok(c1, "x"));
                params->sample1[1] = atoi(strtok(NULL, "x"));

                params->sample2[0] = atoi(strtok(c2, "x"));
                params->sample2[1] = atoi(strtok(NULL, "x"));

                params->sample3[0] = atoi(strtok(c3, "x"));
                params->sample3[1] = atoi(strtok(NULL, "x"));

                if (strcmp(params->extension, "pgm") == 0) {

                    fprintf(stderr, "Il est interdit de faire du sous echantillonnage sur des pgm.\n");
                    exit(EXIT_FAILURE);
                }

                if (params->sample1[0] < 1 || params->sample1[0] > 4 || params->sample1[1] < 1 || params->sample1[1] > 4 ||
                    params->sample2[0] < 1 || params->sample2[0] > 4 || params->sample2[1] < 1 || params->sample2[1] > 4 ||
                    params->sample3[0] < 1 || params->sample3[0] > 4 || params->sample3[1] < 1 || params->sample3[1] > 4 ||
                    params->sample1[0] + params->sample1[1] > 10 || params->sample2[0] + params->sample2[1] > 10 || params->sample3[0] + params->sample3[1] > 10 ||
                    params->sample1[0] % params->sample2[0] != 0 || params->sample1[0] % params->sample3[0] != 0 ||
                    params->sample1[1] % params->sample2[1] != 0 || params->sample1[1] % params->sample3[1] != 0) {

                    fprintf(stderr, "Les valeurs de sous-echantillonnage sont incorrectes\n");
                    exit(EXIT_FAILURE);
                }

            } else if (strcasecmp(option, "--verbose") == 0) {

                params->verbose = true;

            } else {
                fprintf(stderr, "Une des options n'existe pas\n");
                exit(EXIT_FAILURE);
            } 
        }  
    }


    return params;

}

