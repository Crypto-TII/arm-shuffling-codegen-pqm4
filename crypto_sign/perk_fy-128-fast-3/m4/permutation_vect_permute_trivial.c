#include "permutation.h"
#include <string.h>

void sig_perk_perm_vect_permute(vect1_t output, const perm_t p, const vect1_t input) {
    uint16_t buffer[PARAM_N1];
    for (uint32_t i = 0; i < PARAM_N1; i++) {
        buffer[p[i]] = input[i];
    }
    memcpy(output, buffer, sizeof(vect1_t));
}
