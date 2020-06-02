#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "htables.h"
#include "bitstream.h"
#include "huffman.h"
#include "image_reader.h"
#include "command_reader.h"
#include <math.h>
#define PI 3.141592653589793


void DCT_8(uint8_t** pixel, float coeff[8][8]);
void zigzag_8(float Coeff_DCT[8][8], float Coeff_zz[8][8]);
void quanti_8(int16_t res[64], float matriceY[64], uint8_t quantification_table[64]);
void convert_2D_to_1D(float table[8][8], float table1D[64]);
int16_t calcul_DPCM(int16_t coeff_zz_1D[64], int16_t *predicateur);
int8_t calcul_magnitude(int16_t DC_DPCM);
uint16_t calcul_indice_magnitude(int8_t magn, int16_t DC_DPCM);
void calcul_nbRLE(int16_t coeff_zz_1D[64], int16_t RLE[64][2],int *nbRLE);


void encode(uint8_t** bloc, uint8_t quantification_table[64], struct bitstream* bitsream,
                                    struct huff_table* huffman_DC, struct huff_table* huffman_AC, int16_t *predicateur) {


    float coeff_DCT[8][8]={{0}};//Matrice pour placer les coeff de la DCT
    float coeff_zz[8][8]={{0}};

    // calcul de la matrice DCT
    DCT_8(bloc, coeff_DCT);


    // Calcul et affichage de la matrice DCT en zig-zag
    zigzag_8(coeff_DCT, coeff_zz);


    //transformation de cette matrice zigzag 2D en 1D
    float coeff_zz_1D[64]={0};
    convert_2D_to_1D(coeff_zz, coeff_zz_1D);


    // puis, on fait la quantification :
    int16_t res[64]={0};
    quanti_8(res, coeff_zz_1D, quantification_table);


    /*
    le code qui suit appelle des fonctions qui sont décrites en bas de ce fichier
    */


    // calcul magnitude + indice pour DC
    int16_t DC_DPCM = calcul_DPCM(res, predicateur);
    int8_t magn = calcul_magnitude(DC_DPCM);
    uint16_t ind_magn = calcul_indice_magnitude(magn, DC_DPCM);


    // calcul Codage Huffman magnitude DC
    uint8_t *nbbits = NULL;
    nbbits = malloc(sizeof(uint8_t));
    uint32_t code_huff_Y = huffman_table_get_path(huffman_DC, magn, nbbits);
    // ecriture magnitude DC Y codée en huffman:
    bitstream_write_bits(bitsream, code_huff_Y, *nbbits, false);
    // ecriture indice magnitude DC Y codé sur magn bits
    bitstream_write_bits(bitsream, ind_magn, magn, false);


    // calcul magnitude + indice pour AC,
    // TRAITEMENT et ecriture AC
    /* Calcul matrice RLE[i][j] :
    - RLE[i][0] nombre de 0 précédent la j-ième valeur non nulle
    - RLE[i][1] valeur non nulle à garder
    - si EOB RL[i][]=0,0 ; si 16 zeros RL[i][]=F,0
    */
    int16_t RLE[64][2] ;
    int *nbRLE = NULL;
    nbRLE = malloc(sizeof(int));
    *nbRLE = 0;


    calcul_nbRLE(res, RLE, nbRLE);
    uint8_t octet_RLE;
    magn = 0;
    ind_magn = 0;

    for (int k=0; k<*nbRLE; k++){

            //CALCUL MAGNITUDE
            magn = 0; //pour l'eventuel End Of Block 00
            if (RLE[k][1]!=0){magn = calcul_magnitude(RLE[k][1]);}

            //CALCUL INDICE DANS MAGNITUDE
            if ( RLE[k][1]>= 0) {
                ind_magn = RLE[k][1];
            } else {
                ind_magn = (uint16_t)(pow(2,magn)-1+RLE[k][1]);
            }

            octet_RLE = RLE[k][0] * 16 + magn;
            /*variable dans laquelle on a  :
            4 bits codant le nb de 0 successifs, suivis de
            4 bits codant la magnitude AC.*/

            //calcul code huffman de notre RLE
            uint32_t code_huff = huffman_table_get_path(huffman_AC, octet_RLE, nbbits);
            // ecriture nb0+magnitude(=octet_RLE) AC codée en huffman:
            bitstream_write_bits(bitsream, code_huff, *nbbits, false);
            // ecriture indice magnitude AC codé sur magn bits
            bitstream_write_bits(bitsream, ind_magn, magn, false);
    }

}



/*
Les fonctions utilisées dans notre code ci-dessus sont montrées ci-dessous
*/



