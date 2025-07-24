
#ifndef PERK_LEMIRE_SAMPLING_H
#define PERK_LEMIRE_SAMPLING_H

#include "parameters.h"
#include "symmetric.h"

/**
 * Lemire interval rejection sample.
 * url:  https://arxiv.org/pdf/1805.10941.pdf
 */
void perk_sample_uniform_index(uint16_t output_indexes[PARAM_N1], sig_perk_prg_state_t *prg);

#endif  // PERK_LEMIRE_SAMPLING_H
