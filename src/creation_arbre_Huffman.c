#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fonctions_generiques.h"
#include "header_extraction.h"
#include "lecture_flux_bits.h"
#include "macros.h"

/* --------------------------------- STRUCTURE DE FILE
 * --------------------------------- */

struct Element {
  uint8_t profondeur;
  struct arbre* arbre;
  struct Element* suivant;
};

struct File {
  struct Element* premier;
};

void enfiler(struct File* file, uint8_t nouvelle_profondeur,
             struct arbre* nouvel_arbre) /* inspiré d'un exemple sur internet */
{
  struct Element* nouveau = malloc(sizeof(struct Element));
  if (file == NULL || nouveau == NULL) {
    print_error(stderr, "Il n'y a rien à enfiler\n");
    exit(EXIT_FAILURE);
  }

  nouveau->profondeur = nouvelle_profondeur;
  nouveau->arbre = nouvel_arbre;
  nouveau->suivant = NULL;

  if (file->premier != NULL) /* La file n'est pas vide */
  {
    /* On se positionne à la fin de la file */
    struct Element* elementActuel = file->premier;
    while (elementActuel->suivant != NULL) {
      elementActuel = elementActuel->suivant;
    }
    elementActuel->suivant = nouveau;
  } else /* La file est vide, notre élément est le premier */
  {
    file->premier = nouveau;
  }
}

struct Element* defiler(
    struct File* file) /* inspiré d'un exemple sur internet */
{
  /* On vérifie s'il y a quelque chose à défiler */
  if (file == NULL || file->premier == NULL) {
    print_error(stderr, "Il n'y a rien à défiler\n");
    return NULL;
  }

  struct Element* elementDefile = file->premier;
  file->premier = elementDefile->suivant;

  return elementDefile;
}

/* --------------------------------------------------------- STRUCTURE D'ARBRE
 * ----------------------------------------------------------------- */

struct arbre {
  uint8_t valeur;
  struct arbre* gauche;
  struct arbre* droit;
};

void free_arbre(struct arbre* pt_arbre) {
  if (pt_arbre == NULL) {
    print_warning(stdout, "Tentative de libérer l'adresse NULL\n");
    return;
  }
  if (pt_arbre->gauche != NULL) {
    free_arbre(pt_arbre->gauche);
  }
  if (pt_arbre->droit != NULL) {
    free_arbre(pt_arbre->droit);
  }
  free(pt_arbre);
}

struct arbre* arbre_vide(void) {
  struct arbre* arbre = malloc(sizeof(struct arbre));
  arbre->valeur = 0;
  arbre->gauche = NULL;
  arbre->droit = NULL;
  return arbre;
}

/*
    calcule la profondeur de l'arbre
*/
uint8_t profondeur(uint8_t tab[]) {
  uint8_t profondeur_max = 0;
  for (uint8_t i = 0; i < 16; i++) {
    if (tab[i] != 0) {
      profondeur_max = i;
    }
  }
  return profondeur_max + 1;
}

struct arbre* creation_arbre(uint8_t* profondeurs, uint8_t* valeurs) {
  if (profondeurs == NULL) {
    return arbre_vide();
  }

  struct arbre* arbre = malloc(sizeof(struct arbre));
  uint8_t profondeur_arbre = profondeur(profondeurs);
  arbre->valeur = 0;
  arbre->gauche = arbre_vide(); /*On part d'un arbre avec deux fils qui sont des
                                   arbres vides*/
  arbre->droit = arbre_vide();
  struct Element* premier_element_gauche = malloc(sizeof(struct Element));
  premier_element_gauche->profondeur = 1;
  premier_element_gauche->arbre =
      arbre->gauche; /*On met le premier fils gauche dans la file*/
  premier_element_gauche->suivant = NULL;
  struct File* file = malloc(sizeof(struct File));
  file->premier = premier_element_gauche;

  enfiler(file, 1, arbre->droit); /*Puis le premier fils droit*/
  uint8_t indice_valeur = 0;
  while (file->premier != NULL) {
    struct Element* element_courant = defiler(file);
    uint8_t profondeur_courante = element_courant->profondeur;
    struct arbre* arbre_courant = element_courant->arbre;
    free(element_courant);

    if (profondeur_courante < profondeur_arbre &&
        profondeurs[profondeur_courante - 1] == 0) {
      arbre_courant->gauche = arbre_vide();
      arbre_courant->droit = arbre_vide();
      enfiler(file, profondeur_courante + 1,
              arbre_courant->gauche); /* parcours en largeur */
      enfiler(file, profondeur_courante + 1, arbre_courant->droit);
    }

    else if (profondeurs[profondeur_courante - 1] !=
             0) { /* -1 car il n'y a pas de valeurs prévues pour la profondeur 0
                   */
      profondeurs[profondeur_courante - 1] -= 1;
      arbre_courant->valeur = valeurs[indice_valeur];
      indice_valeur += 1;
    }
  }
  free(profondeurs);
  free(file);
  return arbre;
}

/* ------------------------------------------------- FONCTIONS DE DECODAGE
 * ---------------------------------------------- */

uint8_t decode_Huffman_DC(uint8_t indiceDC, struct header* header,
                          struct extended_FILE* stream) {
  struct arbre* pt_arbre = get_arbre_huffman_DC(indiceDC, header);
  while (pt_arbre->gauche != NULL) {
    if (fget_one_bit(stream) == 1) {
      pt_arbre = pt_arbre->droit;
    } else {
      pt_arbre = pt_arbre->gauche;
    }
  }
  return pt_arbre->valeur;
}

uint8_t decode_Huffman_AC(uint8_t indiceAC, struct header* header,
                          struct extended_FILE* stream) {
  struct arbre* pt_arbre = get_arbre_huffman_AC(indiceAC, header);
  while (pt_arbre->gauche != NULL) {
    if (fget_one_bit(stream) == 1) {
      pt_arbre = pt_arbre->droit;
    } else {
      pt_arbre = pt_arbre->gauche;
    }
  }
  return pt_arbre->valeur;
}
