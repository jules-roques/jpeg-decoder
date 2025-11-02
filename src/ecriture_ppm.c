#include "ecriture_ppm.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "MCU.h"
#include "fonctions_generiques.h"
#include "header_extraction.h"
#include "macros.h"
#include "variable_length_decoding.h"

/* -------------------------------------------------- ECRITURE DANS LE FICHIER
 * PPM   ---------------------------------------------------- */

/*
    Sous fonction pour afficher l'en-tête du fichier PPM :
        - P5 si gris ; P6 si couleur
        - Largeur Hauteur
        - Valeur maximale
*/
void ecrit_entete_ppm(struct header* header, FILE* fichier) {
  const char* valeur_magique = get_magic_value(header);
  uint16_t largeur = get_image_width(header);
  uint16_t hauteur = get_image_height(header);
  uint8_t valeur_maximale = (1 << get_precision_image(header)) - 1;
  fprintf(fichier, "%s\n%d %d\n%d\n", valeur_magique, largeur, hauteur,
          valeur_maximale);
}

void ecrit_corps_ppm(struct header* header, struct extended_FILE* stream,
                     FILE* fichier_ppm) {
  uint16_t hauteur_image = get_image_height(header);
  uint16_t largeur_image = get_image_width(header);

  struct MCU* mcu = initialise_MCU(header);
  uint8_t hauteur_mcu = get_mcu_height(mcu);
  uint8_t largeur_mcu = get_mcu_width(mcu);

  uint8_t largeur_mcus_derniere_colonne = (largeur_image - 1) % largeur_mcu + 1;
  uint8_t hauteur_mcus_derniere_ligne = (hauteur_image - 1) % hauteur_mcu + 1;

  uint16_t nb_mcu_par_colonne = (hauteur_image - 1) / hauteur_mcu + 1;
  uint16_t nb_mcu_par_ligne = (largeur_image - 1) / largeur_mcu + 1;

  /* ecriture de toutes les lignes sauf la dernière */
  for (uint16_t i = 0; i < nb_mcu_par_colonne - 1; i++) {
    /* les mcu de la ligne sauf le dernier sont écrits en entiers */
    for (uint16_t j = 0; j < nb_mcu_par_ligne - 1; j++) {
      extrait_nouveau_MCU(mcu, header, stream);
      ecrit_MCU(mcu, largeur_image, fichier_ppm);
      free_partiel_MCU(mcu);
    }
    /* dernier MCU */
    extrait_nouveau_MCU(mcu, header, stream);
    change_width_to_write(largeur_mcus_derniere_colonne,
                          mcu); /* écrit avec la largeur spécifique des blocs de
                                   dernière colonne */
    ecrit_MCU(mcu, largeur_image, fichier_ppm);
    change_width_to_write(largeur_mcu, mcu); /* on remet à la normale */
    free_partiel_MCU(mcu);

    /* on se place au bout du fichier pour commencer à écrire la ligne suivante
     */
    fseek(fichier_ppm, 0, SEEK_END);
  }

  /* ecriture de la dernière ligne (sauf dernier bloc) */
  change_height_to_write(
      hauteur_mcus_derniere_ligne,
      mcu); /* écrit avec la hauteur spécifique des blocs de dernière ligne */
  for (uint16_t j = 0; j < nb_mcu_par_ligne - 1; j++) {
    extrait_nouveau_MCU(mcu, header, stream);
    ecrit_MCU(mcu, largeur_image, fichier_ppm);
    free_partiel_MCU(mcu);
  }
  /* dernier bloc à part */
  change_width_to_write(largeur_mcus_derniere_colonne, mcu);
  extrait_nouveau_MCU(mcu, header, stream);
  ecrit_MCU(mcu, largeur_image, fichier_ppm);
  free_final_MCU(mcu);
}