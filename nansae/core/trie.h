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

#ifndef NSL_TRIE_H
#define NSL_TRIE_H

#include <cstdint>

#include "nansae/core/string.h"

#define NIME_TRIE_WORD_NOT_FOUND UINT32_MAX

namespace NSL {
/**
 * libNansae's implementation of a trie.
 */
class Trie {
 private:
  struct TrieImpl;
  struct IteratorImpl;
  std::unique_ptr<TrieImpl> _impl;

 public:
  /**
   * A struct containing a word and it's id.
   */
  struct WordIdPair {
    String str;
    uint32_t id;
  };

  /**
   * An iterator for enumerating words in the trie.
   */
  class Iterator {
    friend Trie;

   private:
    std::shared_ptr<IteratorImpl> _impl;

   public:
    Iterator();
    ~Iterator();
    WordIdPair operator*();
    WordIdPair operator->();
    Iterator operator++();
    Iterator operator+(uint32_t moveBy);
    bool operator!=(const Iterator &other);
  };

  /**
   * The constructor.
   */
  Trie();

  /**
   * The destructor.
   */
  ~Trie();

  /**
   * Makes the trie editable.
   * The trie cannot be searched when in editing mode.
   */
  void makeEditable();

  /**
   * Freezes the trie.
   * New words cannot be added to a frozen trie.
   */
  void freeze();

  /**
   * Adds a new word to the trie.
   * Can be used to change an existing word's id if the replace parameter
   * is set to true.
   * New words cannot be added when in frozen mode.
   * \param str The new word.
   * \param id The id to use for the new word
   * \param replace Wheather to replace any existing ids
   * \ret The id of the new word. Same as the original id when the word
   * already exists and replace is set to false.
   */
  uint32_t addWord(const String &str, uint32_t id, bool replace = true);

  /**
   * Finds a word in the trie.
   * The trie cannot be searched when in editing mode.
   * \param str The word.
   * \ret The corresponding id.
   */
  uint32_t findWord(const String &str);

  /**
   * Finds all prefixes matching the given string
   * \param str the string
   * \ret a list containing all words and their ids found
   */
  std::vector<WordIdPair> findWordPrefixes(const String &str);

  /**
   * Serializes the trie to an std::ostream.
   * \param s the stream
   */
  void writeToStream(std::ostream &s);

  /**
   * Deserializes the trie from an std::istream.
   * \param s the stream
   */
  void loadFromStream(std::istream &s);

  /**
   * Returns true if the trie is in editing mode.
   * \ret whether the trie is in editing mode
   */
  bool editingMode();

  Iterator begin();
  Iterator end();
};
}

#endif  // NSL_TRIE_H
