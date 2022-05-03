#include <iostream>
#include <immintrin.h>
#include <stdlib.h>
#include <assert.h>

#include "memory_manage.h"
#include "data_pack_unpack.h"


static constexpr int8_t kI = 4;
static constexpr int8_t kS = 5;
static constexpr int8_t kQ = 6;

// map bytes into position
const static __m256i bytegrouping =
  _mm256_setr_epi8(kI, kS, kS, kQ,
                   kI + 3, kS + 3, kS + 3,kQ + 3,
                   kI + 6,kS + 6, kS + 6,kQ +6,
                   kI + 9, kS + 9, kS + 9, kQ + 9,
                   kI + 12, kS + 12, kS + 12, kQ + 12,
                   kI + 15, kS + 15, kS + 15, kQ + 15,
                   kI + 18, kS + 18, kS + 18, kQ + 18, 
                   kI + 21, kS + 21, kS + 21, kQ + 21);

/*
 * Unpacking using avx2 instructions
 * packed    - values to unpack
 * unpacked  - memory to place unpacked result. Must be 32 Byte aligned
 * to_unpack - number of values to unpack.  Must be multiple of 24 Values
 */
void unpack24_32_avx2(uint8_t *packed, __m256i *unpacked, size_t to_unpack) {
  assert(to_unpack % kBytesPerAvx2 == 0);
  assert((reinterpret_cast<intptr_t>(unpacked) % 32) == 0);

  for (size_t i = 0; i < to_unpack / 24; i++) {
    const __m256i aligned_packed = 
       _mm256_loadu_si256(reinterpret_cast<const __m256i *>(
          &packed[(i * kBytesPerAvx2) - 4]));  // load content
    const __m256i shuffle = 
       _mm256_shuffle_epi8(aligned_packed, bytegrouping);  // move bytes around
    const __m256i shift = 
       _mm256_slli_epi16(shuffle, 4);  // shift each 16 bytes chunk left 4 bits
    const __m256i mask  = 
       _mm256_and_si256(shuffle, _mm256_set1_epi32(
          static_cast<int>(0xFFF00000)));  // mask out a nibble in second int
    /* This line takes a lot of time */
    _mm256_store_si256(&unpacked[i], _mm256_blend_epi16(
         shift, mask, 0b10101010));  // merge shift and mask
  }
}

// map bytes into position
const static __m256i bytegrouping_d =
  _mm256_setr_epi8(kI, kS, kS, kQ,
                   kI + 6, kS + 6, kS + 6,kQ + 6,
                   kI + 3,kS + 3, kS + 3,kQ +3,
                   kI + 9, kS + 9, kS + 9, kQ + 9,
                   kI + 12, kS + 12, kS + 12, kQ + 12,
                   kI + 18, kS + 18, kS + 18, kQ + 18,
                   kI + 15, kS + 15, kS + 15, kQ + 15, 
                   kI + 21, kS + 21, kS + 21, kQ + 21);

/*
 * Unpacking using avx2 instructions with two destinations
 * packed    - values to unpack
 * unpacked1  - memory to place first unpacked result. Must be 32 Byte aligned
 * unpacked2  - memory to place second unpacked result. Must be 32 Byte aligned
 * to_unpack - number of values to unpack.  Must be multiple of 24 Values
 */
void unpack24_32_avx2_d(uint8_t *packed, __m128i *unpacked1,
                        __m128i *unpacked2, size_t to_unpack) {
  assert(to_unpack % kBytesPerAvx2 == 0);
  // assert((reinterpret_cast<intptr_t>(unpacked1) % 32) == 0);
  // assert((reinterpret_cast<intptr_t>(unpacked2) % 32) == 0);

  for (size_t j = 0; j < to_unpack / 24; j++) {
    const __m256i aligned_packed =
       _mm256_loadu_si256(reinterpret_cast<const __m256i *>(
                          &packed[(j * kBytesPerAvx2) - 4]));  // load content
    const __m256i shuffle =
       _mm256_shuffle_epi8(aligned_packed, bytegrouping_d);  // move bytes around
    const __m256i shift =
       _mm256_slli_epi16(shuffle, 4);  // shift each 16 bytes chunk left 4 bits
    const __m256i mask  =
       _mm256_and_si256(shuffle, _mm256_set1_epi32(
          static_cast<int>(0xFFF00000)));  // mask out a nibble in second int
    /* This line takes a lot of time */
    const __m256i result = _mm256_blend_epi16(shift, mask, 0b10101010);  // merge shift and mask
    memcpy(&unpacked1[j], &result, 8);
    memcpy(&unpacked2[j], reinterpret_cast<const uint8_t*>(&result) + 8, 8);
    memcpy(reinterpret_cast<uint8_t*>(&unpacked1[j]) + 8,
           reinterpret_cast<const uint8_t*>(&result) + 16, 8);
    memcpy(reinterpret_cast<uint8_t*>(&unpacked2[j]) + 8,
           reinterpret_cast<const uint8_t*>(&result) + 24, 8);
  }
}

/*
 * Naive unpacking
 * packed    - values to unpack
 * unpacked  - memory to place unpacked result
 * to_unpack - number of values to unpack
 */
void unpack24_32_naive(const uint8_t *packed, int16_t *unpacked, 
                       size_t to_unpack) {
  assert(to_unpack % 3 == 0);
  for (size_t i = 0u; i < to_unpack / 3; i++) {
    const size_t packed_index = kPackedBytes * i;
    const size_t unpacked_index = kUnPackedBytesDiv2 * i;

    const auto i_lsb = uint16_t(packed[packed_index]);
    const auto split = uint16_t(packed[packed_index + 1u]);
    const auto q_msb = uint16_t(packed[packed_index + 2u]);
    unpacked[unpacked_index] = int16_t((split << 12u) | (i_lsb << 4u));
    unpacked[unpacked_index + 1] = int16_t((q_msb << 8u) | (split & 0xf0));
  }
}

