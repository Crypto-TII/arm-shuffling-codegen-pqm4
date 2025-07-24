#include "crypto_sort.h"
#include "sample.h"
#include "fips202.h"

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

void sample_fixed_type(poly *r, const unsigned char u[NTRU_SAMPLE_FT_BYTES]) {
    int i, t;
    uint16_t temp;
    int16_t shuffle_indices[NTRU_N - 1];

    rejsamplingmod(shuffle_indices, (const uint16_t*)u);

    for (i = 0; i < NTRU_WEIGHT / 2; i++) {
        r->coeffs[i] = 1;
    }

    for (i = NTRU_WEIGHT / 2; i < NTRU_WEIGHT; i++) {
        r->coeffs[i] = 2;
    }

    for (; i < NTRU_N - 1; i++) {
        r->coeffs[i] = 0;
    }

    for (i = NTRU_N - 2; i > 0; i--) {
        t = shuffle_indices[NTRU_N - 2 - i];
        temp = r->coeffs[t];
        r->coeffs[t] = r->coeffs[i];
        r->coeffs[i] = temp;
    }

    r->coeffs[NTRU_N-1] = 0;
}
