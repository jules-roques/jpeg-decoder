#ifndef _AFFICHAGE_
#define _AFFICHAGE_

#include <stdint.h>

/* --- CHANGE EXTENSION FICHER --- */
void change_extension(char* file_name, char* extension);
int8_t verify_file_name_ext(char* filename);

/* --- COPIE TABLEAU --- */
uint8_t* copy_array_uint8(uint8_t* array, uint8_t length);
uint16_t* copy_array_uint16(uint16_t* array, uint8_t length);
float* copy_array_float(float* array, uint8_t length);

/* --- AFFICHAGE TABLEAUX --- */
/*
Fonction d'affichage d'un tableau,
avec un alinéa et un nombre de valeurs par ligne
(pour un afficage propre)
*/
void affiche_tab_uint8(char* alinea, uint8_t nb_vals_ligne, uint8_t* tab,
                       uint16_t nb_elems);
void affiche_tab_uint16(char* alinea, uint8_t nb_vals_ligne, uint16_t* tab,
                        uint16_t nb_elems);
void affiche_tab_uint32(char* alinea, uint8_t nb_vals_ligne, uint32_t* tab,
                        uint16_t nb_elems);
void affiche_tab_float(char* alinea, uint8_t nb_vals_ligne, float* tab,
                       uint16_t nb_elems);

/* --- AFFICHAGE MATRICEE --- */
/*
Fonction d'affichage d'une de valeurs hexadécimales de un octet,
avec un alinéa et un nombre de valeurs par ligne
(pour un afficage propre)
*/
void affiche_matrice_uint8(char* alinea, uint8_t nb_lignes, uint8_t nb_colonnes,
                           uint8_t** matrice);
void affiche_matrice_uint16(char* alinea, uint8_t nb_lignes,
                            uint8_t nb_colonnes, uint16_t** matrice);
void affiche_matrice_uint32(char* alinea, uint8_t nb_lignes,
                            uint8_t nb_colonnes, uint32_t** matrice);
void affiche_matrice_float(char* alinea, uint8_t nb_lignes, uint8_t nb_colonnes,
                           float** matrice);

/* --- CONVERSION MATRICE --- */
float** matrix_uint16_to_float(uint16_t** matrix_uint, uint16_t nb_lignes,
                               uint16_t nb_colonnes);

/* --- ALLOCATION --- */
uint8_t** alloc_matrice_uint8(uint16_t nb_lignes, uint16_t nb_colonnes);
uint16_t** alloc_matrice_uint16(uint16_t nb_lignes, uint16_t nb_colonnes);
uint32_t** alloc_matrice_uint32(uint16_t nb_lignes, uint16_t nb_colonnes);
float** alloc_matrice_float(uint16_t nb_lignes, uint16_t nb_colonnes);

/* --- FREE --- */
void free_matrice_uint8(uint8_t** matrice, uint8_t nb_lignes);
void free_matrice_uint16(uint16_t** matrice, uint8_t nb_lignes);
void free_matrice_uint32(uint32_t** matrice, uint16_t nb_lignes);
void free_matrice_float(float** matrice, uint16_t nb_lignes);

#endif /* _AFFICHAGE_ */