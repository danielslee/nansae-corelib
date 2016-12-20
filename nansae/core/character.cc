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

#include <codecvt>
#include <map>

#include "nansae/core/character.h"

namespace NSL {
struct Character::CharacterImpl {
  /**
   * The place to store the underlying data for the character when we're the
   * owner, i.e. a Character instance as opposed to a CharacterRef one.
   */
  CharacterDataType _ownedData;

  /**
   * The pointer used for accessing the underlaying data for the character.
   * Points to _ownedData by default.
   */
  CharacterDataType *_data = &_ownedData;

  /**
   * Points the _data pointer to a given location.
   */
  void _pointTo(CharacterDataType *ptr) { _data = ptr; }

  /**
   * Returns the _data pointer, nullptr in case we're using _ownedData
   */
  CharacterDataType *_ptr() {
    if (_data == &_ownedData)
      return nullptr;
    else
      return _data;
  }

  /**
   * Initializes the type and replaces the current value with it.
   */
  void _initializeType(Type type);

  /**
   * The code used to represent encapsulated non-Hangul characters.
   */
  const static uint32_t EncapsulatedNonHangulCode = UINT32_MAX - 1;

////
// Conversions
////

// [conversion arrays generated using 'generate_Character.cpp_conversions.rb']
#define CI UINT8_MAX
  static constexpr uint8_t _compToChoseong[] = {
      0,  1,  CI, 2,  CI, CI, 3,  4,  5,  CI, CI, CI, CI, CI, CI, CI, 6,  7,
      8,  CI, 9,  10, 11, 12, 13, 14, 15, 16, 17, 18, CI, CI, CI, CI, CI, CI,
      CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI,
  };
  static constexpr uint8_t _choseongToComp[] = {
      0, 1, 3, 6, 7, 8, 16, 17, 18, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
  };
  static constexpr uint8_t _compToJungseong[] = {
      CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI,
      CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, 0,  1,  2,  3,  4,  5,
      6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, CI, CI,
  };
  static constexpr uint8_t _jungseongToComp[] = {
      30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
      41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
  };
  static constexpr uint8_t _compToJongseong[] = {
      1,  2,  3,  4,  5,  6,  7,  CI, 8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
      CI, 18, 19, 20, 21, 22, CI, 23, 24, 25, 26, 27, CI, CI, CI, CI, CI, CI,
      CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, CI, 0,  CI,
  };
  static constexpr uint8_t _jongseongToComp[] = {
      51, 0,  1,  2,  3,  4,  5,  6,  8,  9,  10, 11, 12, 13,
      14, 15, 16, 17, 19, 20, 21, 22, 23, 25, 26, 27, 28, 29,
  };
  // [/conversion arrays generated using
  // 'generate_Character.cpp_conversions.rb']

  static inline uint8_t _hangulJamoToChoseong(HangulJamo jamo) {
    return _compToChoseong[static_cast<int>(jamo)];
  }
  static inline uint8_t _hangulJamoToJungseong(HangulJamo jamo) {
    return _compToJungseong[static_cast<int>(jamo)];
  }
  static inline uint8_t _hangulJamoToJongseong(HangulJamo jamo) {
    return _compToJongseong[static_cast<int>(jamo)];
  }

  static inline HangulJamo _choseongToHangulJamo(uint8_t choseong) {
    return static_cast<HangulJamo>(_choseongToComp[choseong]);
  }
  static inline HangulJamo _jungseongToHangulJamo(uint8_t jungseong) {
    return static_cast<HangulJamo>(_jungseongToComp[jungseong]);
  }
  static inline HangulJamo _jongseongToHangulJamo(uint8_t jongseong) {
    return static_cast<HangulJamo>(_jongseongToComp[jongseong]);
  }

  static inline HangulJamo _compatibilityToHangulJamo(uint8_t compatibility) {
    return static_cast<HangulJamo>(compatibility);
  }
  static inline uint8_t _hangulJamoToCompatibility(HangulJamo jamo) {
    return static_cast<uint8_t>(jamo);
  }

  static bool isPositionalUnicodeJamoCodepoint(uint32_t codepoint) {
    return ((codepoint >= 0x1100 && codepoint <= 0x1112) ||
            (codepoint >= 0x1161 && codepoint <= 0x1175) ||
            (codepoint >= 0x11a8 && codepoint <= 0x11c2));
  }

