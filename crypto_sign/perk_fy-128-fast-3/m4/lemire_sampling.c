
#include "lemire_sampling.h"
#include "symmetric.h"

static const uint16_t lemire_table[151] = {
    0,  0,  0,  1,   0,  1,  4,   2,  0,   7,   6,   9,   4,   3,  2,  1,   0,   1,   16,  5,   16, 16, 20, 9,  16, 11,
    16, 7,  16, 25,  16, 2,  0,   31, 18,  16,  16,  9,   24,  16, 16, 18,  16,  4,   20,  16,  32, 18, 16, 23, 36, 1,
    16, 28, 34, 31,  16, 43, 54,  46, 16,  22,  2,   16,  0,   16, 64, 10,  52,  55,  16,  3,   16, 55, 46, 61, 24, 9,
    16, 45, 16, 7,   18, 49, 16,  1,  4,   25,  64,  32,  16,  16, 32, 64,  18,  81,  64,  61,  72, 97, 36, 88, 52, 28,
    16, 16, 28, 52,  88, 27, 86,  46, 16,  109, 100, 101, 112, 16, 46, 86,  16,  75,  22,  100, 64, 36, 16, 4,  0,  4,
    16, 36, 64, 100, 10, 61, 120, 50, 124, 67,  16,  112, 74,  42, 16, 141, 128, 121, 120, 125, 136};

#define USE_BUFFERED_LEMIRE
#ifdef USE_BUFFERED_LEMIRE

#define LEMIRE_RAND_BUFFER_LEN (PRNG_BLOCK_SIZE / 2)

    static int lemire_sample(uint16_t *output_index, uint16_t bound,
                             const uint16_t rand_buffer[LEMIRE_RAND_BUFFER_LEN], uint16_t *rand_buffer_index) {
    if (*rand_buffer_index >= LEMIRE_RAND_BUFFER_LEN) {
        return EXIT_FAILURE;
    }
    uint16_t x = rand_buffer[(*rand_buffer_index)++];
    uint32_t m = (uint32_t)x * (uint32_t)bound;
    uint16_t l = (uint16_t)m;  // m mod 2*^16
    while (l < lemire_table[bound]) {
        if (*rand_buffer_index >= LEMIRE_RAND_BUFFER_LEN) {
            return EXIT_FAILURE;
        }
        x = rand_buffer[(*rand_buffer_index)++];
        m = x * (uint32_t)bound;
        l = (uint16_t)m;  // m mod 2*^16
    }
    *output_index = m >> 16U;
    return EXIT_SUCCESS;
}

void perk_sample_uniform_index(uint16_t output_indexes[PARAM_N1], sig_perk_prg_state_t *prg) {
    uint16_t rand_buffer[LEMIRE_RAND_BUFFER_LEN] = {0};
    uint16_t rand_buffer_index = 0;
    sig_perk_prg(prg, (uint8_t *)rand_buffer, sizeof(uint16_t) * LEMIRE_RAND_BUFFER_LEN);

    uint16_t index = 0;
    while (index < PARAM_N1) {
        if (lemire_sample(&output_indexes[index], index + 1, rand_buffer, &rand_buffer_index) == EXIT_SUCCESS) {
            index++;
        } else {
            sig_perk_prg(prg, (uint8_t *)rand_buffer, sizeof(uint16_t) * LEMIRE_RAND_BUFFER_LEN);
            rand_buffer_index = 0;
        }
    }
}

#else

    static uint16_t lemire_sample(uint16_t bound, sig_perk_prg_state_t *prg) {
    uint16_t x = 0;
    sig_perk_prg(prg, (uint8_t *)&x, sizeof(x));
    uint32_t m = (uint32_t)x * (uint32_t)bound;
    uint16_t l = (uint16_t)m;  // m mod 2*^16
    while (l < lemire_table[bound]) {
        sig_perk_prg(prg, (uint8_t *)&x, sizeof(x));
        m = x * (uint32_t)bound;
        l = (uint16_t)m;  // m mod 2*^16
    }
    return m >> 16U;
}

void perk_sample_uniform_index(uint16_t output_indexes[PARAM_N1], sig_perk_prg_state_t *prg) {
    for (int i = 0; i < PARAM_N1; i++) {
        output_indexes[i] = lemire_sample(i, prg);
    }
}

#endif