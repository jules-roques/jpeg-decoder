#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "creation_arbre_Huffman.h"
#include "decodageDPCM.h"
#include "header_extraction.h"
#include "macros.h"

/* Le predicateur DC vaut 0 pour le premier MCU/bloc */

uint16_t* decode_bloc(uint16_t predicateurDC, uint8_t indiceDC,
                      uint8_t indiceAC, struct header* header,
                      struct extended_FILE* stream) {
  uint16_t* vecteur64 = malloc(64 * sizeof(uint16_t));

  uint8_t magnitudeDC = decode_Huffman_DC(indiceDC, header, stream);
  vecteur64[0] = decodeDPCM_DC(predicateurDC, magnitudeDC,
                               stream); /*premiere case (avec DC)*/

  uint8_t indice_vecteur64 = 1;
  uint8_t resultat_huffman_AC;

  while (indice_vecteur64 < 64) { /* les 63 cases d'après (avec AC)*/
    resultat_huffman_AC = decode_Huffman_AC(indiceAC, header, stream);

    if (resultat_huffman_AC == 0) {
      while (indice_vecteur64 < 64) {
        vecteur64[indice_vecteur64] = 0;
        indice_vecteur64++;
      }
    } else {
      if (resultat_huffman_AC == 0xf0) {
        for (uint8_t j = 0; j < 16; j++) {
          vecteur64[indice_vecteur64] = 0;
          indice_vecteur64++;
        }
      } else {
        uint8_t nombre_de_coefs_nuls = resultat_huffman_AC >> 4;
        /*fprintf (stderr, "nombre_de_coefs_nuls= %u\n",nombre_de_coefs_nuls);*/
        uint8_t magnitudeAC = resultat_huffman_AC & 0x0f;

        /*fprintf (stderr, "magnitudeAC %u\n",magnitudeAC);*/
        if (magnitudeAC == 0) {
          print_error(stderr, "magnitude de 0 impossible pour RLE\n");
          return NULL;
        }

        for (uint8_t j = 0; j < nombre_de_coefs_nuls; j++) {
          vecteur64[indice_vecteur64] = 0;
          indice_vecteur64++;
        }

        vecteur64[indice_vecteur64] = decodeDPCM_AC(magnitudeAC, stream);
        indice_vecteur64++;
      }
    }
  }
  return vecteur64;
}