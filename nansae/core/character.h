/*
 * Copyright (c) 2016 Daniel Shihoon Lee <daniel@nansae.im>
 * All rights reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#ifndef NSL_CHARACTER_H
#define NSL_CHARACTER_H

#include <stdint.h>
#include <boost/format.hpp>
#include <stdexcept>

namespace NSL {
/**
 * The data type used as the underlying storage for a character.
 */
typedef uint32_t CharacterDataType;

/**
 * A class representing a single character.
 */
class Character {
  friend class CharacterRef;

 private:
  struct CharacterImpl;
  std::unique_ptr<CharacterImpl> _impl;

 public:
  ////
  // Member types
  ////
  /**
   * An enum containing all character types.
   */
  enum class Type {
    /**
     * Represents a generic character.
     */
    Character,

    /**
     * Represents a Hangul syllable.
     */
    HangulSyllable,

    /**
     * Represents a single Hangul jamo.
     */
    HangulJamo,

    /**
     * Represents a non-Hangul syllable string temporarily hidden behind an 'S'.
     */
    EncapsulatedNonHangulSyllable
  };

  /**
   * Returns a string representing an element in the CharacterType enum.
   * Used for debugging purposes.
   * \ret The string representation.
   */
  static const char *TypeToString(Type type) {
    switch (type) {
      case Type::Character:
        return "NSL::Character::Type::Character";
      case Type::HangulSyllable:
        return "NSL::Character::Type::HangulSyllable";
      case Type::HangulJamo:
        return "NSL::Character::Type::HangulJamo";
      case Type::EncapsulatedNonHangulSyllable:
        return "NSL::Character::Type::EncapsulatedNonHangulSyllable";
    };
    return "<unknown>";
  }

  /**
   * An exception that is thrown in case a method unsupported on the
   * current character has been called.
   */
  class OperationNotSupportedOnCharacterType : public std::runtime_error {
   public:
    OperationNotSupportedOnCharacterType(const std::string &operation,
                                         Type characterType)
        : std::runtime_error((boost::format("Operation '%1%' is not supported "
                                            "on character type '%2%'.") %
                              operation % TypeToString(characterType))
                                 .str()) {}
  };

  /**
   * A 16-bit type used to represent a single Hangul syllable.
   */
  struct HangulSyllableCode {
    static const uint16_t Symbol = UINT16_MAX;
    static const uint16_t BOS = UINT16_MAX - 1;
    static const uint16_t EOS = UINT16_MAX - 2;
    uint16_t theCode;
    HangulSyllableCode() {}
    explicit HangulSyllableCode(uint16_t code) : theCode(code) {}
  };

  /**
   * An enum containing all Hangul jamos.
   * The order matches Hangul compatibility jamo.
   */
  enum class HangulJamo {
    Giyeok,
    SsangGiyeok,
    GiyeokSiot,
    Nieun,
    NieunJieut,
    NieunHieut,
    Digeut,
    SsangDigeut,
    Rieul,
    RieulGiyeok,
    RieulMieum,
    RieulBieup,
    RieulSiot,
    RieulTieut,
    RieulPieup,
    RieulHieut,
    Mieum,
    Bieup,
    SsangBieup,
    BieupSiot,
    Siot,
    SsangSiot,
    Ieung,
    Jieut,
    SsangJieut,
    Chieut,
    Kieuk,
    Tieut,
    Pieup,
    Hieut,

    A,
    Ae,
    Ya,
    Yae,
    Eo,
    E,
    Yeo,
    Ye,
    O,
    OA,
    OAe,
    OI,
    Yo,
    U,
    UEo,
    UE,
    UI,
    Yu,
    Eu,
    EuI,
    I,

    None,
    Any
  };

  ////
  // Hangul jamo conversion methods
  ////
  /**
   * Converts a positional (cho/jung/jong-seong) unicode jamo codepoint to
   * a HangulJamo.
   */
  static HangulJamo HangulJamoFromPositionalUnicode(uint32_t codepoint);

  /**
   * Checks whether a codepoint is a unicode positional jamo
   */
  static bool isPositionalUnicodeJamoCodepoint(uint32_t codepoint);

  ////
  // Constructor/destructor methods
  ////
  /**
   * Creates a new empty character.
   */
  Character();
  /**
   * Creates a new character from a UTF-8 encoded C string.
   */
  Character(const char *str);

  /**
   * Creates a new character from a UTF-8 encoded std::string.
   */
  Character(const std::string &str);

  /**
   * Creates a new character from a Unicode codepoint.
   */
  Character(uint32_t codepoint);

  /**
   * Creates a new Hangul jamo.
   */
  Character(HangulJamo jamo);

