#include "crypto_memset.h"
#include "lemire_sampling.h"
#include "permutation.h"

int sig_perk_perm_gen_given_random_input(perm_t p, const uint16_t rnd_buff[PARAM_N1]) {
    for (int i = 0; i < PARAM_N1; i++) {
        p[i] = p[rnd_buff[i]];
        p[rnd_buff[i]] = i;
    }
    return EXIT_SUCCESS;
}

void sig_perk_perm_set_random_prg(perm_t p, sig_perk_prg_state_t *prg) {
    uint16_t fy_indexes[PARAM_N1];
    perk_sample_uniform_index(fy_indexes, prg);
    (void)sig_perk_perm_gen_given_random_input(p, fy_indexes);
    memset_zero(fy_indexes, sizeof(uint16_t) * PARAM_N1);
}

void sig_perk_perm_set_random(perm_t p, const uint8_t seed[SEED_BYTES]) {
    sig_perk_prg_state_t prg;
    sig_perk_prg_init(&prg, PRG1, NULL, seed);
    sig_perk_perm_set_random_prg(p, &prg);
}
