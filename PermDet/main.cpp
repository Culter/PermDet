//
//  main.cpp
//  PermDet
//
//  Created by culter on 3/24/16.
//  Copyright © 2016 culter. All rights reserved.
//

#include <array>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "math_utils.h"

uint64_t sum = 0;

// Returns the number of NxN matrices which can be transformed into m using row swaps.
uint64_t degeneracy(Matrix m) {
  constexpr Matrix row_mask = ((uint64_t)1 << N) - 1;
  
  uint64_t answer = factorial(N);
  
  Matrix row = m & row_mask;
  int streak = 1;
  for (int i = 1; i < N; ++i) {
    m >>= N;
    Matrix next_row = m & row_mask;
    if (next_row == row) {
      streak += 1;
    }
    else {
      answer /= factorial(streak);
      streak = 1;
      row = next_row;
    }
  }
  answer /= factorial(streak);
  return answer;
}

uint64_t num(const std::vector<Matrix>& mats) {
  Matrix mask = {};
  for (int i = 0; i < N; ++i) {
    mask.set(i * N + N - 1);
  }
  Matrix big_or = {};
  for (const Matrix& m: mats) {
    big_or |= m;
  }
  uint64_t fixed = (big_or & mask).count();
  uint64_t remaining = N - fixed;
  
  std::cout << "    num: mask = " << mask << ", big_or = " << big_or << ", remaining = " << remaining << std::endl;
  
  return uint64_t(1) << remaining;
}

// Recursive function to count the eligible matrices with
// partially specified rows and a blank last column.
void count_from(const Matrix& m_base,
                bool has_repeat,
                int row_value,
                int row,
                const std::vector<Matrix>& surviving_even,
                const std::vector<Matrix>& surviving_odd,
                uint64_t& local_sum) {
  std::cout << "count_from(" << m_base << ", " << has_repeat << ", " << row_value << ", " << row
  << ", Matrix[" << surviving_even.size() << "], Matrix[" << surviving_odd.size() << "])" << std::endl;
  
  Matrix row_matrix = Matrix(row_value) << (row * N);
  std::cout << "  row_matrix = " << row_matrix << std::endl;
  
  std::vector<Matrix> next_even;
  std::vector<Matrix> next_odd;
//  if (!has_repeat) {
    for (const Matrix& p: surviving_even) {
      if ((p & row_matrix).any()) {
        next_even.push_back(p);
      }
    }
//  }
//  else {
//    next_even = surviving_even;
//  }
  
  for (const Matrix& p: surviving_odd) {
    if ((p & row_matrix).any()) {
      next_odd.push_back(p);
    }
  }
  
  std::cout << "  next = Matrix[" << next_even.size() << "], Matrix[" << next_odd.size() << "]" << std::endl;
  
  Matrix m = m_base | row_matrix;
  std::cout << "  m = " << m << std::endl;
  
  if (row == N - 1) {
    if (has_repeat) {
      local_sum += num(next_odd) * degeneracy(m);
    } else {
      local_sum += (num(next_even) + num(next_odd)) * (factorial(N) / 2);
    }
  } else {
    count_from(m, true, row_value, row + 1, next_even, next_odd, local_sum);
    for (int next = row_value + 1; next < (uint64_t(1) << N); ++next) {
      count_from(m, has_repeat, next, row + 1, next_even, next_odd, local_sum);
    }
  }
}

int main() {
  auto array_even = permutation_matrices(0);
  auto array_odd = permutation_matrices(1);
  std::vector<Matrix> vector_even(array_even.cbegin(), array_even.cend());
  std::vector<Matrix> vector_odd(array_odd.cbegin(), array_odd.cend());
  
  constexpr uint64_t num_threads = (uint64_t)1 << (N - 1);
  std::array<uint64_t, num_threads> subtotals = {};
  
  // Serial execution, by value of first row (without entry for last column)
  for (int i = 0; i < num_threads; ++i) {
    count_from(Matrix(0), false, i + num_threads, 0, vector_even, vector_odd, subtotals[i]);
  }
  
  // Parallel execution, by value of first row (without entry for last column)
//  std::vector<std::thread> threads;
//  std::mutex mutex;
//  for (int i = 0; i < num_threads; ++i) {
//    threads.emplace_back([&, i]{
//      count_from(Matrix(0), false, i + num_threads, 0, vector_even, vector_odd, subtotals[i]);
//      std::lock_guard<std::mutex> lock(mutex);
//      std::cout << "Thread " << i << ": subtotal " << subtotals[i] << std::endl;
//    });
//  }
//  for (auto& thread: threads) {
//    thread.join();
//  }
  
  for (int i = 0; i < num_threads; ++i) {
    std::cout << "Thread " << i << ": subtotal " << subtotals[i] << std::endl;
    sum += subtotals[i];
  }
  std::cout << "sum: " << sum << std::endl;
}
