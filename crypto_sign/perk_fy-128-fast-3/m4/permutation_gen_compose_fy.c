
#include "permutation.h"
#include "lemire_sampling.h"

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