  static HangulJamo HangulJamoFromPositionalUnicode(uint32_t codepoint) {
    if (codepoint >= 0x1100 && codepoint <= 0x1112) {
      uint8_t choseongCode = codepoint - 0x1100;
      return _choseongToHangulJamo(choseongCode);
    } else if (codepoint >= 0x1161 && codepoint <= 0x1175) {
      uint8_t jungseongCode = codepoint - 0x1161;
      return _jungseongToHangulJamo(jungseongCode);
    } else if (codepoint >= 0x11a8 && codepoint <= 0x11c2) {
      // 'none' is not included in unicode postitional jamo -> + 1
      uint8_t jongseongCode = codepoint - 0x11a8 + 1;
      return _jongseongToHangulJamo(jongseongCode);
    } else {
      throw std::invalid_argument(
          "The codepoint isn't a valid unicode positional jamo");
    }
  }
  ////
  // Public method implementation
  ////

  // Constructor/destructor methods
  CharacterImpl();
  CharacterImpl(const std::string &str);
  CharacterImpl(const char *str) : CharacterImpl(std::string(str)) {}
  CharacterImpl(uint32_t codepoint);
  CharacterImpl(HangulJamo jamo);
  CharacterImpl(HangulJamo choseong, HangulJamo jungseong,
                HangulJamo jongseong);
  CharacterImpl(HangulSyllableCode syllableCode);
  void copyFrom(const CharacterImpl &other);

  // General methods
  Type type() const;
  uint32_t unicodeCodepoint() const;
  void setUnicodeCodepoint(uint32_t unicodeCodepoint);
  bool isEqualTo(const CharacterImpl &other) const;

  // HangulSyllable character type
  HangulSyllableCode hangulSyllableCode() const;
  void setToHangulSyllableCode(HangulSyllableCode code);
  HangulJamo choseong() const;
  void setChoseong(HangulJamo choseong);
  HangulJamo jungseong() const;
  void setJungseong(HangulJamo jungseong);
  HangulJamo jongseong() const;
  void setJongseong(HangulJamo jongseong);

