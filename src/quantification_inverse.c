/* ---------------------------------------- SOUS-MODULE : MODIFICATION
 * QUANTIFICATION INVERSE --------------------------------------------- */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
    QUANTIFICATION INVERSE :
    Multiplication terme à terme entre le vecteur issu de Huffman et le vectuer
   quantification issu de l'entête Libère le vecteur sortit du décodage.
*/
uint16_t* inverse_quantification(uint16_t* vecteur_huffman,
                                 uint16_t* vecteur_quantification) {
  uint16_t* vecteur = malloc(sizeof(uint16_t) * 64);

  /* calcule des coefficients*/
  for (uint8_t i = 0; i < 64; i++) {
    vecteur[i] = vecteur_huffman[i] * vecteur_quantification[i];
  }

  free(vecteur_huffman);
  return vecteur;
}