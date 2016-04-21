//
//  CompactMaskTest.cpp
//  PermDet
//
//  Created by culter on 4/20/16.
//  Copyright © 2016 culter. All rights reserved.
//

#include <iostream>

#include "CompactMask.h"
#include "CompactMaskTest.h"

void CompactMaskTest()
{
  CompactMask<10, 2> x;
  x.Reset();
  x.Set(std::array<int,2>{2,3});
  std::cout << "x.mask[0] = " << std::bitset<64>(x.mask[0]) << std::endl;
  std::cout << "x.mask[1] = " << std::bitset<64>(x.mask[1]) << std::endl;
  
  uint64_t lo;
  uint64_t hi;
  GetRowMask<10>(0b1111111010, lo, hi);
  std::cout << "lo = " << std::bitset<64>(lo) << std::endl;
  std::cout << "hi = " << std::bitset<64>(hi) << std::endl;
}