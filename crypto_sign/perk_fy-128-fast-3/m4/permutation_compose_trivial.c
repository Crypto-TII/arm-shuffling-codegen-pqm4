
#include "permutation.h"
#include <string.h>

void sig_perk_perm_compose(perm_t o, const perm_t p1, const perm_t p2) {
    perm_t buffer = {0};
    for (uint32_t i = 0; i < PARAM_N1; i++) {
        buffer[i] = p1[p2[i]];
    }
    memcpy(o, buffer, sizeof(perm_t));
}
