#ifndef _CREATION_ARBRE_HUFFMAN_H_
#define _CREATION_ARBRE_HUFFMAN_H_

#include <stdint.h>
#include <stdio.h>

#include "header_extraction.h"
#include "lecture_flux_bits.h"

struct arbre;

/*
    Renvoie l'arbre correspondant aux tableaux de profondeurs et de valeurs
*/
struct arbre* creation_arbre(uint8_t* profondeurs, uint8_t* valeurs);

void free_arbre(struct arbre* pt_arbre);

uint8_t decode_Huffman_DC(uint8_t indiceDC, struct header* header,
                          struct extended_FILE* stream);

uint8_t decode_Huffman_AC(uint8_t indiceAC, struct header* header,
                          struct extended_FILE* stream);

#endif /*_CREATION_ARBRE_HUFFMAN_H_*/