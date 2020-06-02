

#include <stdint.h>
#include <stdbool.h>

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
struct mybitstream *bitstream_create(const char *filename){
            struct mybitstream *stream=NULL;
            stream=malloc(sizeof(*stream));
            stream->buffer=0;
            stream->nb_bits=0;

            printf("Création et initialisation bitstream\n");
            //stream->f = fopen(filename, "w"); //fait par jpeg_writer */
            return stream;
          }


/*
    Ecrit nb_bits bits dans le bitstream. La valeur portée par cet ensemble de
    bits est value. Le paramètre is_marker permet d'indiquer qu'on est en train
    d'écrire un marqueur de section dans l'entête JPEG ou non (voir section
    2.10.4 du sujet).
*/
void bitstream_write_bits(struct mybitstream *stream,
                                 uint32_t value,
                                 uint8_t nb_b,
                                 bool is_marker){
     //on écrit 8 par 8
     uint8_t octet_ecrit;
     uint32_t value_decale;
     int bit;
     int i;
     i=0;
     value_decale=value<<(32-nb_b);// decalage pour avoir le code "à gauche" de value

     for(i =  1; i <= (nb_b) ; i++)
       {
         bit=value_decale/(0x80000000); // on recupere le bit de poids fort;
         value_decale= value_decale<<1; //on decale d'un bit à gauche
         stream->buffer=((stream->buffer<<1) | (0x00000001 * bit)); //on rejoute le bit au buffer
         stream->nb_bits++; //il y a donc un bit de plus dans le buffer
         // s'il est plein, on l'écrit dans le fichier
         if(stream->nb_bits==32)
             {
                octet_ecrit=(0xFF000000 & stream->buffer)/(0x01000000);
                fputc(octet_ecrit,stream->f);
                if (octet_ecrit==0xFF) fputc(0x00,stream->f);//byte stuffing
                octet_ecrit=(0x00FF0000 & stream->buffer)/(0x00010000);
                fputc(octet_ecrit,stream->f);
                if (octet_ecrit==0xFF) fputc(0x00,stream->f);//byte stuffing
                octet_ecrit=(0x0000FF00 & stream->buffer)/(0x00000100);
                fputc(octet_ecrit,stream->f);
                if (octet_ecrit==0xFF) fputc(0x00,stream->f);//byte stuffing
                octet_ecrit=(0x000000FF & stream->buffer);
                fputc(octet_ecrit,stream->f);
                if (octet_ecrit==0xFF) fputc(0x00,stream->f);//byte stuffing
                stream->nb_bits=0;
                stream->buffer=0;
                printf("32 bits écrits sauf byte stuffing\n");

             };
       };

    }
/*
    Force l'exécution des écritures en attente sur le bitstream, s'il en
    existe.
*/
void bitstream_flush(struct mybitstream *stream){
     // on ecrit le contenu du buffer dans le jpg, en fillant le dernier octet avec des 0
     uint8_t octet_ecrit;
     if (stream->nb_bits!=0){ // si aucun bit à écrire rien
           if (stream->nb_bits <= 8){ //il reste moins de 8 bits à écrire
                  octet_ecrit=stream->buffer;
                  octet_ecrit=octet_ecrit<<(8 - stream->nb_bits);// on décale les bits à gauche de l'octet
                  fputc(octet_ecrit,stream->f);
                }

           else if (stream->nb_bits <= 16){ // entre 8 et 16 bits
                 stream->buffer=stream->buffer<<(16 - stream->nb_bits); // on décale les bits à gauche du mot de 16 bits
                 octet_ecrit=(0x0000FF00 & stream->buffer)/(0x00000100);//selection et recopie 2eme octet
                 fputc(octet_ecrit,stream->f); // ecriture octet poids fort
                 if (octet_ecrit==0xFF) fputc(0x00,stream->f);//byte stuffing
                 octet_ecrit=(0x000000FF & stream->buffer);//selection et recopie octet poids faible
                 fputc(octet_ecrit,stream->f); // ecriture octet poids faible
                 if (octet_ecrit==0xFF) fputc(0x00,stream->f);//byte stuffing
                }
           else if (stream->nb_bits <= 24){ // entre 16 et 24 bits
                 stream->buffer=stream->buffer<<(24 - stream->nb_bits);// on décale les bits à gauche du mot de 24 bits
                 octet_ecrit=(0x00FF0000 & stream->buffer)/(0x00010000);
                 fputc(octet_ecrit,stream->f);// ecriture octet poids fort
                 if (octet_ecrit==0xFF) fputc(0x00,stream->f);//byte stuffing
                 octet_ecrit=(0x0000FF00 & stream->buffer)/(0x00000100);
                 fputc(octet_ecrit,stream->f);// ecriture 2eme octet
                 if (octet_ecrit==0xFF) fputc(0x00,stream->f);//byte stuffing
                 octet_ecrit=(0x000000FF & stream->buffer);
                 fputc(octet_ecrit,stream->f);// ecriture octet poids faible
                 if (octet_ecrit==0xFF) fputc(0x00,stream->f);//byte stuffing
                }
          else {
                stream->buffer=stream->buffer<<(32-stream->nb_bits);// entre 24 et 32 bits
                octet_ecrit=(0xFF000000 & stream->buffer)/(0x01000000);
                fputc(octet_ecrit,stream->f);// ecriture octet poids fort
                if (octet_ecrit==0xFF) fputc(0x00,stream->f);//byte stuffing
                octet_ecrit=(0x00FF0000 & stream->buffer)/(0x00010000);
                fputc(octet_ecrit,stream->f);
                if (octet_ecrit==0xFF) fputc(0x00,stream->f);//byte stuffing
                octet_ecrit=(0x0000FF00 & stream->buffer)/(0x00000100);
                fputc(octet_ecrit,stream->f);
                if (octet_ecrit==0xFF) fputc(0x00,stream->f);//byte stuffing
                octet_ecrit=(0x000000FF & stream->buffer);
                fputc(octet_ecrit,stream->f);// ecriture octet poids faible
                if (octet_ecrit==0xFF) fputc(0x00,stream->f);//byte stuffing
               }
      }
    }



/*
    Détruit le bitstream passé en paramètre, en libérant la mémoire qui lui est
    associée.
*/
void bitstream_destroy(struct mybitstream *stream){
            //fclose(stream->f); //fait par jpeg_writer */
            free(stream);
          }
