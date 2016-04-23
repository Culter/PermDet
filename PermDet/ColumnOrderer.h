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
  
  bool accepts(unsigned row_value) const {
    unsigned backward_jumps = (((row_value << 1) & ~row_value & ((1u << N) - 1)) >> 1);
    return (backward_jumps & ~column_jumps) == 0;
  }
  
  void append(unsigned row_value) {
    unsigned forward_jumps = ~row_value & (row_value >> 1);
    column_jumps |= forward_jumps;
  }
  
  void freeze_stabilizer() {
    unsigned constants = ~column_jumps & ((1u << (N - 1)) - 1);
    
    int column_streak = 1;
    while (constants) {
      if (constants % 2) {
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
  unsigned column_jumps;
  
  // The stabilizer is a tentative version until freeze_stabilizer() is called.
  // It can only increase from 1 to its final value.
  uint64_t stabilizer;
};

#endif /* ColumnOrderer_h */
