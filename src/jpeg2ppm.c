#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "creation_arbre_Huffman.h"
#include "ecriture_ppm.h"
#include "fonctions_generiques.h"
#include "header_extraction.h"
#include "macros.h"
#include "variable_length_decoding.h"

/*
    Programme principal : prend en entrée un fichier jpeg, appelle tous les
   modules et renvoie le fichier PPM correspondant
*/
int main(int argc, char** argv) {
  if (argc != 2 || verify_file_name_ext(argv[1]) != 0) {
    print_error(stderr, "Utilisation: %s file.jpeg\n", argv[0]);
    return -1;
  }

  /* ouverture fichier jpeg en mode lecture */
  struct extended_FILE* stream = open_file(argv[1], "r");
  if (stream == NULL) {
    print_error(
        stderr,
        "Erreur d'initialisation du fichier étendu de mémoire tampon\n");
    return -1;
  }

  /* Extraction de l'en-tête */
  struct header* header = extract_header(stream);
  if (header == NULL) {
    print_error(stderr, "Problème d'extration de l'en-tête\n");
    return -1;
  }

  /* ouverture du fichier ppm en mode écriture */
  char* extension_fichier = get_image_extension(header);
  char* name_ppm = argv[1];
  change_extension(name_ppm, extension_fichier);
  FILE* fichier_ppm = fopen(name_ppm, "wb");

  ecrit_entete_ppm(header, fichier_ppm);
  ecrit_corps_ppm(header, stream, fichier_ppm);

  free_header(header); /* libération en-tête */
  fclose(fichier_ppm); /* femeture fichier ppm */
  close_file(stream);  /* lecture du marqueur de finde flux et fermeture du
                          fichier jpeg */

  return 0;
}
