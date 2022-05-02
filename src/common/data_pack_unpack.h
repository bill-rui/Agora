#include <iostream>
#include <immintrin.h>
#include <stdlib.h>
#include <assert.h>

#include "memory_manage.h"

static constexpr size_t kBitsPerByte = 8;
static constexpr size_t kPackedBits = 24;
static constexpr size_t kBytesPerAvx2 = 24;
static constexpr size_t kBytesUnpackedAvx2 = 32;
static constexpr size_t kPackedBytes = kPackedBits / kBitsPerByte;
static constexpr size_t kUnPackedBytesDiv2 = 2;

/*
 * Unpacking using avx2 instructions
 * packed    - values to unpack
 * unpacked  - memory to place unpacked result. Must be 32 Byte aligned
 * to_unpack - number of values to unpack.  Must be multiple of 24 Values
 */
void unpack24_32_avx2(uint8_t *packed, __m256i *unpacked, size_t to_unpack);

/*
 * Naive unpacking
 * packed    - values to unpack
 * unpacked  - memory to place unpacked result
 * to_unpack - number of values to unpack
 */
void unpack24_32_naive(
  const uint8_t *packed, int16_t *unpacked, size_t to_unpack);

/*
 * Packing using avx2 instructions
 * unpacked  - values to pack
 * packed    - memory to place packed result.
 * to_unpack - number of values to unpack.  Must be multiple of 32 Values
 */
void pack32_24_avx2(uint8_t *unpacked, uint8_t *packed, size_t to_pack);

/*
 * Naive packing
 * unpacked  - values to pack
 * packed    - memory to place packed result.
 * to_unpack - number of values to unpack.  Must be multiple of 32 Values
 */
void pack32_24_naive(
  std::complex<int16_t>* unpacked, uint8_t *packed, size_t to_pack);