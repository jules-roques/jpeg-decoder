#ifndef _CONVERSION_YCBCR_RGB_
#define _CONVERSION_YCBCR_RGB_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint32_t** YCbCr_to_RGB(uint32_t** matrice_Y, uint32_t** matrice_Cb,
                        uint32_t** matrice_Cr, uint8_t hauteur,
                        uint8_t largeur);

#endif /* _CONVERSION_YCBCR_RGB_ */