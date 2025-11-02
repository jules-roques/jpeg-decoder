#ifndef _MCU_
#define _MCU_

#include "header_extraction.h"

struct MCU;

/*
    Renvoie un MCU dont les dimenstions ont été initialisée
    (hauteur, largeur et nombre de composantes par pixel)
*/
struct MCU* initialise_MCU(struct header* header);

/*
    Affiche le MCU sur la sortie standard
*/
void print_MCU(struct MCU* mcu);

/*
    Libère les informations changeantes du mcu (matrice de pixels, blocs des
   composantes)
*/
void free_partiel_MCU(struct MCU* mcu);

/*
    Libère le reste du MCU qui n'est pas libéré par "free_partiel_MCU"
*/
void free_final_MCU(struct MCU* mcu);

void change_height_to_write(uint8_t new_height, struct MCU* mcu);

void change_width_to_write(uint8_t new_witdh, struct MCU* mcu);

/*
    met à jour la matrice de pixels correspondant au prochain MCU du fichier.
    un pixel est représenté par un entier sur 32 bits, les premiers octets
    sevant à stocker l'information (le premier pour nuances de gris,
    les 3 premiers pour RGB)
*/
void extrait_nouveau_MCU(struct MCU* mcu, struct header* header,
                         struct extended_FILE* stream);

/*
    Libère la matrice de pixels du MCU, mais pas le MCU
*/
void free_MCU(struct MCU* mcu);

/*
    Ecrit un MCU de plus dans le fichier ppm (suivant ceux d'avant)
        - hauteur_MCU_a_ecrire et largeur_MCU_a_ecrire correspondent à ce qu'il
   faut écrire du MCU, inférieur ou égale à la taille du MCU, utile en cas de
   troncature
        - nb_de_composantes (1 si gris, 3 si RGB)
*/
void ecrit_MCU(struct MCU* mcu, uint16_t largeur_image, FILE* fichier_ppm);

uint8_t get_mcu_height(struct MCU* mcu);

uint8_t get_mcu_width(struct MCU* mcu);

#endif /* _MCU_ */