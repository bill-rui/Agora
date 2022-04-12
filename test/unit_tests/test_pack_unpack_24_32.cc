#include <iostream>
#include <immintrin.h>
#include <gtest/gtest.h>
#include <stdlib.h>
#include <assert.h>

#include "data_pack_unpack.h"
#include "memory_manage.h"

/* Order of tests matters if they are storing result in the same block of
 *  memory.
 */

/*
 * Unit tests input values
 */
static constexpr size_t kSpeedIterations = kTestReps * kBytesPerAvx2;
static constexpr size_t kMaxCorrectnessCycles = 100;
static constexpr size_t kPackedSizeMinBytes = 3;
static constexpr size_t kUnpackedSizeMinBytes = 4;
static constexpr size_t output_bytes_per_cycle_unpack = kBytesPerAvx2 * 
                                                    kUnpackedSizeMinBytes / 
                                                    kPackedSizeMinBytes;
static constexpr size_t output_bytes_per_cycle_pack = kBytesPerAvx2;
const size_t result_size_bytes = kSpeedIterations * kUnpackedSizeMinBytes;
const size_t speed_input_bytes = kSpeedIterations * kPackedSizeMinBytes;

/* test results storage */
int16_t *truth_result;
int16_t *function_result;
uint8_t *speed_packed_input;
int16_t *speed_result;
int16_t *speed_result2;
int16_t *speed_result3;
int16_t *speed_result4;

/* unpacking tests inputs */
uint8_t random_one[kBytesPerAvx2];
uint8_t zeros[kBytesPerAvx2];
uint8_t uniform[kBytesPerAvx2];
uint8_t three_cycles[kBytesPerAvx2 * 3];
uint8_t hundred_cycles[kBytesPerAvx2 * 100];

/* packing tests inputs */
uint8_t random_one_pack[kBytesUnpackedAvx2];
uint8_t three_cycles_pack[kBytesUnpackedAvx2 * 3];
uint8_t hundred_cycles_pack[kBytesUnpackedAvx2 * 100];
uint8_t known_pack[kBytesUnpackedAvx2] = 
  {0xb0, 0xda, 0xc0, 0xef, 0xb0, 0xda, 0xc0, 0xef, 0xb0, 0xda, 0xc0, 0xef,
   0xb0, 0xda, 0xc0, 0xef, 0xb0, 0xda, 0xc0, 0xef, 0xb0, 0xda, 0xc0, 0xef,
   0xb0, 0xda, 0xc0, 0xef, 0xb0, 0xda, 0xc0, 0xef};

uint8_t known_pack_result[kBytesPerAvx2] =
  {0xab, 0xcd, 0xef, 0xab, 0xcd, 0xef, 0xab, 0xcd, 0xef, 0xab, 0xcd, 0xef, 
   0xab, 0xcd, 0xef, 0xab, 0xcd, 0xef, 0xab, 0xcd, 0xef, 0xab, 0xcd, 0xef};

/* Performance tests */
TEST (Performance, packing) {
  pack32_24_avx2(speed_packed_input, reinterpret_cast<uint8_t*>(speed_result2), 
    speed_input_bytes);
}

TEST (Performance, packingCurrentImplementation) {
  pack32_24_naive(reinterpret_cast<std::complex<int16_t> *>(speed_packed_input), 
    reinterpret_cast<uint8_t*>(speed_result), speed_input_bytes);
}

TEST (Performance, CurrentImplementation) {
  unpack24_32_naive(speed_packed_input, speed_result4, 
                    speed_input_bytes);
}

TEST (Performance, SIMDImplementation) {
  unpack24_32_avx2(speed_packed_input, 
    reinterpret_cast<__m256i*>(speed_result3), speed_input_bytes);
}

/* Unpacking Tests */

TEST (CorrectnessUnpack, KnownUnpack) {
  const size_t output_bytes = output_bytes_per_cycle_unpack;
  unpack24_32_avx2(known_pack_result, 
    reinterpret_cast<__m256i*>(function_result), kBytesPerAvx2);

  ASSERT_EQ(0, memcmp(known_pack, function_result, output_bytes));
}

TEST (CorrectnessUnpack, OneCycleRandomUnpack) {
  const size_t output_bytes = output_bytes_per_cycle_unpack;
  unpack24_32_naive(random_one, truth_result, kBytesPerAvx2);
  unpack24_32_avx2(random_one, reinterpret_cast<__m256i*>(function_result), 
                   kBytesPerAvx2);

  ASSERT_EQ(0, memcmp(truth_result, function_result, output_bytes));
}

