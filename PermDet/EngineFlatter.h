//
//  EngineFlatter.h
//  PermDet
//
//  Created by culter on 3/30/16.
//  Copyright © 2016 culter. All rights reserved.
//

#include <mutex>
#include <thread>
#include <vector>

#include "math_utils.h"
#include "PartialPermutation.h"
#include "RecursiveOffsetMap.h"
#include "ImmediateRowOrderer.h"
#include "ColumnOrderer.h"
#include "CompactMask.h"
#include "OptimalReference.h"

#ifndef EngineFlatter_h
#define EngineFlatter_h

// Set this to 0 to disable parallelization completely.
// A reasonable value is 2, perhaps 3 at the most.
constexpr int kNumParallelLevels = 3;

// This constant is flexible. It can be as little as 0 or as great as N - 2.
constexpr int last_row_for_column_ordering = (N >= 3 ? N - 3 : 0);

class EngineFlatter {
public:
  // There are classes having Hamming weight from 0 to N, inclusive.
  static constexpr int num_row_classes = N + 1;
  // Each row has N bits, so there are 2^N possible values.
  static constexpr int num_row_values = ((uint64_t)1 << N);
  
  EngineFlatter() {
    // Populate row values
    for (int i = 0; i < num_row_values; ++i) {
      row_values[i] = i;
    }
    std::stable_sort(row_values.begin(), row_values.end(), [] (std::bitset<N> a, std::bitset<N> b) {
      return a.count() < b.count();
    });
    
    for (int i = 0; i < num_row_values; ++i) {
      GetRowMask<N>(row_values[i], row_value_masks_lo[i], row_value_masks_hi[i]);
    }
    
    row_class_offsets[0] = 0;
    for (int i = 0; i < num_row_classes; ++i) {
      row_class_offsets[i + 1] = row_class_offsets[i] + factorial(N) / (factorial(i) * factorial(N - i));
    }
  }
  
  static uint64_t Count();
  
  std::array<int, num_row_classes + 1> row_class_offsets;  // The +1 gives us a sentinel.
  std::array<int, num_row_values> row_values;
  
  alignas(32) uint64_t row_value_masks_lo[num_row_values];
  alignas(32) uint64_t row_value_masks_hi[num_row_values];
};

namespace EngineFlatterDetail {
  template<int row>
  uint64_t CountFollowingRows(const EngineFlatter& that,
                              const RecursiveOffsetMap<N, N - 3 - row>& offset_map,
                              OptimalReference<CompactMask<N, N - row - 1>>& mask_even,
                              OptimalReference<CompactMask<N, N - row - 1>>& mask_odd,
                              ImmediateRowOrderer row_orderer,
                              ColumnOrderer column_orderer);
  
  template<int row>
  uint64_t CommitRowValue(const EngineFlatter& that,
                          const RecursiveOffsetMap<N, N - 3 - row>& offset_map,
                          OptimalReference<SplitCompactMask<N, N - row>>& mask_even,
                          OptimalReference<SplitCompactMask<N, N - row>>& mask_odd,
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
      return 0;
    }
    
    if (row <= last_row_for_column_ordering) {
      if (column_orderer.accepts(row_value)) {
        column_orderer.append(row_value);
      } else {
        return 0;
      }
      
      if (row == last_row_for_column_ordering) {
        column_orderer.freeze_stabilizer();
      }
    }
    
    OptimalReference<CompactMask<N, N - row - 1>> child_mask_even;
    OptimalReference<CompactMask<N, N - row - 1>> child_mask_odd;
    
    if (row_orderer.stabilizer == 1 || column_orderer.stabilizer == 1) {
      child_mask_even->Reset();
      offset_map.combine(mask_even->population_mask, row_value, child_mask_even->population_mask);
    }
    child_mask_odd->Reset();
    offset_map.combine(mask_odd->population_mask, row_value, child_mask_odd->population_mask);
    
