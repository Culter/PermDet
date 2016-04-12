//
//  RecursiveMask.h
//  PermDet
//
//  Created by culter on 4/11/16.
//  Copyright © 2016 culter. All rights reserved.
//

#include "PartialPermutation.h"

#ifndef RecursiveMask_h
#define RecursiveMask_h

template<unsigned width, unsigned height>
struct RecursiveMask {
  typedef PartialPermutation<width, height> TPerm;
  typedef PartialPermutation<width, height - 1> TChildPerm;
  
  std::bitset<TPerm::size> mask;
  std::array<std::bitset<TChildPerm::size>, width> pre_mask;
  RecursiveMask<width, height - 1> child;
};

template<unsigned width>
struct RecursiveMask<width, /*height = */ 0> {};

#endif /* RecursiveMask_h */
