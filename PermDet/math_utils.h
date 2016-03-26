//
//  math_utils.h
//  PermDet
//
//  Created by culter on 3/25/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef math_utils_h
#define math_utils_h

#include <algorithm>
#include <array>

// The number of rows and columns of the matrices to count.
constexpr int N = 6;

// A compact representation of a binary matrix.
typedef std::bitset<N*N> Matrix;

// Standard factorial function.
constexpr uint64_t factorial(uint64_t n) {
  return (n == 0) ? 1 : (n * factorial(n - 1));
}

// Returns 1 if perm is an odd permutation; 0 if it is even.
int get_parity(const std::array<int, N>& perm) {
  int parity = 0;
  for (int i = 0; i < N; ++i) {
    for (int j = i + 1; j < N; ++j) {
      if (perm[i] > perm[j]) {
        parity = !parity;
      }
    }
  }
  return parity;
}

// Constructs an array [0, 1, 2, ..., N-1], which serves as the identity permutation.
std::array<int, N> identity() {
  std::array<int, N> perm;
  for (int i = 0; i < N; ++i) {
    perm[i] = i;
  }
  return perm;
}

// Constructs an array of all N!/2 permutations having the specified parity.
std::array<std::array<int, N>, factorial(N) / 2> permutations(int parity) {
  std::array<std::array<int, N>, factorial(N) / 2> answer;
  std::array<int, N> perm = identity();
  int count = 0;
  do {
    if (get_parity(perm) == parity) {
      answer[count] = perm;
      count += 1;
    }
  } while (std::next_permutation(perm.begin(), perm.end()));
  return answer;
}

// Returns the Nx(N-1) matrix representation for a permutation,
// represented using NxN space with a blank last column.
Matrix matrix_from(std::array<int, N> permutation) {
  Matrix bits = 0;
  
  // Fill out N - 1 columns, not N, to leave the last column blank.
  for (int i = 0; i < N - 1; ++i) {
    bits.set(i + N * permutation[i]);
  }
  return bits;
}

#endif /* math_utils_h */
