//
//  ColumnOrderer.h
//  PermDet
//
//  Created by culter on 4/12/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef ColumnOrderer_h
#define ColumnOrderer_h

struct ColumnOrderer {
  ColumnOrderer(): column_jumps(0), stabilizer(1) {}
  
  bool accepts(std::bitset<N> row_value) const {
    std::bitset<N> backward_jumps = (((row_value << 1) & ~row_value) >> 1);
    return !((backward_jumps & ~column_jumps).any());
  }
  
  void append(std::bitset<N> row_value) {
    std::bitset<N> forward_jumps = ~row_value & (row_value >> 1);
    column_jumps |= forward_jumps;
  }
  
  void freeze_stabilizer() {
    std::bitset<N-1> constants = ~(column_jumps.to_ullong());
    
    int column_streak = 1;
    while (constants.any()) {
      if (constants[0]) {
        column_streak += 1;
        stabilizer *= column_streak;
      } else {
        column_streak = 1;
      }
      constants >>= 1;
    }
  }
  
  // A 1 in position i means that in a more significant row than the current row,
  // there is a difference between i and i+1.
  std::bitset<N> column_jumps;
  
  // The stabilizer is a tentative version until freeze_stabilizer() is called.
  // It can only increase from 1 to its final value.
  uint64_t stabilizer;
};

#endif /* ColumnOrderer_h */
