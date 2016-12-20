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

#include "nansae/core/character.h"
#include "gtest/gtest.h"

#include <iostream>

////
// HangulSyllable
////
TEST(Character, HangulJamoToCodepoint) {
  NSL::Character syllable(NSL::Character::HangulJamo::Hieut,
                          NSL::Character::HangulJamo::A,
                          NSL::Character::HangulJamo::Nieun);  // 한

  ASSERT_EQ(syllable.unicodeCodepoint(), 54620);
}

TEST(Character, CodepointToHangulJamo) {
  NSL::Character syllable(54620);  // 한
  ASSERT_EQ(syllable.type(), NSL::Character::Type::HangulSyllable);
  ASSERT_EQ(syllable.choseong(), NSL::Character::HangulJamo::Hieut);
  ASSERT_EQ(syllable.jungseong(), NSL::Character::HangulJamo::A);
  ASSERT_EQ(syllable.jongseong(), NSL::Character::HangulJamo::Nieun);
}

////
// HangulJamo
////
TEST(Character, PositionalToJamoConversion) {
  NSL::Character jamo(0x11af);  // ᆯ (unicode jamo)
  ASSERT_EQ(jamo.hangulJamo(), NSL::Character::HangulJamo::Rieul);
  ASSERT_EQ(jamo, NSL::Character(u8"ㄹ"));  // compatibility jamo
  jamo.setUnicodeCodepoint(0x3139);         // ㄹ (compatibility jamo)
  ASSERT_EQ(jamo.hangulJamo(), NSL::Character::HangulJamo::Rieul);
  ASSERT_EQ(jamo, NSL::Character(u8"ㄹ"));  // compatibility jamo
}
