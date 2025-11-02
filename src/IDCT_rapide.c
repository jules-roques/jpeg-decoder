/* -------------------------------------------------- IDCT RAPIDE
 * ---------------------------------------------------- */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fonctions_generiques.h"
#include "header_extraction.h"
#include "macros.h"

void ronds_noirs(float* o0, float* o1) {
  float I0 = (*o0 + *o1) / 2;
  *o1 = (*o0 - *o1) / 2;
  *o0 = I0;
}

void rectangle_blanc(float* o0, float* o1, uint8_t indice_cos,
                     uint8_t indice_sin, struct header* header) {
  float I0 = *o0 * get_value_idct(indice_cos, header) -
             *o1 * get_value_idct(indice_sin, header);
  float I1 = *o1 * get_value_idct(indice_cos, header) +
             *o0 * get_value_idct(indice_sin, header);
  *o0 = I0;
  *o1 = I1;
}

void rond_blanc(float* o0, struct header* header) {
  *o0 = *o0 / get_value_idct(6, header); /* *o0/sqrt(2) */
}

void inv_stage1(float* tableau) {
  for (uint8_t i = 0; i < 4; i++) {
    ronds_noirs(&tableau[i], &tableau[7 - i]);
  }
}

void inv_stage2(float* tableau, struct header* header) {
  ronds_noirs(&tableau[0], &tableau[3]);
  ronds_noirs(&tableau[1], &tableau[2]);
  rectangle_blanc(&tableau[4], &tableau[7], 2, 3, header);
  rectangle_blanc(&tableau[5], &tableau[6], 0, 1, header);
}

void inv_stage3(float* tableau, struct header* header) {
  ronds_noirs(&tableau[0], &tableau[1]);
  rectangle_blanc(&tableau[2], &tableau[3], 4, 5, header);
  ronds_noirs(&tableau[4], &tableau[6]);
  ronds_noirs(&tableau[7], &tableau[5]);
}

void inv_stage4(float* tableau, struct header* header) {
  ronds_noirs(&tableau[7], &tableau[4]);
  rond_blanc(&tableau[5], header);
  rond_blanc(&tableau[6], header);
}

void inv_stage5(float* tableau) {
  float* tableau_tampon = copy_array_float(tableau, 8);

  tableau[1] = tableau_tampon[4];
  tableau[3] = tableau_tampon[6];
  tableau[4] = tableau_tampon[7];
  tableau[5] = tableau_tampon[3];
  tableau[6] = tableau_tampon[5];
  tableau[7] = tableau_tampon[1];

  free(tableau_tampon);
}

/*
    Transforme un tableau en passant par toutes les étapes inverses
*/
void inv_stages(float* tableau, struct header* header) {
  inv_stage5(tableau);
  inv_stage4(tableau, header);
  inv_stage3(tableau, header);
  inv_stage2(tableau, header);
  inv_stage1(tableau);
}

/*
    Renvoie la matrice spatiale après iDCT
*/
uint8_t** idct_rapide(uint16_t** bloc_frequences, struct header* header) {
  float** matrice_float = matrix_uint16_to_float(bloc_frequences, 8, 8);

  /* opérations sur les colonnes */
  for (uint8_t i = 0; i < 8; i++) {
    float colonne_i[8];
    for (uint8_t j = 0; j < 8; j++) {
      colonne_i[j] = matrice_float[j][i];
    }
    inv_stages(colonne_i, header);
    for (uint8_t j = 0; j < 8; j++) {
      matrice_float[j][i] = colonne_i[j];
    }
  }

  /* opérations sur les lignes */
  for (uint8_t j = 0; j < 8; j++) {
    inv_stages(matrice_float[j], header);
  }

  uint8_t** matrice_uint = alloc_matrice_uint8(8, 8);

  /* Offset, saturation et conversion en matrice uint8_t */
  for (uint8_t j = 0; j < 8; j++) {
    for (uint8_t i = 0; i < 8; i++) {
      float coeff_spatial = matrice_float[j][i] * 8 + 128;
      /* Saturation */
      if (coeff_spatial < 0) {
        coeff_spatial = 0;
      } else if (coeff_spatial > 255) {
        coeff_spatial = 255;
      }
      matrice_uint[j][i] = (uint8_t)coeff_spatial;
    }
  }

  free_matrice_uint16(bloc_frequences, 8);
  free_matrice_float(matrice_float, 8);

  return matrice_uint;
}