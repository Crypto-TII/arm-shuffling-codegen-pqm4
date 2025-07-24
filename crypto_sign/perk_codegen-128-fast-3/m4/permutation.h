
/**
 * @file permutation.h
 * @brief header file for permutation.c
 */

#ifndef SIG_PERK_PERMUTATION_H
#define SIG_PERK_PERMUTATION_H

#include "arithmetic.h"
#include "parameters.h"
#include "theta_tree.h"

/**
 * @brief Permutation perm_t
 *
 * This structure contains an array of integers that is a permutation
 * of size PARAM_N1 plus extra space upt to a multiple of 4 that is needed
 * to be modified by the codegen permutation operations
 */
typedef uint8_t perm_t[(PARAM_N1 + 3) & ~0x3];

/**
 * @brief Generate a random permutation form a initialized prg state
 *
 * @param [out] p a permutation
 * @param [in] prg an initialized prg state
 */
void sig_perk_perm_set_random_prg(perm_t p, sig_perk_prg_state_t *prg);

/**
 * @brief Generate a random permutation form a seed
 *
 * @param [out] p a permutation
 * @param [in] seed a string containing a seed
 */
void sig_perk_perm_set_random(perm_t p, const uint8_t seed[SEED_BYTES]);

/**
 * @brief Generate a random permutation form random values
 *
 * @param [out] p a permutation
 * @param [in] rnd_buff an array containing random values
 */
int sig_perk_perm_gen_given_random_input(perm_t p, const uint16_t rnd_buff[PARAM_N1]);

/**
 * @brief Apply a permutation on a vector
 *
 * @param [out] output a permuted vector
 * @param [in] p a permutation
 * @param [in] input a vector
 */
void sig_perk_perm_vect_permute(vect1_t output, const perm_t p, const vect1_t input);

/**
 * @brief Compute the composition on two permutations
 *
 * o = p1(p2)
 *
 * @param [out] o a permutation
 * @param [in] p1 a permutation
 * @param [in] p2 a permutation
 */
void sig_perk_perm_compose(perm_t o, const perm_t p1, const perm_t p2);

/**
 * @brief Compute the composition p1 compose p2^-1
 *
 * o = p1(p2)
 *
 * @param [out] o a permutation
 * @param [in] p1 a permutation
 * @param [in] p2 a permutation
 */
void sig_perk_perm_compose_inv(perm_t o, const perm_t p1, const perm_t p2);

/**
 * @brief Compute the inverse of a permutation
 *
 * o = p1^(-1)
 *
 * @param [out] o a permutation
 * @param [in] p a permutation
 */
void sig_perk_perm_inverse(perm_t o, const perm_t p);

/**
 * @brief Compute the permutation pi_1
 *
 * @param [out,in] pi_i a pointer to permutations
 * @param [in] pi the secret permutation pi in the scheme
 */
void sig_perk_perm_gen_pi_1(perm_t *pi_i, const perm_t pi);

/**
 * @brief Generate a random permutation form random values and compose
 *
 * @param [out,in] pi_comp a permutation to be composed
 * @param [out] p a permutation
 * @param [in] rnd_buff an array containing random values
 */
int sig_perk_perm_gen_given_random_input_and_compose(perm_t pi_comp, perm_t p, const uint16_t rnd_buff[PARAM_N1]);

/**
 * @brief Generate one pi_i and compose with pi_comp
 *
 * @param [out,in] pi_comp pointer to the permutation to be composed with the sampled one
 * @param [out] pi_i a pointer to the permutation sampled
 * @param [in] salt a salt
 * @param [out,in] rnd_buffer_pi_i memory buffer used to sample randomness.
 *                                 must be erased by the caller
 * @param [out,in] state state for the prg used to sample randomness.
 *                       must be erased by the caller
 * @param [in] theta the seed for the prg
 */
void sig_perk_gen_one_pi_i_and_compose(perm_t pi_comp, perm_t pi_i, const salt_t salt,
                                       uint16_t rnd_buffer_pi_i[PARAM_N1], sig_perk_prg_state_t *state,
                                       const theta_t theta);
#endif