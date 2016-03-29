//
//  EngineNextFaster.h
//  PermDet
//
//  Created by culter on 3/29/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef EngineNextFaster_h
#define EngineNextFaster_h

class EngineNextFaster {
public:
  EngineNextFaster(const std::vector<Matrix>& evens,
                   const std::vector<Matrix>& odds):
  local_sum(0)
  {
    scratch_even[0] = evens;
    scratch_odd[0] = odds;
    for (int i = 1; i <= N; ++i) {
      scratch_even[i].reserve(evens.size());
      scratch_odd[i].reserve(odds.size());
    }
  }
  
  uint64_t Count(int first_row);
  
public:
  std::vector<Matrix> scratch_even[N - 1];
  std::vector<Matrix> scratch_odd[N - 1];
  uint64_t local_sum;
};

void count_all_from(EngineNextFaster& that,
                    int current_streak,
                    uint64_t fact,
                    int row_value,
                    int row) {
  while (row < N) {
    // Add up all matrices with values from this point on.
    // There are N-1-row rows left to fill.
    // Each one can take any value from row_value+1 to (uint64_t(1) << N) - 1 inclusive, which is
    // (uint64_t(1) << N) - 1 - row_value possibilities.
    // Then there are N bits left to assign.
    uint64_t base_value = pow((uint64_t(1) << N) - 1 - row_value, N - 1 - row) << N;
    that.local_sum += base_value * (factorial(N) / (fact * factorial(N - 1 - row)));
    
    // Now consider what happens if the next row is the same as this one...
    current_streak += 1;
    fact *= current_streak;
    row += 1;
  }
}

// Recursive function to count the eligible matrices with
// partially specified rows and a blank last column.
template<int row>
void count_from(EngineNextFaster& that,
                bool has_repeat,
                int current_streak,
                uint64_t fact,
                int row_value) {
  static_assert(row < N - 2, "This function is meant to be called only for rows before the penultimate row.");
  
  Matrix row_matrix = Matrix(row_value) << (row * N);
  
  const std::vector<Matrix>& surviving_even = that.scratch_even[row];
  const std::vector<Matrix>& surviving_odd = that.scratch_odd[row];
  std::vector<Matrix>& next_even = that.scratch_even[row + 1];
  std::vector<Matrix>& next_odd = that.scratch_odd[row + 1];
  next_even.clear();
  next_odd.clear();
  
  if (has_repeat) {
    fact *= current_streak;
  } else {
    for (const Matrix& p: surviving_even) {
      if ((p & row_matrix).any()) next_even.push_back(p);
    }
  }
  
  for (const Matrix& p: surviving_odd) {
    if ((p & row_matrix).any()) next_odd.push_back(p);
  }
  
  // Look for a quick shortcut...
  //    if (next_even.empty() && next_odd.empty()) {
  //      count_all_from(current_streak, fact, row_value, row);
  //      return;
  //    }
  
  count_from<row + 1>(that, true, current_streak + 1, fact, row_value);
  for (int next = row_value + 1; next < (uint64_t(1) << N); ++next) {
    count_from<row + 1>(that, has_repeat, 1, fact, next);
  }
}

template<>
void count_from<N - 2>(EngineNextFaster& that,
                       bool has_repeat,
                       int current_streak,
                       uint64_t fact,
                       int row_value) {
  constexpr int row = N - 2;
  constexpr int last_row = N - 1;
  Matrix row_matrix = Matrix(row_value) << (row * N);
  
  const std::vector<Matrix>& surviving_even = that.scratch_even[row];
  const std::vector<Matrix>& surviving_odd = that.scratch_odd[row];
  
  Matrix even_matrix_per_last_row_bit[N] = {};
  Matrix odd_matrix_per_last_row_bit[N] = {};
  std::bitset<N> even_column_per_bit[N] = {};
  std::bitset<N> odd_column_per_bit[N] = {};
  
  if (has_repeat) {
    fact *= current_streak;
  } else {
    for (const Matrix& p: surviving_even) {
      if ((p & row_matrix).any()) {
        for (int c = 0; c < N; ++c) {
          if (p[last_row * N + c]) {
            even_matrix_per_last_row_bit[c] |= p;
          }
        }
      }
    }
    for (int c = 0; c < N; ++c) {
      for (int b = 0; b < N; ++b) {
        even_column_per_bit[c][b] = even_matrix_per_last_row_bit[c][b * N + N - 1];
      }
    }
  }
  for (const Matrix& p: surviving_odd) {
    if ((p & row_matrix).any()) {
      for (int c = 0; c < N; ++c) {
        if (p[last_row * N + c]) {
          odd_matrix_per_last_row_bit[c] |= p;
        }
      }
    }
  }
  for (int c = 0; c < N; ++c) {
    for (int b = 0; b < N; ++b) {
      odd_column_per_bit[c][b] = odd_matrix_per_last_row_bit[c][b * N + N - 1];
    }
  }
  
  // Single repeat case
  {
    std::bitset<N> last_row_bits(row_value);
    std::bitset<N> odd_taken_column(0);
    for (int c = 0; c < N; ++c) {
      if (last_row_bits[c]) {
        odd_taken_column |= odd_column_per_bit[c];
      }
    }
    uint64_t odd_num = uint64_t(1) << (N - odd_taken_column.count());
    that.local_sum += odd_num * (factorial(N) / (fact * (current_streak + 1)));
  }
  // Continuation cases
  for (int last_row_value = row_value + 1; last_row_value < (uint64_t(1) << N); ++last_row_value) {
    std::bitset<N> last_row_bits(last_row_value);
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
    if (has_repeat) {
      that.local_sum += odd_num * (factorial(N) / fact);
    } else {
      that.local_sum += (even_num + odd_num) * (factorial(N) / 2);
    }
  }
}

uint64_t EngineNextFaster::Count(int first_row)
{
  count_from<0>(*this, false, 1, 1, first_row + ((uint64_t)1 << (N - 1)));
  return local_sum;
}

#endif /* EngineNextFaster_h */
