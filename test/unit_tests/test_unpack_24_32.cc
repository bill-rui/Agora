#include <iostream>
#include <immintrin.h>
#include <gtest/gtest.h>
#include <stdlib.h>
#include <assert.h>

#include "memory_manage.h"

/* Order of tests matters if they are storing result in the same block of
 *  memory.
 */

static constexpr size_t kTestReps = 5000000;

static constexpr size_t kBitsPerByte = 8;
static constexpr size_t kPackedBits = 24;
static constexpr size_t kBytesPerAvx2 = 24;
static constexpr size_t kPackedBytes = kPackedBits / kBitsPerByte;
static constexpr size_t kUnPackedBytesDiv2 = 2;

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
       _mm256_slli_epi16(shuffle, 4);  // shift each 16 bytes chunk left by 4 bits
    const __m256i mask  = 
       _mm256_and_si256(shuffle, _mm256_set1_epi32(
          static_cast<int>(0xFFF00000)));  // mask out a nibble in the second int
    /* This line takes a lot of time */
    _mm256_store_si256(&unpacked[i], _mm256_blend_epi16(
         shift, mask, 0b10101010));  // merge shift and mask
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

    const uint16_t i_lsb = uint16_t(packed[packed_index]);
    const uint16_t split = uint16_t(packed[packed_index + 1u]);
    const uint16_t q_msb = uint16_t(packed[packed_index + 2u]);
    unpacked[unpacked_index] = int16_t((split << 12u) | (i_lsb << 4u));
    unpacked[unpacked_index + 1] = int16_t((q_msb << 8u) | (split & 0xf0));
  }
}

const int8_t a = 1;
const int8_t b = 2;
const int8_t c = 3;
const int8_t d = 0;
const int8_t e = 1;
uint8_t g1[32] = {a, b, c, a + 4, b + 4, c + 4, a + 8, b + 8, c + 8, a + 12, b + 12, c + 12, 0x80, 0x80, 0x80, 0x80, a, b, c, a + 4, b + 4, c + 4, a + 8, b + 8, c + 8, a + 12, b + 12, c + 12, 0x80, 0x80, 0x80, 0x80};
uint8_t g2[32] = {d, e, 0x80, d + 4, e + 4, 0x80, d + 8, e + 8, 0x80, d + 12, e + 12, 0x80,0x80, 0x80, 0x80, 0x80, d, e, 0x80, d + 4, e + 4, 0x80, d + 8, e + 8, 0x80, d + 12, e + 12, 0x80, 0x80, 0x80, 0x80, 0x80};
const static __m256i *grouping1 = reinterpret_cast<const __m256i *>(g1);
const static __m256i *grouping2 = reinterpret_cast<const __m256i *>(g2);
const static __m256i mask = _mm256_setr_epi8(0x00, 0xF0, 0xFF, 0x00, 0xF0, 0xFF, 0x00, 0xF0, 0xFF, 0x00, 0xF0, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFF, 0x00, 0xF0, 0xFF, 0x00, 0xF0, 0xFF, 0x00, 0xF0, 0xFF, 0x00, 0x00, 0x00, 0x00);


void pack(uint8_t *p, uint8_t *ret, size_t n) {
  for (size_t i = 0; i < n / 32; i++) {
      __m256i v = _mm256_load_si256(reinterpret_cast<const __m256i *>(&p[32 * i]));
      __m256i shuffle1 = _mm256_shuffle_epi8(v, *grouping1);
      __m256i shuffle2 = _mm256_shuffle_epi8(v, *grouping2);
      __m256i shift2 = _mm256_srli_epi64(shuffle2, 4);
      shuffle1 = _mm256_and_si256(shuffle1, mask);
      __m256i result = _mm256_or_si256(shuffle1, shift2);
      __m128i *hi = reinterpret_cast<__m128i *>(&result);
      __m128i *lo = hi + 1;
      /* These two lines takes a lot of time */
      _mm_storeu_si128(reinterpret_cast<__m128i *>(&ret[24 * i]), reinterpret_cast<__m128i>(*hi));
      _mm_storeu_si128(reinterpret_cast<__m128i *>(&ret[24 * i + 12]), reinterpret_cast<__m128i>(*lo));
  }
}

void pack24(std::complex<int16_t>& unpacked, uint8_t *packed, size_t n)
{
    for (size_t i = 0; i < n / 4; i++) {
        //0xABCD -> 0xBC
        packed[3 * i + 0u] = static_cast<uint8_t>((static_cast<uint16_t>(unpacked.real()) >> 4u) & 0xFF);
        //0xABCD & 0xEFGH -> 0xGA
        packed[3 * i + 1u] = static_cast<uint8_t>((static_cast<uint16_t>(unpacked.imag()) & 0xF0)  |
                                          (static_cast<uint16_t>(unpacked.real()) >> 12u));
        //0xEFGH -> 0xEF
        packed[3 * i + 2u] = static_cast<uint8_t>((static_cast<uint16_t>(unpacked.imag()) >> 8u) & 0xFF);
    }
}

/*
 * Unit tests input values
 */
static constexpr size_t kSpeedIterations = kTestReps * kBytesPerAvx2;
static constexpr size_t kMaxCorrectnessCycles = 100;
static constexpr size_t kPackedSizeMinBytes = 3;
static constexpr size_t kUnpackedSizeMinBytes = 4;
static constexpr size_t output_bytes_per_cycle = kBytesPerAvx2 * 
                                                    kUnpackedSizeMinBytes / 
                                                    kPackedSizeMinBytes;
const size_t result_size_bytes = kSpeedIterations * kUnpackedSizeMinBytes;
const size_t speed_input_bytes = kSpeedIterations * kPackedSizeMinBytes;

