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

// patterns_by_column[0/1][i] is a list of even/odd permutations that intersect row i on the last column
std::array<std::array<std::array<Matrix, factorial(N) / (2 * N)>, N>, 2> patterns;

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

uint64_t num_1 = 0;
uint64_t num_2 = 0;
uint64_t num_4 = 0;
uint64_t num_8 = 0;

void test(Matrix m) {
  std::cout << "Permutation n1: ";
  for (Matrix t: row_swaps(m)) {
    std::cout << num(t, 1);
  }
  std::cout << std::endl;
}

void count_from(const Matrix& m, bool has_repeat, int last_row_value, int next_row) {
  if (next_row >= N) {
    uint64_t n[2] = {num(m, 0), num(m, 1)};
    uint64_t d;
    test(m);
    if (has_repeat) {
      d = degeneracy(m);
      sum += n[0] * d;
      switch (n[0]) {
        case 1: num_1 += d; break;
        case 2: num_2 += d; break;
        case 4: num_4 += d; break;
        case 8: num_8 += d; break;
      }
    }
    else {
      d = factorial(N);
      sum += n[0] * d/2;
      switch (n[0]) {
        case 1: num_1 += d/2; break;
        case 2: num_2 += d/2; break;
        case 4: num_4 += d/2; break;
        case 8: num_8 += d/2; break;
      }
      sum += n[1] * d/2;
      switch (n[1]) {
        case 1: num_1 += d/2; break;
        case 2: num_2 += d/2; break;
        case 4: num_4 += d/2; break;
        case 8: num_8 += d/2; break;
      }
    }
    std::cout << "m=" << m << " num=" << n[0] << "," << n[1] << " deg=" << d << std::endl;
    if (has_repeat && n[0] != n[1]) {
      std::cout << "Wha??";
    }
  }
  else {
//    for (int value = 0; value < (uint64_t(1) << (N - 1)); ++value) {
//      Matrix m_next = m | (Matrix(value) << (next_row * N));
//      count_from(m_next, value, next_row + 1);
//    }
    
    Matrix m_next = m | (Matrix(last_row_value) << (next_row * N));
    count_from(m_next, (next_row > 0), last_row_value, next_row + 1);
    for (int value = last_row_value + 1; value < (uint64_t(1) << (N - 1)); ++value) {
      m_next = m | (Matrix(value) << (next_row * N));
      count_from(m_next, has_repeat, value, next_row + 1);
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
  
  count_from(Matrix(0), false, 0, 0);
  std::cout << "num_1: " << num_1 << std::endl;
  std::cout << "num_2: " << num_2 << std::endl;
  std::cout << "num_4: " << num_4 << std::endl;
  std::cout << "num_8: " << num_8 << std::endl;
  std::cout << "sum: " << sum << std::endl;
}
