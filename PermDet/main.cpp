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
#include <vector>

constexpr int N = 4;
constexpr int NN = N * N;

std::vector<std::bitset<NN>> patterns;
std::array<std::vector<std::bitset<N * N - N>>, N> patterns_by_row;

uint64_t num(const std::bitset<NN>& bits) {
  for (const auto& pattern: patterns) {
    if ((bits & pattern) == pattern) return 0;
  }
  return 1;
}

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
  do {
    if (is_odd(perm)) {
      std::bitset<NN> bits = 0;
      for (int i = 0; i < N; ++i) {
        bits[i + perm[i] * N] = 1;
      }
//      std::cout << "Odd pattern: " << bits << std::endl;
      patterns.push_back(bits);
      
      std::bitset<N * N - N> first_bits = 0;
      for (int i = 0; i < N - 1; ++i) {
        first_bits.set(i * N + perm[i]);
      }
      patterns_by_row[perm[N-1]].push_back(first_bits);
    }
  } while (std::next_permutation(perm.begin(), perm.end()));
  
  for (int i = 0; i < N; ++i) {
    std::cout << "Patterns at " << i << ":" << std::endl;
    for (const auto& p: patterns_by_row[i]) {
      std::cout << p << std::endl;
    }
  }
  
//  uint64_t sum = 0;
//  for (uint64_t i = 0; i < (uint64_t(1) << (N * N)); ++i) {
//    std::bitset<NN> bits = i;
//    uint64_t n = num(bits);
////    std::cout << i << ": " << bits << ": " << n << std::endl;
//    sum += n;
//  }
  
  uint64_t sum = 0;
  for (uint64_t i = 0; i < (uint64_t(1) << (N * N - N)); ++i) {
    std::bitset<N * N - N> bits = i;
    uint64_t n = num(bits);
    std::cout << i << ": " << bits << ": " << n << std::endl;
    sum += n;
  }
  
  std::cout << sum << std::endl;
}
