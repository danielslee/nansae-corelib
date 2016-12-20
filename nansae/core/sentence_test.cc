/*
 * Copyright (c) 2016 Daniel Shihoon Lee <daniel@nansae.im>
 * All rights reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "gtest/gtest.h"

#include "nansae/core/sentence.h"
#include "nansae/core/trie.h"

TEST(Sentence, wordsLattice) {
  NSL::Sentence testSentence(NSL::String("symbol한글단어"));

  NSL::Trie trie;
  trie.addWord(NSL::String(u8"한"), 1);
  trie.addWord(NSL::String(u8"한글"), 2);
  trie.addWord(NSL::String(u8"글"), 3);
  trie.addWord(NSL::String(u8"단"), 4);
  trie.addWord(NSL::String(u8"단어"), 5);
  trie.freeze();

  auto lattice = testSentence.wordsLattice(trie);

  ASSERT_EQ(lattice.debugString(),
            "\
  0 1 2 3 4\n\
0 x        \n\
1   x x    \n\
2     x    \n\
3       x x\n\
4          ");
}
