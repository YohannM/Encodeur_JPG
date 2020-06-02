
#include <stdint.h>

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
struct my_jpeg *my_jpeg_create(void){
                      struct my_jpeg *jpg=NULL;
                      jpg=malloc(sizeof(*jpg));
                      printf("Création structure JPEG\n");
                      jpg->ppmname=malloc(20*sizeof(char));
                      jpg->jpegname=malloc(20*sizeof(char));
                      jpg->APP0=malloc(18*sizeof(uint8_t));
                      jpg->COM=malloc(4*sizeof(uint8_t));
                      jpg->DQT=malloc(199*sizeof(uint8_t));
                      jpg->SOF0=malloc(19*sizeof(uint8_t)); //On réserve pour 3 composantes
                      jpg->DHT=malloc(2068*sizeof(uint8_t)); //8 tables huffman max
                      jpg->DHT[2]=0;jpg->DHT[3]=2;//initialisation nb tables Huff à 0
                      jpg->DHT[4]=0; //init indice 0
                      jpg->SOS=malloc(14*sizeof(uint8_t)); //si 3 composantes
                      return jpg;
                      }
/*
    Détruit une structure jpeg.
    Toute la mémoire qui lui est associée est libérée.
*/
void my_jpeg_destroy(struct my_jpeg *jpg){
                      free(jpg);
                      }
/*
    Ecrit tout l'en-tête JPEG dans le fichier de sortie à partir des
    informations contenues dans la structure jpeg passée en paramètre.
    En sortie, le bitstream est positionné juste après l'écriture de
    l'en-tête SOS, à l'emplacement du premier octet de données brutes à écrire.
*/
void my_jpeg_write_header(struct my_jpeg *jpg){
                    // il faut ouvrir le fichier et écrire dedans
                    jpg->f = fopen(jpg->jpegname, "w");
                    uint16_t taille;
                    printf("Ecriture entetes JPEG\n");
                    jpg->SOI=0xffd8;
                    printf("SOI=%04hx\n",jpg->SOI);
                    fputc(0xff, jpg->f);fputc(0xd8, jpg->f);
                    uint8_t tab_app0[18]={0xff,0xe0,0x00,0x10,'J','F','I','F','\0',0x1,0x1,0,0,0,0,0,0,0};
                    uint16_t i=0;
                    printf("Section APP0:");
                    for(i = 0 ; i <= 17 ; i++)
                      {
                        jpg->APP0[i]=tab_app0[i];
                        printf("%02hx ",jpg->APP0[i]);
                        fputc(jpg->APP0[i],jpg->f);
                      };
                    printf("\n");
                    uint8_t tab_com[4]={0xff,0xfe,0x00,0x02};
                    printf("Section COM:");
                    for(i = 0 ; i <= 3 ; i++)
                      {
                        jpg->COM[i]=tab_com[i]; // sans commentaire
                        printf("%02hx ",jpg->COM[i]);
                        fputc(jpg->COM[i],jpg->f);
                      };
                    printf("\n");
                    //DQT deja fait
                    printf("Section DQT:");
                    printf("%02hx ",jpg->DQT[0]);printf("%02hx ",jpg->DQT[1]);
                    fputc(jpg->DQT[0],jpg->f);fputc(jpg->DQT[1],jpg->f);
                    taille=(jpg->DQT[2]*256+jpg->DQT[3]);
                    for(i = 0 ; i < taille ; i++)
                      {
                        printf("%02hx ",jpg->DQT[2+i]);
                        fputc(jpg->DQT[2+i],jpg->f);
                      };
                    printf("\n");
                    printf("Section SOF0:");
                    jpg->SOF0[0]=0xff;jpg->SOF0[1]=0xc0; // Marqueur S0F0
                    printf("%02hx ",jpg->SOF0[0]);printf("%02hx ",jpg->SOF0[1]);
                    fputc(jpg->SOF0[0],jpg->f);fputc(jpg->SOF0[1],jpg->f);
                    // longueur déjà fait
                    jpg->SOF0[4]=0x08; //précision
                    fputc(jpg->SOF0[4],jpg->f);
                    //height déjà rempli
                    //width déjà rempli
                    //nb composantes déjà rempli
                    //sampling factors déjà remplis
                    taille=(jpg->SOF0[2]*256+jpg->SOF0[3]);
                    for(i = 0 ; i < taille ; i++)
                      {
                        printf("%02hx ",jpg->SOF0[2+i]);
                        fputc(jpg->SOF0[2+i],jpg->f);
                      };
                    printf("\n");
                    jpg->DHT[0]=0xff;jpg->DHT[1]=0xc4;
                    fputc(jpg->DHT[0],jpg->f);fputc(jpg->DHT[1],jpg->f);
                    // section tables de huffmann déjà remplie
                    printf("Section DHT:");
                    printf("%02hx ",jpg->DHT[0]);printf("%02hx ",jpg->DHT[1]);
                    taille=(jpg->DHT[2]*256+jpg->DHT[3]);
                    for(i = 0 ; i < taille ; i++)
                      {
                        printf("%02hx ",jpg->DHT[2+i]);
                        fputc(jpg->DHT[2+i],jpg->f);
                      };
                    printf("\n");
                    //section SOS:
                    printf("Section SOS:");
                    jpg->SOS[0]=0xff;jpg->SOS[1]=0xda; // Marqueur SOS
                    printf("%02hx ",jpg->SOS[0]);printf("%02hx ",jpg->SOS[1]);
                    fputc(jpg->SOS[0],jpg->f);fputc(jpg->SOS[1],jpg->f);
                    jpg->SOS[4]= jpg->SOF0[9]; //nb composantes
                    jpg->SOS[2]=0x00;jpg->SOS[3]=2*(jpg->SOS[4])+6; // taille
                    uint16_t end;
                    for(i = 0 ; i < (jpg->SOS[4]) ; i++) //pour chaque comp
                      {
                        jpg->SOS[i+5]=i+1; //ic 1 Y 2 Cb 3 Cr
                        jpg->SOS[i+6]=(i+1)*16+(i+1); //iH=iC pour DC / pour AC
                        end=i+6;
                      };
                    jpg->SOS[end+1]=0;jpg->SOS[end+2]=63;jpg->SOS[end+3]=0;
                    taille=((jpg->SOS[2]*256)+(jpg->SOS[3]));
                    for(i = 0 ; i < taille ; i++)
                      {
                        printf("%02hx ",jpg->SOS[2+i]);
                        fputc(jpg->SOS[2+i],jpg->f);
                      };
                    printf("\n");

                    bitstream_create(jpg->jpegname);
                    printf("Création du Bitstream\n");
                    }

