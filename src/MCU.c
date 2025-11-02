#include "MCU.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "IDCT_rapide.h"
#include "conversion_ycbcr_rgb.h"
#include "fonctions_generiques.h"
#include "header_extraction.h"
#include "iDCT.h"
#include "lecture_flux_bits.h"
#include "macros.h"
#include "quantification_inverse.h"
#include "variable_length_decoding.h"
#include "zigzag.h"

/* ------------------------------------------ COMPOSANTE
 * ------------------------------------------ */

/*
    la structure _composante_ contient les information importantes
    utiles pour le décodage d'un bloc d'une certaine composante
    (Y, Cb, ou Cr)
    ainsi que les blocs qui se mettent à jour tout au long du décodage
*/
struct composante {
  uint8_t id_composante;
  uint16_t predicateur;
  uint16_t* table_quantifiaction;
  uint8_t indice_huffman_DC;
  uint8_t indice_huffman_AC;
  uint8_t sampling_factor_horizontal;
  uint8_t sampling_factor_vertical;

  uint8_t nb_blocs;
  uint8_t*** blocs;
};

/*
    Convertit les plusieurs blocs de la composante en une matrice de la taille
   du MCU par up-sampling
*/
uint32_t** up_sampling(struct composante* comp, uint8_t hauteur_mcu,
                       uint8_t largeur_mcu) {
  uint8_t nb_blocs_ligne_comp = comp->sampling_factor_horizontal;
  uint8_t nb_blocs_colonne_comp = comp->sampling_factor_vertical;
  uint8_t nb_blocs_ligne_mcu = largeur_mcu / 8;
  uint8_t nb_blocs_colonne_mcu = hauteur_mcu / 8;
  uint8_t facteur_agrandissement_hauteur =
      nb_blocs_colonne_mcu / nb_blocs_colonne_comp;
  uint8_t facteur_agrandissement_largeur =
      nb_blocs_ligne_mcu / nb_blocs_ligne_comp;
  uint8_t hauteur_bloc_dilate = facteur_agrandissement_hauteur * 8;
  uint8_t largeur_bloc_dilate = facteur_agrandissement_largeur * 8;
  uint32_t** matrice = alloc_matrice_uint32(hauteur_mcu, largeur_mcu);

  /* On parcourt tous les blocs */
  for (uint8_t j = 0; j < nb_blocs_colonne_comp; j++) {
    for (uint8_t i = 0; i < nb_blocs_ligne_comp; i++) {
      /* On écrit chaque bloc dans la matrice */
      uint8_t** bloc = comp->blocs[j * nb_blocs_ligne_comp + i];
      for (uint8_t y = 0; y < hauteur_bloc_dilate; y++) {
        for (uint8_t x = 0; x < largeur_bloc_dilate; x++) {
          uint8_t valeur = bloc[y / facteur_agrandissement_hauteur]
                               [x / facteur_agrandissement_largeur];
          matrice[8 * j + y][8 * i + x] = valeur;
        }
      }
    }
  }
  return matrice;
}

/*
    Affiche la composante sur la sortie standard
*/
void print_composante(char* alinea, struct composante* comp,
                      uint8_t hauteur_mcu, uint8_t largeur_mcu) {
  printf("%s****** composante d'id %u ******\n", alinea, comp->id_composante);
  printf("%ssampling_factor_horizontal: %u\n", alinea,
         comp->sampling_factor_horizontal);
  printf("%ssampling_factor_vertical: %u\n", alinea,
         comp->sampling_factor_vertical);
  printf("%s%u blocs:\n", alinea, comp->nb_blocs);
  char alinea_matrice[100];
  sprintf(alinea_matrice, "%s        ", alinea);
  for (uint8_t i = 0; i < comp->nb_blocs; i++) {
    affiche_matrice_uint8(alinea_matrice, 8, 8, comp->blocs[i]);
    printf("\n");
  }
  printf(
      "%sMatrice composante de la taille du mcu (après éventuellement mise "
      "en forme et dilatation):\n",
      alinea);
  uint32_t** matrice = up_sampling(comp, hauteur_mcu, largeur_mcu);
  affiche_matrice_uint32(alinea_matrice, hauteur_mcu, largeur_mcu, matrice);
  free_matrice_uint32(matrice, hauteur_mcu);
  printf("\n");
}

void free_blocs_composante(struct composante* comp) {
  for (uint8_t i = 0; i < comp->nb_blocs; i++) {
    free_matrice_uint8(comp->blocs[i], 8);
  }
}

void free_composante(struct composante* comp) {
  free(comp->table_quantifiaction);
  free_blocs_composante(comp);
  free(comp->blocs);
  free(comp);
}

