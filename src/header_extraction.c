#include "header_extraction.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "creation_arbre_Huffman.h"
#include "fonctions_generiques.h"
#include "lecture_flux_bits.h"
#include "macros.h"

/* ------------------------------- STRUCTURES ET FONCTIONS ASSOCEES (extraction
 * de l'en-tête, affichage, free) ------------------------ */
/*

Définition de toutes les structures nécéssaires
pour extraire proprement les différentes sections de l'en-tête.

Pour chaque structure on écrit
- La structure
- Sa fonction d'extracion de l'en-tête (prenant soin de gérer les situation non
autorisées par la norme en levant des erreurs)
- Son free
- Sa fonction d'affichage sur la sortie standard

*/

/* ----------------------------------------- SECTION APPO (Application data)
 * ---------------------------------- */

struct APPO_section {
  uint16_t taille;
  char* jfif;
  uint8_t X; /* JFIF version X.Y (doit valoir 1.1)*/
  uint8_t Y;
};

struct APPO_section* extract_APPO_section(struct extended_FILE* stream) {
  struct APPO_section* appo = malloc(sizeof(struct APPO_section));
  appo->taille = fget_2_bytes(stream);
  appo->jfif = malloc(5 * sizeof(char));
  for (uint8_t i = 0; i < 5; i++) {
    appo->jfif[i] = (char)fget_one_byte(stream);
  }
  appo->X = fget_one_byte(stream);
  appo->Y = fget_one_byte(stream);

  /* Gestion des motifs non reconnus */
  if (strcmp(appo->jfif, "JFIF") != 0) {
    print_error(
        stderr,
        "Le format d'encapsulation attendu (JFIF) n'est pas le bon: %s\n",
        appo->jfif);
    return NULL;
  }

  if (appo->X != 1 || appo->Y != 1) {
    print_error(stderr,
                "La version du format d'encapsulation JFIF attendue (1.1) "
                "n'est pas la bonne: %u.%u\n",
                appo->X, appo->Y);
    return NULL;
  }

  /* bits non traités */
  skip_bytes(stream, 7);

  return appo;
}

void free_APPO_section(struct APPO_section* appo) {
  if (appo == NULL) {
    print_warning(stdout, "Tentative de libérer l'adresse NULL\n");
    return;
  }

  free(appo->jfif);
  free(appo);
}

void print_APPO_section(struct APPO_section* appo) {
  printf("[APPO]  taille %u octets\n", appo->taille);
  printf("        Application %s %u.%u\n", appo->jfif, appo->X, appo->Y);
  printf("        7 octets ignorés\n");
}

/* ------------------------------------------------ SECTION DHT (Tables de
 * Huffman) -------------------------------------------- */

struct DHT_section {
  uint16_t taille;
  uint8_t type;
  uint8_t indice;
  uint8_t nb_profondeurs;
  uint8_t* tableau_profondeurs;
  uint16_t nb_symboles;
  uint8_t* tableau_symboles;
};

struct DHT_section* extract_DHT_section(struct extended_FILE* stream) {
  struct DHT_section* dht = malloc(sizeof(struct DHT_section));
  dht->taille = fget_2_bytes(stream);
  uint16_t zeros = fget_bits(stream, 3);
  dht->type = fget_one_bit(stream);
  dht->indice = fget_bits(stream, 4);

  /* Gestion des erreurs de lecture pour section DHT */
  if (zeros != 0) {
    print_error(stderr,
                "les 3 bits avant le type de table de Huffman dans le "
                "flux doivent être à 0\n");
    return NULL;
  }
  if (dht->indice > 3) {
    print_error(stderr,
                "L'indice de la table de Huffman doit être entre 0 et 3 "
                "compris. Indice trouvé: %u\n",
                dht->indice);
    return NULL;
  }

  dht->nb_profondeurs = 16;
  dht->tableau_profondeurs = malloc(dht->nb_profondeurs * sizeof(uint8_t));
  for (uint8_t i = 0; i < dht->nb_profondeurs; i++) {
    dht->tableau_profondeurs[i] = fget_one_byte(stream);
  }

  dht->nb_symboles = dht->taille - 19;
  dht->tableau_symboles = malloc(dht->nb_symboles * sizeof(uint8_t));
  for (uint16_t i = 0; i < dht->nb_symboles; i++) {
    dht->tableau_symboles[i] = fget_one_byte(stream);
  }

  return dht;
}

void free_DHT_section(struct DHT_section* dht) {
  if (dht == NULL) {
    print_warning(stdout, "Tentative de libérer l'adresse NULL\n");
    return;
  }

  free(dht->tableau_profondeurs);
  free(dht->tableau_symboles);
  free(dht);
}

