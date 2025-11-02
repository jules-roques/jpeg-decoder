#ifndef _HEADER_EXTRACTION_H_
#define _HEADER_EXTRACTION_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "header_extraction.h"
#include "lecture_flux_bits.h"

/* --------------------------------------- Pour interragir avec l'en-tête
 * --------------------------------------- */

/* --- STRUCTURE HEADER --- */
struct header;
struct header* extract_header(struct extended_FILE* stream);
void free_header(struct header* entete);
void print_header(struct header* entete);

/* --- ARBRES DE HUFFMAN --- */
struct arbre* get_arbre_huffman_DC(uint8_t indice, struct header* header);
struct arbre* get_arbre_huffman_AC(uint8_t indice, struct header* header);

/* --- TABLES DE QUANTIFICATION --- */
uint16_t* get_table_quantif(uint8_t identifiant_composante,
                            struct header* header);

/* --- COMPOSANTES --- */
uint8_t get_number_of_components(struct header* header);
const char* get_magic_value(struct header* header);
uint8_t get_id_composante(uint8_t indice_apparition_composante,
                          struct header* header);

/* --- CARACTERISTIQUES IMAGE --- */
uint16_t get_image_height(struct header* header);
uint16_t get_image_width(struct header* header);
uint8_t get_precision_image(struct header* header);
uint8_t get_number_of_components(struct header* header);

/* --- FACTEURS ECHANTILLONAGE --- */
uint8_t get_horizontal_sampling_factor_Y(struct header* header);
uint8_t get_vertical_sampling_factor_Y(struct header* header);
uint8_t get_horizontal_sampling_factor(uint8_t id_composante,
                                       struct header* header);
uint8_t get_vertical_sampling_factor(uint8_t id_composante,
                                     struct header* header);

/* --- INDICE DE TABLE DE QUANTIFIACTION --- */
uint16_t* get_table_quantif(uint8_t indice, struct header* header);

/* --- INDICE DE TABLE DE HUFFMAN --- */
uint8_t get_huffman_index_DC(uint8_t indice_appartition_composante,
                             struct header* header);
uint8_t get_huffman_index_AC(uint8_t indice_appartition_composante,
                             struct header* header);

/* --- IDENTIFIANT DE COMPOSANTE --- */
uint8_t get_id_Y(struct header* header);
uint8_t get_id_Cb(struct header* header);
uint8_t get_id_Cr(struct header* header);

char* get_image_extension(struct header* header);

/* valeurs pouor calcul de l'iDCT */
float get_value_idct(uint8_t indice, struct header* header);

#endif /* _HEADER_EXTRACTION_H_ */