struct composante* initialise_composante(uint8_t indice_apparition,
                                         struct header* header) {
  struct composante* comp = malloc(sizeof(struct composante));
  comp->id_composante = get_id_composante(indice_apparition, header);
  comp->predicateur = 0;
  comp->table_quantifiaction = get_table_quantif(comp->id_composante, header);
  comp->indice_huffman_DC = get_huffman_index_DC(indice_apparition, header);
  comp->indice_huffman_AC = get_huffman_index_AC(indice_apparition, header);
  comp->sampling_factor_horizontal =
      get_horizontal_sampling_factor(comp->id_composante, header);
  comp->sampling_factor_vertical =
      get_vertical_sampling_factor(comp->id_composante, header);
  comp->nb_blocs =
      comp->sampling_factor_horizontal * comp->sampling_factor_vertical;
  comp->blocs = malloc(comp->nb_blocs * sizeof(uint8_t**));

  return comp;
}

/* -------------------------------------------- ENSEMBLE DES ETAPES POUR
 * EXTRAIRE UN BLOC 8x8 ------------------------------- */
/*
    renvoie le bloc de taille 8x8 suivant du fichier jpeg   uint8_t
   **extrait_un_bloc(uint16_t *predicateur, uint8_t id_composante, struct header
   *header, struct extended_FILE *stream)

*/
uint8_t** extrait_un_bloc(struct composante* comp, struct header* header,
                          struct extended_FILE* stream) {
  /* vecteur en sortie du décodage */
  uint16_t* vecteur_sortie_h =
      decode_bloc(comp->predicateur, comp->indice_huffman_DC,
                  comp->indice_huffman_AC, header, stream);
  if (vecteur_sortie_h == NULL) {
    print_error(stderr, "Erreur lors du décodage d'un bloc\n");
    return NULL;
  }
  comp->predicateur = vecteur_sortie_h[0]; /* le nouveau predicateur est
                                              l'amplitude DC du nouveau bloc */

  /* IQ */
  uint16_t* vecteur =
      inverse_quantification(vecteur_sortie_h, comp->table_quantifiaction);
  if (vecteur == NULL) {
    print_error(stderr, "Erreur lors de la quantification inverse d'un bloc\n");
    return NULL;
  }

  /* zigzag*/
  uint16_t** bloc_frequences = reordonnancement_zigzag(vecteur, 8);
  if (bloc_frequences == NULL) {
    print_error(stderr, "Erreur lors de l'ordonnancement zigzag d'un bloc\n");
    return NULL;
  }

  /* iDCT */
  uint8_t** bloc = idct_rapide(bloc_frequences, header);
  if (bloc == NULL) {
    print_error(stderr, "Erreur lors de l'iDCT sur un bloc\n");
    return NULL;
  }

  return bloc;
}

/* ------------------------------------------ COMPOSANTE, LA FIN
 * ------------------------------------------ */

void extrait_blocs_composante(struct composante* comp, struct header* header,
                              struct extended_FILE* stream) {
  for (uint8_t i = 0; i < comp->nb_blocs; i++) {
    comp->blocs[i] = extrait_un_bloc(comp, header, stream);
  }
}

/* ------------------------------------------ MCU
 * ------------------------------------------ */

/*
    un MCU resprésente un ensemble de blocs de composantes
    formant un bloc plus grand
*/
struct MCU {
  uint32_t numero; /* Pour affichage et déverminage, éventuellement à enlever */
  uint8_t height;  /* en pixels */
  uint8_t widht;   /* en pixels */
  uint8_t height_to_write; /* en pixels */
  uint8_t witdh_to_write;  /* en pixels */
  uint32_t** matrice_pixels;

  uint8_t nb_composantes;
  struct composante** composantes;
};

/*
    Affiche le MCU sur la sortie standard
*/
void print_MCU(struct MCU* mcu) {
  printf("******************** MCU numéro %u ******************** \n",
         mcu->numero);
  printf("hauteur: %u (%u à écrire)\n", mcu->height, mcu->height_to_write);
  printf("largeur: %u (%u à écrire)\n", mcu->widht, mcu->witdh_to_write);
  printf("%u composantes:\n", mcu->nb_composantes);
  for (uint8_t i = 0; i < mcu->nb_composantes; i++) {
    print_composante("      ", mcu->composantes[i], mcu->height, mcu->widht);
  }
  printf("Matrice de pixels du MCU:\n");
  affiche_matrice_uint32("      ", mcu->height, mcu->widht,
                         mcu->matrice_pixels);
  printf("\n");
}

/*
    Renvoie un MCU dont les dimenstions ont été initialisée
    (hauteur, largeur et nombre de composantes par pixel)
*/
struct MCU* initialise_MCU(struct header* header) {
  struct MCU* mcu = malloc(sizeof(struct MCU));
  mcu->numero = 0;
  mcu->height = 8 * get_vertical_sampling_factor_Y(header);
  mcu->widht = 8 * get_horizontal_sampling_factor_Y(header);
  mcu->height_to_write = mcu->height;
  mcu->witdh_to_write = mcu->widht;

