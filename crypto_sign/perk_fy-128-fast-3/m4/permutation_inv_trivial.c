#include "permutation.h"
#include <string.h>

void sig_perk_perm_inverse(perm_t o, const perm_t p) {
    perm_t buffer = {0};
    for (uint32_t i = 0; i < PARAM_N1; i++) {
        buffer[p[i]] = i;
    }
    memcpy(o, buffer, sizeof(perm_t));
}
