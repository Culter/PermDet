//
//  EngineFlatter.h
//  PermDet
//
//  Created by culter on 3/30/16.
//  Copyright © 2016 culter. All rights reserved.
//

#include "PartialPermutation.h"
#include "RecursiveOffsetMap.h"
#include "RecursiveMask.h"

#ifndef EngineFlatter_h
#define EngineFlatter_h

class EngineFlatter {
public:
  static constexpr int num_row_values = ((uint64_t)1 << N);
  
  EngineFlatter(): offset_maps(), local_sum(0) {
    mask_even.mask = 0;
    mask_odd.mask = 0;
    for (uint64_t i = 0; i < PartialPermutation<N, N>::size; ++i) {
      if (get_parity(PartialPermutation<N, N>(i).rows)) {
        mask_odd.mask[i] = true;
      } else {
        mask_even.mask[i] = true;
      }
    }
    mask_even.pre_mask = offset_maps.split(mask_even.mask);
    mask_odd.pre_mask = offset_maps.split(mask_odd.mask);
  }
  
  uint64_t Count(int first_row);
  
  uint64_t local_sum;
  
private:
  const RecursiveOffsetMap<N, N> offset_maps;
  RecursiveMask<N, N> mask_even;
  RecursiveMask<N, N> mask_odd;
};

namespace EngineFlatterDetail {
  template<int row>
  void CommitRowValue(EngineFlatter& that,
                      const RecursiveOffsetMap<N, N - row>& offset_map,
                      RecursiveMask<N, N - row>& mask_even,
                      RecursiveMask<N, N - row>& mask_odd,
                      int current_streak,
                      uint64_t stabilizer,
                      int row_value,
                      int max_value);
  template<int row>
  void CountFollowingRows(EngineFlatter& that,
                          const RecursiveOffsetMap<N, N - row>& offset_map,
                          RecursiveMask<N, N - row>& mask_even,
                          RecursiveMask<N, N - row>& mask_odd,
                          int current_streak,
                          uint64_t stabilizer,
                          int row_value,
                          int max_value);
  
  template<int row>
  void CommitRowValue(EngineFlatter& that,
                      const RecursiveOffsetMap<N, N - row>& offset_map,
                      RecursiveMask<N, N - row>& mask_even,
                      RecursiveMask<N, N - row>& mask_odd,
                      int current_streak,
                      uint64_t stabilizer,
                      int row_value,
                      int max_value) {
    static_assert(row < N - 1, "This function is meant to be called only for rows before the final row.");
    
    if (row_value == max_value) {
      current_streak += 1;
      stabilizer *= current_streak;
    } else {
      current_streak = 1;
      max_value = row_value;
    }
    
    mask_even.child.mask = offset_map.child.combine(mask_even.pre_mask, row_value);
    mask_odd.child.mask = offset_map.child.combine(mask_odd.pre_mask, row_value);
    
    CountFollowingRows<row>(that,
                            offset_map,
                            mask_even,
                            mask_odd,
                            current_streak,
                            stabilizer,
                            row_value,
                            max_value);
  }
  
  template<int row>
  void CountFollowingRows(EngineFlatter& that,
                          const RecursiveOffsetMap<N, N - row>& offset_map,
                          RecursiveMask<N, N - row>& mask_even,
                          RecursiveMask<N, N - row>& mask_odd,
                          int current_streak,
                          uint64_t stabilizer,
                          int row_value,
                          int max_value) {
    mask_even.child.pre_mask = offset_map.child.split(mask_even.child.mask);
    mask_odd.child.pre_mask = offset_map.child.split(mask_odd.child.mask);
    
    for (int next_row_value = max_value; next_row_value < EngineFlatter::num_row_values; ++next_row_value) {
                                                                  
      CommitRowValue<row + 1>(that,
                              offset_map.child,
                              mask_even.child,
                              mask_odd.child,
                              current_streak,
                              stabilizer,
                              next_row_value,
                              max_value);
    }
  }
  
  template<>
  void CountFollowingRows<N - 2>(EngineFlatter& that,
                                 const RecursiveOffsetMap<N, 2>& offset_map,
                                 RecursiveMask<N, 2>& mask_even,
                                 RecursiveMask<N, 2>& mask_odd,
                                 int current_streak,
                                 uint64_t stabilizer,
                                 int row_value,
                                 int max_value) {
    mask_even.child.mask.flip();
    mask_odd.child.mask.flip();
    
    constexpr uint64_t group_order = factorial(N - 1);
    uint64_t orbit = group_order / stabilizer;
    
    if (stabilizer == 1) {
      that.local_sum += ((uint64_t(1) << mask_odd.child.mask.count()) +
                         (uint64_t(1) << mask_even.child.mask.count())) * (orbit / 2);
    } else {
      that.local_sum += (uint64_t(1) << mask_odd.child.mask.count()) * orbit;
    }
  }
}

uint64_t EngineFlatter::Count(int first_row) {
  EngineFlatterDetail::CommitRowValue<0>(*this,
                                         offset_maps,
                                         mask_even,
                                         mask_odd,
                                         0,  // current_streak
                                         1,  // stabilizer
                                         first_row,
                                         0);
  return local_sum;
}

#endif /* EngineFlatter_h */