  // HangulJamo character type
  HangulJamo hangulJamo() const;
  void setHangulJamo(HangulJamo jamo);
};
// to make the linker happy
constexpr uint8_t Character::CharacterImpl::_compToChoseong[53];
constexpr uint8_t Character::CharacterImpl::_choseongToComp[19];
constexpr uint8_t Character::CharacterImpl::_compToJungseong[53];
constexpr uint8_t Character::CharacterImpl::_jungseongToComp[21];
constexpr uint8_t Character::CharacterImpl::_compToJongseong[53];
constexpr uint8_t Character::CharacterImpl::_jongseongToComp[28];

////
// Private methods
////

void Character::CharacterImpl::_initializeType(Character::Type type) {
  if (type == Type::HangulSyllable)
    setUnicodeCodepoint(0xac00);
  else if (type == Type::HangulJamo)
    setUnicodeCodepoint(0x3131);
  else if (type == Type::EncapsulatedNonHangulSyllable)
    setUnicodeCodepoint(EncapsulatedNonHangulCode);
  else
    setUnicodeCodepoint(0x0);
}

////
// Constructor/destructor methods
////

Character::CharacterImpl::CharacterImpl() { _initializeType(Type::Character); }

Character::CharacterImpl::CharacterImpl(const std::string &str) {
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
  std::u32string codepoints = converter.from_bytes(str);
  setUnicodeCodepoint(codepoints[0]);
}

Character::CharacterImpl::CharacterImpl(uint32_t codepoint) {
  setUnicodeCodepoint(codepoint);
}

Character::CharacterImpl::CharacterImpl(HangulJamo jamo) {
  _initializeType(Type::HangulJamo);
  setHangulJamo(jamo);
}

Character::CharacterImpl::CharacterImpl(HangulJamo choseong,
                                        HangulJamo jungseong,
                                        HangulJamo jongseong) {
  uint8_t newChoseong = _hangulJamoToChoseong(choseong);
  if (newChoseong == CI)
    throw std::invalid_argument("The choseong passed is not a valid choseong.");

  uint8_t newJungseong = _hangulJamoToJungseong(jungseong);
  if (newJungseong == CI)
    throw std::invalid_argument(
        "The jungseong passed is not a valid jungseong.");

  uint8_t newJongseong = _hangulJamoToJongseong(jongseong);
  if (newJongseong == CI)
    throw std::invalid_argument(
        "The jongseong passed is not a valid jongseong.");

  setUnicodeCodepoint(0xac00 + newChoseong * 0x24c + newJungseong * 0x1c +
                      newJongseong);
}

Character::CharacterImpl::CharacterImpl(
    Character::HangulSyllableCode syllableCode) {
  setToHangulSyllableCode(syllableCode);
}

void Character::CharacterImpl::copyFrom(
    const NSL::Character::CharacterImpl &other) {
  // using data stored inside the CharacterImpl class, i.e. a Character (as
  // opposed to CharacterImpl)
  if (other._data == &other._ownedData) {
    _ownedData = other._ownedData;
    _data = &_ownedData;
  } else {  // otherwise just copy the pointer, CharacterRef is a pointer
    _data = other._data;
  }
}

////
// General methods
////
Character::Type Character::CharacterImpl::type() const {
  uint32_t cp = unicodeCodepoint();
  if (cp >= 0xac00 && cp <= 0xd7af) return Type::HangulSyllable;
  // we only support Hangul compatibility jamo for now
  else if (cp >= 0x3131 && cp <= 0x3163)
    return Type::HangulJamo;
  else if (cp == EncapsulatedNonHangulCode)
    return Type::EncapsulatedNonHangulSyllable;
  else
    return Type::Character;
}

uint32_t Character::CharacterImpl::unicodeCodepoint() const { return *_data; }

void Character::CharacterImpl::setUnicodeCodepoint(uint32_t unicodeCodepoint) {
  if (isPositionalUnicodeJamoCodepoint(unicodeCodepoint)) {
    _initializeType(Type::HangulJamo);
    setHangulJamo(HangulJamoFromPositionalUnicode(unicodeCodepoint));
  } else {
    *_data = unicodeCodepoint;
  }
}

bool Character::CharacterImpl::isEqualTo(
    const Character::CharacterImpl &other) const {
  return (*_data == *other._data);
}

////
// HangulSyllable character type
////

Character::HangulJamo Character::CharacterImpl::choseong() const {
  uint32_t cp = unicodeCodepoint();
  uint8_t choseong = (cp - 0xac00) / 0x24c;

  return _choseongToHangulJamo(choseong);
}

void Character::CharacterImpl::setChoseong(Character::HangulJamo choseong) {
  uint32_t cp = unicodeCodepoint();
  uint8_t newChoseong = _hangulJamoToChoseong(choseong);
  if (newChoseong == CI)
    throw std::invalid_argument("The jamo passed is not a valid choseong.");
  uint8_t oldChoseong = (cp - 0xac00) / 0x24c;

  cp -= oldChoseong * 0x24c;
  cp += newChoseong * 0x24c;
  setUnicodeCodepoint(cp);
}

Character::HangulJamo Character::CharacterImpl::jungseong() const {
  uint32_t cp = unicodeCodepoint();
  uint8_t jungseong = ((cp - 0xac00) % 0x24c) / 0x1c;

  return _jungseongToHangulJamo(jungseong);
}

void Character::CharacterImpl::setJungseong(Character::HangulJamo jungseong) {
  uint32_t cp = unicodeCodepoint();
  uint8_t newJungseong = _hangulJamoToJungseong(jungseong);
  if (newJungseong == CI)
    throw std::invalid_argument("The jamo passed is not a valid jungseong.");
  uint8_t oldJungseong = ((cp - 0xac00) % 0x24c) / 0x1c;

  cp -= oldJungseong * 0x1c;
  cp += newJungseong * 0x1c;
  setUnicodeCodepoint(cp);
}

Character::HangulJamo Character::CharacterImpl::jongseong() const {
  uint32_t cp = unicodeCodepoint();
  uint8_t jongseong = ((cp - 0xac00) % 0x24c) % 0x1c;

  return _jongseongToHangulJamo(jongseong);
}

void Character::CharacterImpl::setJongseong(Character::HangulJamo jongseong) {
  uint32_t cp = unicodeCodepoint();
  uint8_t newJongseong = _hangulJamoToJongseong(jongseong);
  if (newJongseong == CI)
    throw std::invalid_argument("The jamo passed is not a valid jongseong.");

  uint8_t oldJongseong = ((cp - 0xac00) % 0x24c) % 0x1c;

  cp -= oldJongseong;
  cp += newJongseong;
  setUnicodeCodepoint(cp);
}

////
// HangulJamo character type
////

Character::HangulJamo Character::CharacterImpl::hangulJamo() const {
  return _compatibilityToHangulJamo(unicodeCodepoint() - 0x3131);
}

void Character::CharacterImpl::setHangulJamo(Character::HangulJamo hangulJamo) {
  uint32_t cp = _hangulJamoToCompatibility(hangulJamo) + 0x3131;
  setUnicodeCodepoint(cp);
}

////
// HangulSyllable and EncapsulatedNonHangulSyllable
////

Character::HangulSyllableCode Character::CharacterImpl::hangulSyllableCode()
    const {
  if (type() == Type::EncapsulatedNonHangulSyllable)
    return HangulSyllableCode(HangulSyllableCode::Symbol);
  else
    return HangulSyllableCode(unicodeCodepoint() - 0xac00);
}

void Character::CharacterImpl::setToHangulSyllableCode(
    Character::HangulSyllableCode code) {
  if (code.theCode == HangulSyllableCode::Symbol)
    _initializeType(Type::EncapsulatedNonHangulSyllable);
  else
    setUnicodeCodepoint(code.theCode + 0xac00);
}

////
// Public interface mapping
////

// Hangul jamo conversion methods

Character::HangulJamo Character::HangulJamoFromPositionalUnicode(
    uint32_t codepoint) {
  return CharacterImpl::HangulJamoFromPositionalUnicode(codepoint);
}

bool Character::isPositionalUnicodeJamoCodepoint(uint32_t codepoint) {
  return CharacterImpl::isPositionalUnicodeJamoCodepoint(codepoint);
}

// Constructor/destructor methods
Character::Character() : _impl(new CharacterImpl()) {}
Character::Character(const char *str) : _impl(new CharacterImpl(str)) {}
Character::Character(const std::string &str) : _impl(new CharacterImpl(str)) {}
Character::Character(uint32_t codepoint)
    : _impl(new CharacterImpl(codepoint)) {}
Character::Character(HangulJamo jamo) : _impl(new CharacterImpl(jamo)) {}
Character::Character(HangulJamo choseong, HangulJamo jungseong,
                     HangulJamo jongseong)
    : _impl(new CharacterImpl(choseong, jungseong, jongseong)) {}
Character::Character(HangulSyllableCode syllableCode)
    : _impl(new CharacterImpl(syllableCode)) {}
Character::Character(const Character &other) { _impl->copyFrom(*other._impl); }
Character::~Character() = default;

// General methods
Character::Type Character::type() const { return _impl->type(); }
uint32_t Character::unicodeCodepoint() const {
  return _impl->unicodeCodepoint();
}
void Character::setUnicodeCodepoint(uint32_t unicodeCodepoint) {
  return _impl->setUnicodeCodepoint(unicodeCodepoint);
}
bool Character::operator==(const Character &other) const {
  return _impl->isEqualTo(*other._impl);
}

// HangulSyllable character type
#define CHECK_TYPE                           \
  if (_impl->type() != Type::HangulSyllable) \
    throw OperationNotSupportedOnCharacterType(__FUNCTION__, _impl->type());

Character::HangulJamo Character::choseong() const {
  CHECK_TYPE return _impl->choseong();
}
void Character::setChoseong(Character::HangulJamo choseong) {
  CHECK_TYPE return _impl->setChoseong(choseong);
}
Character::HangulJamo Character::jungseong() const {
  CHECK_TYPE return _impl->jungseong();
}
void Character::setJungseong(Character::HangulJamo jungseong) {
  CHECK_TYPE return _impl->setJungseong(jungseong);
}

Character::HangulJamo Character::jongseong() const {
  CHECK_TYPE return _impl->jongseong();
}
void Character::setJongseong(Character::HangulJamo jongseong) {
  CHECK_TYPE return _impl->setJongseong(jongseong);
}
#undef CHECK_TYPE

// HangulJamo character type
#define CHECK_TYPE                       \
  if (_impl->type() != Type::HangulJamo) \
    throw OperationNotSupportedOnCharacterType(__FUNCTION__, _impl->type());

Character::HangulJamo Character::hangulJamo() const {
  CHECK_TYPE return _impl->hangulJamo();
}
void Character::setHangulJamo(Character::HangulJamo jamo) {
  CHECK_TYPE return _impl->setHangulJamo(jamo);
}
#undef CHECK_TYPE

// HangulSyllable and EncapsulatedNonHangulSyllable
#define CHECK_TYPE                                          \
  if (_impl->type() != Type::HangulSyllable &&              \
      _impl->type() != Type::EncapsulatedNonHangulSyllable) \
    throw OperationNotSupportedOnCharacterType(__FUNCTION__, _impl->type());

Character::HangulSyllableCode Character::hangulSyllableCode() const {
  CHECK_TYPE return _impl->hangulSyllableCode();
}

void Character::setToHangulSyllableCode(Character::HangulSyllableCode code) {
  CHECK_TYPE return _impl->setToHangulSyllableCode(code);
}
#undef CHECK_TYPE

//////
////
// CharacterRef class
////
//////

struct CharacterRef::CharacterRefImpl {
  /**
   * The underlying character instance to use for manipulating the character.
   */
  std::unique_ptr<Character> _char;

