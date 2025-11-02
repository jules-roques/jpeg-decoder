#include <stdint.h>

#include "lecture_flux_bits.h"
#include "macros.h"

uint16_t calcule_difference(uint8_t magnitude, struct extended_FILE* stream) {
  int16_t difference;
  if (magnitude == 0) {
    difference = 0;
  } else {
    if (magnitude == 1) {
      if (fget_one_bit(stream) == 0) {
        difference = -1;
      } else {
        difference = 1;
      }
    } else {
      if (fget_one_bit(stream) == 0) {
        uint16_t indice_magnitude_tronque = fget_bits(stream, magnitude - 1);
        difference = 1 - (1 << magnitude) + (int16_t)indice_magnitude_tronque;
      } else {
        uint16_t indice_magnitude_tronque = fget_bits(stream, magnitude - 1);
        difference = (1 << (magnitude - 1)) + (int16_t)indice_magnitude_tronque;
      }
    }
  }
  return (uint16_t)difference;
}

uint16_t decodeDPCM_DC(uint16_t predicateur, uint8_t magnitude,
                       struct extended_FILE* stream) {
  return predicateur + calcule_difference(magnitude, stream);
}

uint16_t decodeDPCM_AC(uint8_t magnitude, struct extended_FILE* stream) {
  return calcule_difference(magnitude, stream);
}