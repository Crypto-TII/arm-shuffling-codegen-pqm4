
#include "permutation.h"
#include <string.h>
#include "lemire_sampling.h"
#include "crypto_memset.h"

#include "perk_generator.h"

typedef void (*shuffle_fn_t)(uint8_t []);
typedef void (*compose_fn_t)(uint8_t [], const uint8_t []);
typedef void (*vect_permute_fn_t)(uint8_t[], uint8_t[]);
uint32_t asm_word[9 + 48*4 + ((PARAM_N1 - 48)/4)*26 + (PARAM_N1 % 4)*8];
shuffle_fn_t f = (shuffle_fn_t)((uint32_t)asm_word - 0x20000000 + 1);
compose_fn_t g = (compose_fn_t)((uint32_t)asm_word - 0x20000000 + 1);
vect_permute_fn_t h = (vect_permute_fn_t)((uint32_t)asm_word - 0x20000000 + 1);

void sig_perk_perm_gen_pi_1(perm_t *pi_i, const perm_t pi) {
    sig_perk_perm_inverse(pi_i[0], pi_i[1]);
    for (int i = 2; i < PARAM_N; i++) {
        sig_perk_perm_compose_inv(pi_i[0], pi_i[0], pi_i[i]);
    }
    sig_perk_perm_compose(pi_i[0], pi_i[0], pi);
}

void sig_perk_perm_compose_inv(perm_t o, const perm_t p1, const perm_t p2) {
    sig_perk_perm_compose_inv_gen(p1, p2, PARAM_N1, asm_word);
    f(o);
}

void sig_perk_perm_compose(perm_t o, const perm_t p1, const perm_t p2) {
    sig_perk_perm_compose_gen(p2, PARAM_N1, asm_word);
    g(o, p1);
}

int sig_perk_perm_gen_given_random_input_and_compose(perm_t pi_comp, perm_t p, const uint16_t rnd_buff[PARAM_N1]) {
    sig_perk_perm_gen_given_random_input(p, rnd_buff);
    sig_perk_perm_compose(pi_comp, pi_comp, p);
    return EXIT_SUCCESS;
}

void sig_perk_gen_one_pi_i_and_compose(perm_t pi_comp, perm_t pi_i, const salt_t salt,
                                       uint16_t rnd_buffer_pi_i[PARAM_N1], sig_perk_prg_state_t *state,
                                       const theta_t theta) {
    sig_perk_prg_init(state, PRG1, salt, theta);
    perk_sample_uniform_index(rnd_buffer_pi_i, state);
    (void) sig_perk_perm_gen_given_random_input_and_compose(pi_comp, pi_i, rnd_buffer_pi_i);
}

void sig_perk_perm_inverse(perm_t o, const perm_t p) {
    sig_perk_perm_inverse_gen(p, PARAM_N1, asm_word);
    f(o);
}

int sig_perk_perm_gen_given_random_input(perm_t p, const uint16_t rnd_buff[PARAM_N1]) {
    sig_perk_perm_gen_given_random_input_gen(rnd_buff, PARAM_N1, asm_word);
    f(p);
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

void sig_perk_perm_vect_permute(vect1_t output, const perm_t p, const vect1_t input) {
    uint8_t input_deinterleaved[2 * ((PARAM_N1 + 3) & ~0x3)];
    uint8_t output_deinterleaved[2 * ((PARAM_N1 + 3) & ~0x3)];
    sig_perk_perm_vect_permute_deinterleave_16bit(input_deinterleaved, input, PARAM_N1);
    sig_perk_perm_vect_permute_gen(p, PARAM_N1, asm_word);
    h(output_deinterleaved, input_deinterleaved);
    h(&output_deinterleaved[(PARAM_N1 + 3) & ~0x3], &input_deinterleaved[(PARAM_N1 + 3) & ~0x3]);
    sig_perk_perm_vect_permute_interleave_16bit(output, output_deinterleaved, PARAM_N1);
}