    return CountFollowingRows<row>(that,
                                   offset_map,
                                   child_mask_even,
                                   child_mask_odd,
                                   row_orderer,
                                   column_orderer);
  }
  
  template<int row>
  uint64_t CountFollowingRows(const EngineFlatter& that,
                              const RecursiveOffsetMap<N, N - 3 - row>& offset_map,
                              OptimalReference<CompactMask<N, N - row - 1>>& mask_even,
                              OptimalReference<CompactMask<N, N - row - 1>>& mask_odd,
                              ImmediateRowOrderer row_orderer,
                              ColumnOrderer column_orderer) {
    OptimalReference<SplitCompactMask<N, N - row - 1>> child_mask_even;
    OptimalReference<SplitCompactMask<N, N - row - 1>> child_mask_odd;
    
    uint64_t local_sum = 0;
    
    if (row_orderer.stabilizer == 1 || column_orderer.stabilizer == 1) {
      child_mask_even->Reset();
      offset_map.split(mask_even->population_mask, child_mask_even->population_mask);
    }
    child_mask_odd->Reset();
    offset_map.split(mask_odd->population_mask, child_mask_odd->population_mask);
    
    if (row + 1 < kNumParallelLevels) {
      std::vector<std::thread> threads;
      std::mutex sum_mutex;
      for (int next_class = row_orderer.max_row_class; next_class < EngineFlatter::num_row_classes; ++next_class) {
        if (row_orderer.accepts_class(next_class)) {
          for (int next_row_index = that.row_class_offsets[next_class];
               next_row_index < that.row_class_offsets[next_class + 1];
               ++next_row_index) {
            if ((row + 1 > last_row_for_column_ordering) || column_orderer.accepts(that.row_values[next_row_index])) {
              threads.emplace_back([&, next_class, next_row_index]{
                auto subtotal = CommitRowValue<row + 1>(that,
                                                        offset_map.child,
                                                        child_mask_even,
                                                        child_mask_odd,
                                                        row_orderer,
                                                        next_class,
                                                        next_row_index,
                                                        column_orderer);
                {
                  std::lock_guard<std::mutex> lock(sum_mutex);
                  local_sum += subtotal;
                }
              });
            }
          }
        }
      }
      for (auto& thread: threads) {
        thread.join();
      }
    } else {
      for (int next_class = row_orderer.max_row_class; next_class < EngineFlatter::num_row_classes; ++next_class) {
        if (row_orderer.accepts_class(next_class)) {
          for (int next_row_index = that.row_class_offsets[next_class];
               next_row_index < that.row_class_offsets[next_class + 1];
               ++next_row_index) {
            local_sum += CommitRowValue<row + 1>(that,
                                                 offset_map.child,
                                                 child_mask_even,
                                                 child_mask_odd,
                                                 row_orderer,
                                                 next_class,
                                                 next_row_index,
                                                 column_orderer);
          }
        }
      }
    }
    
    return local_sum;
  }
  
  template<>
  uint64_t CountFollowingRows<N - 3>(const EngineFlatter& that,
                                     const RecursiveOffsetMap<N, 0>& offset_map,
                                     OptimalReference<CompactMask<N, 2>>& mask_even,
                                     OptimalReference<CompactMask<N, 2>>& mask_odd,
                                     ImmediateRowOrderer row_orderer,
                                     ColumnOrderer column_orderer) {
    uint64_t local_sum = 0;
    
    for (int next_class = row_orderer.max_row_class; next_class < EngineFlatter::num_row_classes;) {
      auto temp_row_orderer = row_orderer;
      if (temp_row_orderer.accepts_class(next_class)) {
        temp_row_orderer.append(next_class, 0);  // TODO: Split up the usage here
        
        // The (N-1)! comes from row reordering.
        // The N! comes from column reordering.
        constexpr uint64_t group_order = factorial(N - 1) * factorial(N);
        uint64_t orbit = group_order / (temp_row_orderer.stabilizer * column_orderer.stabilizer);
        
        int after_class = (next_class == row_orderer.max_row_class) ? (next_class + 1) : (EngineFlatter::num_row_classes);
        
        int a = that.row_class_offsets[next_class];
        int b = round_up(that.row_class_offsets[next_class]);
        int c = round_down(that.row_class_offsets[after_class]);
        int d = that.row_class_offsets[after_class];
        
        next_class = after_class;
        
        for (int next_row_index = a; next_row_index < b; ++next_row_index) {
          if (temp_row_orderer.stabilizer == 1 || column_orderer.stabilizer == 1) {
            local_sum += (MaskAndCountReference(mask_odd->population_mask,
                                                that.row_value_masks_lo[next_row_index],
                                                that.row_value_masks_hi[next_row_index]) +
                          MaskAndCountReference(mask_even->population_mask,
                                                that.row_value_masks_lo[next_row_index],
                                                that.row_value_masks_hi[next_row_index])) * (orbit / 2);
          } else {
            local_sum += MaskAndCountReference(mask_odd->population_mask,
                                               that.row_value_masks_lo[next_row_index],
                                               that.row_value_masks_hi[next_row_index]) * orbit;
          }
        }
        
        if (temp_row_orderer.stabilizer == 1 || column_orderer.stabilizer == 1) {
          local_sum += (MaskAndCountFast(mask_odd->population_mask,
                                         &that.row_value_masks_lo[b],
                                         &that.row_value_masks_hi[b],
                                         (c-b)/4) +
                        MaskAndCountFast(mask_even->population_mask,
                                         &that.row_value_masks_lo[b],
                                         &that.row_value_masks_hi[b],
                                         (c-b)/4)) * (orbit / 2);
        } else {
          local_sum += MaskAndCountFast(mask_odd->population_mask,
                                        &that.row_value_masks_lo[b],
                                        &that.row_value_masks_hi[b],
                                        (c-b)/4) * orbit;
        }
        
        for (int next_row_index = c; next_row_index < d; ++next_row_index) {
          if (temp_row_orderer.stabilizer == 1 || column_orderer.stabilizer == 1) {
            local_sum += (MaskAndCountReference(mask_odd->population_mask,
                                                that.row_value_masks_lo[next_row_index],
                                                that.row_value_masks_hi[next_row_index]) +
                          MaskAndCountReference(mask_even->population_mask,
                                                that.row_value_masks_lo[next_row_index],
                                                that.row_value_masks_hi[next_row_index])) * (orbit / 2);
          } else {
            local_sum += MaskAndCountReference(mask_odd->population_mask,
                                               that.row_value_masks_lo[next_row_index],
                                               that.row_value_masks_hi[next_row_index]) * orbit;
          }
        }
      }
    }
    
    return local_sum;
  }
}

uint64_t EngineFlatter::Count() {
  // Populate masks
  OptimalReference<CompactMask<N, N>> mask_even;
  OptimalReference<CompactMask<N, N>> mask_odd;
  mask_even->Reset();
  mask_odd->Reset();
  for (uint64_t i = 0; i < PartialPermutation<N, N>::size; ++i) {
    PartialPermutation<N, N> perm{i};
    if (get_parity(PartialPermutation<N, N>(i).rows)) {
      mask_odd->Set(perm);
    } else {
      mask_even->Set(perm);
    }
  }
  
  return EngineFlatterDetail::CountFollowingRows<-1>(EngineFlatter{},
                                                     RecursiveOffsetMap<N, N - 2>{},
                                                     mask_even,
                                                     mask_odd,
                                                     ImmediateRowOrderer{},
                                                     ColumnOrderer{});
}

#endif /* EngineFlatter_h */
