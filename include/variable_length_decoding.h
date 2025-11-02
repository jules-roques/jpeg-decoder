#ifndef _VARIABLE_LENGTH_DECODING_H_
#define _VARIABLE_LENGTH_DECODING_H_

#include <stdint.h>
#include <stdio.h>

#include "creation_arbre_Huffman.h"
#include "header_extraction.h"

uint16_t* decode_bloc(uint16_t predicateurDC, uint8_t indiceDC,
                      uint8_t indiceAC, struct header* header,
                      struct extended_FILE* stream);

#endif /*_VARIABLE_LENGTH_DECODING_H_*/