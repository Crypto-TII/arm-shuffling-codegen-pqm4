
#include "permutation.h"
#include "djbsort.h"
#include "symmetric.h"
#include "crypto_memset.h"

int sig_perk_perm_gen_given_random_input(perm_t p, const uint16_t rnd_buff[PARAM_N1]) {
    uint32_t buffer[PARAM_N1];
    // Use 16 bits for randomness
    for (int i = 0; i < PARAM_N1; i++) {
        buffer[i] = (((uint32_t)rnd_buff[i]) << 16) | i;
    }
    // sort
    uint32_sort(buffer, PARAM_N1);
    // check that no double random values were produced
    for (int i = 1; i < PARAM_N1; i++) {
        if ((buffer[i - 1] >> 16) == (buffer[i] >> 16)) {
            return EXIT_FAILURE;
        }
    }
    // extract permutation from buffer
    for (int i = 0; i < PARAM_N1; i++) {
        p[i] = (uint16_t)(buffer[i]);
    }
    return EXIT_SUCCESS;
}

void sig_perk_perm_set_random_prg(perm_t p, sig_perk_prg_state_t *prg) {
    uint16_t rnd_buff[PARAM_N1];
    sig_perk_prg(prg, (uint8_t *)rnd_buff, sizeof(rnd_buff));

    while (sig_perk_perm_gen_given_random_input(p, rnd_buff) != EXIT_SUCCESS) {
        sig_perk_prg(prg, (uint8_t *)rnd_buff, sizeof(rnd_buff));
    }
    memset_zero(rnd_buff, sizeof(rnd_buff));
}

void sig_perk_perm_set_random(perm_t p, const uint8_t seed[SEED_BYTES]) {
    sig_perk_prg_state_t prg;
    sig_perk_prg_init(&prg, PRG1, NULL, seed);
    sig_perk_perm_set_random_prg(p, &prg);
}