  ////
  // Public method implementation
  ////

  CharacterRefImpl() {
    _char = std::unique_ptr<Character>(new Character());
    _char->_impl->_pointTo(nullptr);
  }

  CharacterRefImpl(CharacterDataType *ptr) {
    _char = std::unique_ptr<Character>(new Character());
    _char->_impl->_pointTo(ptr);
  }

  void pointTo(CharacterDataType *ptr) {
    _char = std::unique_ptr<Character>(new Character());
    _char->_impl->_pointTo(ptr);
  }

  Character *character() const { return _char.get(); }

  CharacterDataType *get() const { return _char->_impl->_ptr(); }

  void copyFrom(const CharacterRefImpl &other) {
    _char->_impl->_pointTo(other._char->_impl->_data);
  }

  bool isEqualTo(const CharacterRefImpl &other) const {
    return (get() == other.get());
  }

  bool isEqualTo(const CharacterDataType *ptr) const { return (get() == ptr); }

  void increment() { _char->_impl->_pointTo(get() + 1); }

  void decrement() { _char->_impl->_pointTo(get() - 1); }
};

CharacterRef::CharacterRef() : _impl(new CharacterRefImpl()) {}
CharacterRef::~CharacterRef() = default;
CharacterRef::CharacterRef(CharacterDataType *ptr)
    : _impl(new CharacterRefImpl(ptr)) {}
CharacterRef::CharacterRef(const CharacterRef &other)
    : _impl(new CharacterRefImpl()) {
  _impl->copyFrom(*other._impl);
}
CharacterRef &CharacterRef::pointTo(CharacterDataType *ptr) {
  _impl->pointTo(ptr);
  return *this;
}
CharacterRef &CharacterRef::operator=(CharacterDataType *ptr) {
  _impl->pointTo(ptr);
  return *this;
}
CharacterRef &CharacterRef::operator=(const CharacterRef &other) {
  _impl->copyFrom(*other._impl);
  return *this;
}
bool CharacterRef::operator==(const CharacterDataType *ptr) const {
  return _impl->isEqualTo(ptr);
}
bool CharacterRef::operator==(const CharacterRef &other) const {
  return _impl->isEqualTo(*other._impl);
}
bool CharacterRef::operator!=(const CharacterDataType *ptr) const {
  return !(*this == ptr);
}
bool CharacterRef::operator!=(const CharacterRef &other) const {
  return !(*this == other);
}
CharacterRef &CharacterRef::operator++() {
  _impl->increment();
  return *this;
}
CharacterRef CharacterRef::operator++(int) {
  CharacterRef original(*this);
  _impl->increment();
  return original;
}
CharacterRef &CharacterRef::operator--() {
  _impl->decrement();
  return *this;
}
CharacterRef CharacterRef::operator--(int) {
  CharacterRef original(*this);
  _impl->decrement();
  return original;
}
CharacterRef CharacterRef::operator+(size_t amount) {
  CharacterRef copy(*this);
  copy._impl->pointTo(_impl->get() + amount);
  return copy;
}
CharacterRef CharacterRef::operator-(size_t amount) {
  CharacterRef copy(*this);
  copy._impl->pointTo(_impl->get() - amount);
  return copy;
}
size_t CharacterRef::operator-(const CharacterRef &other) {
  return _impl->get() - other._impl->get();
}
Character *CharacterRef::operator->() const { return _impl->character(); }
Character *CharacterRef::operator*() const { return _impl->character(); }
CharacterDataType *CharacterRef::get() const { return _impl->get(); }
}
