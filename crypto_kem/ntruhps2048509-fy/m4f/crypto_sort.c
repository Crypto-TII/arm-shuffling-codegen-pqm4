// XXX: Temporary placeholder for a faster sort.
// Copied from supercop-20190110/crypto_sort/int32/portable3

#include <stdint.h>

#include "crypto_sort.h"

// Modified in NTRU-sampling to improve performance:
// -inline assembly using conditional execution
// -reduce instruction count by optimizing loads and stores, reducing temporaries
#define int32_MINMAX_M_M(x, offset1, offset2)                          \
    do {                                                               \
        int32_t a, b; /* temporaries */                                \
        asm volatile(                                                  \
            "ldr   %0, [%4, %5, lsl #2]\n"                             \
            "ldr   %1, [%4, %6, lsl #2]\n"                             \
            "cmp   %0, %1              \n"                             \
            "itt   gt                  \n"                             \
            "strgt %1, [%4, %5, lsl #2]\n"                             \
            "strgt %0, [%4, %6, lsl #2]\n"                             \
            : "=&r" (a), "=&r" (b), "=m"(x[offset1]), "=m"(x[offset2]) \
            : "r" (x), "r" (offset1), "r" (offset2)                    \
            : "cc"                                                     \
        );                                                             \
    } while(0)

#define int32_MINMAX_V_M(a, x, offset)               \
    do {                                             \
        int32_t b; /* temporary */                   \
        asm(                                         \
            "ldr   %1, [%3, %4, lsl #2]\n"           \
            "cmp   %0, %1\n"                         \
            "itt   gt    \n"                         \
            "strgt %0, [%3, %4, lsl #2]\n"           \
            "movgt %0, %1\n"                         \
            : "+&r" (a), "=&r" (b), "=m" (x[offset]) \
            : "r" (x), "r" (offset)                  \
            : "cc"                                   \
        );                                           \
    } while(0)

// Modified in NTRU-sampling to improve performance:
// -Replace djbsort's "portable3" version by "portable4"
// -replace long long by long
void crypto_sort(int32_t *x, long n)
{
    long top, p, q, r, i, j;

    if (n < 2) {
        return;
    }
    top = 1;
    while (top < n - top) {
        top += top;
    }

    for (p = top; p >= 1; p >>= 1) {
        i = 0;
        while (i + 2 * p <= n) {
            for (j = i; j < i + p; ++j) {
                int32_MINMAX_M_M(x, j, j + p);
            }
            i += 2 * p;
        }
        for (j = i; j < n - p; ++j) {
            int32_MINMAX_M_M(x, j, j + p);
        }

        i = 0;
        j = 0;
        for (q = top; q > p; q >>= 1) {
            if (j != i) { 
                for (;;) {
                    if (j == n - q) {
                        goto done;
                    }
                    int32_t a = x[j + p];
                    for (r = q; r > p; r >>= 1) {
                        int32_MINMAX_V_M(a, x, j + r);
                    }
                    x[j + p] = a;
                    ++j;
                    if (j == i + p) {
                        i += 2 * p;
                        break;
                    }
                }
            }
            while (i + p <= n - q) {
                for (j = i; j < i + p; ++j) {
                    int32_t a = x[j + p];
                    for (r = q; r > p; r >>= 1) {
                        int32_MINMAX_V_M(a, x, j + r);
                    }
                    x[j + p] = a;
                }
                i += 2 * p;
            }
            /* now i + p > n - q */
            j = i;
            while (j < n - q) {
                int32_t a = x[j + p];
                for (r = q; r > p; r >>= 1) {
                    int32_MINMAX_V_M(a, x, j + r);
                }
                x[j + p] = a;
                ++j;
            }

            done: ;
        }
    }
}