/* Ecrit le footer JPEG (marqueur EOI) dans le fichier de sortie. */
void my_jpeg_write_footer(struct my_jpeg *jpg){
                    jpg->EOI=0xffd9;
                    printf("EOI=%04hx\n",jpg->EOI);
                    fputc(0xff, jpg->f);fputc(0xd9, jpg->f);
                    fclose(jpg->f);
                    }

/****************************************************/
/* Gestion des paramètres de l'encodeur via le jpeg */
/****************************************************/

/* Ecrit le nom de fichier PPM ppm_filename dans la structure jpeg. */
void my_jpeg_set_ppm_filename(struct my_jpeg *jpg,
                                   char *ppm_filename){
                    jpg->ppmname=ppm_filename;
                      printf("Nom fichier ppm\n");
                    }

/* Ecrit le nom du fichier de sortie jpeg_filename dans la structure jpeg. */
void my_jpeg_set_jpeg_filename(struct my_jpeg *jpg,
                                   char *jpeg_filename){
                     jpg->jpegname=jpeg_filename;
                     printf("Nom fichier jpeg\n");
                     }


/*
    Ecrit la hauteur de l'image traitée, en nombre de pixels,
    dans la structure jpeg.
*/
void my_jpeg_set_image_height(struct my_jpeg *jpg,
                                  uint32_t image_height){
                    uint8_t h_lo=(uint8_t)((image_height%256)); //on met sur 2*8 bits
                    uint8_t h_hi=(uint8_t)((image_height/256));
                    jpg->SOF0[5]=h_hi;jpg->SOF0[6]=h_lo;
                    printf("Hauteur\n");
                    }

/*
    Ecrit la largeur de l'image traitée, en nombre de pixels,
    dans la structure jpeg.
*/
void my_jpeg_set_image_width(struct my_jpeg *jpg,
                                 uint32_t image_width){
                   uint8_t w_lo=(uint8_t)((image_width%256)); //on met sur 2*8 bits
                   uint8_t w_hi=(uint8_t)((image_width/256));
                   jpg->SOF0[7]=w_hi;jpg->SOF0[8]=w_lo;
                     printf("Largeur\n");
                   }

/*
    Ecrit le nombre de composantes de couleur de l'image traitée
    dans la structure jpeg.
*/
void my_jpeg_set_nb_components(struct my_jpeg *jpg,
                                   uint8_t nb_components){
                     jpg->SOF0[9]=nb_components;
                       printf("Nbcomp\n");
                     }

