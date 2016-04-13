//
//  ImmediateRowOrderer.h
//  PermDet
//
//  Created by culter on 4/12/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef ImmediateRowOrderer_h
#define ImmediateRowOrderer_h

struct ImmediateRowOrderer {
  ImmediateRowOrderer(): max_row_class(0), current_streak(0), stabilizer(1) {}
  
  bool accepts_class(int row_class) const {
    return true;  // Assume that the engine knows better than to propose classes out of order
  }
  bool accepts_value(int row_value) const {
    return true;  // This orderer doesn't even care about the exact value.
  }
  
  void append(int row_class, int row_value) {
    if (row_class == max_row_class) {
      current_streak += 1;
      stabilizer *= current_streak;
    } else {
      current_streak = 1;
      max_row_class = row_class;
    }
  }
  
  int max_row_class;
  int current_streak;
  uint64_t stabilizer;
};

#endif /* ImmediateRowOrderer_h */
