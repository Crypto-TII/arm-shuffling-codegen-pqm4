// clang-format off

#include "sample.h"

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

const uint16_t vt[] = {
  4,   133, 262, 391, 16,  146, 276, 406, 36,  167, 298, 429, 64,  196, 328, 460,
  100, 233, 366, 10,  144, 278, 412, 61,  196, 331, 466, 120, 256, 392, 50,  187,
  324, 461, 124, 262, 400, 67,  206, 345, 16,  156, 296, 436, 112, 253, 394, 74,
  216, 358, 42,  185, 328, 16,  160, 304, 448, 141, 286, 431, 128, 274, 420, 121,
  268, 415, 120, 268, 416, 125, 274, 423, 136, 286, 2,   153, 304, 24,  176, 328,
  52,  205, 358, 86,  240, 394, 126, 281, 16,  172, 328, 67,  224, 381, 124, 282,
  28,  187, 346, 96,  256, 9,   170, 331, 88,  250, 10,  173, 336, 100, 264, 31,
  196, 361, 132, 298, 72,  239, 16,  184, 352, 133, 302, 86,  256, 43,  214, 4,
  176, 348, 142, 315, 112, 286, 86,  261, 64,  240, 46,  223, 32,  210, 22,  201,
  16,  196, 14,  195, 16,  198, 22,  205, 32,  216, 46,  231, 64,  250, 86,  273,
  112, 300, 142, 331, 176, 23,  214, 64,  256, 109, 302, 158, 16,  211, 72,  268,
  132, 329, 196, 65,  264, 136, 10,  211, 88,  290, 170, 52,  256, 141, 28,  234,
  124, 16,  224, 119, 16,  226, 126, 28,  240, 145, 52,  266, 176, 88,  2,   219,
  136, 55,  274, 196, 120, 46,  268, 197, 128, 61,  286, 222, 160, 100, 42,  271,
  216, 163, 112, 63,  16,  250, 206, 164, 124, 86,  50,  16,  256, 225, 196, 169,
  144, 121, 100, 81,  64,  49,  36,  25,  16,  9,   4,   1,   0,   1,   4,   9,
  16,  25,  36,  49,  64,  81,  100, 121, 144, 169, 196, 225, 16,  50,  86,  124,
  164, 206, 16,  63,  112, 163, 216, 42,  100, 160, 222, 61,  128, 197, 46,  120,
  196, 55,  136, 2,   88,  176, 52,  145, 28,  126, 16,  119, 16,  124, 28,  141,
  52,  170, 88,  10,  136, 65,  196, 132, 72,  16,  158, 109, 64,  23,  176, 142,
  112, 86,  64,  46,  32,  22,  16,  14,  16,  22,  32,  46,  64,  86,  112, 142,
  4,   43,  86,  133, 16,  72,  132, 31,  100, 10,  88,  9,   96,  28,  124, 67,
  16,  126, 86,  52,  24,  2,   136, 125, 120, 121, 128, 141, 16,  42,  74,  112,
  16,  67,  124, 50,  120, 61,  10,  100, 64,  36,  16,  4,   0,   4,   16,  36,
  64,  100, 22,  75,  16,  86,  46,  16,  112, 101, 100, 109, 16,  46,  86,  27,
  88,  52,  28,  16,  16,  28,  52,  88,  36,  97,  72,  61,  64,  81,  18,  64,
  32,  16,  16,  32,  64,  25,  4,   1,   16,  49,  18,  7,   16,  45,  16,  9,
  24,  61,  46,  55,  16,  3,   16,  55,  52,  10,  64,  16,  0,   16,  2,   22,
  16,  46,  54,  43,  16,  31,  34,  28,  16,  1,   36,  23,  16,  18,  32,  16,
  20,  4,   16,  18,  16,  16,  24,  9,   16,  16,  18,  31,  0,   2,   16,  25,
  16,  7,   16,  11,  16,  9,   20,  16,  16,  5,   16,  1,   0,   1,   2,   3,
  4,   9,   6,   7,   0,   2,   4,   1,   0,   1,   0,   0,
};

// Unlike Algorithm 5 in the paper, we use the batch random number generation idea, discussed in Section 4. Moreover,
// although not required in non-SIMD versions, we use disjoint ranges in the random number array for the initial
// sampling and the rejection sampling fixup procedure, as discussed in "SIMD implementation of Algorithm 5" in
// Section 4, to ensure interoperability of KATs between the scalar and SIMD versions.
void rejsamplingmod(
  int16_t shuffle_indices[], const uint16_t u[NTRU_SAMPLE_FT_BYTES / 2]) {
  int i, j = NTRU_N - 1;

  for (i = 0; i < NTRU_N - 1; i++)
  {
    uint32_t m;
    uint16_t s, t, l;

    s = NTRU_N - 1 - i;
    t = vt[i];

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

void sample_fixed_type(
  poly *r, const unsigned char u[NTRU_SAMPLE_FT_BYTES]) {
  int16_t shuffle_indices[NTRU_N - 1];
  int16_t* si = shuffle_indices;
  uint16_t* c = r->coeffs;

  rejsamplingmod(shuffle_indices, (const uint16_t*)u);

  int c0 = NTRU_N - 1 - NTRU_WEIGHT, c01 = NTRU_N - 1 - NTRU_WEIGHT / 2;

#if 0
  for (int i = 0; i < NTRU_N - 1; i++)
  {
    int t0, t1;
    t0 = (p - c0) >> 31;
    t1 = (p - c01) >> 31;

    c0 += t0;
    c01 += t1;

    r->coeffs[i] = 2 + t0 + t1;
  }
#else
  while (c != &r->coeffs[NTRU_N])
  {
    int p = *si++;

    asm("cmp        %[p],  %[c0]    \n"
        "sbc       %[c0],     #0    \n"
        "sbc    %[out_i], %[one], #0\n"
        "cmp        %[p], %[c01]    \n"
        "sbc      %[c01],     #0    \n"
        "sbc    %[out_i],    #-1    \n"
        : [c0] "+&r"(c0), [c01] "+&r"(c01), [out_i] "=&r"(*c)
        : [p] "r"(p), [one] "r"(1)
        : "cc");

    c++;
  }
#endif

  r->coeffs[NTRU_N-1] = 0;
}

// clang-format on