int16_t *truth_result;
int16_t *function_result;
uint8_t *speed_packed_input;
int16_t *speed_result;
int16_t *speed_result2;
int16_t *speed_result3;
int16_t *speed_result4;

uint8_t random_one[kBytesPerAvx2];
uint8_t zeros[kBytesPerAvx2];
uint8_t uniform[kBytesPerAvx2];
uint8_t three_cycles[kBytesPerAvx2 * 3];
uint8_t hundred_cycles[kBytesPerAvx2 * 100];


TEST (Performance, packing) {
  pack(speed_packed_input, reinterpret_cast<uint8_t*>(speed_result2), speed_input_bytes);
}

TEST (Performance, packingCurrentImplementation) {
  pack24(reinterpret_cast<std::complex<int16_t> &>(speed_packed_input), reinterpret_cast<uint8_t*>(speed_result), speed_input_bytes);
}

TEST (Performance, CurrentImplementation) {
  unpack24_32_naive(speed_packed_input, speed_result4, 
                    speed_input_bytes);
}

TEST (Performance, SIMDImplementation) {
  unpack24_32_avx2(speed_packed_input, reinterpret_cast<__m256i*>(speed_result3), 
                   speed_input_bytes);
}

TEST (Correctness, OneCycleRandom) {
  const size_t output_bytes = output_bytes_per_cycle;
  unpack24_32_naive(random_one, truth_result, kBytesPerAvx2);
  unpack24_32_avx2(random_one, reinterpret_cast<__m256i*>(function_result), 
                   kBytesPerAvx2);

  ASSERT_EQ(0, memcmp(truth_result, function_result, output_bytes));
}

TEST (Correctness, OneCycleZeros) {
  const size_t output_bytes = output_bytes_per_cycle;
  unpack24_32_naive(zeros, truth_result, kBytesPerAvx2);
  unpack24_32_avx2(zeros, reinterpret_cast<__m256i*>(function_result), 
                   kBytesPerAvx2);

  ASSERT_EQ(0, memcmp(truth_result, function_result, output_bytes));
}

TEST (Correctness, OneCylceUniform) {
  const size_t output_bytes = output_bytes_per_cycle;
  unpack24_32_naive(uniform, truth_result, kBytesPerAvx2);
  unpack24_32_avx2(uniform, reinterpret_cast<__m256i*>(function_result), 
                   kBytesPerAvx2);

  ASSERT_EQ(0, memcmp(truth_result, function_result, output_bytes));
}

TEST (Correctness, ThreeCyclesRandom) {
  const size_t output_bytes = 3 * output_bytes_per_cycle;
  unpack24_32_naive(three_cycles, truth_result, kBytesPerAvx2);
  unpack24_32_avx2(three_cycles, reinterpret_cast<__m256i*>(function_result), 
                   kBytesPerAvx2);

  ASSERT_EQ(0, memcmp(truth_result, function_result, 3 * output_bytes));
}

TEST (Correctness, 100CyclesRandom) {
  const size_t output_bytes = 100 * output_bytes_per_cycle;
  unpack24_32_naive(hundred_cycles, truth_result, kBytesPerAvx2);
  unpack24_32_avx2(hundred_cycles, reinterpret_cast<__m256i*>(function_result), 
                   kBytesPerAvx2);

  ASSERT_EQ(0, memcmp(truth_result, function_result, 100 * output_bytes));
}

void setup() {
  truth_result = static_cast<int16_t *>(Agora_memory::PaddedAlignedAlloc(
      Agora_memory::Alignment_t::kAlign64, result_size_bytes));

  function_result = static_cast<int16_t *>(Agora_memory::PaddedAlignedAlloc(
      Agora_memory::Alignment_t::kAlign64, result_size_bytes));
  
  speed_result = static_cast<int16_t *>(Agora_memory::PaddedAlignedAlloc(
      Agora_memory::Alignment_t::kAlign64, result_size_bytes));
  speed_result2 = static_cast<int16_t *>(Agora_memory::PaddedAlignedAlloc(
      Agora_memory::Alignment_t::kAlign64, result_size_bytes));
  speed_result3 = static_cast<int16_t *>(Agora_memory::PaddedAlignedAlloc(
      Agora_memory::Alignment_t::kAlign64, result_size_bytes));
  speed_result4 = static_cast<int16_t *>(Agora_memory::PaddedAlignedAlloc(
      Agora_memory::Alignment_t::kAlign64, result_size_bytes));
  
  speed_packed_input = static_cast<uint8_t *>(Agora_memory::PaddedAlignedAlloc(
      Agora_memory::Alignment_t::kAlign64, speed_input_bytes));


  for (size_t i = 0 ; i < kBytesPerAvx2; i++) {
    random_one[i] = rand() % 100;
  }

  for (size_t i = 0 ; i < kBytesPerAvx2; i++) {
    zeros[i] = 0;
  }

  for (size_t i = 0 ; i < kBytesPerAvx2; i++) {
    uniform[i] = 0xBB;
  }

  for (size_t i = 0 ; i < 3 * kBytesPerAvx2; i++) {
    three_cycles[i] = static_cast<uint8_t>(rand() % 100);
  }

  for (size_t i = 0; i < 100 * kBytesPerAvx2; i++) {
    hundred_cycles[i] = static_cast<uint8_t>(rand() % 100);
  }

  for (size_t i = 0; i < speed_input_bytes; i++) {
    speed_packed_input[i] = static_cast<uint8_t>(rand() % 100);
  }
}

void teardown() {
  free(truth_result);
  free(function_result);
  free(speed_result);
  free(speed_packed_input);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  setup();
  int ret = RUN_ALL_TESTS();
  teardown();
  return ret;
}
