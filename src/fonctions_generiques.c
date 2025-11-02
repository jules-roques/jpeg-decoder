#include "fonctions_generiques.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "macros.h"

#define MKDIR(path) mkdir(path, 0755)

void ensure_out_dir_exists() {
  // Try to create the directory
  if (MKDIR("out") == -1 && errno != EEXIST) {
    perror("Error creating 'out' directory");
    exit(EXIT_FAILURE);
  }
}

/*

Module qui contient des fonctions génériques sur
des objets connus (matrices, tableaux)

*/

/*
    Verifie le nom du fichier passé en argument (inspiré de la solution Stack
   Overflow) renvoie 0 si c'est bon
*/
int8_t verify_file_name_ext(char* filename) {
  const char* dot = strrchr(filename, '.');
  if (!dot || dot == filename) return -1;
  dot++;
  if (strcmp(dot, "jpg") != 0 && strcmp(dot, "jpeg")) return -1;
  return 0;
}

void change_dir_and_extension(char* file_name, char* new_extension) {
  ensure_out_dir_exists();

  char* base_name = strrchr(file_name, '/');
  if (base_name == NULL) {
    base_name = file_name;
  } else {
    base_name++;
  }

  char temp_name[256];
  strcpy(temp_name, base_name);

  char* dot = strrchr(temp_name, '.');
  if (dot == NULL) {
    dot = temp_name + strlen(temp_name);
    *dot = '.';
  }
  dot++;

  char* cursor = dot;
  while (*new_extension != '\0') {
    *cursor = *new_extension;
    cursor++;
    new_extension++;
  }
  *cursor = '\0';  // Add the null terminator

  strcpy(file_name, "out/");
  strcat(file_name, temp_name);
}

/* ------------------------------ COPIE DE TABLEAU
 * ------------------------------- */

uint8_t* copy_array_uint8(uint8_t* array, uint8_t length) {
  uint8_t* array_copy = malloc(length * sizeof(uint8_t));
  for (uint8_t i = 0; i < length; i++) {
    array_copy[i] = array[i];
  }
  return array_copy;
}

uint16_t* copy_array_uint16(uint16_t* array, uint8_t length) {
  uint16_t* array_copy = malloc(length * sizeof(uint16_t));
  for (uint8_t i = 0; i < length; i++) {
    array_copy[i] = array[i];
  }
  return array_copy;
}

float* copy_array_float(float* array, uint8_t length) {
  float* array_copy = malloc(length * sizeof(float));
  for (uint8_t i = 0; i < length; i++) {
    array_copy[i] = array[i];
  }
  return array_copy;
}

/* ------------------------------ CONVERSION DE MATRICE
 * ------------------------------- */

/*
    convertit en float signé (passage par int16_t avantde convertir en float)
*/
float** matrix_uint16_to_float(uint16_t** matrix_uint, uint16_t nb_lignes,
                               uint16_t nb_colonnes) {
  float** matrix_float = alloc_matrice_float(nb_lignes, nb_colonnes);
  for (uint16_t j = 0; j < nb_lignes; j++) {
    for (uint16_t i = 0; i < nb_colonnes; i++) {
      matrix_float[j][i] = (float)(int16_t)matrix_uint[j][i];
    }
  }

  return matrix_float;
}

/* ----------------------------- AFFICHAGE DE TABLEAUX
 * ---------------------------- */

/*
Fonction d'affichage d'un tableau de valeurs hexadécimales d'un octet,
avec un alinéa et un nombre de valeurs par ligne
(pour un afficage propre)
*/
void affiche_tab_uint8(char* alinea, uint8_t nb_vals_ligne, uint8_t* tab,
                       uint16_t nb_elems) {
  if (tab == NULL) {
    print_error(stderr, "Tableau vide passé en argument\n");
  }

  printf("%s0x%x ", alinea, tab[0]);
  for (uint16_t i = 1; i < nb_elems; i++) {
    if (i % nb_vals_ligne == 0) {
      printf("\n%s", alinea);
    }
    printf("0x%x ", tab[i]);
  }
  printf("\n");
}

/*
Fonction d'affichage d'un tableau de valeurs hexadécimales de deux octet,
avec un alinéa et un nombre de valeurs par ligne
(pour un afficage propre)
*/
void affiche_tab_uint16(char* alinea, uint8_t nb_vals_ligne, uint16_t* tab,
                        uint16_t nb_elems) {
  if (tab == NULL) {
    print_error(stderr, "Tableau vide passé en argument\n");
  }

  printf("%s0x%x ", alinea, tab[0]);
  for (uint16_t i = 1; i < nb_elems; i++) {
    if (i % nb_vals_ligne == 0) {
      printf("\n%s", alinea);
    }
    printf("0x%x ", tab[i]);
  }
  printf("\n");
}

/*
Fonction d'affichage d'un tableau de valeurs hexadécimales de deux octet,
avec un alinéa et un nombre de valeurs par ligne
(pour un afficage propre)
*/
void affiche_tab_uint32(char* alinea, uint8_t nb_vals_ligne, uint32_t* tab,
                        uint16_t nb_elems) {
  if (tab == NULL) {
    print_error(stderr, "Tableau vide passé en argument\n");
  }

  printf("%s0x%x ", alinea, tab[0]);
  for (uint16_t i = 1; i < nb_elems; i++) {
    if (i % nb_vals_ligne == 0) {
      printf("\n%s", alinea);
    }
    printf("0x%x ", tab[i]);
  }
  printf("\n");
}

