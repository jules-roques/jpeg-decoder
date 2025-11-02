#ifndef _ECRITURE_PPM_
#define _ECRITURE_PPM_

#include "header_extraction.h"

void ecrit_entete_ppm(struct header* header, FILE* fichier);

void ecrit_corps_ppm(struct header* header, struct extended_FILE* stream,
                     FILE* fichier_ppm);

#endif /* _ECRITURE_PPM_ */