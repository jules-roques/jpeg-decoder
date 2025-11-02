#include "conversion_ycbcr_rgb.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fonctions_generiques.h"

// #include <stdint.h>
// #include <fonctions_generiques.h>

/*
    Convertit trois composantes Y, CB, CR en RGB sur un entier sur 32bits
    le premier octet est inutile, 2eme pour B, 3eme G, dernier pour R
*/
uint32_t conversion(int16_t Y, int16_t Cb, int16_t Cr) {
  /* Conversion */
  int16_t R = Y + 1.402 * (Cr - 128);
  int16_t G = Y - 0.34414 * (Cb - 128) - 0.71414 * (Cr - 128);
  int16_t B = Y + 1.772 * (Cb - 128);

  /* Saturation */
  if (R < 0) {
    R = 0;
  } else if (R > 255) {
    R = 255;
  }

  if (G < 0) {
    G = 0;
  } else if (G > 255) {
    G = 255;
  }

  if (B < 0) {
    B = 0;
  } else if (B > 255) {
    B = 255;
  }

  /* Mise dans la bonne forme */
  uint32_t R_unsigned = (uint32_t)R;
  uint32_t G_unsigned = (uint32_t)G;
  uint32_t B_unsigned = (uint32_t)B;

  /* Mise dans la bonne forme */
  R_unsigned = R_unsigned;
  G_unsigned = G_unsigned << 8;
  B_unsigned = B_unsigned << 16;

  return B_unsigned + G_unsigned + R_unsigned;
}

/*
    Convertit 3 matrices Y, Cb, et Cr de même dimensions en une matrice RGB de
   uint32
*/
uint32_t** YCbCr_to_RGB(uint32_t** matrice_Y, uint32_t** matrice_Cb,
                        uint32_t** matrice_Cr, uint8_t hauteur,
                        uint8_t largeur) {
  uint32_t** matrice_RGB = malloc(hauteur * sizeof(uint32_t*));
  for (uint8_t i = 0; i < hauteur; i++) {
    matrice_RGB[i] = malloc(largeur * sizeof(uint32_t));
    for (uint8_t j = 0; j < largeur; j++) {
      matrice_RGB[i][j] =
          conversion((int16_t)matrice_Y[i][j], (int16_t)matrice_Cb[i][j],
                     (int16_t)matrice_Cr[i][j]);
    }
  }

  free_matrice_uint32(matrice_Y, hauteur);
  free_matrice_uint32(matrice_Cb, hauteur);
  free_matrice_uint32(matrice_Cr, hauteur);

  return matrice_RGB;
}