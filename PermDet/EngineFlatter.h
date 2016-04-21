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
#include "ImmediateRowOrderer.h"
#include "ColumnOrderer.h"

#include "CompactMask.h"

#ifndef EngineFlatter_h
#define EngineFlatter_h

static constexpr bool k_profile = false;

class EngineFlatter {
public:
  static constexpr int num_row_values = ((uint64_t)1 << N);
  
  EngineFlatter(): offset_maps(), local_sum(0) {
    // Populate masks
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
    
    // Populate row values
    for (int i = 0; i < num_row_values; ++i) {
      row_values[i] = i;
    }
    std::stable_sort(row_values.begin(), row_values.end(), [] (std::bitset<N> a, std::bitset<N> b) {
      return a.count() < b.count();
    });
    
    row_class_offsets[0] = 0;
    for (int i = 0; i < num_row_classes; ++i) {
      row_class_offsets[i + 1] = row_class_offsets[i] + factorial(N) / (factorial(i) * factorial(N - i));
    }
    
    if (k_profile) std::fill(work_counts.begin(), work_counts.end(), 0);
  }
  
  uint64_t Count(int first_row_index);
  bool Accepts(int first_row_index) {
    return ColumnOrderer{}.accepts(row_values[first_row_index]);
  }
  
  uint64_t local_sum;
  
  // There are classes having Hamming weight from 0 to N, inclusive.
  static constexpr int num_row_classes = N + 1;
  std::array<int, num_row_classes + 1> row_class_offsets;
  std::array<int, num_row_values> row_values;
  std::array<uint64_t, num_row_values> row_value_masks;
  
  std::array<uint64_t, N> work_counts;
  
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
                      ImmediateRowOrderer row_orderer,
                      int row_class,
                      int row_index,
                      ColumnOrderer column_orderer);
  template<int row>
  void CountFollowingRows(EngineFlatter& that,
                          const RecursiveOffsetMap<N, N - row>& offset_map,
                          RecursiveMask<N, N - row>& mask_even,
                          RecursiveMask<N, N - row>& mask_odd,
                          ImmediateRowOrderer row_orderer,
                          int row_class,
                          int row_index,
                          ColumnOrderer column_orderer);
  
  template<int row>
  void CommitRowValue(EngineFlatter& that,
                      const RecursiveOffsetMap<N, N - row>& offset_map,
                      RecursiveMask<N, N - row>& mask_even,
                      RecursiveMask<N, N - row>& mask_odd,
                      ImmediateRowOrderer row_orderer,
                      int row_class,
                      int row_index,
                      ColumnOrderer column_orderer) {
    static_assert(row < N - 1, "This function is meant to be called only for rows before the final row.");
    
    // Note there is no need to call row_orderer.accepts_class() here; it is assumed the caller already checked that.
    
    int row_value = that.row_values[row_index];
    
    if (row_orderer.accepts_value(row_value)) {
      row_orderer.append(row_class, row_value);
    } else {
      return;
    }
    
    // This constant is flexible. It can be as little as 0 or as great as N - 2.
    constexpr int last_row_for_column_ordering = (N >= 3 ? N - 3 : 0);
    
    if (row <= last_row_for_column_ordering) {
      if (column_orderer.accepts(row_value)) {
        column_orderer.append(row_value);
      } else {
        return;
      }
      
      if (row == last_row_for_column_ordering) {
        column_orderer.freeze_stabilizer();
      }
    }
    
    if (row_orderer.stabilizer == 1 || column_orderer.stabilizer == 1) {
      mask_even.child.mask = offset_map.child.combine(mask_even.pre_mask, row_value);
    }
    mask_odd.child.mask = offset_map.child.combine(mask_odd.pre_mask, row_value);
    
    CountFollowingRows<row>(that,
                            offset_map,
                            mask_even,
                            mask_odd,
                            row_orderer,
                            row_class,
                            row_index,
                            column_orderer);
  }
  
  template<int row>
  void CountFollowingRows(EngineFlatter& that,
                          const RecursiveOffsetMap<N, N - row>& offset_map,
                          RecursiveMask<N, N - row>& mask_even,
                          RecursiveMask<N, N - row>& mask_odd,
                          ImmediateRowOrderer row_orderer,
                          int row_class,
                          int row_index,
                          ColumnOrderer column_orderer) {
    if (k_profile) that.work_counts[row]++;
      
    if (row_orderer.stabilizer == 1 || column_orderer.stabilizer == 1) {
      mask_even.child.pre_mask = offset_map.child.split(mask_even.child.mask);
    }
    mask_odd.child.pre_mask = offset_map.child.split(mask_odd.child.mask);
    
    for (int next_class = row_orderer.max_row_class; next_class < EngineFlatter::num_row_classes; ++next_class) {
      if (row_orderer.accepts_class(next_class)) {
        for (int next_row_index = that.row_class_offsets[next_class];
             next_row_index < that.row_class_offsets[next_class + 1];
             ++next_row_index) {
          CommitRowValue<row + 1>(that,
                                  offset_map.child,
                                  mask_even.child,
                                  mask_odd.child,
                                  row_orderer,
                                  next_class,
                                  next_row_index,
                                  column_orderer);
        }
      }
    }
  }
  
  template<>
  void CountFollowingRows<N - 2>(EngineFlatter& that,
                                 const RecursiveOffsetMap<N, 2>& offset_map,
                                 RecursiveMask<N, 2>& mask_even,
                                 RecursiveMask<N, 2>& mask_odd,
                                 ImmediateRowOrderer row_orderer,
                                 int row_class,
                                 int row_index,
                                 ColumnOrderer column_orderer) {
    if (k_profile) that.work_counts[N - 2]++;
    
    if (row_orderer.stabilizer == 1 || column_orderer.stabilizer == 1) {
      mask_even.child.mask.flip();
    }
    mask_odd.child.mask.flip();
    
    // The (N-1)! comes from row reordering.
    // The N! comes from column reordering.
    constexpr uint64_t group_order = factorial(N - 1) * factorial(N);
    uint64_t orbit = group_order / (row_orderer.stabilizer * column_orderer.stabilizer);
    
    if (row_orderer.stabilizer == 1 || column_orderer.stabilizer == 1) {
      that.local_sum += ((uint64_t(1) << mask_odd.child.mask.count()) +
                         (uint64_t(1) << mask_even.child.mask.count())) * (orbit / 2);
    } else {
      that.local_sum += (uint64_t(1) << mask_odd.child.mask.count()) * orbit;
    }
  }
}

uint64_t EngineFlatter::Count(int first_row_index) {
  // The loop code needs to know which row class we started with, so just compute it here.
  int first_row_class = 0;
  while (first_row_index >= row_class_offsets[first_row_class + 1]) {
    first_row_class++;
  }
  
  ImmediateRowOrderer row_orderer;
  ColumnOrderer column_orderer;
  
  EngineFlatterDetail::CommitRowValue<0>(*this,
                                         offset_maps,
                                         mask_even,
                                         mask_odd,
                                         row_orderer,
                                         first_row_class,
                                         first_row_index,
                                         column_orderer);
  if (k_profile) {
    for (int i = 0; i < N; ++i) {
      if (work_counts[i] > 0) {
        std::cout << "work_counts[" << i << "] = " << work_counts[i] << std::endl;
      }
    }
  }
  return local_sum;
}

#endif /* EngineFlatter_h */
