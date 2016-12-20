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
#include "gtest/gtest.h"

#include <iostream>

TEST(String, substring) {
  NSL::String str(u8"김정은개새끼");
  ASSERT_EQ(str.substring(1, 2), NSL::String(u8"정은"));
}

TEST(String, characterAt) {
  NSL::String str(u8"김정은개새끼");
  ASSERT_EQ(str.characterAt(3)->unicodeCodepoint(), 44060);  // 44060 - 개
}

TEST(String, prepend) {
  NSL::String str = NSL::String(u8"개새끼").prepend(NSL::String(u8"김정은"));
  ASSERT_EQ(str, NSL::String(u8"김정은개새끼"));
}

TEST(String, append) {
  NSL::String str = NSL::String(u8"김정은").append(NSL::String(u8"개새끼"));
  ASSERT_EQ(str, NSL::String(u8"김정은개새끼"));
}

TEST(String, jamo) {
  NSL::String str(u8"ᆫ가");
  ASSERT_EQ(NSL::String(str.characterAt(0)), NSL::String(u8"ㄴ"));
  ASSERT_EQ(NSL::String(str.characterAt(1)), NSL::String(u8"가"));
  ASSERT_EQ(str, NSL::String(u8"ㄴ가"));
}

TEST(String, copyEncapsulatedNonHangul) {
  NSL::String::EncapsulatedNonHangul copyTarget;
  NSL::String str(u8"latin한글");
  NSL::String str2(u8"latin한글");

  str2.encapsulateNonHangul();
  auto enh = str.encapsulateNonHangul();

  copyTarget = NSL::String::EncapsulatedNonHangul(enh);

  str.restoreNonHangul(copyTarget);
  ASSERT_EQ(str, NSL::String(u8"latin한글"));

  str2.restoreNonHangul(enh);
  ASSERT_EQ(str2, NSL::String(u8"latin한글"));
}
TEST(String, hangulString) {
  NSL::String string = NSL::String(u8"안녕하세요");
  NSL::HangulString hstr = string.toHangulString();
  NSL::String restored = NSL::String(hstr);
  ASSERT_EQ(restored, NSL::String(u8"안녕하세요"));
}

TEST(String, encapsulateNonHangulSyllableSymbols) {
  NSL::String str(u8"安寧하세요");
  NSL::String::EncapsulatedNonHangul enh = str.encapsulateNonHangul();
  ASSERT_EQ(str.toStdString(), u8"S하세요");
  str.restoreNonHangul(enh);
  ASSERT_EQ(str.toStdString(), u8"安寧하세요");

  NSL::String str2("latin한글漢字한글ㅈㅏㅁㅗ");
  NSL::String::EncapsulatedNonHangul enh2 = str2.encapsulateNonHangul();
  ASSERT_EQ(str2.toStdString(), u8"S한글S한글S");
  str2.restoreNonHangul(enh2);
  ASSERT_EQ(str2.toStdString(), "latin한글漢字한글ㅈㅏㅁㅗ");
}

TEST(String, hangulSyllableDecomposition) {
  NSL::String str(u8"안녕");
  ASSERT_EQ(str.characterAt(0)->choseong(), NSL::Character::HangulJamo::Ieung);
  ASSERT_EQ(str.characterAt(0)->jungseong(), NSL::Character::HangulJamo::A);
  ASSERT_EQ(str.characterAt(0)->jongseong(), NSL::Character::HangulJamo::Nieun);
  ASSERT_EQ(str.characterAt(1)->choseong(), NSL::Character::HangulJamo::Nieun);
  ASSERT_EQ(str.characterAt(1)->jungseong(), NSL::Character::HangulJamo::Yeo);
  ASSERT_EQ(str.characterAt(1)->jongseong(), NSL::Character::HangulJamo::Ieung);
}

TEST(String, startsWith) {
  NSL::String str(u8"안녕하세요");
  ASSERT_EQ(str.startsWith(u8"안녕"), true);
  ASSERT_EQ(str.startsWith(u8"다른거"), false);
}
