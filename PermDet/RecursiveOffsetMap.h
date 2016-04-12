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
  
  std::array<uint64_t, TPerm::size> map;
  RecursiveOffsetMap<width, height - 1> child;
};

template<unsigned width>
struct RecursiveOffsetMap<width, /*height = */ 0> {
};

#endif /* RecursiveOffsetMap_h */
