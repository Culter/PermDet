//
//  PartialPermutation.h
//  PermDet
//
//  Created by culter on 4/11/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef PartialPermutation_h
#define PartialPermutation_h

// A class representing the abstract relationship between a partial permutation on (width) letters,
// occupying (height) rows in matrix form. If height = 1, this degenerates to a single number on [0, width).
// If height = width, this represents a full permutation.
template<unsigned width, unsigned height>
struct PartialPermutation
{
  static_assert(height > 0, "height should be > 0");
  static_assert(height <= width, "height should be <= width");
  static_assert(width < 16, "width should be < 16");
  
  static constexpr uint64_t next_size = PartialPermutation<width, height - 1>::size;
  static constexpr uint64_t size = (width + 1 - height) * next_size;
  static_assert(size % width == 0, "size should be divisible by width");
  static constexpr uint64_t bin_size = size / width;
  
  PartialPermutation(): rows{} {}
  PartialPermutation(std::array<int, height> in_rows): rows(in_rows) {}
  // Constuct the index-th permutation in lexicographic order.
  PartialPermutation(uint64_t index);
  // Get the index of this permutation in lexicographic order.
  uint64_t Index() const;
  // Form the partial permutation copied from all but the first row.
  PartialPermutation<width, height - 1> pop_front() const;
  
  std::array<int, height> rows;
};

template<unsigned width>
struct PartialPermutation<width, /*height = */ 0>
{
  // There is exactly width^0 = 1 map from the empty set of rows to [0, width).
  static constexpr uint64_t size = 1;
  
  PartialPermutation() {}
  PartialPermutation(std::array<int, 0> in_rows) {}
  PartialPermutation(uint64_t index) {}
  constexpr uint64_t Index() const { return 0; }
  std::array<int, 0> rows;
};

template<unsigned width, unsigned height>
PartialPermutation<width, height>::PartialPermutation(uint64_t index) {
  int unused_indices[width];
  for (int i = 0; i < width; ++i) {
    unused_indices[i] = i;
  }
  
  unsigned w = width;
  unsigned b = bin_size;
  for (int row = 0; row < height; ++row) {
    uint64_t top_index_position = index / b;
    rows[row] = unused_indices[top_index_position];
    if (row == height - 1) break;
    
    // Now that unused_indices[top_index_position] has been used, prevent it from being used in the next row.
    std::copy(unused_indices + top_index_position + 1,
              unused_indices + w,
              unused_indices + top_index_position);
    
    index = index % b;
    w -= 1;
    b /= w;
  }
}

template<unsigned width, unsigned height>
uint64_t PartialPermutation<width, height>::Index() const {
  std::array<int, width> value_of_index;
  for (int i = 0; i < width; ++i) {
    value_of_index[i] = i;
  }
  
  uint64_t answer = 0;
  unsigned w = width;
  for (int row = 0; row < height; ++row) {
    int top_index = rows[row];
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

template<unsigned width, unsigned height>
PartialPermutation<width, height - 1> PartialPermutation<width, height>::pop_front() const
{
  PartialPermutation<width, height - 1> answer;
  std::copy(rows.begin() + 1, rows.end(), answer.rows.begin());
  return answer;
}

#endif /* PartialPermutation_h */
