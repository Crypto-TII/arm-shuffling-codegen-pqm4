
#include "permutation.h"
#include "djbsort.h"

void sig_perk_perm_compose(perm_t o, const perm_t p1, const perm_t p2) {
    uint32_t buffer[PARAM_N1];
    perm_t tmp;
    sig_perk_perm_inverse(tmp, p2);
    for (int i = 0; i < PARAM_N1; ++i) {
        buffer[i] = (((uint32_t)tmp[i]) << 16) | p1[i];
    }
    uint32_sort(buffer, PARAM_N1);
    for (int i = 0; i < PARAM_N1; ++i) {
        o[i] = (uint16_t)(buffer[i]);
    }
}
