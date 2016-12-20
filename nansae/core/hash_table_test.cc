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
#include "gtest/gtest.h"

#include <iostream>
#include <unordered_map>

TEST(HashTable, test32) {
  NSL::HashTable<uint32_t> ht(65536);
  std::stringstream s;

  for (int i = 0; i < 30000; i++) {
    ASSERT_EQ(ht.insert(i, 20 * i), 0);
  }

  for (int i = 0; i < 30000; i++) {
    ASSERT_EQ(ht.insert(i, 3 * i), 1);
    ASSERT_EQ(ht.insert(i, 1.1 * i), 1);
  }

  ht.writeToStream(s);

  NSL::HashTable<uint32_t> ht2(s);

  for (int i = 29999; i >= 0; i--) {
    double x = ht2.retrieve(i);
    ASSERT_DOUBLE_EQ(x, 1.1 * i);
  }
}

TEST(HashTable, test64) {
  NSL::HashTable<uint64_t> ht(65536);
  std::stringstream s;

  for (int i = 0; i < 3000000; i += 100) {
    ASSERT_EQ(ht.insert(i * 500000000, 20 * i), 0);
  }

  for (int i = 0; i < 3000000; i += 100) {
    ASSERT_EQ(ht.insert(i * 500000000, 3 * i), 1);
    ASSERT_EQ(ht.insert(i * 500000000, 1.1 * i), 1);
  }

  ht.writeToStream(s);

  NSL::HashTable<uint64_t> ht2(s);

  for (int i = 2999900; i >= 0; i -= 100) {
    double x = ht2.retrieve(i * 500000000);
    ASSERT_DOUBLE_EQ(x, 1.1 * i);
  }
}

TEST(HashTable, exists) {
  NSL::HashTable<uint32_t> h1(256);
  h1.insert(2, 0.3);
  h1.insert(36, 0.1);
  ASSERT_EQ(h1.exists(2), true);
  ASSERT_EQ(h1.exists(36), true);
  ASSERT_EQ(h1.exists(44), false);
  ASSERT_EQ(h1.exists(623), false);
}

TEST(HashTable, iteration) {
  NSL::HashTable<uint32_t> ht(256);
  ht.insert(2, 0.3);
  ht.insert(36, 0.1);
  ht.insert(42, 0.7);

  // ids and values for verification
  std::unordered_map<uint32_t, double> verificationMap = {
      {2, 0.3}, {36, 0.1}, {42, 0.7}};

  ASSERT_EQ((ht.end() != ht.end()), false);

  int i = 0;
  for (NSL::HashTable<uint32_t>::Entry e : ht) {
    ASSERT_EQ(verificationMap[e.id], e.value);
    i++;
  }
  ASSERT_EQ(i, 3);

  i = 0;
  NSL::HashTable<uint32_t> h(256);
  for (NSL::HashTable<uint32_t>::Entry e : h) {
    i++;
  }
  ASSERT_EQ(i, 0);
}
