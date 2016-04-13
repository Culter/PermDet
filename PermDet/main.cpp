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

//__uint128_t asdf = 0;

int main() {
  typedef EngineFlatter ChosenEngine;
  constexpr bool serial = true;
  constexpr bool first_three = false;
  
  uint64_t sum = 0;
  constexpr int num_threads = (first_three && N >= 7) ? 3 : ChosenEngine::num_row_values;
  std::array<uint64_t, num_threads> subtotals = {};
  
  if (serial) {
    ChosenEngine seed;
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
    std::vector<ChosenEngine> seeds;
    std::vector<int> first_rows;
    
    seeds.emplace_back();
    first_rows.push_back(0);
    
    for (int i = 1; i < num_threads; ++i) {
      if (seeds.front().Accepts(i)) {
        seeds.push_back(seeds.front());
        first_rows.push_back(i);
      }
    }
    
    std::vector<std::thread> threads;
    std::mutex io_mutex;
    
    for (int i = 0; i < seeds.size(); ++i) {
      std::cout << "Scheduling thread " << i << " with value " << first_rows[i] << std::endl;
      int first_row = first_rows[i];
      ChosenEngine& engine = seeds[i];
      uint64_t& subtotal = subtotals[first_row];
      
      threads.emplace_back([&engine, first_row, &subtotal, &io_mutex]{
        {
          std::lock_guard<std::mutex> lock(io_mutex);
          std::cout << "Starting thread with value " << first_row << std::endl;
        }
        
        subtotal = engine.Count(first_row);
        
        {
          std::lock_guard<std::mutex> lock(io_mutex);
          std::cout << "Thread value " << first_row << ": subtotal " << subtotal << std::endl;
        }
      });
    }
    for (auto& thread: threads) {
      thread.join();
    }
    
    // Re-print the thread results in deterministic order.
    for (int i = 0; i < num_threads; ++i) {
      if (subtotals[i]) {
        std::cout << "Thread " << i << ": subtotal " << subtotals[i] << std::endl;
        sum += subtotals[i];
      }
    }
  }
  
  std::cout << "sum: " << sum << std::endl;
}