  mcu->nb_composantes = get_number_of_components(header);
  mcu->composantes = malloc(mcu->nb_composantes * sizeof(struct composante*));
  for (uint8_t i = 0; i < mcu->nb_composantes; i++) {
    mcu->composantes[i] = initialise_composante(i, header);
  }
  return mcu;
}

/*
    Change les dimensions à écrire du MCU
*/
void change_height_to_write(uint8_t new_height, struct MCU* mcu) {
  mcu->height_to_write = new_height;
}

void change_width_to_write(uint8_t new_witdh, struct MCU* mcu) {
  mcu->witdh_to_write = new_witdh;
}

/*
    met à jour la matrice de pixels correspondant au prochain MCU du fichier.
    un pixel est représenté par un entier sur 32 bits, les valeurs stockées en
    mode BGR (les 3 octets de poids faible pour stocker l'info)
*/
void extrait_nouveau_MCU(struct MCU* mcu, struct header* header,
                         struct extended_FILE* stream) {
  for (uint8_t i = 0; i < mcu->nb_composantes; i++) {
    extrait_blocs_composante(mcu->composantes[i], header, stream);
  }

  /* On différencie selon si on a une image de nuances de gris ou une image RGB
   */
  if (mcu->nb_composantes == 1) {
    /* IMAGE EN NUANCES DE GRIS */
    mcu->matrice_pixels =
        up_sampling(mcu->composantes[0], mcu->height, mcu->widht);
  } else {
    /* IMAGE EN COULEUR */
    uint32_t** matrice_Y;
    uint32_t** matrice_Cb;
    uint32_t** matrice_Cr;
    for (uint8_t i = 0; i < mcu->nb_composantes; i++) {
      if (mcu->composantes[i]->id_composante == get_id_Y(header)) {
        matrice_Y = up_sampling(mcu->composantes[i], mcu->height, mcu->widht);
      } else if (mcu->composantes[i]->id_composante == get_id_Cb(header)) {
        matrice_Cb = up_sampling(mcu->composantes[i], mcu->height, mcu->widht);
      } else {
        matrice_Cr = up_sampling(mcu->composantes[i], mcu->height, mcu->widht);
      }
    }
    mcu->matrice_pixels = YCbCr_to_RGB(matrice_Y, matrice_Cb, matrice_Cr,
                                       mcu->height, mcu->widht);
    mcu->numero++;
  }
}

/*
    Libère les informations changeantes du mcu (matrice de pixels, blocs des
   composantes) Il n'y aura après ce free plus que
*/
void free_partiel_MCU(struct MCU* mcu) {
  for (uint8_t i = 0; i < mcu->nb_composantes; i++) {
    free_blocs_composante(mcu->composantes[i]);
  }
  free_matrice_uint32(mcu->matrice_pixels, mcu->height);
}

/*
    Libère le MCU en entier
*/
void free_final_MCU(struct MCU* mcu) {
  for (uint8_t i = 0; i < mcu->nb_composantes; i++) {
    free_composante(mcu->composantes[i]);
  }
  free(mcu->composantes);
  free_matrice_uint32(mcu->matrice_pixels, mcu->height);
  free(mcu);
}

/*
    Ecrit un MCU de plus dans le fichier ppm (suivant ceux d'avant)
        - hauteur_MCU_a_ecrire et largeur_MCU_a_ecrire correspondent à ce qu'il
   faut écrire du MCU, inférieur ou égale à la taille du MCU, utile en cas de
   troncature
        - nb_de_composantes (1 si gris, 3 si RGB)
*/
void ecrit_MCU(struct MCU* mcu, uint16_t largeur_image, FILE* fichier_ppm) {
  for (uint16_t j = 0; j < mcu->height_to_write; j++) {
    /* On écrit la ligne i de la matrice (pas en entier si troncature) */
    for (uint8_t i = 0; i < mcu->witdh_to_write; i++) {
      fwrite(&mcu->matrice_pixels[j][i], mcu->nb_composantes, 1, fichier_ppm);
    }
    /* On avance dans le fichier de largeur - largeur à écrire (pour se placer
     * correctment pour écrire la prochiane ligne du mcu) */
    fseek(fichier_ppm,
          mcu->nb_composantes * (largeur_image - mcu->witdh_to_write),
          SEEK_CUR);
  }
  /* on se replace bien pour écrire le prochain mcu de la ligne de mcu*/
  fseek(fichier_ppm,
        mcu->nb_composantes *
            (mcu->witdh_to_write - mcu->height_to_write * largeur_image),
        SEEK_CUR);
}

uint8_t get_mcu_height(struct MCU* mcu) { return mcu->height; }

uint8_t get_mcu_width(struct MCU* mcu) { return mcu->widht; }