void print_DHT_section(struct DHT_section* dht) {
  printf("[DHT]   taille %u octets\n", dht->taille);
  printf("        Table Hufmann de type %s\n", dht->type == 0 ? "DC" : "AC");
  printf("        Table de Hufmann d'indice %u\n", dht->indice);
  printf("        Profondeurs (%u):\n", dht->nb_profondeurs);
  affiche_tab_uint8("                ", 40, dht->tableau_profondeurs,
                    (uint16_t)dht->nb_profondeurs);
  printf("        Symboles (%u):\n", dht->nb_symboles);
  affiche_tab_uint8("                ", 40, dht->tableau_symboles,
                    dht->nb_symboles);
}

/* ------------------------------------------ SECTION DQT (Tables de
 * quantification) ----------------------------------------------- */

struct DQT_section {
  uint16_t taille;
  uint8_t precision;            /* sur 4 bits dans le bitstream */
  uint8_t indice_table_quantif; /* sur 4 bits dans le bitstream */
  uint8_t nb_elems_table_quantif;
  uint16_t* table_quantif; /* des octets que l'on stocke sur des uint16 */
};

struct DQT_section* extract_DQT_section(struct extended_FILE* stream) {
  struct DQT_section* dqt = malloc(sizeof(struct DQT_section));
  dqt->taille = fget_2_bytes(stream);

  dqt->precision = fget_bits(stream, 4);
  if (dqt->precision != 0 && dqt->precision != 1) {
    print_error(stderr, "Indice de précision doit être 0 ou 1. Trouvé: %u\n",
                dqt->precision);
    return NULL;
  }

  dqt->indice_table_quantif = fget_bits(stream, 4);
  if (dqt->indice_table_quantif > 3) {
    print_error(stderr,
                "Indice de table de quantification doit être entre 0 et 3 "
                "compris. Trouvé: %u\n",
                dqt->indice_table_quantif);
    return NULL;
  }
  if (dqt->indice_table_quantif > 1) {
    print_error(stderr,
                "Notre programme ne supporte pas correctement de tables de "
                "quantification d'indice de plus de 2. Trouvé: %u\n",
                dqt->indice_table_quantif);
    return NULL;
  }

  dqt->nb_elems_table_quantif = 64;
  dqt->table_quantif = malloc(dqt->nb_elems_table_quantif * sizeof(uint16_t));
  for (uint8_t i = 0; i < dqt->nb_elems_table_quantif; i++) {
    dqt->table_quantif[i] = (uint16_t)fget_one_byte(stream);
  }

  return dqt;
}

void free_DQT_section(struct DQT_section* dqt) {
  if (dqt == NULL) {
    print_warning(stdout, "Tentative de libérer l'adresse NULL\n");
    return;
  }

  free(dqt->table_quantif);
  free(dqt);
}

void print_DQT_section(struct DQT_section* dqt) {
  char* precision_en_bits;
  switch (dqt->precision) {
    case 0:
      precision_en_bits = "8";
      break;
    case 1:
      precision_en_bits = "16";
      break;
    default:
      precision_en_bits = "?";
      break;
  }
  printf("[DQT]   taille %u octets\n", dqt->taille);
  printf("        précision quantification %s bits\n", precision_en_bits);
  printf("        Table de quantification d'indice %u\n",
         dqt->indice_table_quantif);
  printf("        Table de quantification (%u elems):\n",
         dqt->nb_elems_table_quantif);
  affiche_tab_uint16("                ", 40, dqt->table_quantif,
                     (uint16_t)dqt->nb_elems_table_quantif);
}

/* --------------------------------------------------- SECTION SOFO (Start of
 * frame) ------------------------------------------------ */

/* --- SOUS SECTION (une pour chaque comporante, N composantes, cf sujet) --- */

struct SOFO_sub_section {
  uint8_t id_composante; /* 1 pour Y, 2 pour Cb et 3 pour Cr (seulement pour
                            notre décodeur) */
  uint8_t facteur_ech_horizontal;
  uint8_t facteur_ech_vertical;
  uint8_t ind_table_quantif;
};

char* nom_composante(uint8_t indice_appartition) {
  switch (indice_appartition) {
    case 0:
      return "Y";
    case 1:
      return "Cb";
    case 2:
      return "Cr";
    default:
      return "?";
  }
}

