#ifndef _ZIG_ZAG_
#define _ZIG_ZAG_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
    Réordonnancement zigzag pour un vecteur de taille n²
    Libère le vecteur passé en paramètre.
*/
uint16_t** reordonnancement_zigzag(uint16_t* vect, uint8_t n);

#endif /* _ZIG_ZAG_ */