TEST (CorrectnessUnpack, OneCycleZerosUnpack) {
  const size_t output_bytes = output_bytes_per_cycle_unpack;
  unpack24_32_naive(zeros, truth_result, kBytesPerAvx2);
  unpack24_32_avx2(zeros, reinterpret_cast<__m256i*>(function_result), 
                   kBytesPerAvx2);

  ASSERT_EQ(0, memcmp(truth_result, function_result, output_bytes));
}

TEST (CorrectnessUnpack, OneCylceUniformUnpack) {
  const size_t output_bytes = output_bytes_per_cycle_unpack;
  unpack24_32_naive(uniform, truth_result, kBytesPerAvx2);
  unpack24_32_avx2(uniform, reinterpret_cast<__m256i*>(function_result), 
                   kBytesPerAvx2);

  ASSERT_EQ(0, memcmp(truth_result, function_result, output_bytes));
}

TEST (CorrectnessUnpack, ThreeCyclesRandomUnpack) {
  const size_t output_bytes = 3 * output_bytes_per_cycle_unpack;
  unpack24_32_naive(three_cycles, truth_result, kBytesPerAvx2);
  unpack24_32_avx2(three_cycles, reinterpret_cast<__m256i*>(function_result), 
                   kBytesPerAvx2);

  ASSERT_EQ(0, memcmp(truth_result, function_result, 3 * output_bytes));
}

TEST (CorrectnessUnpack, 100CyclesRandomUnpack) {
  const size_t output_bytes = 100 * output_bytes_per_cycle_unpack;
  unpack24_32_naive(hundred_cycles, truth_result, kBytesPerAvx2);
  unpack24_32_avx2(hundred_cycles, reinterpret_cast<__m256i*>(function_result), 
                   kBytesPerAvx2);

  ASSERT_EQ(0, memcmp(truth_result, function_result, 100 * output_bytes));
}

/* Packing Tests */

TEST (CorrectnessPack, KnownPack) {
  const size_t output_bytes = output_bytes_per_cycle_pack;
  pack32_24_avx2(known_pack, reinterpret_cast<uint8_t *>(function_result), 
    kBytesUnpackedAvx2);

  ASSERT_EQ(0, memcmp(known_pack_result, function_result, output_bytes));
}

TEST (CorrectnessPack, OneCycleRandomPack) {
  const size_t output_bytes = output_bytes_per_cycle_pack;
  pack32_24_naive(reinterpret_cast<std::complex<int16_t> *>(random_one_pack), 
    reinterpret_cast<uint8_t *>(truth_result), kBytesUnpackedAvx2);
  pack32_24_avx2(random_one_pack, reinterpret_cast<uint8_t *>(function_result), 
    kBytesUnpackedAvx2);

  ASSERT_EQ(0, memcmp(truth_result, function_result, output_bytes));
}

TEST (CorrectnessPack, HundredCyclesPack) {
  const size_t output_bytes = 100 * output_bytes_per_cycle_pack;
  pack32_24_naive(
    reinterpret_cast<std::complex<int16_t> *>(hundred_cycles_pack), 
    reinterpret_cast<uint8_t *>(truth_result), 100 * kBytesUnpackedAvx2);
  pack32_24_avx2(hundred_cycles_pack,
    reinterpret_cast<uint8_t *>(function_result), 100 * kBytesUnpackedAvx2);

  ASSERT_EQ(0, memcmp(truth_result, function_result, output_bytes));
}

TEST (CorrectnessOverall, UnpackThenPackThreeCycle) {
  const size_t output_bytes = 3 * kBytesPerAvx2;
  unpack24_32_avx2(three_cycles, reinterpret_cast<__m256i*>(function_result), 
                   3 * kBytesPerAvx2);
  pack32_24_avx2(reinterpret_cast<uint8_t *>(function_result), 
    reinterpret_cast<uint8_t *>(truth_result), 3 * kBytesUnpackedAvx2);

  ASSERT_EQ(0, memcmp(truth_result, three_cycles, output_bytes));
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

  for (size_t i = 0 ; i < kBytesUnpackedAvx2; i++) {
    if (i % 4 == 0 || i % 4 == 2) {
      random_one_pack[i] = static_cast<uint8_t>(rand() % 100) & 0xF0;
    } else {
      random_one_pack[i] = rand() % 100;
    }
  }
  
  for (size_t i = 0 ; i < kBytesUnpackedAvx2 * 100; i++) {
    if (i % 4 == 0 || i % 4 == 2) {
      hundred_cycles_pack[i] = static_cast<uint8_t>(rand() % 100) & 0xF0;
    } else {
      hundred_cycles_pack[i] = rand() % 100;
    }
  }
}

void teardown() {
  free(truth_result);
  free(function_result);
  free(speed_result);
  free(speed_packed_input);
  free(speed_result2);
  free(speed_result3);
  free(speed_result4);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  setup();
  int ret = RUN_ALL_TESTS();
  teardown();
  return ret;
}
