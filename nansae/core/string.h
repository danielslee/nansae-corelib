/*
 * Copyright (c) 2016 Daniel Shihoon Lee <daniel@nansae.im>
 * All rights reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#ifndef NSL_STRING_H
#define NSL_STRING_H

#include <stdint.h>
#include <vector>

#include "nansae/core/character.h"

namespace NSL {
/**
 * Represents an 8-bit Hangul string containing each jamo as a separate
 * 8-bit character.
 */
struct HangulString {
  static const uint8_t NonHangulCode = 29;
  std::string theString;
  HangulString() = default;
  explicit HangulString(const std::string &str) : theString(str) {}
};

class String {
  friend struct std::hash<String>;

 private:
  struct StringImpl;
  std::unique_ptr<StringImpl> _impl;

 public:
  /**
   * Creates a new empty string.
   */
  String();

  /**
   * Creates a new string of size 1 containing the given character.
   * \param c The character.
   */
  String(const Character &c);

  /**
   * Creates a new string of size 1 containing the given character.
   * \param c The character.
   */
  String(const CharacterRef &c);

  /**
   * Creates a new string from a UTF-8 encoded C string.
   * \param str The string.
   */
  String(const char *str);

  /**
   * Creates a new string from a UTF-8 encoded std::string.
   * \param str The string.
   */
  String(const std::string &str);

  /**
   * Creates a new string from an 8-bit HangulString.
   */
  String(const HangulString &hstr);

  /**
   * Creates a copy of another string.
   * \param other The other string.
   */
  String(const String &other);

  /**
   * The operator = for copying.
   */
  String &operator=(const String &other);

  /**
   * Moves another string instance.
   * \param other The other string.
   */
  String(String &&other);

  /**
   * The operator = for moving.
   */
  String &operator=(String &&other);

  /**
   * Destroys the string.
   */
  ~String();

  /**
   * Appends another string onto the end.
   * \param str The string.
   */
  String &append(const String &str);

  /**
   * Appends a character.
   * \param c The character.
   */
  String &append(const Character &c);

  /**
   * Appends a character reference.
   * \param c The character reference.
   */
  String &append(const CharacterRef &c);

  /**
   * Prepends the current string by another string.
   * \param str The string to prepend.
   */
  String &prepend(const String &str);

  /**
   * Prepends the current string with a character.
   * \param c The character to prepend.
   */
  String &prepend(const Character &c);

  /**
   * Prepends the current string with a character reference.
   * \param c The character to prepend.
   */
  String &prepend(const CharacterRef &c);

  /**
   * Returns a new string that is a substring of the instance.
   * \param start The first index.
   * \param end The second index.
   */
  String substring(int start, int end) const;

  /**
   * Deletes all characters of the string.
   */
  String &clear();

  /**
   * Returns the length of the string.
   */
  int length() const;

  /**
   * Performs a comparison with another string.
   */
  bool operator==(const String &other) const;

  /**
   * Returns a reference to a character at index.
   * \param i The index.
   */
  CharacterRef characterAt(int i) const;

  /**
   * Checks whether the string begins with another string.
   */
  bool startsWith(const String &str) const;

  /**
   * A class for storing encapsulated non-Hangul syllable symbols
   * Cannot be interacted with by the user in any way.
   */
  class EncapsulatedNonHangul {
    friend class String;

   private:
    struct EncapsulatedNonHangulImpl;
    std::unique_ptr<EncapsulatedNonHangulImpl> _impl;

   public:
    EncapsulatedNonHangul();
    EncapsulatedNonHangul(const EncapsulatedNonHangul &other);
    EncapsulatedNonHangul &operator=(const EncapsulatedNonHangul &other);
    ~EncapsulatedNonHangul();
  };

  /**
   * Replaces all non-Hangul syllables symbols with an 'S' and stores and
   * returns them for future use.
   */
  EncapsulatedNonHangul encapsulateNonHangul();

  /**
   * Restores all non-Hangul syllables symbols hidden behind an 'S'
   * \param encapsulatedNonHangul The encapsulated symbols.
   */
  String &restoreNonHangul(const EncapsulatedNonHangul &encapsulatedNonHangul);

  class CannotConvertContainsNonHangulSyllableSymbolsException
      : public std::runtime_error {
   public:
    CannotConvertContainsNonHangulSyllableSymbolsException()
        : std::runtime_error(
              "Cannot convert to NSL::HangulString because\
                  the NSL::String contains non-Hangul syllable symbols.") {}
  };

  /**
   * Returns a HangulString representation of the string
   * \throws CannotConvertContainsNonHangulSyllableSymbolsException
   */
  HangulString toHangulString() const;

  /**
   * Returns true if the string only contains hangul syllables
   */
  bool isPureHangul() const;

  /**
   * Converts the String to a regular UTF-8 encoded std::string
   */
  std::string toStdString() const;

  /**
   * Finds all matches ending with a specified jamo starting at a specified
   * index.
   * \param startingIndex The index to start the search at.
   * \param jamo          The final jamo to look for.
   * \ret   A list of ints indicating the distance from the starting index.
   */
  std::vector<int> findMatchesEndingWithJamo(int startingIndex,
                                             Character::HangulJamo jamo);

  CharacterRef begin() const;
  CharacterRef end() const;
};
}

namespace std {
template <>
struct hash<NSL::String> {
  std::size_t operator()(const NSL::String &str) const;
};
}

#endif  // NSL_STRING
