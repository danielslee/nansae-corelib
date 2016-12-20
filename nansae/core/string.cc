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

#include "nansae/core/string.h"
#include "nansae/core/character.h"

#include <array>
#include <boost/algorithm/string/predicate.hpp>
#include <cassert>
#include <codecvt>
#include <string>

namespace NSL {
struct String::EncapsulatedNonHangul::EncapsulatedNonHangulImpl {
  std::vector<std::u32string> _nonHangulStrings;
  void copyFrom(const EncapsulatedNonHangulImpl &other) {
    _nonHangulStrings = other._nonHangulStrings;
  }
};

String::EncapsulatedNonHangul::EncapsulatedNonHangul()
    : _impl(new EncapsulatedNonHangulImpl()) {}

String::EncapsulatedNonHangul::EncapsulatedNonHangul(
    const EncapsulatedNonHangul &other) {
  _impl = std::unique_ptr<EncapsulatedNonHangulImpl>(
      new EncapsulatedNonHangulImpl());
  _impl->copyFrom(*other._impl);
}

String::EncapsulatedNonHangul &String::EncapsulatedNonHangul::operator=(
    const NSL::String::EncapsulatedNonHangul &other) {
  _impl->copyFrom(*other._impl);
  return *this;
}

String::EncapsulatedNonHangul::~EncapsulatedNonHangul() = default;
struct String::StringImpl {
  /**
   * The internal string representation, roughly corresponds to UTF-32.
   */
  std::u32string _str;

  const static uint32_t EncapsulatedNonHangulCode = UINT32_MAX - 1;

  std::size_t _hash();

  void _normalize() {
    for (char32_t &ch : _str) {
      if (Character::isPositionalUnicodeJamoCodepoint(ch)) {
        CharacterRef chRef((uint32_t *)&ch);
        chRef->setUnicodeCodepoint(ch);  // should trigger conversion
      }
    }
  }

