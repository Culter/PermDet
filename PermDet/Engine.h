//
//  Engine.h
//  PermDet
//
//  Created by culter on 3/28/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef Engine_h
#define Engine_h

class Engine {
public:
  Engine(const std::vector<Matrix>& evens,
         const std::vector<Matrix>& odds):
  local_sum(0),
  mask(0)
  {
    for (int i = 0; i < N; ++i) {
      mask.set(i * N + N - 1);
    }
    
    scratch_even[0] = evens;
    scratch_odd[0] = odds;
    for (int i = 1; i <= N; ++i) {
      scratch_even[i].reserve(evens.size());
      scratch_odd[i].reserve(odds.size());
    }
  }
  
  uint64_t Count(int first_row)
  {
    count_from(false, 1, 1, first_row + ((uint64_t)1 << (N - 1)), 0);
    return local_sum;
  }
  
private:
  uint64_t num(const std::vector<Matrix>& mats) {
    Matrix big_or = {};
    for (const Matrix& m: mats) {
      big_or |= m;
    }
    uint64_t fixed = (big_or & mask).count();
    uint64_t remaining = N - fixed;
    
    return uint64_t(1) << remaining;
  }
  
  // Recursive function to count the eligible matrices with
  // partially specified rows and a blank last column.
  void count_from(bool has_repeat,
                  int current_streak,
                  uint64_t fact,
                  int row_value,
                  int row) {
    Matrix row_matrix = Matrix(row_value) << (row * N);
    
    const std::vector<Matrix>& surviving_even = scratch_even[row];
    const std::vector<Matrix>& surviving_odd = scratch_odd[row];
    std::vector<Matrix>& next_even = scratch_even[row + 1];
    std::vector<Matrix>& next_odd = scratch_odd[row + 1];
    next_even.clear();
    next_odd.clear();
    
    if (has_repeat) {
      fact *= current_streak;
    } else {
      for (const Matrix& p: surviving_even) {
        if ((p & row_matrix).any()) {
          next_even.push_back(p);
        }
      }
    }
    
    for (const Matrix& p: surviving_odd) {
      if ((p & row_matrix).any()) {
        next_odd.push_back(p);
      }
    }
    
    if (next_even.empty() && next_odd.empty()) {
      while (row < N) {
        // Add up all matrices with values from this point on.
        // There are N-1-row rows left to fill.
        // Each one can take any value from row_value+1 to (uint64_t(1) << N) - 1 inclusive, which is
        // (uint64_t(1) << N) - 1 - row_value possibilities.
        // Then there are N bits left to assign.
        uint64_t base_value = pow((uint64_t(1) << N) - 1 - row_value, N - 1 - row) << N;
        local_sum += base_value * (factorial(N) / (fact * factorial(N - 1 - row)));
        
        // Now consider what happens if the next row is the same as this one...
        current_streak += 1;
        fact *= current_streak;
        row += 1;
      }
      return;
    }
    
    if (row == N - 1) {
      if (has_repeat) {
        local_sum += num(next_odd) * (factorial(N) / fact);
      } else {
        local_sum += (num(next_even) + num(next_odd)) * (factorial(N) / 2);
      }
    } else {
      count_from(true, current_streak + 1, fact, row_value, row + 1);
      for (int next = row_value + 1; next < (uint64_t(1) << N); ++next) {
        count_from(has_repeat, 1, fact, next, row + 1);
      }
    }
  }
  
  std::vector<Matrix> scratch_even[N + 1];
  std::vector<Matrix> scratch_odd[N + 1];
  Matrix mask = {};
  uint64_t local_sum;
};

#endif /* Engine_h */