struct SOFO_sub_section* extract_SOFO_sub_section(
    struct extended_FILE* stream) {
  struct SOFO_sub_section* sub_sofo = malloc(sizeof(struct SOFO_sub_section));
  sub_sofo->id_composante = fget_one_byte(stream);
  sub_sofo->facteur_ech_horizontal = fget_bits(stream, 4);
  sub_sofo->facteur_ech_vertical = fget_bits(stream, 4);
  sub_sofo->ind_table_quantif = fget_one_byte(stream);

  if (sub_sofo->facteur_ech_horizontal < 1 ||
      sub_sofo->facteur_ech_horizontal > 4) {
    print_error(
        stderr,
        " Composante d'identifiant %u: "
        "le facteur d'échantillonage horizontal doit être comris entre 1 et 4."
        " On trouve %u\n",
        sub_sofo->id_composante, sub_sofo->facteur_ech_horizontal);
    return NULL;
  }
  if (sub_sofo->facteur_ech_vertical < 1 ||
      sub_sofo->facteur_ech_vertical > 4) {
    print_error(
        stderr,
        " Composante d'identifiant %u: "
        "le facteur d'échantillonage vertical doit être comris entre 1 et 4."
        " On trouve %u\n",
        sub_sofo->id_composante, sub_sofo->facteur_ech_vertical);
    return NULL;
  }
  if (sub_sofo->ind_table_quantif > 3) {
    print_error(stderr,
                " Composante d'identifiant %u: "
                "l'indice de table de quantification associée doit être entre "
                "0 et 3 compris."
                " On trouve %u\n",
                sub_sofo->id_composante, sub_sofo->ind_table_quantif);
    return NULL;
  }

  return sub_sofo;
}

void free_SOFO_sub_section(struct SOFO_sub_section* sub_sofo) {
  free(sub_sofo);
}

void print_SOFO_sub_section(char* alinea, uint8_t indice_appartition,
                            struct SOFO_sub_section* sub_sofo) {
  printf("%sComposante %s\n", alinea, nom_composante(indice_appartition));
  printf("%s        id %u\n", alinea, sub_sofo->id_composante);
  printf("%s        facteurs d'échantillonage (hxv) %ux%u\n", alinea,
         sub_sofo->facteur_ech_horizontal, sub_sofo->facteur_ech_vertical);
  printf("%s        table quantification indice %u\n", alinea,
         sub_sofo->ind_table_quantif);
}

/* --- SECTION --- */

struct SOFO_section {
  uint16_t taille;
  uint8_t precision;
  uint16_t hauteur_image;
  uint16_t largeur_image;
  uint8_t nb_composantes;                  /* 1 si gris, 3 si couleurs*/
  struct SOFO_sub_section** sous_sections; /* ordre des composantes est toujours
                                              le même : Y puis Cb puis Cr*/
};

struct SOFO_section* extract_SOFO_section(struct extended_FILE* stream) {
  struct SOFO_section* sofo = malloc(sizeof(struct SOFO_section));
  sofo->taille = fget_2_bytes(stream);
  sofo->precision = fget_one_byte(stream);
  sofo->hauteur_image = fget_2_bytes(stream);
  sofo->largeur_image = fget_2_bytes(stream);
  sofo->nb_composantes = fget_one_byte(stream);

  uint8_t somme_de_produits = 0;
  sofo->sous_sections =
      malloc(sofo->nb_composantes * sizeof(struct SOFO_sub_section*));
  for (uint8_t i = 0; i < sofo->nb_composantes; i++) {
    sofo->sous_sections[i] = extract_SOFO_sub_section(stream);
    if (sofo->sous_sections[i] == NULL) {
      print_error(stderr,
                  "Erreur lors de l'excration s'une sous-section SOFO\n");
      return NULL;
    }
    if (sofo->sous_sections[0]->facteur_ech_horizontal %
            sofo->sous_sections[i]->facteur_ech_horizontal !=
        0) {
      print_error(stderr,
                  "Le facteur d'échantillonage horizontal de la composante %s "
                  "ne divise pas parfaitement celui de Y. "
                  "(%u ne divise pas %u)\n",
                  nom_composante(i),
                  sofo->sous_sections[i]->facteur_ech_horizontal,
                  sofo->sous_sections[0]->facteur_ech_horizontal);
      return NULL;
    }
    if (sofo->sous_sections[0]->facteur_ech_vertical %
            sofo->sous_sections[i]->facteur_ech_vertical !=
        0) {
      print_error(stderr,
                  "Le facteur d'échantillonage vertical de la composante %s ne "
                  "divise pas parfaitement celui de Y. "
                  "(%u ne divise pas %u)\n",
                  nom_composante(i),
                  sofo->sous_sections[i]->facteur_ech_vertical,
                  sofo->sous_sections[0]->facteur_ech_vertical);
      return NULL;
    }
    somme_de_produits += sofo->sous_sections[i]->facteur_ech_horizontal *
                         sofo->sous_sections[i]->facteur_ech_vertical;
  }