/* pour commencer : calcul de la DCT
   bloc de 8*8 pixel et renvoie la matrice des coefficients
de la DCT coresspondante */
void DCT_8(uint8_t** pixel, float coeff[8][8]) {
    int u=0, v=0;
    int x=0, y=0;
    float au=0, av=0;

    for(v=0;v<=7;v++)
    {
        for(u=0;u<=7;u++)
        {
            //Calcul sommes en parcourant pixel
            for(y=0;y<=7;y++)
            {
                for(x=0;x<=7;x++)
                {
                    coeff[v][u]+=((float)pixel[y][x]-128.f)*(cos((PI*(2.f*(float)x+1.f)*(float)u)/16.f)*cos((PI*(2.f*(float)y+1.f)*(float)v)/16.f));
                }
            }

            //Calcul des facteurs d'ortogonalité
            if(u==0){au=(1.f/sqrt(2.f));}
            else {au=1.f;}

            if(v==0){av=(1.f/sqrt(2.f));}
            else{ av=1.f;}

            //Calcule du coefficient complet
            coeff[v][u]=coeff[v][u]*0.25*au*av;
        }
    }
}





/* Fonction zigzag: on passe à cette fonction un bloc de
8*8 coefficients DCT et elle renvoie la matrice des coefficients ordonnées en zigzag*/

void zigzag_8(float Coeff_DCT[8][8], float Coeff_zz[8][8])
{
    int Horizontal=1;
    int i=0, j=0;
    int k=0, l=0;
    for(k=0;k<=3;k++)
    {
        for(l=0;l<=7;l++)
        {
          Coeff_zz[k][l]=Coeff_DCT[i][j]; // Premiere moitié
          Coeff_zz[7-k][7-l]=Coeff_DCT[7-i][7-j]; // seconde moitié
                if (Horizontal) // lorsque j croit horizontal
                    { j++;
                      if (i>0)
                            { i--;
                            }
                      else
                            { Horizontal=0;
                            }
                    }
                else // lorsque i croit vertical
                    { i++;
                      if (j>0)
                            { j--;
                            }
                      else
                            { Horizontal=1;
                            }
                    }
        }
    }

}





// fonction : QUANTIFICATION d'un MCU, résultat dans res[64]
void quanti_8(int16_t res[64], float matriceY[64], uint8_t quantification_table[64]) {
  int i = 0;
  for (i=0; i<=63 ; i++){ //parcours matrice
      res[i] = (int16_t) round(matriceY[i] / (float) quantification_table[i]);
    }
}


/*
fonction permettant de transformer un tableau 2 Dimensions (8*8) en un
tableau 1 dimension (64). En effet par la suite on aura besoin de tableaux 1D.
*/
void convert_2D_to_1D(float table[8][8], float table1D[64])
{
  int k=0;
  int j=0;
  int i=0;
  for (i=0; i<=7 ; i++) //parcours ligne matrice
  {
    for (j=0;j<=7;j++) //parcours colonne matrice
    {
      table1D[k] = table[i][j];
      k++;
    }
  }
}




 //les 3 fonctions suivantes servent pour traitement DC :

 int16_t calcul_DPCM(int16_t coeff_zz_1D[64], int16_t *predicateur) {
   //CALCUL DPCM
   int16_t DC_DPCM = 0;
   DC_DPCM = round(coeff_zz_1D[0]- *predicateur);
   *predicateur = coeff_zz_1D[0];
   return(DC_DPCM);
 }

//(fonction calcul magnitude utile aussi pour AC ! )
int8_t calcul_magnitude(int16_t DC_DPCM){
    //CALCUL MAGNITUDE
    uint8_t magn = 0;
    if (DC_DPCM!=0){magn = (uint8_t)(log2(abs(DC_DPCM)))+1;}
    return magn;
}

uint16_t calcul_indice_magnitude(int8_t magn, int16_t DC_DPCM) {
     //CALCUL INDICE DANS MAGNITUDE
     uint16_t ind_magn = 0;
     if (DC_DPCM >= 0) {
       ind_magn = DC_DPCM;
     }
     else {
       ind_magn = (uint16_t)(pow(2,magn)-1+DC_DPCM);
     }
     return ind_magn;
}


//la fonction suivante sert pour notre traitement des AC :
//permet de calculer RLE et le nombre qu'on aura
void calcul_nbRLE(int16_t coeff_zz_1D[64], int16_t RLE[64][2],int *nbRLE) {
    int8_t nb_0 = 0 ;
    int8_t quotient;
    int8_t reste;
    for(int i=1;i<64;i++){
      if (coeff_zz_1D[i]==0){
           nb_0++;
       }
      else {
           quotient=nb_0/16;
           reste=nb_0%16;
           for(int j=1;j<=quotient;j++){
                 RLE[*nbRLE][0] = 0x0F;
                 RLE[*nbRLE][1] = 0x00;
                 (*nbRLE)++;
            };
            RLE[*nbRLE][0] = reste;
            RLE[*nbRLE][1] = coeff_zz_1D[i];
            (*nbRLE)++;
            nb_0=0;
         }
    }
    if (nb_0!=0){ //composantes restantes nulles code EOB: 0x00 (End Of Block)
      RLE[*nbRLE][0] = 0;
      RLE[*nbRLE][1] = 0;
      (*nbRLE)++;
    }
}
