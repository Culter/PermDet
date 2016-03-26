//
//  math_utils.h
//  PermDet
//
//  Created by culter on 3/25/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef math_utils_h
#define math_utils_h

// A compact representation of a binary matrix.
typedef std::bitset<N*N> Matrix;

// Standard factorial function
constexpr uint64_t factorial(uint64_t n) {
  return (n == 0) ? 1 : (n * factorial(n - 1));
}

// Returns 1 if perm is an odd permutation; 0 if it is even
bool get_parity(const std::array<int, N>& perm) {
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

std::array<int, N> identity() {
  std::array<int, N> perm;
  for (int i = 0; i < N; ++i) {
    perm[i] = i;
  }
  return perm;
}

std::array<std::array<int, N>, factorial(N)> permutations() {
  std::array<std::array<int, N>, factorial(N)> answer;
  std::array<int, N> perm = identity();
  int count = 0;
  do {
    answer[count] = perm;
    count += 1;
  } while (std::next_permutation(perm.begin(), perm.end()));
  return answer;
}

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

Matrix matrix_from(std::array<int, N> permutation) {
  Matrix bits = 0;
  
  // Fill out N - 1 columns, not N, to leave the last column intentionally blank
  for (int i = 0; i < N - 1; ++i) {
    bits.set(i + N * permutation[i]);
  }
  return bits;
}

Matrix matrix_from_rows(std::array<Matrix, N> rows) {
  Matrix bits = 0;
  for (int i = 0; i < N; ++i) {
    bits |= (rows[i] << i * N);
  }
  return bits;
}

std::array<Matrix, N> rows_from_matrix(Matrix m) {
  constexpr Matrix row_mask = ((uint64_t)1 << N) - 1;
  std::array<Matrix, N> rows;
  for (int i = 0; i < N; ++i) {
    rows[i] = m & row_mask;
    m >>= N;
  }
  return rows;
}

Matrix apply(std::array<int, N> permutation, Matrix m) {
  std::array<Matrix, N> rows = rows_from_matrix(m);
  std::array<Matrix, N> new_rows;
  for (int i = 0; i < N; ++i) {
    new_rows[permutation[i]] = rows[i];
  }
  return matrix_from_rows(new_rows);
}

std::array<Matrix, factorial(N)> row_swaps(Matrix m) {
  std::array<Matrix, factorial(N)> answer;
  int count = 0;
  for (auto permutation: permutations()) {
    answer[count] = apply(permutation, m);
    count += 1;
  }
  return answer;
}

#endif /* math_utils_h */