  if (somme_de_produits > 10) {
    print_error(stderr,
                "La somme des produits hxv doit être inférieure ou égale à 10. "
                "On trouve %u\n",
                somme_de_produits);
    return NULL;
  }

  return sofo;
}

void free_SOFO_section(struct SOFO_section* sofo) {
  if (sofo == NULL) {
    print_warning(stdout, "Tentative de libérer l'adresse NULL\n");
    return;
  }

  for (uint8_t i = 0; i < sofo->nb_composantes; i++) {
    free_SOFO_sub_section(sofo->sous_sections[i]);
  }
  free(sofo->sous_sections);
  free(sofo);
}

void print_SOFO_section(struct SOFO_section* sofo) {
  printf("[SOFO]  taille %u octets\n", sofo->taille);
  printf("        précision coeffs %u bits\n", sofo->precision);
  printf("        hauteur image %u\n", sofo->hauteur_image);
  printf("        largeur image %u\n", sofo->largeur_image);
  printf("        nb de composantes %u\n", sofo->nb_composantes);
  for (uint8_t i = 0; i < sofo->nb_composantes; i++) {
    print_SOFO_sub_section("        ", i, sofo->sous_sections[i]);
  }
}

/* ------------------------------------- SECTION SOS (Start of scan)
 * -------------------------------------- */

/* --- SOUS SECTION --- */

struct SOS_sub_section {
  uint8_t id_composante;
  uint8_t ind_table_huffman_DC;
  uint8_t ind_table_huffman_AC;
};

struct SOS_sub_section* extract_SOS_sub_section(struct extended_FILE* stream) {
  struct SOS_sub_section* sub_sos = malloc(sizeof(struct SOS_sub_section));
  sub_sos->id_composante = fget_one_byte(stream);
  sub_sos->ind_table_huffman_DC = fget_bits(stream, 4);
  sub_sos->ind_table_huffman_AC = fget_bits(stream, 4);

  return sub_sos;
}

void free_SOS_sub_section(struct SOS_sub_section* sub_sos) { free(sub_sos); }

void print_SOS_sub_section(char* alinea, struct SOS_sub_section* sub_sos) {
  printf("%sassocié à composante d'identifiant %u\n", alinea,
         sub_sos->id_composante);
  printf("%sassocié à table Huffman DC d'indice %u\n", alinea,
         sub_sos->ind_table_huffman_DC);
  printf("%sassocié à table Huffman AC d'indice %u\n", alinea,
         sub_sos->ind_table_huffman_AC);
}

/* --- SECTION --- */

struct SOS_section {
  uint16_t taille;
  uint8_t nb_composantes;
  struct SOS_sub_section** sous_sections;
  uint8_t Ss;
  uint8_t Se;
  uint8_t Ah; /* 4 bits sur le bitstream*/
  uint8_t Al; /* 4 bits sur le bitstream*/
};

struct SOS_section* extract_SOS_section(struct extended_FILE* stream) {
  struct SOS_section* sos = malloc(sizeof(struct SOS_section));
  sos->taille = fget_2_bytes(stream);
  sos->nb_composantes = fget_one_byte(stream);

  sos->sous_sections =
      malloc(sos->nb_composantes * sizeof(struct SOS_sub_section*));
  for (uint8_t i = 0; i < sos->nb_composantes; i++) {
    sos->sous_sections[i] = extract_SOS_sub_section(stream);
  }
  sos->Ss = fget_one_byte(stream);
  sos->Se = fget_one_byte(stream);
  sos->Ah = fget_bits(stream, 4);
  sos->Al = fget_bits(stream, 4);

  /* Gestion des cas non conformes au BASELINE */
  if (sos->Ss != 0 || sos->Se != 63) {
    print_error(stderr,
                "Les indices de la sélection spectrale trouvés (%u,%u) "
                "ne sont pas conformes au mode baseline (qui attend les "
                "indices 0,63)\n",
                sos->Ss, sos->Se);
    return NULL;
  }

  if (sos->Ah != 0 || sos->Al != 0) {
    print_error(
        stderr,
        "ERREUR: Les informations d'approximation des coeffs AC pour le mode "
        "progressif "
        "doivent être nulles pour en mode baseline. Trouvés: %u et %u\n",
        sos->Ah, sos->Al);
    return NULL;
  }

  return sos;
}

