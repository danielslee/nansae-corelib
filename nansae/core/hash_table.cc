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

#include "nansae/core/hash_table.h"
#include "nansae/core/stream_binary_io.h"

#include <stdio.h>
#include <cstdlib>
#include <stdexcept>
#include <utility>

namespace NSL {

template <typename T>
struct HashTable<T>::Bucket {
  T id;
  ValueType value;
  bool used;
};

inline uint32_t hash(uint32_t h) {
  /*x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);*/
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

inline uint64_t hash(uint64_t h) {
  h ^= h >> 33;
  h *= 0xff51afd7ed558ccd;
  h ^= h >> 33;
  h *= 0xc4ceb9fe1a85ec53;
  h ^= h >> 33;
  return h;
}

template <typename T>
HashTable<T>::HashTable(T bucketsNo) : _bucketsNo(bucketsNo) {
  this->_buckets = (Bucket*)std::calloc(bucketsNo, sizeof(Bucket));
}

template <typename T>
HashTable<T>::HashTable(const HashTable<T>& other)
    : _bucketsNo(other._bucketsNo), _usedUpBuckets(other._usedUpBuckets) {
  _buckets = (Bucket*)std::calloc(_bucketsNo, sizeof(Bucket));
  for (int i = 0; i < _bucketsNo; i++) {
    _buckets[i] = other._buckets[i];
  }
}

template <typename T>
HashTable<T>::HashTable(HashTable<T>&& other)
    : _bucketsNo(other._bucketsNo), _usedUpBuckets(other._usedUpBuckets) {
  _buckets = other._buckets;
  other._buckets = nullptr;
  other._bucketsNo = 0;
  other._usedUpBuckets = 0;
}

template <typename T>
void HashTable<T>::loadFromStream(std::istream& is) {
  _bucketsNo = StreamBinaryRead<T>(is);
  _buckets = (Bucket*)std::calloc(_bucketsNo, sizeof(Bucket));

  for (T i = 0; i < _bucketsNo; i++) {
    _buckets[i].id = StreamBinaryRead<T>(is);
    _buckets[i].value = StreamBinaryRead<double>(is);
    _buckets[i].used = true;
    _buckets[i].used = StreamBinaryRead<bool>(is);
  }
}

template <typename T>
void HashTable<T>::writeToStream(std::ostream& os) {
  StreamBinaryWrite<T>(os, _bucketsNo);
  for (T i = 0; i < _bucketsNo; i++) {
    StreamBinaryWrite<T>(os, _buckets[i].id);
    StreamBinaryWrite<double>(os, _buckets[i].value);
    StreamBinaryWrite<bool>(os, _buckets[i].used);
  }
}

template <typename T>
void HashTable<T>::rehash(T bucketsNo) {
  Bucket* originalBuckets = _buckets;
  T originalBucketsNo = _bucketsNo;

  _bucketsNo = bucketsNo;
  _buckets = (Bucket*)std::calloc(_bucketsNo * 2, sizeof(Bucket));
  _usedUpBuckets = 0;

  for (T i = 0; i < originalBucketsNo; i++) {
    if (originalBuckets[i].used) {
      this->insert(originalBuckets[i].id, originalBuckets[i].value);
    }
  }

  std::free(originalBuckets);
}

template <typename T>
int HashTable<T>::insert(T id, double value) {
  if (_usedUpBuckets > .8f * _bucketsNo) {
    this->rehash(2 * _bucketsNo);
  }

  T hashValue = hash(id);
  T desiredPosition = hashValue % _bucketsNo;

  // find a non-used bucket
  bool reachedStart = false;
  for (T position = desiredPosition;
       !(reachedStart && (position == _bucketsNo - 1));
       position = (position + 1) % _bucketsNo) {
    if (!_buckets[position].used) {
      _buckets[position].id = id;
      _buckets[position].value = value;
      _buckets[position].used = true;
      _usedUpBuckets++;
      return 0;
    } else if (_buckets[position].id == id) {
      _buckets[position].value = value;
      return 1;
    } else {
      T dist = position - desiredPosition;
      T currentBucketDist =
          position - (hash(_buckets[position].id) % _bucketsNo);

      // swap if the current bucket is closer to its desired position than we
      // are to ours
      if (currentBucketDist < dist) {
        desiredPosition = hash(_buckets[position].id) % _bucketsNo;
        std::swap(id, _buckets[position].id);
        hashValue = hash(id);
        std::swap(value, _buckets[position].value);
      }
    }
    if (position == 0) reachedStart = true;
  }
  return (-1);
}

template <typename T>
double HashTable<T>::retrieve(T id) const {
  T hashValue = hash(id);
  T startingLocation = hashValue % _bucketsNo;

  // find the bucket with the correct id
  for (T i = startingLocation; i < _bucketsNo; i = (i + 1) % _bucketsNo) {
    int dist = i - startingLocation;
    if (!_buckets[i].used)
      return 0;
    else if (dist > i - (hash(_buckets[i].id) % _bucketsNo))
      return 0;
    else if (_buckets[i].id == id) {
      return _buckets[i].value;
    }
  }
  return 0;
}

template <typename T>
bool HashTable<T>::exists(T id) const {
  T hashValue = hash(id);
  T startingLocation = hashValue % _bucketsNo;

  // find the bucket with the correct id
  for (T i = startingLocation; i < _bucketsNo; i = (i + 1) % _bucketsNo) {
    int dist = i - startingLocation;
    if (!_buckets[i].used)
      return false;
    else if (dist > i - (hash(_buckets[i].id) % _bucketsNo))
      return false;
    else if (_buckets[i].id == id) {
      return true;
    }
  }
  return false;
}

template <typename T>
HashTable<T>::~HashTable() {
  std::free(this->_buckets);
}

template <typename T>
typename HashTable<T>::Entry HashTable<T>::Iterator::operator*() {
  typename HashTable<T>::Bucket* b = _ht->_buckets + _position;
  Entry e;
  e.id = b->id;
  e.value = b->value;
  return e;
}

template <typename T>
typename HashTable<T>::Entry HashTable<T>::Iterator::operator->() {
  return this->operator*();
}

template <typename T>
typename HashTable<T>::Iterator HashTable<T>::Iterator::operator++() {
  for (T newPosition = _position + 1; newPosition < _ht->_bucketsNo;
       newPosition++) {
    typename HashTable<T>::Bucket* b = _ht->_buckets + newPosition;
    if (b->used) {
      _position = newPosition;
      return *this;
    }
  }

  Iterator end = _ht->end();
  _position = end._position;
  return *this;
  // throw std::out_of_range("The NSL::HashTable doesn't contain any more
  // entries.");
}

template <typename T>
typename HashTable<T>::Iterator HashTable<T>::Iterator::operator+(T moveBy) {
  for (T i = 0; i < moveBy - 1; ++i) {
    this->operator++();
  }
  return this->operator++();
}

template <typename T>
bool HashTable<T>::Iterator::operator!=(const Iterator& other) {
  return (_position != other._position) || (_ht != other._ht);
}

template <typename T>
typename HashTable<T>::Iterator HashTable<T>::begin() {
  for (T pos = 0; pos < _bucketsNo; pos++) {
    Bucket* b = _buckets + pos;
    if (b->used) {
      Iterator it;
      it._position = pos;
      it._ht = this;
      return it;
    }
  }

  return this->end();
}

template <typename T>
typename HashTable<T>::Iterator HashTable<T>::end() {
  Iterator it;
  it._position = _bucketsNo + 1;
  it._ht = this;
  return it;
}

template class HashTable<uint32_t>;
template class HashTable<uint64_t>;
}
