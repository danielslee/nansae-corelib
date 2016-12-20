/*
 * Copyright (c) 2016 Daniel Shihoon Lee <daniel@nansae.im>
 * All rights reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#ifndef NSL_SEGMENTATIONS_H
#define NSL_SEGMENTATIONS_H

#include <boost/dynamic_bitset.hpp>
#include <memory>
#include <sstream>

#include "nansae/core/string.h"
#include "nansae/core/trie.h"

namespace NSL {
class Segmentations {
 private:
  int _sentenceLength;
  boost::dynamic_bitset<> _words;

  size_t bitForWord(int startPos, int endPos) const {
    if (startPos > endPos)
      throw new std::invalid_argument(
          "The starting position may not be greater than the ending one");
    else if (startPos > _sentenceLength)
      throw new std::invalid_argument(
          "The starting position may not be greater than the ending one");

    return startPos * _sentenceLength + endPos;
  }

 public:
  /**
   * The constructor.
   */
  Segmentations(int sentenceLength) {
    _sentenceLength = sentenceLength;
    int bits = sentenceLength * sentenceLength;
    _words = boost::dynamic_bitset<>(bits);
  }

  static Segmentations ForSentence(const String& unsegmentedSentence, Trie& t) {
    Segmentations s(unsegmentedSentence.length());

    for (int i = 0; i < unsegmentedSentence.length(); ++i) {
      auto prefixes = t.findWordPrefixes(
          unsegmentedSentence.substring(i, unsegmentedSentence.length() - 1));

      if (unsegmentedSentence.characterAt(i)->type() ==
          Character::Type::EncapsulatedNonHangulSyllable) {
        s.addWord(i, i);
      }
      for (auto p : prefixes) {
        s.addWord(i, i + p.str.length() - 1);
      }
    }

    return s;
  }

  int sentenceLength() const { return _sentenceLength; }

  bool word(int startPos, int endPos) const {
    return _words[bitForWord(startPos, endPos)];
  }

  void addWord(int startPos, int endPos) {
    _words[bitForWord(startPos, endPos)] = 1;
  }

  void removeWord(int startPos, int endPos) {
    _words[bitForWord(startPos, endPos)] = 0;
  }

  std::vector<int> wordsStartingAt(int startPos) const {
    std::vector<int> words;
    for (int i = startPos; i < _sentenceLength; ++i) {
      if (word(startPos, i)) words.push_back(i);
    }
    return words;
  }

  std::vector<int> wordsEndingAt(int endPos) const {
    std::vector<int> words;
    for (int i = endPos; i >= 0; --i) {
      if (word(i, endPos)) words.push_back(i);
    }
    return words;
  }

  std::string debugString() const {
    std::stringstream resultStream;
    resultStream << "  ";
    for (int e = 0; e < _sentenceLength; e++) {
      resultStream << e;
      if (e < _sentenceLength - 1) resultStream << " ";
    }
    resultStream << "\n";

    for (int s = 0; s < _sentenceLength; ++s) {
      resultStream << s << " ";
      for (int e = 0; e < _sentenceLength; ++e) {
        if (e < s) {
          resultStream << "  ";
          continue;
        }

        resultStream << (word(s, e) ? "x" : " ");
        if (e < _sentenceLength - 1) resultStream << " ";
      }

      if (s < _sentenceLength - 1) resultStream << "\n";
    }

    return resultStream.str();
  }
};
}

#endif  // NSL_SEGMENTATIONS_H