  /**
   * Creates a new Hangul syllable by combining jamos.
   */
  Character(HangulJamo choseong, HangulJamo jungseong, HangulJamo jongseong);

  /**
   * Creates a new Hangul syllable given its Hangul syllable code..
   */
  Character(HangulSyllableCode syllableCode);

  /**
   * Copies the character.
   */
  Character(const Character &other);

  /**
   * Destroys the character.
   */
  ~Character();

  ////
  // General methods
  ////
  /**
   * Returns the character type.
   * \ret The character type.
   */
  Type type() const;

  /**
   * Returns the Unicode codepoint representation of the character.
   * \ret The Unicode codepoint.
   */
  uint32_t unicodeCodepoint() const;

  /**
   * Sets the character to a specific Unicode codepoint.
   * \param codepoint The Unicode codepoint.
   */
  void setUnicodeCodepoint(uint32_t unicodeCodepoint);

  /**
   * Performs a comparison with another character.
   * \ret True if the characters are equal, false otherwise.
   */
  bool operator==(const Character &other) const;

  ////
  // HangulSyllable character type
  ////
  /**
   * Returns the cheoseong of a Hangul syllable.
   * Only supported on the HangulSyllable character type.
   * \ret The cheoseong.
   * \throws std::invalid_argument if the jamo passed is not a valid choseong.
   */
  HangulJamo choseong() const;

  /**
   * Sets the cheoseong of a Hangul syllable.
   * Only supported on the HangulSyllable character type.
   * \param choseong The cheoseong.
   */
  void setChoseong(HangulJamo choseong);

  /**
   * Returns the jungseong of a Hangul syllable.
   * Only supported on the HangulSyllable character type.
   * \ret The jungseong.
   */
  HangulJamo jungseong() const;

  /**
   * Sets the jungseong of a Hangul syllable.
   * Only supported on the HangulSyllable character type.
   * \param jungseong The jungseong.
   */
  void setJungseong(HangulJamo jungseong);

  /**
   * Returns the jongseong of a Hangul syllable.
   * Only supported on the HangulSyllable character type.
   * \ret The jongseong.
   */
  HangulJamo jongseong() const;

  /**
   * Sets the cheoseong of a Hangul syllable.
   * Only supported on the HangulSyllable character type.
   * \param jungseong The jungseong.
   */
  void setJongseong(HangulJamo jongseong);

  ////
  // HangulJamo character type
  ////
  /**
   * Returns the HangulJamo enum representation of a Hangul jamo.
   * Only supported on the HangulJamo character type.
   * \ret The Hangul jamo.
   */
  HangulJamo hangulJamo() const;

  /**
   * Sets the Hangul jamo to a jamo represented by the HangulJamo enum.
   * Only supported on the HangulJamo character type.
   * \param jamo The Hangul jamo.
   */
  void setHangulJamo(HangulJamo jamo);

  ////
  // HangulSyllable and EncapsulatedNonHangulSyllable character type
  ////
  /**
   * Returns a corresponding Hangul syllable code.
   * Only supported on the HangulSyllable and EncapsulatedNonHangulSyllable
   * character type.
   * \ret The Hangul syllable code.
   */
  HangulSyllableCode hangulSyllableCode() const;

  /**
   * Sets the character to a corresponding Hangul syllable code.
   * Only supported on the HangulSyllable and EncapsulatedNonHangulSyllable
   * character type.
   * \param code The Hangul syllable code.
   */
  void setToHangulSyllableCode(HangulSyllableCode code);
};

class CharacterRef {
 private:
  struct CharacterRefImpl;
  std::unique_ptr<CharacterRefImpl> _impl;

 public:
  CharacterRef();
  ~CharacterRef();
  CharacterRef(CharacterDataType *ptr);
  CharacterRef(const CharacterRef &other);
  CharacterRef &pointTo(CharacterDataType *ptr);
  CharacterRef &operator=(CharacterDataType *ptr);
  CharacterRef &operator=(const CharacterRef &other);
  bool operator==(const CharacterDataType *ptr) const;
  bool operator==(const CharacterRef &other) const;
  bool operator!=(const CharacterDataType *ptr) const;
  bool operator!=(const CharacterRef &other) const;
  CharacterRef &operator++();
  CharacterRef operator++(int);
  CharacterRef &operator--();
  CharacterRef operator--(int);
  CharacterRef operator+(size_t amount);
  CharacterRef operator-(size_t amount);
  size_t operator-(const CharacterRef &other);
  Character *operator->() const;
  Character *operator*() const;
  CharacterDataType *get() const;
};
}

#endif  // NSL_CHARACTER_H
