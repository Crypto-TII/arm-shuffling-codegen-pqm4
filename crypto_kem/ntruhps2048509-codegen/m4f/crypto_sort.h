#ifndef CRYPTO_SORT
#define CRYPTO_SORT

// Modified in NTRU-sampling to match new function prototype from djbsort's "portable4" version
#include <stdint.h>

void crypto_sort(int32_t *array, long n);

#endif
