/*
 * Copyright (c) 2016 Daniel Shihoon Lee <daniel@nansae.im>
 * All rights reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "nansae/core/segmentations.h"
#include "gtest/gtest.h"

TEST(Segmentations, segmentationsForSentence) {
  NSL::String str(u8"symbol한글단어");
  str.encapsulateNonHangul();

  NSL::Trie trie;
  trie.addWord(NSL::String(u8"한"), 1);
  trie.addWord(NSL::String(u8"한글"), 2);
  trie.addWord(NSL::String(u8"글"), 3);
  trie.addWord(NSL::String(u8"단"), 4);
  trie.addWord(NSL::String(u8"단어"), 5);
  trie.freeze();

  auto segmentations = NSL::Segmentations::ForSentence(str, trie);

  ASSERT_EQ(segmentations.debugString(),
            "\
  0 1 2 3 4\n\
0 x        \n\
1   x x    \n\
2     x    \n\
3       x x\n\
4          ");
}

TEST(Segmentations, access) {
  NSL::Segmentations s(10);
  ASSERT_EQ(s.word(3, 4), false);
}

TEST(Segmentations, addWord) {
  NSL::Segmentations s(10);
  s.addWord(3, 4);
  ASSERT_EQ(s.word(3, 4), true);
}

TEST(Segmentations, removeWord) {
  NSL::Segmentations s(10);
  s.addWord(3, 4);
  s.removeWord(3, 4);
  ASSERT_EQ(s.word(3, 4), false);
}

TEST(Segmentations, wordsStartingAt) {
  NSL::Segmentations s(10);

  std::vector<int> expectedResult = {2, 3, 5, 7};
  for (int endPos : expectedResult) {
    s.addWord(2, endPos);
  }

  ASSERT_EQ(s.wordsStartingAt(2), expectedResult);
}

TEST(Segmentations, wordsEndingAt) {
  NSL::Segmentations s(10);

  std::vector<int> expectedResult = {7, 5, 3, 2};
  for (int startPos : expectedResult) {
    s.addWord(startPos, 9);
  }

  ASSERT_EQ(s.wordsEndingAt(9), expectedResult);
}

TEST(Segmentations, debugString) {
  NSL::Segmentations s(10);
  s.addWord(0, 2);
  s.addWord(0, 1);
  s.addWord(1, 2);
  std::vector<int> endings = {2, 3, 5, 7};
  for (int e : endings) {
    s.addWord(2, e);
  }
  s.addWord(3, 7);
  s.addWord(5, 7);
  s.addWord(7, 9);

  auto debugString = s.debugString();
  ASSERT_EQ(debugString,
            "\
  0 1 2 3 4 5 6 7 8 9\n\
0   x x              \n\
1     x              \n\
2     x x   x   x    \n\
3               x    \n\
4                    \n\
5               x    \n\
6                    \n\
7                   x\n\
8                    \n\
9                    ");
}
