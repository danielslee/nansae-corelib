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

#include "nansae/core/trie.h"
#include "gtest/gtest.h"

#include <cstring>
#include <iostream>

TEST(Trie, findWord) {
  NSL::Trie t;
  t.addWord(NSL::String(u8"빨"), 7);
  t.addWord(NSL::String(u8"빨갛"), 0);
  t.addWord(NSL::String(u8"빨간"), 1);
  t.addWord(NSL::String(u8"빨개"), 2);
  t.addWord(NSL::String(u8"파랗"), 3);
  t.addWord(NSL::String(u8"파란"), 4);
  t.addWord(NSL::String(u8"빨래"), 5);
  t.addWord(NSL::String(u8"빨리"), 6);
  t.freeze();

  ASSERT_EQ(t.findWord(NSL::String(u8"빨간")), 1);
  ASSERT_EQ(t.findWord(NSL::String(u8"파랗")), 3);
  ASSERT_EQ(t.findWord(NSL::String(u8"빨")), 7);
  ASSERT_EQ(t.findWord(NSL::String(u8"빨가")), NIME_TRIE_WORD_NOT_FOUND);
  ASSERT_EQ(t.findWord(NSL::String(u8"빨간색")), NIME_TRIE_WORD_NOT_FOUND);
}

TEST(Trie, findWordIdPaires) {
  NSL::Trie t;
  t.addWord(NSL::String(u8"빨"), 7);
  t.addWord(NSL::String(u8"빨갛"), 0);
  t.addWord(NSL::String(u8"빨간"), 1);
  t.addWord(NSL::String(u8"빨개"), 2);
  t.addWord(NSL::String(u8"파랗"), 3);
  t.addWord(NSL::String(u8"파란"), 4);
  t.addWord(NSL::String(u8"빨래"), 5);
  t.addWord(NSL::String(u8"빨리"), 6);
  t.addWord(NSL::String(u8"파"), 9);

  t.freeze();

  std::vector<NSL::Trie::WordIdPair> prefixes =
      t.findWordPrefixes(NSL::String(u8"빨간색"));
  ASSERT_EQ(prefixes[0].str, NSL::String(u8"빨"));
  ASSERT_EQ(prefixes[1].str, NSL::String(u8"빨간"));

  prefixes = t.findWordPrefixes(NSL::String(u8"파랗다"));
  ASSERT_EQ(prefixes[0].str, NSL::String(u8"파"));
  ASSERT_EQ(prefixes[1].str, NSL::String(u8"파랗"));
}

TEST(Trie, writeLoad) {
  NSL::Trie t;
  t.addWord(NSL::String(u8"빨갛"), 0);
  t.addWord(NSL::String(u8"빨간"), 1);
  t.addWord(NSL::String(u8"빨개"), 2);
  t.addWord(NSL::String(u8"파랗"), 3);
  t.addWord(NSL::String(u8"파란"), 4);
  t.addWord(NSL::String(u8"빨래"), 5);
  t.addWord(NSL::String(u8"빨리"), 6);
  t.freeze();

  std::stringstream s;
  t.writeToStream(s);

  NSL::Trie t2;
  t2.freeze();
  t2.loadFromStream(s);
  ASSERT_EQ(t2.findWord(NSL::String(u8"빨간")), 1);
  ASSERT_EQ(t2.findWord(NSL::String(u8"파랗")), 3);
  ASSERT_EQ(t2.findWord(NSL::String(u8"빨가")), NIME_TRIE_WORD_NOT_FOUND);
  ASSERT_EQ(t2.findWord(NSL::String(u8"빨간색")), NIME_TRIE_WORD_NOT_FOUND);
}

TEST(Trie, doubleInsert) {
  NSL::Trie t;
  t.addWord(NSL::String(u8"빨갛"), 0);
  t.addWord(NSL::String(u8"빨간"), 1);
  t.addWord(NSL::String(u8"빨개"), 2);
  t.addWord(NSL::String(u8"파랗"), 3);
  t.addWord(NSL::String(u8"파란"), 4);
  t.addWord(NSL::String(u8"빨래"), 5);
  t.addWord(NSL::String(u8"빨리"), 6);

  ASSERT_EQ(t.addWord(NSL::String(u8"빨개"), 0, false), 2);
  ASSERT_EQ(t.addWord(NSL::String(u8"파랗"), 0, false), 3);
  ASSERT_EQ(t.addWord(NSL::String(u8"빨래"), 0, false), 5);
  ASSERT_EQ(t.addWord(NSL::String(u8"빨리"), 0, false), 6);

  t.freeze();

  std::stringstream s;
  t.writeToStream(s);

  NSL::Trie t2;
  t2.freeze();
  t2.loadFromStream(s);
  ASSERT_EQ(t2.findWord(NSL::String(u8"빨간")), 1);
  ASSERT_EQ(t2.findWord(NSL::String(u8"파랗")), 3);
  ASSERT_EQ(t2.findWord(NSL::String(u8"빨가")), NIME_TRIE_WORD_NOT_FOUND);
  ASSERT_EQ(t2.findWord(NSL::String(u8"빨간색")), NIME_TRIE_WORD_NOT_FOUND);
}

TEST(Trie, doubleInsert2) {
  NSL::Trie t;
  ASSERT_EQ(t.addWord(NSL::String(u8"자기완성"), 0, false), 0);
  ASSERT_EQ(t.addWord(NSL::String(u8"자"), 1, false), 1);
  ASSERT_EQ(t.addWord(NSL::String(u8"자기"), 2, false), 2);

  ASSERT_EQ(t.addWord(NSL::String(u8"자기완성"), 0, false), 0);
  ASSERT_EQ(t.addWord(NSL::String(u8"자"), 1, false), 1);
  ASSERT_EQ(t.addWord(NSL::String(u8"자기"), 2, false), 2);

  t.freeze();

  ASSERT_EQ(t.findWord(NSL::String(u8"자")), 1);
  ASSERT_EQ(t.findWord(NSL::String(u8"자기완성")), 0);
}

TEST(Trie, iteration) {
  NSL::Trie t;
  t.addWord(NSL::String(u8"빨갛"), 0);
  t.addWord(NSL::String(u8"빨간"), 1);
  t.addWord(NSL::String(u8"빨개"), 2);
  t.addWord(NSL::String(u8"파랗"), 3);
  t.addWord(NSL::String(u8"파란"), 4);
  t.addWord(NSL::String(u8"빨래"), 5);
  t.addWord(NSL::String(u8"빨리"), 6);
  t.freeze();

  std::vector<NSL::String> words = {
      NSL::String("빨갛"),   NSL::String(u8"빨간"), NSL::String(u8"빨개"),
      NSL::String(u8"파랗"), NSL::String(u8"파란"), NSL::String(u8"빨래"),
      NSL::String(u8"빨리")};
  bool idFound[] = {false, false, false, false, false, false, false};

  for (NSL::Trie::WordIdPair wip : t) {
    ASSERT_EQ(words[wip.id], wip.str);
    NSL::Trie::WordIdPair copy(wip);
    ASSERT_EQ(words[copy.id].toStdString(), copy.str.toStdString());
    idFound[wip.id] = true;
    // std::cout << wip.str.toStdString() << ": " << wip.id << "\n";
  }

  for (int i = 0; i < 7; i++) {
    ASSERT_EQ(idFound[i], true);
  }
}
