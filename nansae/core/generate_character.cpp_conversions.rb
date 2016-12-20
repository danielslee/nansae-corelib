#!/usr/bin/env ruby

#
# Copyright (c) 2015-2017 Daniel Shihoon Lee <daniel@nansae.im>
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

hangulJamo = ["Giyeok", "SsangGiyeok", "GiyeokSiot", "Nieun", "NieunJieut",
              "NieunHieut", "Digeut", "SsangDigeut", "Rieul", "RieulGiyeok",
              "RieulMieum", "RieulBieup", "RieulSiot", "RieulTieut", "RieulPieup",
              "RieulHieut", "Mieum", "Bieup", "SsangBieup", "BieupSiot", "Siot",
              "SsangSiot", "Ieung", "Jieut", "SsangJieut", "Chieut", "Kieuk",
              "Tieut", "Pieup", "Hieut",

              "A", "Ae", "Ya", "Yae", "Eo", "E", "Yeo", "Ye", "O", "OA", "OAe",
              "OI", "Yo", "U", "UEo", "UE", "UI", "Yu", "Eu", "EuI", "I",

              "None", "Any"]

choseong = Hash[["Giyeok", "SsangGiyeok", "Nieun", "Digeut", "SsangDigeut", "Rieul",
            "Mieum", "Bieup", "SsangBieup", "Siot", "SsangSiot", "Ieung", "Jieut",
            "SsangJieut", "Chieut", "Kieuk", "Tieut", "Pieup", "Hieut"].map.with_index.to_a]

jungseong = Hash[["A", "Ae", "Ya", "Yae", "Eo", "E", "Yeo", "Ye", "O", "OA", "OAe",
             "OI", "Yo", "U", "UEo", "UE", "UI", "Yu", "Eu", "EuI", "I"].map.with_index.to_a]

jongseong = Hash[["None", "Giyeok", "SsangGiyeok", "GiyeokSiot", "Nieun", "NieunJieut",
             "NieunHieut", "Digeut", "Rieul", "RieulGiyeok", "RieulMieum",
             "RieulBieup", "RieulSiot", "RieulTieut", "RieulPieup", "RieulHieut",
             "Mieum", "Bieup", "BieupSiot", "Siot", "SsangSiot", "Ieung", "Jieut",
             "Chieut", "Kieuk", "Tieut", "Pieup", "Hieut"].map.with_index.to_a]

conversions = {"choseong" => choseong, "jungseong" => jungseong, "jongseong" => jongseong}

hangulJamoHash = Hash[hangulJamo.map.with_index.to_a]

print "#define CI UINT8_MAX\n"

conversions.each do |conv_name, conv|
  print "static constexpr uint8_t _compTo#{conv_name.capitalize}[] = {"
  for jamo in hangulJamo
    if conv.has_key? jamo
      print "#{conv[jamo]},"
    else
      print "CI,"
    end
  end
  print "};\n"

  print "static constexpr uint8_t _#{conv_name}ToComp[] = {"
  for src in conv.keys
    print "#{hangulJamoHash[src]},"
  end
  print "};\n"
end

print "// to make the linker happy\n"
conversions.each do |conv_name, conv|
  print "constexpr uint8_t Character::CharacterImpl::_compTo#{conv_name.capitalize}[#{hangulJamo.size}];\n"
  print "constexpr uint8_t Character::CharacterImpl::_#{conv_name}ToComp[#{conv.size}];\n"
end
