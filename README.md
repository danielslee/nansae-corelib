# Nansae Corelib
Nansae is a Korean language NLP (Natural Language Processing) project currently
in development. The opensource Corelib contains some of the basic types and
algorithms needed for manipulating Korean text.

## Hangul Processing
The Corelib contains its own implementation of a string and character. It's main
feature is its ability to handle the Korean alphabet, Hangul.

## Build
Nansae is built with Bazel by Google. To also get tensorflow you should
clone recursively. You can then build everything and run the tests.

```
$ git clone --recursive https://github.com/danielslee/nansae-corelib
$ bazel test //nansae/core/..
```

### NSL::Character
The character class handles decomposing a Hangul syllable into the Korean
alphabet's letters (Jamo) and handles different ways of encoding the Jamo.

```
  NSL::Character syllable(54620);  // 한
  ASSERT_EQ(syllable.type(), NSL::Character::Type::HangulSyllable);
  ASSERT_EQ(syllable.choseong(), NSL::Character::HangulJamo::Hieut);
  ASSERT_EQ(syllable.jungseong(), NSL::Character::HangulJamo::A);
  ASSERT_EQ(syllable.jongseong(), NSL::Character::HangulJamo::Nieun);

  NSL::Character jamo(0x11af);  // ᆯ (unicode positional jamo)
  ASSERT_EQ(jamo.hangulJamo(), NSL::Character::HangulJamo::Rieul);
  ASSERT_EQ(jamo, NSL::Character(u8"ㄹ"));  // unicode compatibility jamo
  jamo.setUnicodeCodepoint(0x3139);         // ㄹ (unicode compatibility jamo)
  ASSERT_EQ(jamo.hangulJamo(), NSL::Character::HangulJamo::Rieul);
  ASSERT_EQ(jamo, NSL::Character(u8"ㄹ"));  // unicode compatibility jamo
}
```

### NSL::String
The string handles a multitude of things. Apart from basic operations such as
prepend, append, substring and more, it has features useful in the development
of Korean language NLP software.

#### Encapsulate Non-Hangul
The NSL::String lets you easily get separate all non-Hangul (i.e. numerals,
latin and hanja), do your processing on the Hangul-only string and then
return all of the non-Hangul strings back in place. The non-Hangul string is
represented by a special character, which is represented by the letter 'S' when
converted to std::string.

```
NSL::String str(u8"安寧하세요");
NSL::String::EncapsulatedNonHangul enh = str.encapsulateNonHangul();
ASSERT_EQ(str.toStdString(), u8"S하세요");
str.restoreNonHangul(enh);
ASSERT_EQ(str.toStdString(), u8"安寧하세요");

NSL::String str2("latin한글漢字한글ㅈㅏㅁㅗ");
NSL::String::EncapsulatedNonHangul enh2 = str2.encapsulateNonHangul();
ASSERT_EQ(str2.toStdString(), u8"S한글S한글S");
str2.restoreNonHangul(enh2);
ASSERT_EQ(str2.toStdString(), u8"latin한글漢字한글ㅈㅏㅁㅗ");
```

### NSL::Trie
The trie stores Hangul syllables decomposed into its letters which allows
much more efficient storage and lookup of Hangul strings. It is especially
well suited for storing different inflictions such as the verb root 하(爲) and
its various forms such as 한 (하 + ㄴ), 할 (하 + ㄹ) and the like.

You can look for words, prefixes and iterate through the trie. The trie has two
states frozen and unfrozen, lookups can be only performed in its frozen state
and it can be only edited in the unfrozen one.

```
NSL::Trie t;
t.addWord(NSL::String(u8"빨"), 7);
t.addWord(NSL::String(u8"빨갛"), 0);
t.addWord(NSL::String(u8"빨간"), 1);
t.addWord(NSL::String(u8"빨개"), 2);
t.addWord(NSL::String(u8"파랗"), 3);
t.addWord(NSL::String(u8"파란"), 4);
t.addWord(NSL::String(u8"빨래"), 5);
t.addWord(NSL::String(u8"빨리"), 6);
t.freeze();

ASSERT_EQ(t.findWord(NSL::String(u8"빨간")), 1);
ASSERT_EQ(t.findWord(NSL::String(u8"파랗")), 3);
ASSERT_EQ(t.findWord(NSL::String(u8"빨")), 7);
ASSERT_EQ(t.findWord(NSL::String(u8"빨가")), NIME_TRIE_WORD_NOT_FOUND);
ASSERT_EQ(t.findWord(NSL::String(u8"빨간색")), NIME_TRIE_WORD_NOT_FOUND);

std::vector<NSL::Trie::WordIdPair> prefixes =
    t.findWordPrefixes(NSL::String(u8"빨간색"));
ASSERT_EQ(prefixes[0].str, NSL::String(u8"빨"));
ASSERT_EQ(prefixes[1].str, NSL::String(u8"빨간"));

for (NSL::Trie::WordIdPair wip : t) {
  std::cout << wip.str.toStdString() << '\n';
}
```

### NSL::HashTable
The hash table stores 64 or 32-bit integer keys and double values. This
is intended to store training values. The NSL::Hash can be saved to disk and
loaded right into memory, reducing load time greatly at a small cost of taking
more disk space. It can also be iterated.

```
NSL::HashTable<uint32_t> ht(65536);
std::stringstream s;

for (int i = 0; i < 30000; i++) {
  ASSERT_EQ(ht.insert(i, 20 * i), 0);
}

for (int i = 0; i < 30000; i++) {
  ASSERT_EQ(ht.insert(i, 3 * i), 1);
  ASSERT_EQ(ht.insert(i, 1.1 * i), 1);
}

ht.writeToStream(s);

NSL::HashTable<uint32_t> ht2(s);

for (int i = 29999; i >= 0; i--) {
  double x = ht2.retrieve(i);
  ASSERT_DOUBLE_EQ(x, 1.1 * i);
}
```

### NSL::Segmentations
While Korean does have spacing it is not necessarily adhered to especially in
informal contexts on the internet. Even if everything is correctly spaced
according to the rules, most words contain multiple morphemes that must be
treated as separate in NLP.

The Segmentations class given a sentence and a trie containing our dictionary
solves this by generating a lattice containing all of its possible
segmentations. This can be then used as input for further processing.

The debug string represents the start of a word on Y (vertical) axis and
the end on its X (horizontal axis). The end equals the start + word length.
Thus a one character word starting at 0 ends at 1.

```
NSL::String str(u8"symbol한글단어");
str.encapsulateNonHangul();

NSL::Trie trie;
trie.addWord(NSL::String(u8"한"), 1);
trie.addWord(NSL::String(u8"한글"), 2);
trie.addWord(NSL::String(u8"글"), 3);
trie.addWord(NSL::String(u8"단"), 4);
trie.addWord(NSL::String(u8"단어"), 5);
trie.freeze();

auto segmentations = NSL::Segmentations::ForSentence(str, trie);

ASSERT_EQ(segmentations.debugString(),
          "\
0 1 2 3 4\n\
0 x        \n\
1   x x    \n\
2     x    \n\
3       x x\n\
4          ");
```

### Python binding & Tensorflow
Nansae Corelib includes a Python binding along with Tensorflow to allow for
further processing of Korean text.
