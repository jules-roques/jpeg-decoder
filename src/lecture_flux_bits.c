#include "lecture_flux_bits.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "macros.h"

/*

FONCTIONS DE TRAITEMENT DU FLUX BINAIRE

Module pour la lectures d'informations dans le flux binaire
Pour par exemple lire des bits, octets,
avancer dans le flux sans rien faire, etc ...

*/

/* --------------------------------------------- STRUCTURES POUR LA LECTURE DANS
 * LE STREAM -------------------------------------------- */

/* --- BYTE BUFFER --- */

/*
    byte_buffer contient
    - l'octet en cours de lecture
    - l'indince du prochain bit à lire dans l'octet
*/
struct byte_buffer {
  /* indice indiquant le prochain bit à lire dans le buffer
  entre 7 (bit le plus à gauche) et 0 (bit le plus à droite) et  */
  uint8_t bit_to_read_index;
  /* l'octet courant */
  uint8_t byte;
};

/*
    Allocation mémoire et initialisation du buffer
*/
struct byte_buffer* alloc_byte_buffer() {
  struct byte_buffer* byte_buffer = malloc(sizeof(struct byte_buffer));
  byte_buffer->bit_to_read_index = 7;
  byte_buffer->byte = 0;

  return byte_buffer;
}

/*
    Libère la mémoire allouée au buffer
*/
void free_byte_buffer(struct byte_buffer* byte_buffer) { free(byte_buffer); }

/*
    Affiche le byte_buffer sur la sortie standard
*/
void print_byte_buffer(struct byte_buffer* byte_buffer) {
  uint8_t bit;

  /* Les chaines suivantes changent en fonction de l'indice
  du bit à lire */
  char* alinea;
  char* name_bit_to_read;
  char* following_byte;
  if (byte_buffer->bit_to_read_index == 7) {
    alinea = "                             ";
    name_bit_to_read = "bit_to_read (position 7 of next byte, not known yet)";
    following_byte = "[...    ...]";
  } else {
    alinea = "         ";
    name_bit_to_read = "bit_to_read";
    following_byte = "";
  }

  /* ligne 1 */
  printf("Current byte_buffer:\n");

  /* ligne 2 */
  printf("         [ ");
  for (uint8_t i = 0; i < 8; i++) {
    bit = (byte_buffer->byte >> (7 - i)) & 1;
    printf("%u ", bit);
  }
  printf("] %s\n", following_byte);

  /* ligne 3 */
  printf("%s", alinea);
  for (uint8_t i = 0; i < 7 - byte_buffer->bit_to_read_index; i++) {
    printf("  ");
  }
  printf("  ^\n");

  /* ligne 4 */
  printf("%s", alinea);
  for (uint8_t i = 0; i < 7 - byte_buffer->bit_to_read_index; i++) {
    printf("  ");
  }
  printf("  |\n");

  /* ligne 5 */
  printf("%s", alinea);
  for (uint8_t i = 0; i < 7 - byte_buffer->bit_to_read_index; i++) {
    printf("  ");
  }
  printf("%s", name_bit_to_read);
  if (byte_buffer->bit_to_read_index != 7) {
    printf(", position %u", byte_buffer->bit_to_read_index);
  }
  printf("\n");
}

/* --- EXTENDED FILE --- */

/*
    extended_FILE contient
        - le fichier (FILE*)
        - le buffer d'un octet (byte_buffer*)
*/
struct extended_FILE {
  FILE* file;
  struct byte_buffer* byte_buffer;
};

/*
    Permet de placer le curseur de lecture au début d'un octet dans le fichier.
    (mets l'indice du bit à lire à 7)
*/
void set_cursor(struct extended_FILE* stream) {
  stream->byte_buffer->bit_to_read_index = 7;
}

/*
    Ouvre le ficher, ainsi qu'une mémoire tampon pour contenir l'octet courant
*/
struct extended_FILE* open_file(const char* file_name,
                                const char* oppening_mode) {
  struct extended_FILE* stream = malloc(sizeof(struct extended_FILE));
  stream->file = fopen(file_name, oppening_mode);
  if (stream->file == NULL) {
    print_error(stderr, "Impossible d'ouvrir le fichier %s\n", file_name);
    return NULL;
  }

  stream->byte_buffer = alloc_byte_buffer();
  return stream;
}

/*
    Vérifie le marqueur de fin de flux et
    libère la mémoire allouée au fichier étendu du "buffer"
*/
void close_file(struct extended_FILE* stream) {
  set_cursor(stream);
  uint16_t enf_of_file = fget_2_bytes(stream);

  if (enf_of_file != 0xffd9) {
    print_warning(
        stdout,
        "Le marqueur de fin de flux JPEG 0xffd9 n'est pas reconnu: 0x%x\n",
        enf_of_file);
  }

  fclose(stream->file);
  free_byte_buffer(stream->byte_buffer);
  free(stream);
}

/*
    Affiche le buffer
*/
void print_extended_file(struct extended_FILE* stream) {
  print_byte_buffer(stream->byte_buffer);
}

/* --------------------------------------------- LECTURE DE BITS
 * --------------------------------------------- */

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
uint8_t fget_one_bit(struct extended_FILE* stream) {
  /* Si on a finit de lire le buffer précédent (index = 7) alors
  on doit récupérer l'octet suivant. */
  if (stream->byte_buffer->bit_to_read_index == 7) {
    /* si 0x00 suit 0xff dans le stream,
    alors on rejette l'octet 0x00 (cd 2.9.2 sujet) */
    uint8_t next_byte = fgetc(stream->file);
    if (stream->byte_buffer->byte == 0xff && next_byte == 0x00) {
      next_byte = fgetc(stream->file);
    }
    stream->byte_buffer->byte = next_byte;
  }

  /* On récupère le bit qui nous intéresse */
  uint8_t bit_to_read =
      (stream->byte_buffer->byte >> stream->byte_buffer->bit_to_read_index) & 1;

  /* Si on a finit de lire le buffer alors l'indice passe de souveau à 7,
  sinon l'indice renseigne le prochain bit. */
  if (stream->byte_buffer->bit_to_read_index == 0) {
    stream->byte_buffer->bit_to_read_index = 7;
  } else {
    stream->byte_buffer->bit_to_read_index--;
  }

  return bit_to_read;
}

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
uint16_t fget_bits(struct extended_FILE* stream, uint8_t n) {
  uint16_t res = 0x0;
  for (uint8_t i = 0; i < n; i++) {
    res = (res << 1) + (uint16_t)fget_one_bit(stream);
  }
  return res;
}

/* --------------------------------------------- LECTURE D'OCTETS
 * --------------------------------------------- */

/*
    Renvoie l'entier associé à la valeur binaire lue sur 1 octets de _stream_
    Et avance de 1 octets sur _stream_
*/
uint8_t fget_one_byte(struct extended_FILE* stream) {
  return fget_bits(stream, 8);
}

/*
    Renvoie l'entier associé à la valeur binaire lue sur 2 octets de _stream_
    Et avance de 2 octets sur _stream_
*/
uint16_t fget_2_bytes(struct extended_FILE* stream) {
  return fget_bits(stream, 16);
}

/*
    Pour avancer de _n_ octets dans le stream sans récupérer de valeur
*/
void skip_bytes(struct extended_FILE* stream, uint16_t n) {
  for (uint16_t i = 0; i < n; i++) {
    (void)fgetc(stream->file);
  }
}