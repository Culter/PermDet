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
#include "Engine.h"
#include "EngineNext.h"

int main() {
  auto array_even = permutation_matrices(0);
  auto array_odd = permutation_matrices(1);
  std::vector<Matrix> vector_even(array_even.cbegin(), array_even.cend());
  std::vector<Matrix> vector_odd(array_odd.cbegin(), array_odd.cend());
  
  uint64_t sum = 0;
  constexpr uint64_t num_threads = (uint64_t)1 << (N - 1);
  std::array<uint64_t, num_threads> subtotals = {};
  
  bool serial = true;
  if (serial) {
    // Serial execution, by value of first row (without entry for last column)
    for (int i = 0; i < num_threads; ++i) {
      subtotals[i] = Engine(vector_even, vector_odd).Count(i);
      std::cout << "Thread " << i << ": subtotal " << subtotals[i] << std::endl;
      sum += subtotals[i];
    }
  } else {
    // Parallel execution, by value of first row (without entry for last column)
    std::vector<std::thread> threads;
    std::mutex mutex;
    for (int i = 0; i < num_threads; ++i) {
      threads.emplace_back([&, i]{
        subtotals[i] = Engine(vector_even, vector_odd).Count(i);
        std::lock_guard<std::mutex> lock(mutex);
        std::cout << "Thread " << i << ": subtotal " << subtotals[i] << std::endl;
        sum += subtotals[i];
      });
    }
    for (auto& thread: threads) {
      thread.join();
    }
    
    // Re-print the thread results in deterministic order.
    for (int i = 0; i < num_threads; ++i) {
      std::cout << "Thread " << i << ": subtotal " << subtotals[i] << std::endl;
    }
  }
  
  std::cout << "sum: " << sum << std::endl;
}
