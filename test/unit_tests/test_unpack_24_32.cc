#include <iostream>
#include <immintrin.h>
#include <gtest/gtest.h>
#include <stdlib.h>
#include <assert.h>

#define TESTSZ 100

int8_t i = 4;
int8_t s = 5;
int8_t q = 6;

// map bytes into position
const static __m256i bytegrouping =
  _mm256_setr_epi8(i,s, s,q,  i + 3,s + 3, s + 3,q + 3,  i + 6,s + 6, s + 6,q +6,  i + 9,s + 9, s + 9,q + 9,
                    i + 12,s + 12, s + 12,q + 12,  i + 15, s + 15, s + 15,q + 15,   i + 18, s + 18, s + 18,q + 18,   i + 21,s + 21, s + 21,q + 21);


/*
 * the new way
 */
void unpack(uint8_t *p, __m256i *r, size_t s)
{
  __m256i v, shuffle, shift, mask;
  assert(s % 24 == 0);

  __m256i *ret = (__m256i*) r;
  for (size_t i = 0; i < s / 24; i++) {
    v = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(p - 4));  // load content
    shuffle = _mm256_shuffle_epi8(v, bytegrouping);  // move bytes around
    shift = _mm256_slli_epi16(shuffle, 4);  // shift each 16 bytes chunk left by 4 bits
    mask  = _mm256_and_si256(shuffle, _mm256_set1_epi32(static_cast<int>(0xFFF00000)));  // mask out a nibble in the second int
    _mm256_store_si256(&ret[i], _mm256_blend_epi16(shift, mask, 0b10101010));  // merge shift and mask
    p += 24;
  }
}

/*
 * Current way of unpacking
 */
void n_unpack(uint8_t *p, uint16_t *r, size_t s)
{
  assert(s % 3 == 0);
  for (size_t i = 0; i < s / 3; i++) {
    const uint16_t i_lsb = uint16_t(p[3 * i]);
    const uint16_t split = uint16_t(p[3 * i + 1u]);
    const uint16_t q_msb = uint16_t(p[3 * i + 2u]);
    r[i * 2] = int16_t((split << 12u) | (i_lsb << 4u));
    r[i * 2 + 1] = int16_t((q_msb << 8u) | (split & 0xf0));
  }
}

/*
 * Unit tests input values
 */
  
uint8_t random_one[24];
uint8_t zeros[24];
uint8_t uniform[24];
uint8_t three_cycles[72];
uint8_t hundred_cycles[2400];
uint8_t speed_test[24 * TESTSZ];
uint16_t speed_ref[16 * TESTSZ];

TEST (Unpack24To32, NaiveSpeed) {
  uint16_t *ret_n = (uint16_t*) malloc(32 * TESTSZ);
  n_unpack(speed_test, ret_n, sizeof(speed_test));
  
  ASSERT_EQ(0, memcmp(ret_n, speed_ref, sizeof(ret_n))); 
}

TEST (Unpack24To32, ModifiedSpeed) {
  __m256i *ret_t;
  if ((ret_t = (__m256i*) malloc(32 * TESTSZ)) == NULL) {
    printf("malloc fail\n");
  }
  unpack(speed_test, ret_t, sizeof(speed_test));
  ASSERT_EQ(0, memcmp(ret_t, speed_ref, sizeof(ret_t)));
}

TEST (Unpack24To32, RandomValue) {
  uint16_t ret_n[16];
  __m256i ret_t[1];
  
  n_unpack(random_one, ret_n, sizeof(random_one));
  unpack(random_one, ret_t, sizeof(random_one));

  ASSERT_EQ(0, memcmp(ret_n, ret_t, sizeof(ret_n)));
}

TEST (Unpack24To32, ZeroValues) {
  uint16_t ret_n[16];
  __m256i ret_t[1];
  
  n_unpack(zeros, ret_n, sizeof(zeros));
  unpack(zeros, ret_t, sizeof(zeros));

  ASSERT_EQ(0, memcmp(ret_n, ret_t, sizeof(ret_n)));
}

TEST (Unpack24To32, UniformValues) {
  uint16_t ret_n[16];
  __m256i ret_t[1];
  
  n_unpack(uniform, ret_n, sizeof(uniform));
  unpack(uniform, ret_t, sizeof(uniform));

  ASSERT_EQ(0, memcmp(ret_n, ret_t, sizeof(ret_n)));
}

TEST (Unpack24To32, Three256Cycles) {
  uint16_t ret_n[48];
  __m256i ret_t[3];
  
  n_unpack(three_cycles, ret_n, sizeof(three_cycles));
  unpack(three_cycles, ret_t, sizeof(three_cycles));

  ASSERT_EQ(0, memcmp(ret_n, ret_t, sizeof(ret_n)));
}

TEST (Unpack24To32, AHundred256Cycles) {
  uint16_t ret_n[1600];
  __m256i ret_t[100];
  
  n_unpack(hundred_cycles, ret_n, sizeof(hundred_cycles));
  unpack(hundred_cycles, ret_t, sizeof(hundred_cycles));

  ASSERT_EQ(0, memcmp(ret_n, ret_t, sizeof(ret_n)));
}

void setup() {
  for (size_t i = 0 ; i < sizeof(random_one); i++) {
    random_one[i] = rand() % 100;
  }

  for (size_t i = 0 ; i < sizeof(zeros); i++) {
    zeros[i] = 0;
  }

  for (size_t i = 0 ; i < sizeof(uniform); i++) {
    uniform[i] = 0xBB;
  }

  for (size_t i = 0 ; i < sizeof(three_cycles); i++) {
    three_cycles[i] = static_cast<uint8_t>(rand() % 100);
  }

  for (size_t i = 0; i < sizeof(hundred_cycles); i++) {
    hundred_cycles[i] = static_cast<uint8_t>(rand() % 100);
  }

  for (size_t i = 0; i < sizeof(speed_test); i++) {
    speed_test[i] = static_cast<uint8_t>(rand() % 100);
  }

  n_unpack(speed_test, speed_ref, sizeof(speed_test));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  setup();
  return RUN_ALL_TESTS();
}
