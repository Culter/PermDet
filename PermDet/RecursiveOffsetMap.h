//
//  RecursiveOffsetMap.h
//  PermDet
//
//  Created by culter on 4/11/16.
//  Copyright © 2016 culter. All rights reserved.
//

#include "PartialPermutation.h"

#ifndef RecursiveOffsetMap_h
#define RecursiveOffsetMap_h

template<unsigned width, unsigned height>
struct RecursiveOffsetMap {
  typedef PartialPermutation<width, height> TPerm;
  typedef PartialPermutation<width, height - 1> TChildPerm;
  
  static constexpr uint64_t bin_size = TPerm::size / width;
  
  RecursiveOffsetMap() {
    for (uint64_t i = 0; i < TPerm::size; ++i) {
      map[i] = TPerm(i).pop_front().Index();
    }
  }
  
  // Determine the indicies of surviving permutation fragments at the next level,
  // given the currently occupied fragments and a new mask over their first elements.
  std::bitset<TChildPerm::size> children(std::bitset<TPerm::size> occupation,
                                         std::bitset<width> mask) const {
    std::bitset<TChildPerm::size> answer = 0;
    for (int m = 0; m < width; ++m) {
      if (mask[m]) {
        for (uint64_t i = m * bin_size; i < (m + 1) * bin_size; ++i) {
          if (occupation[i]) answer[map[i]] = true;
        }
      }
    }
    return answer;
  }
  
  std::array<std::bitset<TChildPerm::size>, width> split(const std::bitset<TPerm::size>& occupation) const {
    std::array<std::bitset<TChildPerm::size>, width> answer = {};
    for (int m = 0; m < width; ++m) {
      for (uint64_t i = m * bin_size; i < (m + 1) * bin_size; ++i) {
        if (occupation[i]) {
          answer[m][map[i]] = true;
        }
      }
    }
    return answer;
  }
  
  void split(const std::array<uint64_t, TPerm::size * 2>& occupation,
             std::array<uint64_t, width * TChildPerm::size * 2>& answer) const {
    for (int m = 0; m < width; ++m) {
      for (uint64_t i = m * bin_size; i < (m + 1) * bin_size; ++i) {
        answer[(m * TChildPerm::size + map[i])*2 + 0] = occupation[i*2 + 0];
        answer[(m * TChildPerm::size + map[i])*2 + 1] = occupation[i*2 + 1];
      }
    }
  }
  
  std::bitset<TPerm::size> combine(const std::array<std::bitset<TPerm::size>, width>& occupation,
                                   const std::bitset<width>& mask) const {
    std::bitset<TPerm::size> answer = 0;
    for (int m = 0; m < width; ++m) {
      if (mask[m]) {
        answer |= occupation[m];
      }
    }
    return answer;
  }
  
  void combine(const std::array<uint64_t, width * TPerm::size * 2>& occupation,
               const std::bitset<width>& mask,
               std::array<uint64_t, TPerm::size * 2>& answer) const {
    for (int m = 0; m < width; ++m) {
      if (mask[m]) {
        for (int b = 0; b < TPerm::size; ++b) {
          answer[b*2 + 0] &= occupation[(m * TPerm::size + b)*2 + 0];
          answer[b*2 + 1] &= occupation[(m * TPerm::size + b)*2 + 1];
        }
      }
    }
  }
  
  std::array<uint64_t, TPerm::size> map;
  RecursiveOffsetMap<width, height - 1> child;
};

template<unsigned width>
struct RecursiveOffsetMap<width, /*height = */ 0> {
  void combine(const std::array<uint64_t, width * 2>& occupation,
               const std::bitset<width>& mask,
               std::array<uint64_t, 2>& answer) const {
    for (int m = 0; m < width; ++m) {
      if (mask[m]) {
        answer[0] &= occupation[m*2 + 0];
        answer[1] &= occupation[m*2 + 1];
      }
    }
  }
};

#endif /* RecursiveOffsetMap_h */
