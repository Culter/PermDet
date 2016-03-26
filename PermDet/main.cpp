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

constexpr int N = 4;

constexpr uint64_t factorial(uint64_t n) {
  return (n == 0) ? 1 : (n * factorial(n - 1));
}

std::array<std::array<std::bitset<N * N - N>, factorial(N) / (2 * N)>, N> patterns_by_row;

bool is_free(const std::bitset<N * N - N>& bits, int row) {
  for (const auto& pattern: patterns_by_row[row]) {
    if ((bits & pattern) == pattern) return false;
  }
  return true;
}

uint64_t num(const std::bitset<N * N - N>& bits) {
  int free = 0;
  for (int i = 0; i < N; ++i) {
    if (is_free(bits, i)) {
      free += 1;
    }
  }
  return uint64_t(1) << free;
}

bool is_odd(const std::array<int, N>& perm) {
  bool answer = false;
  for (int i = 0; i < N; ++i) {
    for (int j = i + 1; j < N; ++j) {
      if (perm[i] > perm[j]) {
        answer = !answer;
      }
    }
  }
  return answer;
}

int main() {
  std::array<int, N> perm;
  for (int i = 0; i < N; ++i) {
    perm[i] = i;
  }
  std::array<int, N> known_perms = {};
  do {
    if (is_odd(perm)) {
      std::bitset<N * N - N> bits = 0;
      for (int i = 0; i < N - 1; ++i) {
        bits.set(i * N + perm[i]);
      }
      int row = perm[N - 1];
      patterns_by_row[row][known_perms[row]] = bits;
      known_perms[row] += 1;
    }
  } while (std::next_permutation(perm.begin(), perm.end()));
  
  for (int i = 0; i < N; ++i) {
    if (known_perms[i] != patterns_by_row[i].size()) {
      std::cout << "Fail!" << std::endl;
      return 1;
    }
  }
  
  uint64_t sum = 0;
  for (uint64_t i = 0; i < (uint64_t(1) << (N * N - N)); ++i) {
    std::bitset<N * N - N> bits = i;
    uint64_t n = num(bits);
//    if ((N * N - N - 24) < 0 || i % (uint64_t(1) << (N * N - N - 24)) == 0) {
//      std::cout << i << ": " << bits << ": " << n << std::endl;
//    }
    sum += n;
  }
  
  std::cout << sum << std::endl;
}
