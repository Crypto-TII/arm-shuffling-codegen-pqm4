
#include "permutation.h"

void sig_perk_perm_gen_pi_1(perm_t *pi_i, const perm_t pi) {
    sig_perk_perm_inverse(pi_i[0], pi_i[1]);
    for (int i = 2; i < PARAM_N; i++) {
        sig_perk_perm_compose_inv(pi_i[0], pi_i[0], pi_i[i]);
    }
    sig_perk_perm_compose(pi_i[0], pi_i[0], pi);
}
