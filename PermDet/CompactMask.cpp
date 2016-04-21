//
//  CompactMask.cpp
//  PermDet
//
//  Created by culter on 4/20/16.
//  Copyright © 2016 culter. All rights reserved.
//

#include <bitset>

#include "immintrin.h"

#include "CompactMask.h"

uint64_t MaskAndCountReference(const uint64_t population_mask[2],
                               const uint64_t row_mask_lo,
                               const uint64_t row_mask_hi) {
  const uint64_t masked_population_lo = row_mask_lo | population_mask[0];
  const uint64_t masked_population_hi = row_mask_hi | population_mask[1];
  const uint64_t masked_population_5 = masked_population_lo & masked_population_hi;
  const uint64_t masked_population_4 = masked_population_5 & (masked_population_5 >> 1);
  const uint64_t masked_population_2 = masked_population_4 & (masked_population_4 >> 2);
  const uint64_t masked_population = masked_population_2 & (masked_population_2 >> 1);
  
  const uint64_t count = std::bitset<64>(masked_population).count();
  const uint64_t subtotal = (uint64_t)1 << count;
  
  return subtotal;
}

void MaskAndCountFast(const uint64_t population_mask[2],
                      const uint64_t row_mask_lo[],
                      const uint64_t row_mask_hi[],
                      int num_rows,
                      uint64_t output[4]) {
  const __m256i ones = _mm256_set1_epi64x(1ull);
  const __m256i first_bits = _mm256_set1_epi64x(0b00000000000000000000000000001111ull);
  const __m256i multiplier = _mm256_set1_epi64x(0b10000010000010000010000010000000ull);
  
  // --43210-43210-43210-43210-43210-;--43210-43210-43210-43210-43210-
  // --99999-88888-77777-66666-55555-;--44444-33333-22222-11111-00000-
  const __m256i m_population_lo = _mm256_set1_epi64x(population_mask[0]);
  // --98765-98765-98765-98765-98765-;--98765-98765-98765-98765-98765-
  // --99999-88888-77777-66666-55555-;--44444-33333-22222-11111-00000-
  const __m256i m_population_hi = _mm256_set1_epi64x(population_mask[1]);
  
  __m256i total = _mm256_setzero_si256();
  
  for (int i = 0; i < num_rows; ++i) {
    // --43210-43210-43210-43210-43210-;--43210-43210-43210-43210-43210-
    const __m256i m_row_mask_lo = _mm256_load_si256((__m256i*)row_mask_lo + i);
    // --98765-98765-98765-98765-98765-;--98765-98765-98765-98765-98765-
    const __m256i m_row_mask_hi = _mm256_load_si256((__m256i*)row_mask_hi + i);
    
    // --99999-88888-77777-66666-55555-;--44444-33333-22222-11111-00000- (only considering 43210)
    const __m256i m_masked_population_lo = _mm256_or_si256(m_population_lo, m_row_mask_lo);
    // --99999-88888-77777-66666-55555-;--44444-33333-22222-11111-00000- (only considering 98765)
    const __m256i m_masked_population_hi = _mm256_or_si256(m_population_hi, m_row_mask_hi);
    
    // --99999-88888-77777-66666-55555-;--44444-33333-22222-11111-00000-
    const __m256i m_masked_population_5 = _mm256_and_si256(m_masked_population_lo, m_masked_population_hi);
    // ---9999--8888--7777--6666--5555-;---4444--3333--2222--1111--0000-
    const __m256i m_masked_population_4 = _mm256_and_si256(m_masked_population_5, _mm256_srli_epi64(m_masked_population_5, 1));
    // -----99----88----77----66----55-;-----44----33----22----11----00-
    const __m256i m_masked_population_2 = _mm256_and_si256(m_masked_population_4, _mm256_srli_epi64(m_masked_population_4, 2));
    // ------9-----8-----7-----6-----5-;------4-----3-----2-----1-----0-
    const __m256i m_masked_population = _mm256_and_si256(m_masked_population_2, _mm256_srli_epi64(m_masked_population_2, 1));
    
    // ------9-----8-----7-----6-----5-;-----**----**----**----**----**-
    const __m256i m_compact_population = _mm256_add_epi64(m_masked_population, _mm256_srli_epi64(m_masked_population, 32));
    
    // ------??---???---???--????--****;--????---???---???----??--------
    const __m256i count_shifted = _mm256_mul_epu32(m_compact_population, multiplier);
    
    // --------------------------------;------??---???---???--????--****
    const __m256i count_with_garbage = _mm256_srli_epi64(count_shifted, 32);
    
    const __m256i count = _mm256_and_si256(count_with_garbage, first_bits);
    const __m256i subtotal = _mm256_sllv_epi64(ones, count);
    
    total = _mm256_add_epi64(total, subtotal);
  }
  
  _mm256_store_si256((__m256i*)output, total);
}