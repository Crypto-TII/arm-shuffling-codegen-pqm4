#include "permutation.h"
#include <string.h>

void sig_perk_perm_compose_inv(perm_t o, const perm_t p1, const perm_t p2) {
    perm_t buffer = {0};
    for (uint32_t i = 0; i < PARAM_N1; i++) {
        buffer[p2[i]] = p1[i];
    }
    memcpy(o, buffer, sizeof(perm_t));
}
