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
#include "EngineFlatter.h"

int main() {  
  typedef EngineFlatter ChosenEngine;
  constexpr bool serial = false;
  constexpr bool first_three = false;
  
  uint64_t sum = 0;
  constexpr int num_threads = (first_three && N >= 7) ? 3 : ChosenEngine::num_row_values;
  std::array<uint64_t, num_threads> subtotals = {};
  
  ChosenEngine seed;
  
  if (serial) {
    // Serial execution, by value of first row (without entry for last column)
    for (int i = 0; i < num_threads; ++i) {
      subtotals[i] = ChosenEngine(seed).Count(i);
      if (subtotals[i]) {
        std::cout << "Thread " << i << ": subtotal " << subtotals[i] << std::endl;
        sum += subtotals[i];
      }
    }
  } else {
    // Parallel execution, by value of first row (without entry for last column)
    std::vector<std::thread> threads;
    std::mutex mutex;
    for (int i = 0; i < num_threads; ++i) {
      threads.emplace_back([&, i]{
        subtotals[i] = ChosenEngine(seed).Count(i);
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
