#ifndef _QUANTIF_INVERSE_
#define _QUANTIF_INVERSE_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
    QUANTIFICATION INVERSE :
    Multiplication terme à terme entre le vecteur issu de Huffman et le vectuer
   quantification issu de l'entête Libère le vecteur sortit du décodage.
*/
uint16_t* inverse_quantification(uint16_t* vecteur_huffman,
                                 uint16_t* vecteur_quantification);

#endif /* _QUANTIF_INVERSE_ */