//
//  CompactMask.h
//  PermDet
//
//  Created by culter on 4/18/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef CompactMask_h
#define CompactMask_h

#include "PartialPermutation.h"

// --99999-88888-77777-66666-55555-;--44444-33333-22222-11111-00000-
constexpr int MaskIndexFromLastRow(int last_row) {
  return 1 + (6 * last_row) + (last_row >= 5 ? 2 : 0);
}

// --43210-43210-43210-43210-43210-;--43210-43210-43210-43210-43210-
// or
// --98765-98765-98765-98765-98765-;--98765-98765-98765-98765-98765-
constexpr int MaskBlockFromPenultimateRow(int penultimate_row) {
  return penultimate_row / 5;
}
constexpr int MaskOffsetFromPenultimateRow(int penultimate_row) {
  return penultimate_row % 5;
}

constexpr uint64_t EmptyPopulationMask(int width) {
  return (width <= 0 ?
          0 :
          EmptyPopulationMask(width - 1) | (0b11111ull << MaskIndexFromLastRow(width - 1)));
}

template<unsigned width, unsigned height>
struct CompactMask {
  typedef PartialPermutation<width, height> TPerm;
  typedef PartialPermutation<width, height - 2> TPermMinusTwo;
  
  void Reset() {
    constexpr uint64_t empty = EmptyPopulationMask(width);
    std::fill(std::begin(population_mask), std::end(population_mask), empty);
  }
  
  void Set(const TPerm& perm) {
    const int next_last_row = perm.rows[height - 2];
    const int last_row = perm.rows[height - 1];
    TPermMinusTwo outer_index_bearer = perm.pop_back().pop_back();
    
    uint64_t outer_index = 2 * outer_index_bearer.Index() + MaskBlockFromPenultimateRow(next_last_row);
    int bit_index = MaskIndexFromLastRow(last_row) + MaskOffsetFromPenultimateRow(next_last_row);
    
    population_mask[outer_index] &= ~((uint64_t)1 << bit_index);
  }
  
  alignas(32) std::array<uint64_t, TPermMinusTwo::size * 2> population_mask;
};

template<unsigned width>
struct CompactMask<width, /*height = */ 1> {};

template<unsigned width, unsigned height>
struct SplitCompactMask {
  typedef PartialPermutation<width, height - 3> TPermMinusThree;
  
  void Reset() {
    constexpr uint64_t empty = EmptyPopulationMask(width);
    std::fill(std::begin(population_mask), std::end(population_mask), empty);
  }
  
  alignas(32) std::array<uint64_t, width * TPermMinusThree::size * 2> population_mask;
};

template<unsigned width>
void GetRowMask(int row_value, uint64_t& lo, uint64_t& hi) {
  lo = 0;
  hi = 0;
  
  std::bitset<width> row_bits(row_value);
  for (int i = 0; i < width; ++i) {
    if (!row_bits[i]) {
      uint64_t& block = (MaskBlockFromPenultimateRow(i) ? hi : lo);
      for (int j = 0; j < width; ++j) {
        block |= ((uint64_t)1 << (MaskIndexFromLastRow(j) + MaskOffsetFromPenultimateRow(i)));
      }
    }
  }
}

uint64_t MaskAndCountReference(const std::array<uint64_t, 2>& population_mask,
                               const uint64_t row_mask_lo,
                               const uint64_t row_mask_hi);

void MaskAndCountFast(const std::array<uint64_t, 2>& population_mask,
                      const uint64_t row_mask_lo[],
                      const uint64_t row_mask_hi[],
                      int num_rows,
                      uint64_t output[4]);
#endif /* CompactMask_h */
