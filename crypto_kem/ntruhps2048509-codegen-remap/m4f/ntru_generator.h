#ifndef NTRU_GENERATOR_H
#define NTRU_GENERATOR_H

#include <stddef.h>
#include <stdint.h>

void initialize_packed_ntru_shuffling_array(uint8_t v[], size_t n);
void unpack_ntru_shuffling_array(uint16_t v_unpacked[], const uint8_t v_packed[], size_t n);
void shuffling_ntru(const int16_t si[], size_t n, uint32_t asm_word[]);

#endif  // NTRU_GENERATOR_H