void free_SOS_section(struct SOS_section* sos) {
  if (sos == NULL) {
    print_warning(stdout, "Tentative de libérer l'adresse NULL\n");
    return;
  }

  for (uint8_t i = 0; i < sos->nb_composantes; i++) {
    free_SOS_sub_section(sos->sous_sections[i]);
  }
  free(sos->sous_sections);
  free(sos);
}

void print_SOS_section(struct SOS_section* sos) {
  printf("[SOS]   taille %u octets\n", sos->taille);
  printf("        nombre de composantes dans le scan %u\n",
         sos->nb_composantes);
  for (uint8_t i = 0; i < sos->nb_composantes; i++) {
    printf("        indice scan %u\n", i);
    print_SOS_sub_section("                ", sos->sous_sections[i]);
  }
}

/* -------------------------------------------- SECTION COM (COMMENTAIRES)
 * -------------------------------------- */

struct COM_section {
  uint32_t taille;
  char* commentaire;
};

struct COM_section* extract_COM_section(struct extended_FILE* stream) {
  struct COM_section* com = malloc(sizeof(struct COM_section));
  com->taille = fget_2_bytes(stream);
  com->commentaire = malloc((com->taille - 1) * sizeof(char));
  for (uint32_t i = 0; i < com->taille - 2; i++) {
    com->commentaire[i] = (char)fget_one_byte(stream);
  }
  com->commentaire[com->taille - 2] = '\0';

  return com;
}

void free_COM_section(struct COM_section* com) {
  if (com == NULL) {
    print_warning(stdout, "Tentative de libérer l'adresse NULL\n");
    return;
  }
  free(com->commentaire);
  free(com);
}

void print_COM_section(struct COM_section* com) {
  printf("[COM]   taille %u octets\n", com->taille);
  printf("        Commentaire trouvé: \"%s\"\n", com->commentaire);
}

/* --------------------------------------------- EN-TETE (RASSEMBLEMENT DES
 * SECTIONS) -------------------------------------------------- */
/*
On va maintenant rassembler toutes les structures définies plus haut
dans une structure englobante.
*/

struct header {
  /* Valeurs des cos, sin et sqrt(2) pour la mémoïsation de l'iDCT rapide */
  float* tab_memoisation;

  /* Sections uniques */
  struct APPO_section* appo;
  struct SOFO_section* sofo;
  struct SOS_section*
      sos; /* (Il y a plusieurs SOS dans le décodeur progressif) */

  /* Sections pouvant apparaître plusieurs fois */
  uint8_t nb_dht;
  uint8_t nb_dqt;
  uint8_t nb_com;
  struct DHT_section** dht_sections;
  struct arbre***
      matrice_arbres; /* organisé en matrice: le type de matrice pour les ligne
                         et l'incide de matrice pour les colonnes */
  struct DQT_section** dqt_sections;
  struct COM_section** com_sections;
};

struct header* extract_header(struct extended_FILE* stream) {
  /* marqueur d'une section JPEG */
  uint16_t marqueur;

  /* on véirie le marqeur de début d'image,
  on renvoie une erreur si le motif n'est pas reconnu */
  marqueur = fget_2_bytes(stream);
  if (marqueur != 0xffd8) {
    print_error(
        stderr,
        "Le marqueur de début de section 0xffd8 n'est pas reconnu: 0x%x\n",
        marqueur);
    return NULL;
  }

  /* Calloc pour initialiser la struct header à 0 */
  struct header* header = calloc(1, sizeof(struct header));

