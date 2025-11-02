#ifndef _LECTURE_FLUX_BITS_
#define _LECTURE_FLUX_BITS_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* ------------------ STRUCTURES ET FONCTIONS POUR LA LECTURE DANS LE STREAM
 * -----------------------*/

/* --- Structure fichier étendu de la mémoire tampon --- */

/*
    extended_FILE contient
        - le fichier (FILE*)
        - le buffer d'un octet (byte_buffer*)
*/
struct extended_FILE;

/*
    Ouvre le ficher, ainsi qu'une mémoire tampon pour contenir l'octet courant
*/
struct extended_FILE* open_file(const char* file_name,
                                const char* oppening_mode);

/*
    Vérifie le marqueur de fin de flux et
    libère la mémoire allouée au fichier étendu du "buffer"
*/
void close_file(struct extended_FILE* stream);

/*
    Affiche le buffer
*/
void print_extended_file(struct extended_FILE* stream);

/* --- MANIPULATION BITS --- */

/*
    Renvoie le prochain bit du _stream_
    fait avancer le stream de 1 bit (grâce au buffer)

    La valeur est stockée dans un uint8_t,
    le bit de poids faible étant le bit lu dans le flux
    et les autres à 0
    valeur retour: Ob[0000000][0 ou 1]
                         ^        ^
                         |        |
                        /          \
               7 bits à 0         bit lu dans le flux
*/
uint8_t fget_one_bit(struct extended_FILE* stream);

/*
    Renvoie les _n_ prochains bits du _stream_ dans un uint16_t
    (si n > 16 alors on perd de l'information,
    car la valeur retournée contient les 16 derniers bits lus)
    fait avancer le stream de ces _n_ bits (grâce au buffer)

    Si par exemple n = 4, les 4 bits extraits seront les bits de poids faible
    de la valeur retournée, tous les autres bits à 0:
    valeur retour: Ob[000000000000][0110]
                            ^         ^
                            |         |
                           /           \
                16-n bits à 0          n bits lus dans le flux
*/
uint16_t fget_bits(struct extended_FILE* stream, uint8_t n);

/* --- MANIPULATION OCTETS --- */

/*
    Renvoie l'entier associé à la valeur binaire lue sur 1 octets de _stream_
    Et avance de 1 octets sur _stream_
*/
uint8_t fget_one_byte(struct extended_FILE* stream);

/*
    Renvoie l'entier associé à la valeur binaire lue sur 2 octets de _stream_
    Et avance de 2 octets sur _stream_
*/
uint16_t fget_2_bytes(struct extended_FILE* stream);

/*
    Pour avancer dans le stream sans récupérer de valeur
*/
void skip_bytes(struct extended_FILE* stream, uint16_t n);

#endif /* _LECTURE_FLUX_BITS_ */