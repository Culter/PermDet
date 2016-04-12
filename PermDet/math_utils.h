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
#include <vector>

// The number of rows and columns of the matrices to count.
constexpr int N = 3;

// A compact representation of a binary matrix.
typedef std::bitset<N*N> Matrix;

// An ordinary representation of a permutation: sigma[i] is sigma(i).
// The uint32_t is inefficient on space, but efficient on time, as it allows simpler CPU instructions.
typedef std::array<uint32_t, N> Permutation;

// A permutation with an extra bit of data representing sigma^{-1}(N-1).
typedef std::array<uint32_t, N + 1> AugmentedPermutation;

// Standard power function.
uint64_t pow(uint64_t base, int exponent) {
  uint64_t answer = 1;
  for (int i = 0; i < exponent; ++i) {
    answer *= base;
  }
  return answer;
}

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

// Returns 1 if perm is an odd permutation; 0 if it is even.
int get_parity(const Permutation& perm) {
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
Permutation identity() {
  Permutation perm;
  for (int i = 0; i < N; ++i) {
    perm[i] = i;
  }
  return perm;
}

// Returns the NxN matrix representation for a permutation,
Matrix matrix_from(Permutation permutation) {
  Matrix bits = 0;
  for (int i = 0; i < N ; ++i) {
    bits.set(i + N * permutation[i]);
  }
  return bits;
}

// Constructs an array of all N!/2 permutations having the specified parity.
std::array<Permutation, factorial(N) / 2> permutations(int parity) {
  static_assert(N >= 2, "N must be at least 2 to provide exactly N!/2 patterns per parity.");
  
  std::array<Permutation, factorial(N) / 2> answer;
  Permutation perm = identity();
  int count = 0;
  do {
    if (get_parity(perm) == parity) {
      answer[count] = perm;
      count += 1;
    }
  } while (std::next_permutation(perm.begin(), perm.end()));
  return answer;
}

// Constructs a vector of all permutations having the specified parity.
std::vector<Permutation> vector_permutations(int parity) {
  std::vector<Permutation> answer;
  Permutation perm = identity();
  do {
    if (get_parity(perm) == parity) answer.push_back(perm);
  } while (std::next_permutation(perm.begin(), perm.end()));
  return answer;
}

AugmentedPermutation augment(Permutation p) {
  AugmentedPermutation answer = {};
  std::copy(p.cbegin(), p.cend(), answer.begin());
  for (int i = 0; i < N; ++i) {
    if (p[i] == N - 1) {
      answer[N] = i;
      break;
    }
  }
  return answer;
}

// Constructs a vector of all permutations having the specified parity.
std::vector<AugmentedPermutation> vector_augmented_permutations(int parity) {
  std::vector<AugmentedPermutation> answer;
  Permutation perm = identity();
  do {
    if (get_parity(perm) == parity) answer.push_back(augment(perm));
  } while (std::next_permutation(perm.begin(), perm.end()));
  return answer;
}

// Constructs an array of all N!/2 permutations having the specified parity.
std::array<Matrix, factorial(N) / 2> permutation_matrices(int parity) {
  static_assert(N >= 2, "N must be at least 2 to provide exactly N!/2 patterns per parity.");
  
  std::array<Matrix, factorial(N) / 2> answer;
  Permutation perm = identity();
  int count = 0;
  do {
    if (get_parity(perm) == parity) {
      answer[count] = matrix_from(perm);
      count += 1;
    }
  } while (std::next_permutation(perm.begin(), perm.end()));
  return answer;
}

#endif /* math_utils_h */
