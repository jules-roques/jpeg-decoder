/* -------------------------------------------------- IDCT NAIVE
 * ---------------------------------------------------- */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fonctions_generiques.h"
#include "macros.h"

/*
    Sous-fonctions pour l'idct :
     - renvoie le coefficient i,j du MCU en paramètre
    modifié selon la transformée (formule naive 2.4)
     - renvoie la valeur associée au coefficient C
*/
float calcule_C(uint8_t x) {
  if (x == 0) {
    return (float)1 / sqrt(2);
  } else {
    return (float)1;
  }
}

/*
    Coefficient S(x,y) de l'iDCT naïve
*/
uint8_t conversion_spatiale(int16_t** matrice_frequences, uint8_t x,
                            uint8_t y) {
  uint8_t n = 8;
  float coeff_spatial = 0;

  /* Calcul de S(x,y) */
  float first_cos;
  float second_cos;
  for (uint8_t lambda = 0; lambda < 8; lambda++) {
    for (uint8_t mu = 0; mu < 8; mu++) {
      first_cos = cosf(((2 * x + 1) * lambda * _pi_ / (2 * n)));
      second_cos = cosf((2 * y + 1) * mu * _pi_ / (2 * n));
      coeff_spatial += calcule_C(lambda) * calcule_C(mu) * first_cos *
                       second_cos * matrice_frequences[lambda][mu];
    }
  }
  coeff_spatial = (float)coeff_spatial / sqrt(2 * n);
  coeff_spatial += 128;

  /* Saturation */
  if (coeff_spatial < 0) {
    coeff_spatial = 0;
  } else if (coeff_spatial > 255) {
    coeff_spatial = 255;
  }

  /* Envoie d'un entier sur un octet */
  return (uint8_t)coeff_spatial;
}

/*
    Renvoie la matrice modifiée par l'iDCT et libère la matrice passée en
   argument
*/
uint8_t** modification_iDCT_naive(uint16_t** matrice) {
  /* Allocation mémoire */
  uint8_t** nouvelle_matrice = alloc_matrice_uint8(8, 8);

  /* Assignation */
  for (uint8_t x = 0; x < 8; x++) {
    for (uint8_t y = 0; y < 8; y++) {
      nouvelle_matrice[x][y] = conversion_spatiale((int16_t**)matrice, x, y);
    }
  }

  /* on libère l'ancienne matrice */
  free_matrice_uint16(matrice, 8);

  return nouvelle_matrice;
}