/*
 * Naive unpacking with two destinations
 * packed    - values to unpack
 * unpacked1  - memory to place first unpacked result
 * unpacked2  - memory to place second unpacked result
 * to_unpack - number of values to unpack
 */
void unpack24_32_d_naive(const uint8_t *packed, int16_t *unpacked1,
                         int16_t *unpacked2, size_t to_unpack) {
  assert(to_unpack % 3 == 0);
  for (size_t i = 0u; i < to_unpack / 3; i++) {
    const size_t packed_index = kPackedBytes * i;
    const size_t unpacked_index = i / 2 * 2;

    const auto i_lsb = uint16_t(packed[packed_index]);
    const auto split = uint16_t(packed[packed_index + 1u]);
    const auto q_msb = uint16_t(packed[packed_index + 2u]);
    if (i % 2 == 0) {
      unpacked1[unpacked_index] = int16_t((split << 12u) | (i_lsb << 4u));
      unpacked1[unpacked_index + 1] = int16_t((q_msb << 8u) | (split & 0xf0));
    } else {
      unpacked2[unpacked_index] = int16_t((split << 12u) | (i_lsb << 4u));
      unpacked2[unpacked_index + 1] = int16_t((q_msb << 8u) | (split & 0xf0));
    }

  }
}

const static int8_t kA = 1;
const static int8_t kB = 2;
const static int8_t kC = 3;
const static int8_t kD = 0;
const static int8_t kE = 1;
uint8_t g1[32] = 
  {kA, kB, kC, kA + 4, kB + 4, kC + 4, kA + 8, kB + 8, kC + 8, 
   kA + 12, kB + 12, kC + 12, 0x80, 0x80, 0x80, 0x80,
   kA, kB, kC, kA + 4, kB + 4, kC + 4, kA + 8, kB + 8, kC + 8, 
kA + 12, kB + 12, kC + 12, 0x80, 0x80, 0x80, 0x80};
uint8_t g2[32] = 
  {kD, kE, 0x80, kD + 4, kE + 4, 0x80, kD + 8, kE + 8, 0x80, kD + 12, kE + 12, 
   0x80, 0x80, 0x80, 0x80, 0x80,
   kD, kE, 0x80, kD + 4, kE + 4, 0x80, kD + 8, kE + 8, 0x80, kD + 12, kE + 12,
   0x80, 0x80, 0x80, 0x80, 0x80};
const static __m256i *grouping1 = reinterpret_cast<const __m256i *>(g1);
const static __m256i *grouping2 = reinterpret_cast<const __m256i *>(g2);
const static __m256i mask = 
  _mm256_setr_epi8(0x00, 0xF0, 0xFF, 0x00, 0xF0, 0xFF, 0x00, 0xF0, 
                   0xFF, 0x00, 0xF0, 0xFF, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0xF0, 0xFF, 0x00, 0xF0, 0xFF, 0x00, 0xF0, 
                   0xFF, 0x00, 0xF0, 0xFF, 0x00, 0x00, 0x00, 0x00);


/*
 * Packing using avx2 instructions
 * unpacked  - values to pack
 * packed    - memory to place packed result.
 * to_unpack - number of values to unpack.  Must be multiple of 32 Values
 */
void pack32_24_avx2(uint8_t *unpacked, uint8_t *packed, size_t to_pack) {
  assert(to_pack % 32 == 0);
  for (size_t i = 0; i < to_pack / 32; i++) {
      __m256i aligned_unpacked = _mm256_load_si256(
        reinterpret_cast<const __m256i *>(&unpacked[32 * i]));
      __m256i shuffle1 = _mm256_shuffle_epi8(aligned_unpacked, *grouping1);
      __m256i shuffle2 = _mm256_shuffle_epi8(aligned_unpacked, *grouping2);
      __m256i shift2 = _mm256_srli_epi64(shuffle2, 4);
      shuffle1 = _mm256_and_si256(shuffle1, mask);
      __m256i result = _mm256_or_si256(shuffle1, shift2);
      auto *hi = reinterpret_cast<__m128i *>(&result);
      __m128i *lo = hi + 1;
      /* These two lines take a lot of time */
      _mm_storeu_si128(reinterpret_cast<__m128i *>(&packed[24 * i]), 
        reinterpret_cast<__m128i>(*hi));
      _mm_storeu_si128(reinterpret_cast<__m128i *>(&packed[24 * i + 12]), 
        reinterpret_cast<__m128i>(*lo));
  }
}

/*
 * Naive packing
 * unpacked  - values to pack
 * packed    - memory to place packed result.
 * to_unpack - number of values to unpack.  Must be multiple of 32 Values
 */
void pack32_24_naive(
  std::complex<int16_t>* unpacked, uint8_t *packed, size_t to_pack) {
    for (size_t i = 0; i < to_pack / 4; i++) {
        //0xABCD -> 0xBC
        packed[3 * i + 0u] = static_cast<uint8_t>(
          (static_cast<uint16_t>(unpacked[i].real()) >> 4u) & 0xFF);
        //0xABCD & 0xEFGH -> 0xGA
        packed[3 * i + 1u] = static_cast<uint8_t>(
          (static_cast<uint16_t>(unpacked[i].imag()) & 0xF0) | 
          (static_cast<uint16_t>(unpacked[i].real()) >> 12u));
        //0xEFGH -> 0xEF
        packed[3 * i + 2u] = static_cast<uint8_t>(
          (static_cast<uint16_t>(unpacked[i].imag()) >> 8u) & 0xFF);
    }
}