  do {
    /* On récupère le marqueur de la section suivante dans le bitstream */
    marqueur = fget_2_bytes(stream);
    switch (marqueur) {
      case 0xffe0: /* APPO */
        header->appo = extract_APPO_section(stream);
        if (header->appo == NULL) {
          print_error(stderr, "Erreur d'extraction de de la section APPO\n");
          return NULL;
        }
        break;

      case 0xffc0: /* SOFO */
        header->sofo = extract_SOFO_section(stream);
        if (header->sofo == NULL) {
          print_error(stderr, "Erreur d'extraction de de la section SOFO\n");
          return NULL;
        }
        break;

      case 0xffda: /* SOS */
        header->sos = extract_SOS_section(stream);
        if (header->sos == NULL) {
          print_error(stderr, "Erreur d'extraction de la section SOS\n");
          return NULL;
        }
        break;

      case 0xffc4: /* DHT */
        header->dht_sections =
            realloc(header->dht_sections,
                    (header->nb_dht + 1) * sizeof(struct DHT_section*));
        struct DHT_section* dht = extract_DHT_section(stream);
        if (dht == NULL) {
          print_error(stderr, "Erreur d'extraction de la section DHT\n");
          return NULL;
        }
        header->dht_sections[header->nb_dht] = dht;
        header->nb_dht++;
        break;

      case 0xffdb: /* DQT */
        header->dqt_sections =
            realloc(header->dqt_sections,
                    (header->nb_dqt + 1) * sizeof(struct DQT_section*));
        struct DQT_section* dqt = extract_DQT_section(stream);
        if (dqt == NULL) {
          print_error(stderr, "Erreur d'extraction de la section DQT\n");
          return NULL;
        }
        header->dqt_sections[header->nb_dqt] = dqt;
        header->nb_dqt++;
        break;

      case 0xfffe: /* COM */
        header->com_sections =
            realloc(header->com_sections,
                    (header->nb_com + 1) * sizeof(struct COM_section*));
        struct COM_section* com = extract_COM_section(stream);
        if (com == NULL) {
          print_error(stderr, "Erreur d'extraction de la section COM\n");
          return NULL;
        }
        header->com_sections[header->nb_com] = com;
        header->nb_com++;
        break;

      default:
        print_error(stderr,
                    "Marqueur de début de section non reconnu dans le fichier "
                    "jpeg: 0x%x\n",
                    marqueur);
        return NULL;
    }
  } while (marqueur !=
           0xffda); /* Une fois que l'on a lu la section S0S on s'arrête */

  /* Tri des du tablau header->sections_dqt, pour que l'indice de la table de
   * quantif soit le même que sa position dans le tableau */
  struct DQT_section** dqt_sections_triees =
      malloc(header->nb_dqt * sizeof(struct DQT_section*));
  for (uint8_t i = 0; i < header->nb_dqt; i++) {
    dqt_sections_triees[header->dqt_sections[i]->indice_table_quantif] =
        header->dqt_sections[i];
  }
  free(header->dqt_sections);
  header->dqt_sections = dqt_sections_triees;

  /* Remplissage de la matrice d'arbres de Huffman (normalement de taille 2x2)

      - les indices de la matrice correnspondes auc indices de la table de
     Huffman les lignes pour l'indice i_H les colonnes pour l'indice i_AC/DC

              Matrice principale:
                              [ DC0 , DC1 ]
                              [ AC0 , AC1 ]

      - chaque sous matrice contient deux lignes
          une pour un tableau de profondeurs
          une pour un tableau de symboles

              Sous-matrices:
                              [profondeur 1, ... , profondeur 16]
                              [symbole 0, ... , symbole n]
  */
  /* Allocation et initialisation à NULL */
  header->matrice_arbres = calloc(2, sizeof(struct arbre**));
  for (uint8_t i = 0; i < 2; i++) {
    header->matrice_arbres[i] = calloc(2, sizeof(struct arbre*));
  }
  /* Assignation */
  for (uint8_t i = 0; i < header->nb_dht; i++) {
    uint8_t ligne = header->dht_sections[i]->type;
    uint8_t colonne = header->dht_sections[i]->indice;
    uint8_t* profondeurs = header->dht_sections[i]->tableau_profondeurs;
    uint8_t nb_profondeurs = header->dht_sections[i]->nb_profondeurs;
    uint8_t* symboles = header->dht_sections[i]->tableau_symboles;
    header->matrice_arbres[ligne][colonne] =
        creation_arbre(copy_array_uint8(profondeurs, nb_profondeurs), symboles);
  }

  /* Tableau qui contient les valeurs utiles que l'on ne recalculera
  pas à chaque iDCT */
  header->tab_memoisation = malloc(7 * sizeof(float));
  header->tab_memoisation[0] = cosf(_pi_ / 16);
  header->tab_memoisation[1] = sinf(_pi_ / 16);
  header->tab_memoisation[2] = cosf(3 * _pi_ / 16);
  header->tab_memoisation[3] = sinf(3 * _pi_ / 16);
  header->tab_memoisation[4] = cosf(3 * _pi_ / 8) / sqrt(2);
  header->tab_memoisation[5] = sinf(3 * _pi_ / 8) / sqrt(2);
  header->tab_memoisation[6] = 1 / sqrt(2);

  return header;
}

