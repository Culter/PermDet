//
//  main.cpp
//  PermDet
//
//  Created by culter on 3/24/16.
//  Copyright © 2016 culter. All rights reserved.
//

#include <algorithm>
#include <array>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

// The number of rows and columns of the matrices to count.
constexpr int N = 6;

static_assert(N > 2, "N must be at least 3 to provide exactly N!/2N patterns per row.");

#include "math_utils.h"

// patterns[0/1][i] is a list of even/odd permutations that intersect row i on the last column
std::array<std::array<std::array<Matrix, factorial(N) / (2 * N)>, N>, 2> patterns = {};

uint64_t sum = 0;

// Returns true if no pattern results from completing m with a 1 in row in the last column
bool is_free(const Matrix& m, int parity, int row) {
  for (const auto& pattern: patterns[parity][row]) {
    if ((m & pattern) == pattern) return false;
  }
  return true;
}

// Returns the number of target matrices that can be formed by completing the last row of m
uint64_t num(const Matrix& m, int parity) {
  int free = 0;
  for (int i = 0; i < N; ++i) {
    if (is_free(m, parity, i)) {
      free += 1;
    }
  }
  return uint64_t(1) << free;
}

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

void count_from(const Matrix& m, bool has_repeat, int last_row_value, int next_row, uint64_t& local_sum) {
  if (next_row >= N) {
    if (has_repeat) {
      local_sum += num(m, 0) * degeneracy(m);
    } else {
      local_sum += (num(m, 0) + num(m, 1)) * (factorial(N) / 2);
    }
  }
  else {
    Matrix m_next = m | (Matrix(last_row_value) << (next_row * N));
    count_from(m_next, (next_row > 0), last_row_value, next_row + 1, local_sum);
    for (int value = last_row_value + 1; value < (uint64_t(1) << (N - 1)); ++value) {
      m_next = m | (Matrix(value) << (next_row * N));
      count_from(m_next, has_repeat, value, next_row + 1, local_sum);
    }
  }
}

int main() {
  // Record both even and odd permutations
  for (int parity = 0; parity < 2; ++parity) {
    std::array<int, N> known_perms = {};
    for (const auto& perm: permutations(parity)) {
      int row = perm[N - 1];
      patterns[parity][row][known_perms[row]] = matrix_from(perm);
      known_perms[row] += 1;
    }
  }
  
  constexpr uint64_t num_threads = (uint64_t)1 << (N - 1);
  std::array<uint64_t, num_threads> subtotals = {};
  
  // Serial execution, by value of first row (without entry for last column)
//  for (int i = 0; i < num_threads; ++i) {
//    count_from(Matrix(i), false, i, 1, subtotals[i]);
//  }
  
  // Parallel execution, by value of first row (without entry for last column)
  std::vector<std::thread> threads;
  std::mutex mutex;
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&, i]{
      count_from(Matrix(i), false, i, 1, subtotals[i]);
      std::lock_guard<std::mutex> lock(mutex);
      std::cout << "Thread " << i << ": subtotal " << subtotals[i] << std::endl;
    });
  }
  for (auto& thread: threads) {
    thread.join();
  }
  
  for (int i = 0; i < num_threads; ++i) {
    std::cout << "Thread " << i << ": subtotal " << subtotals[i] << std::endl;
    sum += subtotals[i];
  }
  std::cout << "sum: " << sum << std::endl;
}
