//
//  EngineNextEvenFaster.h
//  PermDet
//
//  Created by culter on 3/29/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef EngineNextEvenFaster_h
#define EngineNextEvenFaster_h

#include <algorithm>
#include <unordered_set>

namespace std
{
  template<typename T, size_t N>
  struct hash<array<T, N> >
  {
    typedef array<T, N> argument_type;
    typedef size_t result_type;
    
    result_type operator()(const argument_type& a) const
    {
      result_type h = 0;
      for (result_type i = 0; i < N; ++i)
      {
        // Because we know that there aren't many bits in a
        h = h << 3 + a[i];
      }
      return h;
    }
  };
}

static constexpr int num_row_values = (uint64_t)1 << (N - 1);

class EngineNextEvenFaster {
public:
  EngineNextEvenFaster(const std::vector<AugmentedPermutation>& evens,
                       const std::vector<AugmentedPermutation>& odds):
  scratch_weights(),
  local_sum(0)
  {
    scratch_even[0] = std::unordered_set<AugmentedPermutation>(evens.cbegin(), evens.cend());
    scratch_odd[0] = std::unordered_set<AugmentedPermutation>(odds.cbegin(), odds.cend());
    for (int i = 1; i <= N; ++i) {
      scratch_even[i].reserve(evens.size());
      scratch_odd[i].reserve(odds.size());
    }
    
    for (int i = 0; i < num_row_values; ++i) {
      row_values[i] = i + num_row_values;
    }
    std::stable_sort(row_values.begin(), row_values.end(), [] (std::bitset<N> a, std::bitset<N> b) {
      return a.count() < b.count();
    });
  }
  
  uint64_t Count(int first_row);
  
public:
  std::unordered_set<AugmentedPermutation> scratch_even[N - 1];
  std::unordered_set<AugmentedPermutation> scratch_odd[N - 1];
  std::array<std::array<int, N - 1>, N> scratch_weights;
  std::array<std::bitset<N>, num_row_values> row_values;
  uint64_t local_sum;
};

// Recursive function to count the eligible matrices with
// partially specified rows and a blank last column.
template<int row>
void count_from(EngineNextEvenFaster& that,
                bool has_repeat,
                int current_streak,
                uint64_t fact,
                int row_value) {
  static_assert(row < N - 2, "This function is meant to be called only for rows before the penultimate row.");
  
  auto row_matrix = that.row_values[row_value];
  
  that.scratch_weights[row + 1] = that.scratch_weights[row];
  for (int i = 0; i < N - 1; ++i) {
    if (row_matrix[i]) {
      that.scratch_weights[row + 1][i] += 1;
    }
  }
  
  const auto& surviving_even = that.scratch_even[row];
  const auto& surviving_odd = that.scratch_odd[row];
  auto& next_even = that.scratch_even[row + 1];
  auto& next_odd = that.scratch_odd[row + 1];
  next_even.clear();
  next_odd.clear();
  
  if (has_repeat) {
    fact *= current_streak;
  } else {
    for (const auto& p: surviving_even) {
      if (row_matrix[p[row]]) { auto q = p; q[row] = 0; next_even.insert(q); }
    }
  }
  
  for (const auto& p: surviving_odd) {
    if (row_matrix[p[row]]) { auto q = p; q[row] = 0; next_odd.insert(q); }
  }
  
  count_from<row + 1>(that, true, current_streak + 1, fact, row_value);
  for (int next = row_value + 1; next < num_row_values; ++next) {
    count_from<row + 1>(that, has_repeat, 1, fact, next);
  }
}

template<>
void count_from<N - 2>(EngineNextEvenFaster& that,
                       bool has_repeat,
                       int current_streak,
                       uint64_t fact,
                       int row_value) {
  constexpr int row = N - 2;
  constexpr int last_row = N - 1;
  auto row_matrix = that.row_values[row_value];
  
  that.scratch_weights[row + 1] = that.scratch_weights[row];
  for (int i = 0; i < N - 1; ++i) {
    if (row_matrix[i]) {
      that.scratch_weights[row + 1][i] += 1;
    }
  }
  
  uint64_t column_degeneracy = 1;
  int column_streak = 1;
  int column_max = that.scratch_weights[row + 1][0];
  for (int i = 1; i < N - 1; ++i) {
    auto c = that.scratch_weights[row + 1][i];
    if (c < column_max) {
      // Columns are out of order. Discount this arrangement.
      return;
    } else if (c == column_max) {
      column_streak += 1;
      column_degeneracy *= column_streak;
    } else {
      column_streak = 1;
      column_max = c;
    }
  }
  uint64_t column_multiplier = factorial(N - 1) / column_degeneracy;
  
  const auto& surviving_even = that.scratch_even[row];
  const auto& surviving_odd = that.scratch_odd[row];
  
  std::bitset<N> even_column_per_bit[N] = {};
  std::bitset<N> odd_column_per_bit[N] = {};
  
  if (has_repeat) {
    fact *= current_streak;
  } else {
    for (const auto& p: surviving_even) {
      if (row_matrix[p[row]]) {
        even_column_per_bit[p[last_row]][p[N]] = true;
      }
    }
  }
  for (const auto& p: surviving_odd) {
    if (row_matrix[p[row]]) {
      odd_column_per_bit[p[last_row]][p[N]] = true;
    }
  }
  
  // Continuation cases
  uint64_t degeneracy = factorial(N - 1) / fact;
  for (int last_row_value = 0; last_row_value < num_row_values; ++last_row_value) {
    std::bitset<N> last_row_bits(that.row_values[last_row_value]);
    if (has_repeat) {
      std::bitset<N> odd_taken_column(0);
      for (int c = 0; c < N; ++c) {
        if (last_row_bits[c]) {
          odd_taken_column |= odd_column_per_bit[c];
        }
      }
      uint64_t odd_num = uint64_t(1) << (N - odd_taken_column.count());
      that.local_sum += odd_num * degeneracy * column_multiplier;
    } else {
      std::bitset<N> even_taken_column(0);
      std::bitset<N> odd_taken_column(0);
      for (int c = 0; c < N; ++c) {
        if (last_row_bits[c]) {
          even_taken_column |= even_column_per_bit[c];
          odd_taken_column |= odd_column_per_bit[c];
        }
      }
      uint64_t even_num = uint64_t(1) << (N - even_taken_column.count());
      uint64_t odd_num = uint64_t(1) << (N - odd_taken_column.count());
      that.local_sum += (even_num + odd_num) * (factorial(N - 1) / 2) * column_multiplier;
    }
  }
}

uint64_t EngineNextEvenFaster::Count(int first_row)
{
  scratch_weights[0].fill(0);
  count_from<0>(*this, false, 1, 1, first_row);
  return local_sum;
}

#endif /* EngineNextEvenFaster_h */