void free_header(struct header* header) {
  if (header == NULL) {
    print_warning(stdout, "Tentative de libérer l'adresse NULL\n");
    return;
  }

  free(header->tab_memoisation);

  /* free section APPO */
  free_APPO_section(header->appo);

  /* free section SOFO */
  free_SOFO_section(header->sofo);

  /* free sections SOS */
  free_SOS_section(header->sos);

  /* free sections DHT */
  for (uint8_t i = 0; i < header->nb_dht; i++) {
    free_DHT_section(header->dht_sections[i]);
  }
  free(header->dht_sections);

  /* libération de la matrice d'arbre de huffman */
  for (uint8_t i = 0; i < 2; i++) {
    for (uint8_t j = 0; j < 2; j++) {
      if (header->matrice_arbres[i][j] != NULL) {
        free_arbre(header->matrice_arbres[i][j]);
      }
    }
    free(header->matrice_arbres[i]);
  }
  free(header->matrice_arbres);

  /* free sections DQT */
  for (uint8_t i = 0; i < header->nb_dqt; i++) {
    free_DQT_section(header->dqt_sections[i]);
  }
  free(header->dqt_sections);

  /* free sections COM */
  for (uint8_t i = 0; i < header->nb_com; i++) {
    free_COM_section(header->com_sections[i]);
  }
  free(header->com_sections);

  /* free de toute l'en-tête */
  free(header);
}

void print_header(struct header* header) {
  /* print section APPO */
  print_APPO_section(header->appo);
  printf("\n");

  /* print sections COM */
  for (uint8_t i = 0; i < header->nb_com; i++) {
    print_COM_section(header->com_sections[i]);
  }
  if (header->nb_com != 0) printf("\n");

  /* print sections DQT */
  for (uint8_t i = 0; i < header->nb_dqt; i++) {
    print_DQT_section(header->dqt_sections[i]);
  }
  printf("\n");

  /* print section SOFO */
  print_SOFO_section(header->sofo);
  printf("\n");

  /* print sections DHT */
  for (uint8_t i = 0; i < header->nb_dht; i++) {
    print_DHT_section(header->dht_sections[i]);
  }
  printf("\n");

  /* print sections SOS */
  print_SOS_section(header->sos);
  printf("\n");
}

/* -------------------------------------------------------------- FONCTIONS POUR
 * RECUPERER INFOS -----------------------------------------------------------
 */

/* --- ARBRES DE HUFFMAN --- */

/*
    Renvoie l'arbre de Huffman en fonction
        - de son type (CD ou AC représentés par 0 ou 1 respectivement)
        - de son indice (0 ou 1)
*/
struct arbre* get_arbre_huffman(uint8_t type, uint8_t indice,
                                struct header* header) {
  if (type > 1) {
    print_error(
        stderr,
        "Le type doit être inférieur ou égal à 1. %u passé en argument\n",
        type);
    return NULL;
  }
  if (indice > 1) {
    print_error(
        stderr,
        "L'indice doit être inférieur ou égal à 1. %u passé en argument\n",
        indice);
    return NULL;
  }

  return header->matrice_arbres[type][indice];
}

struct arbre* get_arbre_huffman_DC(uint8_t indice, struct header* header) {
  return get_arbre_huffman(0, indice, header);
}

struct arbre* get_arbre_huffman_AC(uint8_t indice, struct header* header) {
  return get_arbre_huffman(1, indice, header);
}

/* --- TABLES DE QUANTIFICATION --- */

/*
    Fonction pour récupérer une table de quantification associée à une
   composante (à son identifiant)
*/
uint16_t* get_table_quantif(uint8_t identifiant_composante,
                            struct header* header) {
  uint8_t indice_table_quantif;
  for (uint8_t i = 0; i < header->sofo->nb_composantes; i++) {
    if (identifiant_composante ==
        header->sofo->sous_sections[i]->id_composante) {
      indice_table_quantif = header->sofo->sous_sections[i]->ind_table_quantif;
    }
  }
  uint16_t* table_quantif =
      header->dqt_sections[indice_table_quantif]->table_quantif;
  uint8_t taille_table_quantif =
      header->dqt_sections[indice_table_quantif]->nb_elems_table_quantif;
  return copy_array_uint16(table_quantif, taille_table_quantif);
}

/* --- INDICES TABLES DE HUFFMAN --- */

/*
    Renvoie l'indice de la table de huffman pour les coeffs DC
    de la _indice_appartition_composante_ ième composante apparaissant dans le
   MCU
*/
uint8_t get_huffman_index_DC(uint8_t indice_appartition_composante,
                             struct header* header) {
  return header->sos->sous_sections[indice_appartition_composante]
      ->ind_table_huffman_DC;
}

/*
    Renvoie l'indice de la table de huffman pour les coeffs DC
    de la _indice_appartition_composante_ ième composante apparaissant dans le
   MCU
*/
uint8_t get_huffman_index_AC(uint8_t indice_appartition_composante,
                             struct header* header) {
  return header->sos->sous_sections[indice_appartition_composante]
      ->ind_table_huffman_AC;
}

