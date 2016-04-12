//
//  EngineFlatter.h
//  PermDet
//
//  Created by culter on 3/30/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef EngineFlatter_h
#define EngineFlatter_h

template<unsigned width, unsigned height>
struct PermutationTree
{
  static_assert(height > 0, "height should be > 0");
  static_assert(height <= width, "height should be <= width");
  static_assert(width < 16, "width should be < 16");
  
  static constexpr uint64_t next_size = PermutationTree<width, height - 1>::size;
  static constexpr uint64_t size = (width + 1 - height) * next_size;
  static_assert(size % width == 0, "size should be divisible by width");
  static constexpr uint64_t bin_size = size / width;
  
  static std::array<int, height> perm_from_index(uint64_t index) {
    int unused_indices[width];
    for (int i = 0; i < width; ++i) {
      unused_indices[i] = i;
    }
    
    std::array<int, height> answer;
    unsigned w = width;
    unsigned b = bin_size;
    for (int row = 0; row < height; ++row) {
      uint64_t top_index_position = index / b;
      answer[row] = unused_indices[top_index_position];
      if (row == height - 1) break;
      
      // Now that unused_indices[top_index_position] has been used, prevent it from being used in the next row.
      std::copy(unused_indices + top_index_position + 1,
                unused_indices + w,
                unused_indices + top_index_position);
      
      index = index % b;
      w -= 1;
      b /= w;
    }
    return answer;
  }
  
  static uint64_t index_from_perm(std::array<int, height> perm) {
    std::array<int, width> value_of_index;
    for (int i = 0; i < width; ++i) {
      value_of_index[i] = i;
    }
    
    uint64_t answer = 0;
    unsigned w = width;
    for (int row = 0; row < height; ++row) {
      int top_index = perm[row];
      answer += value_of_index[top_index];
      if (row == height - 1) break;
      
      // Now that top_index has been used, reduce the value of subsequent indices.
      for (int i = top_index + 1; i < width; ++i) {
        value_of_index[i] -= 1;
      }
      w -= 1;
      answer *= w;
    }
    return answer;
  }
  
  typedef std::array<uint64_t, size> OffsetMap;
  
  struct RecursiveOffsetMap {
    typedef PermutationTree<width, height - 1> TChild;
    
    RecursiveOffsetMap() {
      for (uint64_t i = 0; i < size; ++i) {
        std::array<int, height> p = perm_from_index(i);
        std::array<int, height - 1> truncate;
        std::copy(p.begin() + 1, p.end(), truncate.begin());
        map[i] = TChild::index_from_perm(truncate);
      }
    }
    
    // Determine the indicies of surviving permutation fragments at the next level,
    // given the currently occupied fragments and a new mask over their first elements.
    std::bitset<next_size> children(std::bitset<size> occupation,
                                    std::bitset<width> mask) const {
      std::bitset<next_size> answer = 0;
      for (int m = 0; m < width; ++m) {
        if (mask[m]) {
          for (uint64_t i = m * bin_size; i < (m + 1) * bin_size; ++i) {
            if (occupation[i]) answer[map[i]] = true;
          }
        }
      }
      return answer;
    }
    
    std::array<std::bitset<next_size>, width> split(const std::bitset<size>& occupation) const {
      std::array<std::bitset<next_size>, width> answer = {};
      for (int m = 0; m < width; ++m) {
        for (uint64_t i = m * bin_size; i < (m + 1) * bin_size; ++i) {
          if (occupation[i]) {
            answer[m][map[i]] = true;
          }
        }
      }
      return answer;
    }
    
    std::bitset<size> combine(const std::array<std::bitset<size>, width>& occupation,
                              const std::bitset<width>& mask) const {
      std::bitset<size> answer = 0;
      for (int m = 0; m < width; ++m) {
        if (mask[m]) {
          answer |= occupation[m];
        }
      }
      return answer;
    }
    
    OffsetMap map;
    typename TChild::RecursiveOffsetMap child;
  };
  
  struct RecursiveMask {
    std::bitset<size> mask;
    std::array<std::bitset<next_size>, width> pre_mask;
    typename PermutationTree<width, height - 1>::RecursiveMask child;
  };
};

template<unsigned width>
struct PermutationTree<width, /*height = */ 0>
{
  static constexpr uint64_t size = 1;
  static uint64_t index_from_perm(std::array<int, 0>) { return 0; }
  struct RecursiveOffsetMap {};
  struct RecursiveMask {};
};

class EngineFlatter {
public:
  static constexpr int num_row_values = ((uint64_t)1 << N);
  
  EngineFlatter(): offset_maps(), local_sum(0) {
    mask_even.mask = 0;
    mask_odd.mask = 0;
    for (uint64_t i = 0; i < PermutationTree<N, N>::size; ++i) {
      if (get_parity(PermutationTree<N, N>::perm_from_index(i))) {
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
  const PermutationTree<N, N>::RecursiveOffsetMap offset_maps;
  PermutationTree<N, N>::RecursiveMask mask_even;
  PermutationTree<N, N>::RecursiveMask mask_odd;
};

namespace EngineFlatterDetail {
  template<int row>
  void CommitRowValue(EngineFlatter& that,
                      const typename PermutationTree<N, N - row>::RecursiveOffsetMap& offset_map,
                      typename PermutationTree<N, N - row>::RecursiveMask& mask_even,
                      typename PermutationTree<N, N - row>::RecursiveMask& mask_odd,
                      int current_streak,
                      uint64_t stabilizer,
                      int row_value,
                      int max_value);
  template<int row>
  void CountFollowingRows(EngineFlatter& that,
                          const typename PermutationTree<N, N - row>::RecursiveOffsetMap& offset_map,
                          typename PermutationTree<N, N - row>::RecursiveMask& mask_even,
                          typename PermutationTree<N, N - row>::RecursiveMask& mask_odd,
                          int current_streak,
                          uint64_t stabilizer,
                          int row_value,
                          int max_value);
  
  template<int row>
  void CommitRowValue(EngineFlatter& that,
                      const typename PermutationTree<N, N - row>::RecursiveOffsetMap& offset_map,
                      typename PermutationTree<N, N - row>::RecursiveMask& mask_even,
                      typename PermutationTree<N, N - row>::RecursiveMask& mask_odd,
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
                          const typename PermutationTree<N, N - row>::RecursiveOffsetMap& offset_map,
                          typename PermutationTree<N, N - row>::RecursiveMask& mask_even,
                          typename PermutationTree<N, N - row>::RecursiveMask& mask_odd,
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
                                 const typename PermutationTree<N, 2>::RecursiveOffsetMap& offset_map,
                                 typename PermutationTree<N, 2>::RecursiveMask& mask_even,
                                 typename PermutationTree<N, 2>::RecursiveMask& mask_odd,
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
