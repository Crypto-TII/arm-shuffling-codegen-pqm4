#include "crypto_sort.h"
#include "sample.h"
#include "fips202.h"
#include "ntru_generator.h"
#include <stdint.h>

void sample_fg(poly *f, poly *g, const unsigned char uniformbytes[NTRU_SAMPLE_FG_BYTES]) {
    sample_iid(f, uniformbytes);
    sample_fixed_type(g, uniformbytes + NTRU_SAMPLE_IID_BYTES);
}

void sample_rm(poly *r, poly *m, const unsigned char uniformbytes[NTRU_SAMPLE_RM_BYTES]) {
    sample_iid(r, uniformbytes);
    sample_fixed_type(m, uniformbytes + NTRU_SAMPLE_IID_BYTES);
}

void sample_iid(poly *r, const unsigned char uniformbytes[NTRU_SAMPLE_IID_BYTES]) {
    int i;
    /* {0,1,...,255} -> {0,1,2}; Pr[0] = 86/256, Pr[1] = Pr[-1] = 85/256 */
    for (i = 0; i < NTRU_N - 1; i++) {
        r->coeffs[i] = mod3(uniformbytes[i]);
    }

    r->coeffs[NTRU_N - 1] = 0;
}

void rejsamplingmod(
    int16_t shuffle_indices[], const uint16_t u[NTRU_SAMPLE_FT_BYTES / 2]) {
    int i, j = NTRU_N - 1;
  
    for (i = 0; i < NTRU_N - 1; i++)
    {
      uint32_t m;
      uint16_t s, t, l;
  
      s = NTRU_N - 1 - i;
      t = 65536 % s;
  
      m = (uint32_t)u[i] * s;
      l = m;
  
      while (l < t)
      {
        m = (uint32_t)u[j++] * s;
        l = m;
      }
  
      shuffle_indices[i] = m >> 16;
    }
}

typedef void (*shuffle_fn_t)(uint8_t[]);
uint32_t asm_word[763 + ((NTRU_N - 1) % 16) * 8 + (((NTRU_N - 1) - 192) / 16) * 97];

void sample_fixed_type(poly *r, const unsigned char u[NTRU_SAMPLE_FT_BYTES]) {
    int i;
    uint16_t temp;
    int16_t shuffle_indices[NTRU_N - 1];
    uint8_t r_packed[(NTRU_N - 1 + 3) / 4];
    shuffle_fn_t f = (shuffle_fn_t)((uint32_t)asm_word - 0x20000000 + 1);

    rejsamplingmod(shuffle_indices, (const uint16_t*)u);

    initialize_packed_ntru_shuffling_array(r_packed, NTRU_N - 1);
    shuffling_ntru(shuffle_indices, NTRU_N - 1, asm_word);
    f(r_packed);
    unpack_ntru_shuffling_array(r->coeffs, r_packed, NTRU_N - 1);

    r->coeffs[NTRU_N-1] = 0;
}