/*
    Ecrit dans la structure jpeg le facteur d'échantillonnage sampling_factor
    à utiliser pour la composante de couleur cc et la direction dir.
*/
void my_jpeg_set_sampling_factor(struct my_jpeg *jpg,
                                     enum color_component cc,
                                     enum direction dir,
                                     uint8_t sampling_factor){
                   //Si Y offset 10 à 12
                   //Si Cb offset 13 à 15
                   //Si Cr offset 16 à 18
                   printf("Sampling Factors\n");
                   int i;
                   uint8_t longueur; //longueur de la section selon le nombre de composantes
                   uint8_t octet_sampling=0;
                   if (dir==H){
                      octet_sampling=16*sampling_factor;
                   };
                   if (dir==V){
                      octet_sampling=sampling_factor;
                   };
                   if (cc==Y){
                     i=0;
                     longueur=11;
                   };
                   if (cc==Cb){
                     i=1;
                     longueur=14;
                   };
                   if (cc==Cr){
                     i=2;
                     longueur=17;
                   };
                   jpg->SOF0[10+3*i]=i+1; // Id ic
                   jpg->SOF0[11+3*i]=(jpg->SOF0[11+3*i]) + octet_sampling;
                   jpg->SOF0[12+3*i]=i; //iq table quant associée
                   jpg->SOF0[2]=0; jpg->SOF0[3]=longueur;
                  }

/*
    Ecrit dans la structure jpeg la table de Huffman huff_table à utiliser
    pour encoder les données de la composante fréquentielle acdc, pour la
    composante de couleur cc.
*/
void my_jpeg_set_huffman_table(struct my_jpeg *jpg,
                                   enum sample_type acdc,
                                   enum color_component cc,
                                   struct huff_table *harbre){
                       printf("Table Huffman\n");
                      // on recupere le vecteur des longueur et la table des symboles
                      uint8_t *length_vector=huffman_table_get_length_vector(harbre);
                      uint8_t *table_symboles=huffman_table_get_symbols(harbre);;
                      uint8_t length_table=0;
                      // on calcule la longueur de la table symboles
		                  length_table=htables_nb_symbols[acdc][cc];
                      //table_symboles=malloc(length_table*sizeof(uint8_t))
                      // on recupere la longueur actuelle de la section DHT
                      uint16_t long_prec=jpg->DHT[2]*256+jpg->DHT[3];
                      // on calcule l'offset pour ecrire la nouvelle table
                      uint16_t offset=2+long_prec;
                      // indice iH:on pose Y indice 1 Cb indice 2 et Cr indice 3
                      if (cc==Y) jpg->DHT[offset]=1;
                      if (cc==Cb) jpg->DHT[offset]=2;
                      if (cc==Cr) jpg->DHT[offset]=3;
                      // bit DC/AC = 1/0
                      if (acdc==DC) jpg->DHT[offset]|=0x10; // mise à 1 5e bit
                      // ecriture des 16 nombres de symboles pas taille
                      int i=0;
                      for (i=0; i<=15; i++)
                            jpg->DHT[offset+1+i]=length_vector[i];
                      // ecriture de la table des symboles
                      for (i=0; i<length_table; i++)
                            jpg->DHT[offset+17+i]=table_symboles[offset+17+i];
                      // calcul et mise à jour longueur section sur 16 bits
                      // 1 table = 17 octets + length_table octets;
                      long_prec+=(17+length_table);
                      jpg->DHT[3]=(uint8_t)(long_prec & 0x00FF);
                      jpg->DHT[2]=(uint8_t)((long_prec & 0xFF00)/256);
                    }

/*
    Ecrit dans la structure jpeg la table de quantification à utiliser
    pour compresser les coefficients de la composante de couleur cc.
*/
void my_jpeg_set_quantization_table(struct my_jpeg *jpg,
                                        enum color_component cc,
                                        uint8_t *qtable){
                          printf("Table quantif\n");
                          //Si Y la table de (1 octet entete + 64 valeurs) commence à l'indice 04d
                          //Si Cb la table de (1 octet entete + 64 valeurs) commence à l'indice 68d
                          //Si Cr la table de (1 octet entete + 64 valeurs) commence à l'indice 133d
                          int i;
                          int offset;
                          jpg->DQT[0]=0xff;jpg->DQT[1]=0xdb; // marqueur DQT
                          if (cc==Y){
                            jpg->DQT[2]=0x00;jpg->DQT[3]=67; // une seule matrice q
                            offset=4;
                            jpg->DQT[offset]=0x00; //précision 8b iQ=0
                          };
                          if (cc==Cb){
                            jpg->DQT[2]=0x00;jpg->DQT[3]=132; // deux matrices q
                            offset=69;
                            jpg->DQT[offset]=0x01; //précision 8b iQ=1
                          };
                          if (cc==Cr){
                            jpg->DQT[2]=0x00;jpg->DQT[3]=197; // trois matrices q
                            offset=134;
                            jpg->DQT[offset]=0x02; //précision 8b iQ=2
                          };
                          for(i = offset+1 ; i <= (offset+64) ; i++)
                            {
                              jpg->DQT[i]=qtable[i-(offset+1)];// ecriture table
                            };

                        }
struct mybitstream *my_jpeg_get_bitstream(struct my_jpeg *jpg){

                   return bitstream_create(jpg->jpegname);

                   }
