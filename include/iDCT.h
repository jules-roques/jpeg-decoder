#ifndef _IDCT_
#define _IDCT_

#include <stdint.h>

/*
    Renvoie la matrice modifiée par l'iDCT et libère la matrice passée en
   argument (matrice 8x8)
*/
uint8_t** modification_iDCT_naive(uint16_t** matrice);

#endif /* _IDCT_ */