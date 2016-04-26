//
//  OptimalReference.h
//  PermDet
//
//  Created by culter on 4/25/16.
//  Copyright © 2016 culter. All rights reserved.
//

#ifndef OptimalReference_h
#define OptimalReference_h

template<typename T>
struct StackReference {
  T _data;
  T* operator->() { return &_data; }
};

template<typename T>
struct HeapReference {
  std::shared_ptr<T> _data_ptr;
  HeapReference() : _data_ptr{std::make_shared<T>()} {}
  std::shared_ptr<T>& operator->() { return _data_ptr; }
};

template<typename T>
struct OptimalReference : std::conditional<sizeof(T) >= 1020*8, HeapReference<T>, StackReference<T>>::type {};

#endif /* OptimalReference_h */
