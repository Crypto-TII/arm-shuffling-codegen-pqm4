
#include "permutation.h"
#include "djbsort.h"

void sig_perk_perm_vect_permute(vect1_t output, const perm_t p, const vect1_t input) {
    uint32_t buffer[PARAM_N1];
    for (int i = 0; i < PARAM_N1; ++i) {
        buffer[i] = (((uint32_t)p[i]) << 16) | input[i];
    }
    uint32_sort(buffer, PARAM_N1);
    for (int i = 0; i < PARAM_N1; ++i) {
        output[i] = (uint16_t)(buffer[i]);
    }
}
