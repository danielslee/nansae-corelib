/*
 * Copyright (c) 2015-2017 Daniel Shihoon Lee <daniel@nansae.im>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NSL_HASH_TABLE_H
#define NSL_HASH_TABLE_H

#include <cstdint>
#include <iostream>

namespace NSL {
typedef double ValueType;

template <typename T>
class HashTable {
 private:
  struct Bucket;
  T _bucketsNo;
  Bucket* _buckets;
  T _usedUpBuckets = 0;

  void rehash(T bucketsNo);

 public:
  struct Entry {
    T id;
    ValueType value;
  };

  class Iterator {
    friend HashTable;

   private:
    T _position;
    HashTable* _ht;

   public:
    Entry operator*();
    Entry operator->();
    Iterator operator++();
    Iterator operator+(T moveBy);
    bool operator!=(const Iterator& other);
  };

  HashTable(T bucketsNo = 256);
  HashTable(std::istream& is) { this->loadFromStream(is); }
  ~HashTable();

  HashTable(const HashTable& other);
  HashTable(HashTable&& other);

  void writeToStream(std::ostream& os);
  void loadFromStream(std::istream& is);

  int insert(T id, ValueType value);
  ValueType retrieve(T id) const;
  bool exists(T id) const;

  uint32_t bucketsNo() { return this->_bucketsNo; }

  Iterator begin();
  Iterator end();
};
}
#endif  // NSL_HASH_TABLE_H
