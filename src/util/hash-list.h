// util/hash-list.h

// Copyright 2009-2011   Microsoft Corporation
//                2013   Johns Hopkins University (author: Daniel Povey)

// See ../../COPYING for clarification regarding multiple authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.


#ifndef KALDI_UTIL_HASH_LIST_H_
#define KALDI_UTIL_HASH_LIST_H_
#include <vector>
#include <set>
#include <algorithm>
#include <limits>
#include <cassert>
#include "util/stl-utils.h"


/* This header provides utilities for a structure that's used in a decoder (but
   is quite generic in nature so we implement and test it separately).
   Basically it's a singly-linked list, but implemented in such a way that we
   can quickly search for elements in the list.  We give it a slightly richer
   interface than just a hash and a list.  The idea is that we want to separate
   the hash part and the list part: basically, in the decoder, we want to have a
   single hash for the current frame and the next frame, because by the time we
   need to access the hash for the next frame we no longer need the hash for the
   previous frame.  So we have an operation that clears the hash but leaves the
   list structure intact.  We also control memory management inside this object,
   to avoid repeated new's/deletes.

   See hash-list-test.cc for an example of how to use this object.
*/


namespace kaldi {

template<class I, class T> class HashList {
 public:
  struct Elem {//Elem可以构成一个List结构。
    I key;
    T val;
    Elem *tail;//指向下一个Elem的位置
  };

  /// Constructor takes no arguments.
  /// Call SetSize to inform it of the likely size.
  HashList();

  /// Clears the hash and gives the head of the current list to the user;
  /// ownership is transferred to the user (the user must call Delete()
  /// for each element in the list, at his/her leisure).
  Elem *Clear();

  /// Gives the head of the current list to the user.  Ownership retained in the
  /// class.  Caution: in December 2013 the return type was changed to const
  /// Elem* and this function was made const.  You may need to change some types
  /// of local Elem* variables to const if this produces compilation errors.
  const Elem *GetList() const;

  /// Think of this like delete().  It is to be called for each Elem in turn
  /// after you "obtained ownership" by doing Clear().  This is not the opposite
  /// of. Insert, it is the opposite of New.  It's really a memory operation.
  inline void Delete(Elem *e);

  /// This should probably not be needed to be called directly by the user.
  /// Think of it as opposite
  /// to Delete();
  inline Elem *New();

  /// Find tries to find this element in the current list using the hashtable.
  /// It returns NULL if not present.  The Elem it returns is not owned by the
  /// user, it is part of the internal list owned by this object, but the user
  /// is free to modify the "val" element.
  inline Elem *Find(I key);

  /// Insert inserts a new element into the hashtable/stored list.  By calling
  /// this,
  /// the user asserts that it is not already present (e.g. Find was called and
  /// returned NULL).  With current code, calling this if an element already
  ///  exists will result in duplicate elements in the structure, and Find()
  ///  will find the first one that was added.
  /// [but we don't guarantee this behavior].
  inline void Insert(I key, T val);

  /// Insert inserts another element with same key into the hashtable/
  /// stored list.
  /// By calling this, the user asserts that one element with that key is
  /// already present.
  /// We insert it that way, that all elements with the same key
  /// follow each other.
  /// Find() will return the first one of the elements with the same key.
  inline void InsertMore(I key, T val);

  /// SetSize tells the object how many hash buckets to allocate (should
  /// typically be at least twice the number of objects we expect to go in the
  /// structure, for fastest performance).  It must be called while the hash
  /// is empty (e.g. after Clear() or after initializing the object, but before
  /// adding anything to the hash.
  void SetSize(size_t sz);

  /// Returns current number of hash buckets.
  inline size_t Size() { return hash_size_; }

  ~HashList();
 private:
  
  //HashList中一个hash值对应的Hash块的结构
  struct HashBucket {
    size_t prev_bucket;  // index to next bucket (-1 if list tail).  Note:
    // list of buckets goes in opposite direction to list of Elems.
    // 下一个Hash块对应在buckets_中的位置，注意hash块顺序与Elem list顺序是相反的
    Elem *last_elem;  // pointer to last element in this bucket (NULL if empty)，当前Hash块最后一个Elem的位置
    inline HashBucket(size_t i, Elem *e): prev_bucket(i), last_elem(e) {}
  };

  Elem *list_head_;  // head of currently stored list.当前Elem list的头的位置
  size_t bucket_list_tail_;  // tail of list of active hash buckets. buckets_中最后一个激活的hush块在vector（hash表）中的指标

  size_t hash_size_;  // number of hash buckets. hash块的数目

  std::vector<HashBucket> buckets_; //hash块组成的hash表

  Elem *freed_head_;  // head of list of currently freed elements. [ready for
  // allocation] 当前未利用的Elem的头的位置，对应于allocated_中最后一个元素指向的内存块的第一个未被使用的Elem的位置，如果全被使用，则为NULL

  std::vector<Elem*> allocated_;  // list of allocated blocks. 已分配的保存Elem的内存块位置，除了最后一个元素对于的内存块，
   //其余内存块均已经被填满

  static const size_t allocate_block_size_ = 1024;  // Number of Elements to
  // allocate in one block.  Must be largish so storing allocated_ doesn't
  // become a problem. 单个内存块中可保存的Elem的数目
};


}  // end namespace kaldi

#include "util/hash-list-inl.h"

#endif  // KALDI_UTIL_HASH_LIST_H_
