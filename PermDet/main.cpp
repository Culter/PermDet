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

// The number of rows and columns of the matrices to count.
constexpr int N = 3;

static_assert(N > 2, "N must be at least 3 to provide exactly N!/2N patterns per row.");

#include "math_utils.h"

// patterns_by_column[i] is a list of odd permutations that intersect row i on the last column
std::array<std::array<Matrix, factorial(N) / (2 * N)>, N> patterns_by_row;

uint64_t sum = 0;

// Returns true if no pattern results from completing m with a 1 in row in the last column
bool is_free(const Matrix& m, int row) {
  for (const auto& pattern: patterns_by_row[row]) {
    if ((m & pattern) == pattern) return false;
  }
  return true;
}

// Returns the number of target matrices that can be formed by completing the last row of m
uint64_t num(const Matrix& m) {
  int free = 0;
  for (int i = 0; i < N; ++i) {
    if (is_free(m, i)) {
      free += 1;
    }
  }
  return uint64_t(1) << free;
}

uint64_t degeneracy(Matrix m) {
  constexpr Matrix row_mask = ((uint64_t)1 << N) - 1;
  
  uint64_t answer = factorial(N);
  
  Matrix row = m & row_mask;
//  std::cout << row;
  int streak = 1;
  for (int i = 1; i < N; ++i) {
    m >>= N;
    Matrix next_row = m & row_mask;
//    std::cout << "," << next_row;
    if (next_row == row) {
      streak += 1;
    }
    else {
      answer /= factorial(streak);
      streak = 1;
      row = next_row;
    }
//    std::cout << ",s" << streak;
  }
  answer /= factorial(streak);
//  std::cout << " " << answer << std::endl;
  return answer;
}

uint64_t num_representatives = 0;
uint64_t num_represented = 0;
uint64_t num_1 = 0;
uint64_t num_2 = 0;
uint64_t num_4 = 0;
uint64_t num_8 = 0;

void count_from(const Matrix& m, int last_row_value, int next_row) {
  if (next_row >= N) {
    auto n = num(m);
    auto d = degeneracy(m);
    d = 1;
    sum += n * d;
    num_representatives += 1;
    num_represented += d;
    switch (n) {
      case 1: num_1 += d; break;
      case 2: num_2 += d; break;
      case 4: num_4 += d; break;
      case 8: num_8 += d; break;
    }
    std::cout << "m=" << m << " num=" << n << " deg=" << d << std::endl;
  }
  else {
    for (int value = 0; value < (uint64_t(1) << (N - 1)); ++value) {
      Matrix m_next = m | (Matrix(value) << (next_row * N));
      count_from(m_next, value, next_row + 1);
    }
  }
}

int main() {
  std::array<int, N> known_perms = {};
  for (const auto& perm: odd_permutations()) {
    int row = perm[N - 1];
    patterns_by_row[row][known_perms[row]] = matrix_from(perm);
    known_perms[row] += 1;
  }
  
  count_from(Matrix(0), 0, 0);
  std::cout << "num_representatives: " << num_representatives << std::endl;
  std::cout << "num_represented: " << num_represented << std::endl;
  std::cout << "num_1: " << num_1 << std::endl;
  std::cout << "num_2: " << num_2 << std::endl;
  std::cout << "num_4: " << num_4 << std::endl;
  std::cout << "num_8: " << num_8 << std::endl;
  std::cout << "sum: " << sum << std::endl;
}