/* --- COMPOSANTES --- */

/*
    Permet de récupérer le nombre de composantes de l'image
    (3 pour RGB, 1 pour niveaux de gris)
*/
uint8_t get_number_of_components(struct header* header) {
  return header->sofo->nb_composantes;
}

/*
    Fonction pour récupérer la valeur magique de l'image
    Ex: P5 pour des niveaux de gris, P6 pour le YCbCr
*/
const char* get_magic_value(struct header* header) {
  if (header->sofo->nb_composantes == 1) {
    return "P5";
  } else if (header->sofo->nb_composantes == 3) {
    return "P6";
  }
  print_error(
      stderr,
      "Nombre de composantes non reconnu. On attend soit 1 pour des niveaux de "
      "gris, soit 3 pour une imagee RGB. Cependant on trouve %u\n",
      header->sofo->nb_composantes);
  return NULL;
}

/*
    Pour récupérer l'identifiant d'une composantes en fonction
    de son indice d'apparition dans le flux jpeg.
*/
uint8_t get_id_composante(uint8_t indice_apparition_composante,
                          struct header* header) {
  return header->sos->sous_sections[indice_apparition_composante]
      ->id_composante;
}

/* --- CARACTERISTIQUES IMAGE --- */

/*
    Fonction pour récupérer la hauteur de l'image
*/
uint16_t get_image_height(struct header* header) {
  return header->sofo->hauteur_image;
}

/*
    Fonction pour récupérer la largeur de l'image
*/
uint16_t get_image_width(struct header* header) {
  return header->sofo->largeur_image;
}

/*
    Pour récupérer  la précision en bits de l'image
*/
uint8_t get_precision_image(struct header* header) {
  if (header->sofo->precision != 8) {
    print_error(
        stderr,
        "On attend une précision de 8 bits en mode Baseline. On trouve  %u\n",
        header->sofo->precision);
    return -1;
  }
  return header->sofo->precision;
}

/* --- FACTEURS ECHANTILLONAGE --- */

/*
    Pour récupérer le facteur d'échantillonage
    horizontal de la composante Y
*/
uint8_t get_horizontal_sampling_factor_Y(struct header* header) {
  return header->sofo->sous_sections[0]->facteur_ech_horizontal;
}

/*
    Pour récupérer le facteur d'échantillonage
    vertical de la composante Y
*/
uint8_t get_vertical_sampling_factor_Y(struct header* header) {
  return header->sofo->sous_sections[0]->facteur_ech_vertical;
}

/*
    Pour récupérer le facteur d'échantillonage
    horizontal de la composante d'un certain identifiant
*/
uint8_t get_horizontal_sampling_factor(uint8_t id_composante,
                                       struct header* header) {
  for (uint8_t i = 0; i < header->sofo->nb_composantes; i++) {
    if (id_composante == header->sofo->sous_sections[i]->id_composante) {
      return header->sofo->sous_sections[i]->facteur_ech_horizontal;
    }
  }
  print_error(stderr, "Identifiant de composante %u non trouvé.\n",
              id_composante);
  return -1;
}

/*
    Pour récupérer le facteur d'échantillonage
    vertical de la composante d'un certain identifiant
*/
uint8_t get_vertical_sampling_factor(uint8_t id_composante,
                                     struct header* header) {
  for (uint8_t i = 0; i < header->sofo->nb_composantes; i++) {
    if (id_composante == header->sofo->sous_sections[i]->id_composante) {
      return header->sofo->sous_sections[i]->facteur_ech_vertical;
    }
  }
  print_error(stderr, "Identifiant de composante %u non trouvé.\n",
              id_composante);
  return -1;
}

/* --- IDENTIFIANT DE COMPOSANTE --- */

uint8_t get_id_Y(struct header* header) {
  return header->sofo->sous_sections[0]->id_composante;
}

uint8_t get_id_Cb(struct header* header) {
  return header->sofo->sous_sections[1]->id_composante;
}

uint8_t get_id_Cr(struct header* header) {
  return header->sofo->sous_sections[2]->id_composante;
}

/*
    Pour avoir l'extension du fichier d'écriture (ppm ou pgm)
*/
char* get_image_extension(struct header* header) {
  if (header->sofo->nb_composantes == 1) {
    return "pgm";
  } else {
    return "ppm";
  }
}

/* Pour récupérer les valeurs utiles de l'iDCT */

float get_value_idct(uint8_t indice, struct header* header) {
  return header->tab_memoisation[indice];
}