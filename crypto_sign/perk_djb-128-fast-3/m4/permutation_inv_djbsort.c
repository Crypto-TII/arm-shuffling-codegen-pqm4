
#include "permutation.h"
#include "djbsort.h"

void sig_perk_perm_inverse(perm_t o, const perm_t p) {
    uint32_t buffer[PARAM_N1];
    for (int i = 0; i < PARAM_N1; i++) {
        buffer[i] = (((uint32_t)p[i]) << 16) | i;
    }
    uint32_sort(buffer, PARAM_N1);

    for (int i = 0; i < PARAM_N1; i++) {
        o[i] = (uint16_t)(buffer[i]);
    }
}