  StringImpl() {}
  StringImpl(const Character &c);
  StringImpl(const CharacterRef &c);
  StringImpl(const char *str);
  StringImpl(const std::string &str);
  StringImpl(const HangulString &hstr);
  StringImpl(const StringImpl &other);
  StringImpl &operator=(const StringImpl &other);
  StringImpl(StringImpl &&other);
  StringImpl &operator=(StringImpl &&other);
  StringImpl &append(const StringImpl &str);
  StringImpl &append(const Character &c);
  StringImpl &append(const CharacterRef &c);
  StringImpl &prepend(const StringImpl &str);
  StringImpl &prepend(const Character &c);
  StringImpl &prepend(const CharacterRef &c);
  StringImpl substring(int start, int end) const;
  StringImpl &clear();
  int length() const;
  bool compareTo(const StringImpl &other) const;
  CharacterRef characterAt(int i) const;
  bool startsWith(const StringImpl &str) const;
  EncapsulatedNonHangul encapsulateNonHangul();
  StringImpl &restoreNonHangul(
      const EncapsulatedNonHangul &encapsulatedNonHangul);
  HangulString toHangulString() const;
  bool isPureHangul() const;
  std::string toStdString() const;
  std::vector<int> findMatchesEndingWithJamo(int startingIndex,
                                             Character::HangulJamo jamo);
  CharacterRef begin() const;
  CharacterRef end() const;
};

String::StringImpl::StringImpl(const Character &c) {
  _str.resize(1, c.unicodeCodepoint());
  _normalize();
}

String::StringImpl::StringImpl(const CharacterRef &c) {
  _str.resize(1, c->unicodeCodepoint());
  _normalize();
}

String::StringImpl::StringImpl(const char *str)
    : StringImpl(std::string(str)) {}

String::StringImpl::StringImpl(const std::string &str) {
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
  _str = converter.from_bytes(str);
  _normalize();
}

String::StringImpl::StringImpl(const HangulString &hstr) {
  uint32_t hangulSyllableCodepoint;
  std::array<uint8_t, 3> jamos;
  int jamoCounter = 0;

  for (char c : hstr.theString) {
    if (c == HangulString::NonHangulCode) {
      _str.append(1, StringImpl::EncapsulatedNonHangulCode);
      continue;
    }

    jamos[jamoCounter] = c - 1;
    jamoCounter++;

    if (jamoCounter == 3) {
      hangulSyllableCodepoint =
          0xac00 + jamos[0] * 0x24c + jamos[1] * 0x1c + jamos[2];
      _str.append(1, hangulSyllableCodepoint);
      jamoCounter = 0;
    }
  }
}

String::StringImpl::StringImpl(const StringImpl &other) { _str = other._str; }

String::StringImpl &String::StringImpl::operator=(
    const String::StringImpl &other) {
  _str = other._str;
  return *this;
}

String::StringImpl::StringImpl(String::StringImpl &&other) {
  _str = std::move(other._str);
}

String::StringImpl &String::StringImpl::operator=(String::StringImpl &&other) {
  _str = std::move(other._str);
  return *this;
}

String::StringImpl &String::StringImpl::append(const String::StringImpl &str) {
  _str.append(str._str);
  return *this;
}

String::StringImpl &String::StringImpl::append(const Character &c) {
  _str.append(1, c.unicodeCodepoint());
  _normalize();
  return *this;
}

String::StringImpl &String::StringImpl::append(const CharacterRef &c) {
  _str.append(1, c->unicodeCodepoint());
  _normalize();
  return *this;
}

String::StringImpl &String::StringImpl::prepend(const String::StringImpl &str) {
  _str.insert(0, str._str);
  return *this;
}

String::StringImpl &String::StringImpl::prepend(const Character &c) {
  _str.insert(0, 1, c.unicodeCodepoint());
  _normalize();
  return *this;
}

String::StringImpl &String::StringImpl::prepend(const CharacterRef &c) {
  _str.insert(0, 1, c->unicodeCodepoint());
  _normalize();
  return *this;
}

String::StringImpl String::StringImpl::substring(int start, int end) const {
  assert(end >= start);
  StringImpl substring;
  substring._str = _str.substr(start, end - start + 1);
  return substring;
}

String::StringImpl &String::StringImpl::clear() {
  _str.clear();
  return *this;
}

int String::StringImpl::length() const { return _str.length(); }

bool String::StringImpl::compareTo(const String::StringImpl &other) const {
  return (_str == other._str);
}

CharacterRef String::StringImpl::characterAt(int i) const {
  assert(i >= 0);
  assert(i < _str.length());
  return CharacterRef((uint32_t *)&_str[i]);
}

bool String::StringImpl::startsWith(const String::StringImpl &str) const {
  return boost::starts_with(_str, str._str);
}

String::EncapsulatedNonHangul String::StringImpl::encapsulateNonHangul() {
  EncapsulatedNonHangul enh;
  std::u32string buffer;
  std::u32string::iterator startPos;
  for (auto i = _str.begin(); i != _str.end(); ++i) {
    // if non hangul syllable
    if (!(*i >= 0xac00 && *i <= 0xd7af)) {
      if (buffer.length() == 0) {
        startPos = i;
      }
      buffer.append(1, *i);
    } else if (buffer.length() > 0) {
      enh._impl->_nonHangulStrings.push_back(buffer);
      _str.replace(startPos, startPos + buffer.length(), 1,
                   EncapsulatedNonHangulCode);
      i -= buffer.length();
      buffer.clear();
    }
  }

  if (buffer.length() > 0) {
    enh._impl->_nonHangulStrings.push_back(buffer);
    _str.replace(startPos, _str.end(), 1, EncapsulatedNonHangulCode);
  }

  return enh;
}

String::StringImpl &String::StringImpl::restoreNonHangul(
    const String::EncapsulatedNonHangul &encapsulatedNonHangul) {
  auto nonHangulStringsIter =
      encapsulatedNonHangul._impl->_nonHangulStrings.begin();
  auto nonHangulStringsEnd =
      encapsulatedNonHangul._impl->_nonHangulStrings.end();
  for (auto i = _str.begin(); i != _str.end(); ++i) {
    if (*i == EncapsulatedNonHangulCode) {
      _str.replace(i, i + 1, *nonHangulStringsIter);
      nonHangulStringsIter++;
      if (nonHangulStringsIter == nonHangulStringsEnd) break;
    }
  }
  return *this;
}

HangulString String::StringImpl::toHangulString() const {
  HangulString hstr;
  for (char32_t c : _str) {
    // if non hangul syllable
    if (!(c >= 0xac00 && c <= 0xd7af) && c != EncapsulatedNonHangulCode) {
      throw CannotConvertContainsNonHangulSyllableSymbolsException();
    } else {
      if (c == EncapsulatedNonHangulCode)
        hstr.theString.append(1, HangulString::NonHangulCode);
      else {
        uint8_t choseong = (c - 0xac00) / 0x24c;
        uint8_t jungseong = ((c - 0xac00) % 0x24c) / 0x1c;
        uint8_t jongseong = ((c - 0xac00) % 0x24c) % 0x1c;
        hstr.theString.append(1, choseong + 1);
        hstr.theString.append(1, jungseong + 1);
        hstr.theString.append(1, jongseong + 1);
      }
    }
  }
  return hstr;
}

bool String::StringImpl::isPureHangul() const {
  for (char32_t c : _str) {
    // if non hangul syllable
    if (!(c >= 0xac00 && c <= 0xd7af) && c != EncapsulatedNonHangulCode) {
      return false;
    }
  }
  return true;
}

std::string String::StringImpl::toStdString() const {
  std::u32string utf32Str = _str;
  for (char32_t &c : utf32Str) {
    if (c == EncapsulatedNonHangulCode) c = 'S';
  }

  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
  std::string utf8Str = converter.to_bytes(utf32Str);
  return utf8Str;
}

std::vector<int> String::StringImpl::findMatchesEndingWithJamo(
    int startingIndex, Character::HangulJamo jamo) {
  std::vector<int> result;

  CharacterRef c;
  for (auto iter = _str.begin() + startingIndex; iter != _str.end(); ++iter) {
    c = (CharacterDataType *)&(*iter);
    if (((c->type() == Character::Type::HangulSyllable) &&
         (c->jungseong() == jamo || c->jongseong() == jamo)) ||
        ((c->type() == Character::Type::HangulJamo) &&
         (c->hangulJamo() == jamo)))
      result.push_back(iter - (_str.begin() + startingIndex));
  }
  return result;
}

std::size_t String::StringImpl::_hash() {
  return std::hash<std::u32string>()(_str);
}

CharacterRef String::StringImpl::begin() const {
  return CharacterRef((uint32_t *)&_str[0]);
}

CharacterRef String::StringImpl::end() const {
  return CharacterRef((uint32_t *)&_str[0] + _str.length());
}

////
// Public interface mapping
////
String::String() : _impl(new StringImpl()) {}
String::String(const Character &c) : _impl(new StringImpl(c)) {}
String::String(const CharacterRef &c) : _impl(new StringImpl(c)) {}
String::String(const char *str) : _impl(new StringImpl(str)) {}
String::String(const std::string &str) : _impl(new StringImpl(str)) {}
String::String(const HangulString &str) : _impl(new StringImpl(str)) {}
String::String(const String &other) : _impl(new StringImpl(*other._impl)) {}
String &String::operator=(const String &other) {
  _impl = std::unique_ptr<StringImpl>(new StringImpl(*other._impl));
  return *this;
}
String::String(String &&other) { _impl = std::move(other._impl); }

String::~String() = default;

String &String::operator=(String &&other) {
  _impl = std::move(other._impl);
  return *this;
}

String &String::append(const String &str) {
  _impl->append(*str._impl);
  return *this;
}

String &String::append(const Character &c) {
  _impl->append(c);
  return *this;
}

String &String::append(const CharacterRef &c) {
  _impl->append(c);
  return *this;
}

String &String::prepend(const String &str) {
  _impl->prepend(*str._impl);
  return *this;
}

String &String::prepend(const Character &c) {
  _impl->prepend(c);
  return *this;
}

String &String::prepend(const CharacterRef &c) {
  _impl->prepend(c);
  return *this;
}

String String::substring(int start, int end) const {
  String substr;
  *(substr._impl.get()) = _impl->substring(start, end);
  return substr;
}

String &String::clear() {
  _impl->clear();
  return *this;
}

int String::length() const { return _impl->length(); }

bool String::operator==(const String &other) const {
  return _impl->compareTo(*other._impl);
}

CharacterRef String::characterAt(int i) const { return _impl->characterAt(i); }

bool String::startsWith(const String &str) const {
  return _impl->startsWith(*str._impl);
}

String::EncapsulatedNonHangul String::encapsulateNonHangul() {
  return _impl->encapsulateNonHangul();
}

String &String::restoreNonHangul(
    const String::EncapsulatedNonHangul &encapsulatedNonHangul) {
  _impl->restoreNonHangul(encapsulatedNonHangul);
  return *this;
}

HangulString String::toHangulString() const { return _impl->toHangulString(); }

bool String::isPureHangul() const { return _impl->isPureHangul(); }

std::string String::toStdString() const { return _impl->toStdString(); }

std::vector<int> String::findMatchesEndingWithJamo(int startingIndex,
                                                   Character::HangulJamo jamo) {
  return _impl->findMatchesEndingWithJamo(startingIndex, jamo);
}

CharacterRef String::begin() const { return _impl->begin(); }
CharacterRef String::end() const { return _impl->end(); }
}

namespace std {
size_t hash<NSL::String>::operator()(const NSL::String &str) const {
  return str._impl->_hash();
}
}
