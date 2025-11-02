#ifndef _IDCT_RAPIDE_
#define _IDCT_RAPIDE_

#include <stdint.h>

#include "header_extraction.h"

/*
    Renvoie la matrice modifiée par l'iDCT et libère la matrice passée en
   argument (matrice 8x8)
*/
uint8_t** idct_rapide(uint16_t** bloc_frequences, struct header* header);

#endif /* _IDCT_RAPIDE_ */