/* --------------------------------------------- MODIFICATION ZIGZAG
 * -------------------------------------------------- */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t* descente_diag_1(uint8_t i, uint8_t j, uint8_t k, uint16_t* vect,
                         uint16_t** tab) {
  while (j != 0) {
    tab[i][j] = vect[k];
    i++;
    j--;
    k++;
  }
  tab[i][j] = vect[k];
  i++;
  k++;
  tab[i][j] = vect[k];
  uint8_t* indices = malloc(sizeof(uint8_t) * 3);
  indices[0] = i;
  indices[1] = j;
  indices[2] = k;
  return indices;
}

uint8_t* descente_grande_diag(uint8_t i, uint8_t j, uint8_t k, uint16_t* vect,
                              uint16_t** tab) {
  while (j != 0) {
    tab[i][j] = vect[k];
    i++;
    j--;
    k++;
  }
  tab[i][j] = vect[k];

  uint8_t* indices = malloc(sizeof(uint8_t) * 3);
  indices[0] = i;
  indices[1] = j;
  indices[2] = k;
  return indices;
}

uint8_t* montee_diag_1(uint8_t i, uint8_t j, uint8_t k, uint16_t* vect,
                       uint16_t** tab) {
  while (i != 0) {
    i--;
    j++;
    k++;
    tab[i][j] = vect[k];
  }
  tab[i][j] = vect[k];
  j++;
  tab[i][j] = vect[k];
  k++;
  uint8_t* indices = malloc(sizeof(uint8_t) * 3);
  indices[0] = i;
  indices[1] = j;
  indices[2] = k;
  return indices;
}

uint8_t* montee_diag_2(uint8_t i, uint8_t j, uint8_t k, uint8_t n,
                       uint16_t* vect, uint16_t** tab) {
  j++;
  k++;
  tab[i][j] = vect[k];
  while (j != n - 1) {
    tab[i][j] = vect[k];
    i--;
    j++;
    k++;
  }
  tab[i][j] = vect[k];

  uint8_t* indices = malloc(sizeof(uint8_t) * 3);
  indices[0] = i;
  indices[1] = j;
  indices[2] = k;
  return indices;
}

uint8_t* descente_diag_2(uint8_t i, uint8_t j, uint8_t k, uint8_t n,
                         uint16_t* vect, uint16_t** tab) {
  i++;
  tab[i][j] = vect[k];
  k++;
  while (i != n - 1) {
    tab[i][j] = vect[k];
    i++;
    j--;
    k++;
  }
  tab[i][j] = vect[k];

  uint8_t* indices = malloc(sizeof(uint8_t) * 3);
  indices[0] = i;
  indices[1] = j;
  indices[2] = k;
  return indices;
}

/*
    Réordonnancement zigzag pour un vecteur de taille n²
    Libère le vecteur passé en paramètre.
*/
uint16_t** reordonnancement_zigzag(uint16_t* vect, uint8_t n) {
  /* on crée la matrice qui accueillera le vecteur réorganisé en zigzag */

  uint16_t** tab = malloc(sizeof(uint16_t*) * n);
  for (uint8_t i = 0; i < n; i++) {
    tab[i] = malloc(sizeof(uint16_t) * n);
  }

  /* on initialise les indices i et j de la matrice à remplir et k qui parcourt
   * les éléments de vect */
  uint8_t i = 0;
  uint8_t j = 1;
  uint8_t k = 1;
  uint8_t* indices;
  /* Première moitié de la matrice */
  tab[0][0] = vect[0];
  while (i != n - 1) {
    if (i == 0 && j == n - 1) {
      indices = descente_grande_diag(i, j, k, vect, tab);
      i = indices[0];
      j = indices[1];
      k = indices[2];
      free(indices);
    } else {
      indices = descente_diag_1(i, j, k, vect, tab);
      i = indices[0];
      j = indices[1];
      k = indices[2];
      free(indices);
      indices = montee_diag_1(i, j, k, vect, tab);
      i = indices[0];
      j = indices[1];
      k = indices[2];
      free(indices);
    }
  }

  /* Deuxième moitié de la matrice */
  while (i != n - 1 || j != n - 2) {
    indices = montee_diag_2(i, j, k, n, vect, tab);
    i = indices[0];
    j = indices[1];
    k = indices[2];
    free(indices);
    indices = descente_diag_2(i, j, k, n, vect, tab);
    i = indices[0];
    j = indices[1];
    k = indices[2];
    free(indices);
  }
  tab[n - 1][n - 1] = vect[n * n - 1];

  free(vect);
  return tab;
}