void affiche_tab_float(char* alinea, uint8_t nb_vals_ligne, float* tab,
                       uint16_t nb_elems) {
  if (tab == NULL) {
    print_error(stderr, "Tableau vide passé en argument\n");
  }

  printf("%s%f ", alinea, tab[0]);
  for (uint16_t i = 1; i < nb_elems; i++) {
    if (i % nb_vals_ligne == 0) {
      printf("\n%s", alinea);
    }
    printf("%f ", tab[i]);
  }
  printf("\n");
}

/* ----------------------------- AFFICHAGE DE MATRICE
 * ---------------------------- */

/*
Fonction d'affichage d'une de valeurs hexadécimales de un octet,
avec un alinéa et un nombre de valeurs par ligne
(pour un afficage propre)
*/
void affiche_matrice_uint8(char* alinea, uint8_t nb_lignes, uint8_t nb_colonnes,
                           uint8_t** matrice) {
  if (matrice == NULL) {
    print_error(stderr, "Matrice vide passée en argument\n");
  }

  for (uint8_t i = 0; i < nb_lignes; i++) {
    affiche_tab_uint8(alinea, nb_colonnes, matrice[i], nb_colonnes);
  }
}

/*
Fonction d'affichage d'une de valeurs hexadécimales de deux octet,
avec un alinéa et un nombre de valeurs par ligne
(pour un afficage propre)
*/
void affiche_matrice_uint16(char* alinea, uint8_t nb_lignes,
                            uint8_t nb_colonnes, uint16_t** matrice) {
  if (matrice == NULL) {
    print_error(stderr, "Matrice vide passée en argument\n");
  }

  for (uint8_t i = 0; i < nb_lignes; i++) {
    affiche_tab_uint16(alinea, nb_colonnes, matrice[i], nb_colonnes);
  }
}

/*
Fonction d'affichage d'une de valeurs hexadécimales de deux octet,
avec un alinéa et un nombre de valeurs par ligne
(pour un afficage propre)
*/
void affiche_matrice_uint32(char* alinea, uint8_t nb_lignes,
                            uint8_t nb_colonnes, uint32_t** matrice) {
  if (matrice == NULL) {
    print_error(stderr, "Matrice vide passée en argument\n");
  }

  for (uint8_t i = 0; i < nb_lignes; i++) {
    affiche_tab_uint32(alinea, nb_colonnes, matrice[i], nb_colonnes);
  }
}

void affiche_matrice_float(char* alinea, uint8_t nb_lignes, uint8_t nb_colonnes,
                           float** matrice) {
  if (matrice == NULL) {
    print_error(stderr, "Matrice vide passée en argument\n");
  }

  for (uint8_t i = 0; i < nb_lignes; i++) {
    affiche_tab_float(alinea, nb_colonnes, matrice[i], nb_colonnes);
  }
}

/* ------------------------------- ALLOCATION DE MATRICE
 * ------------------------------- */

uint8_t** alloc_matrice_uint8(uint16_t nb_lignes, uint16_t nb_colonnes) {
  uint8_t** matrice = malloc(nb_lignes * sizeof(uint8_t*));
  for (uint16_t i = 0; i < nb_lignes; i++) {
    matrice[i] = malloc(nb_colonnes * sizeof(uint8_t));
  }
  return matrice;
}

uint16_t** alloc_matrice_uint16(uint16_t nb_lignes, uint16_t nb_colonnes) {
  uint16_t** matrice = malloc(nb_lignes * sizeof(uint16_t*));
  for (uint16_t i = 0; i < nb_lignes; i++) {
    matrice[i] = malloc(nb_colonnes * sizeof(uint16_t));
  }
  return matrice;
}

uint32_t** alloc_matrice_uint32(uint16_t nb_lignes, uint16_t nb_colonnes) {
  uint32_t** matrice = malloc(nb_lignes * sizeof(uint32_t*));
  for (uint16_t i = 0; i < nb_lignes; i++) {
    matrice[i] = malloc(nb_colonnes * sizeof(uint32_t));
  }
  return matrice;
}

float** alloc_matrice_float(uint16_t nb_lignes, uint16_t nb_colonnes) {
  float** matrice = malloc(nb_lignes * sizeof(float*));
  for (uint16_t i = 0; i < nb_lignes; i++) {
    matrice[i] = malloc(nb_colonnes * sizeof(float));
  }
  return matrice;
}

/* ----------------------------------------- FREE DE MATRICE
 * ---------------------------------- */

void free_matrice_uint8(uint8_t** matrice, uint8_t nb_lignes) {
  for (uint8_t i = 0; i < nb_lignes; i++) {
    free(matrice[i]);
  }
  free(matrice);
}

void free_matrice_uint16(uint16_t** matrice, uint8_t nb_lignes) {
  for (uint8_t i = 0; i < nb_lignes; i++) {
    free(matrice[i]);
  }
  free(matrice);
}

void free_matrice_uint32(uint32_t** matrice, uint16_t nb_lignes) {
  for (uint8_t i = 0; i < nb_lignes; i++) {
    free(matrice[i]);
  }
  free(matrice);
}

void free_matrice_float(float** matrice, uint16_t nb_lignes) {
  for (uint8_t i = 0; i < nb_lignes; i++) {
    free(matrice[i]);
  }
  free(matrice);
}