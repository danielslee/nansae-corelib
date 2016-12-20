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

#include <cstdint>
#include <cstdlib>
#include <sstream>

#include "nansae/core/stream_binary_io.h"
#include "nansae/core/string.h"
#include "nansae/core/trie.h"

namespace NSL {
struct Trie::IteratorImpl {
  char *_snaPointer;
  uint8_t _childrenLeft;
  std::shared_ptr<IteratorImpl> _parent;
  std::string _prefix;
  Trie::TrieImpl *_trie;

  ////
  // Public methods
  ////
  WordIdPair value();
  IteratorImpl next(bool descend = true);
  bool notEqualTo(const IteratorImpl &other);
};

struct Trie::TrieImpl {
  /**
   * The struct representing a node of the trie in editing mode.
   */
  struct TrieNode {
    /**
     * The node's value.
     */
    std::string value;

    /**
     * The node's children.
     */
    std::vector<TrieNode> children;

    /**
     * The nodes id.
     * Do not attemp to use this value if the node isn't a leaf node.
     */
    uint32_t id;
  };

  /**
   * Signifies whether the trie is in editing mode or frozen.
   */
  bool _editingMode = true;

  /**
   * Contains the children of root.
   */
  std::vector<TrieNode> _rootChildren;

  /**
   * Contains the serialized node array when in frozen mode.
   */
  char *_serializedNodeArray = nullptr;

  /*
   * The size of the serialized node array.
   */
  uint32_t _serializedNodeArraySize = 0;

  /**
   * Prints out the serialized node array in a human readable format.
   */
  void _debugSNA();

  ////
  // Serialization helper functions.
  ////
  static uint8_t getChildrenNo(char *na);
  static uint32_t getChildrenOffset(char *na, char *start = nullptr);
  static uint32_t getId(char *na);
  static std::string getValue(char *na);
  static char *getValuePtr(char *na);
  static uint8_t getLenght(char *na);
  static TrieNode readNodeAndChildren(char *na);
  static uint32_t getBranchLength(const std::vector<TrieNode> &children,
                                  uint32_t length);
  static uint32_t writeChildren(char *na,
                                const std::vector<TrieNode> &children);
  static int compareHStr(uint8_t *hstr1, uint8_t *hstr2);
  static int compareHStr(std::string hstr1, std::string hstr2);
  static int compareHStr(std::string hstr1, std::string hstr2, size_t offset);

  ////
  // Public methods
  ////
  TrieImpl() = default;
  ~TrieImpl();
  void makeEditable();
  void freeze();
  uint32_t addWord(const String &str, uint32_t id, bool replace);
  uint32_t findWord(const String &str);
  std::vector<WordIdPair> findWordPrefixes(const String &str);
  void writeToStream(std::ostream &s);
  void loadFromStream(std::istream &s);
  bool editingMode();
  IteratorImpl begin();
  IteratorImpl end();
};

////
// Serialization helper functions.
////

/* SERIALIZATION FORMAT
 * [(uint8_t)root children no],[(uint8_t)children no, (uint16_t)children offset,
 * (uint8_t[])value, \0], ...
 * children offset is counted from the beginning of the current node
 */

uint8_t Trie::TrieImpl::getChildrenNo(char *na) {
  uint8_t r = *((uint8_t *)na);
  return r;
}

uint32_t Trie::TrieImpl::getChildrenOffset(char *na, char *start) {
  if (na - start == 0) return sizeof(uint8_t);
  uint32_t r = *((uint32_t *)(na + sizeof(uint8_t)));
  return r;
}

uint32_t Trie::TrieImpl::getId(char *na) {
  uint32_t r = *((uint32_t *)(na + sizeof(uint8_t)));
  return r;
}

std::string Trie::TrieImpl::getValue(char *na) {
  return std::string((char *)na + sizeof(uint8_t) + sizeof(uint32_t));
}

char *Trie::TrieImpl::getValuePtr(char *na) {
  return (char *)na + sizeof(uint8_t) + sizeof(uint32_t);
}

uint8_t Trie::TrieImpl::getLenght(char *na) {
  return sizeof(uint8_t) + sizeof(uint32_t) + std::strlen(getValuePtr(na)) + 1;
}

Trie::TrieImpl::TrieNode Trie::TrieImpl::readNodeAndChildren(char *na) {
  std::string value = getValue(na);
  uint32_t childrenOffset = getChildrenOffset(na);
  uint8_t childrenNo = getChildrenNo(na);

  TrieNode t;
  t.value = value;
  if (childrenNo > 0)
    t.children.reserve(childrenNo);
  else
    t.id = getId(na);

  uint32_t childrenRead = 0;
  for (int i = 0; i < childrenNo; i++) {
    t.children.push_back(
        readNodeAndChildren(na + childrenOffset + childrenRead));
    childrenRead += getLenght(na + childrenOffset + childrenRead);
  }

  return t;
}

uint32_t Trie::TrieImpl::getBranchLength(
    const std::vector<Trie::TrieImpl::TrieNode> &children, uint32_t length) {
  for (const TrieNode &t : children) {
    // if (t.children.size() == 0) // leaf node
    length += sizeof(uint8_t) + sizeof(uint32_t) + t.value.length() + 1;
    // else // non-leaf node
    // length += sizeof(uint8_t) + sizeof(uint16_t) + t.value.length() + 1;

    length = getBranchLength(t.children, length);
  }
  return length;
}

uint32_t Trie::TrieImpl::writeChildren(
    char *na, const std::vector<Trie::TrieImpl::TrieNode> &children) {
  // 1. write the nodes
  uint32_t currentLevelSize = 0;  // size of the current children combined
  for (const TrieNode &t : children) {
    // write the node
    *(na + currentLevelSize) = t.children.size();
    // leaf node:
    if (t.children.size() == 0) {
      // write the node id
      *((uint32_t *)(na + currentLevelSize + sizeof(uint8_t))) = t.id;
      // and the string
      std::strcpy((na + currentLevelSize + sizeof(uint8_t) + sizeof(uint32_t)),
                  t.value.c_str());
    } else {  // non-leaf node
      std::strcpy((na + currentLevelSize + sizeof(uint8_t) + sizeof(uint32_t)),
                  t.value.c_str());
    }

    // increment the currentLevelSize
    currentLevelSize +=
        sizeof(uint8_t) + sizeof(uint32_t) + t.value.length() + 1;
  }

  // 2. write their children and set their children offset
  uint32_t offset = 0;
  for (const TrieNode &t : children) {
    if (t.children.size() > 0) {
      *((uint32_t *)(na + offset + sizeof(uint8_t))) =
          currentLevelSize - offset;
      currentLevelSize += writeChildren(na + currentLevelSize, t.children);
    }
    offset += getLenght(na + offset);
  }

  return currentLevelSize;
}

int Trie::TrieImpl::compareHStr(uint8_t *hstr1, uint8_t *hstr2) {
  int i = 0;
  while (hstr1[i] != 0 && hstr2[i] != 0) {
    if (hstr1[i] == hstr2[i])
      i++;
    else
      return i;
  }
  return i;
}

int Trie::TrieImpl::compareHStr(std::string hstr1, std::string hstr2) {
  return compareHStr((uint8_t *)hstr1.c_str(), (uint8_t *)hstr2.c_str());
}

int Trie::TrieImpl::compareHStr(std::string hstr1, std::string hstr2,
                                size_t offset) {
  return compareHStr((uint8_t *)hstr1.c_str() + offset,
                     (uint8_t *)hstr2.c_str());
}

////
// Method implementation
////

void Trie::TrieImpl::makeEditable() {
  uint8_t rootChildrenNo = 0;
  _rootChildren.clear();
  _editingMode = true;

  if (_serializedNodeArray == nullptr) return;

  char *naPtr = _serializedNodeArray;
  rootChildrenNo = getChildrenNo(naPtr);
  naPtr += sizeof(uint8_t);  // move to the start

  _rootChildren.reserve(rootChildrenNo);

  for (int i = 0; i < rootChildrenNo; i++) {
    _rootChildren.push_back(readNodeAndChildren(naPtr));
    naPtr += getLenght(naPtr);
  }

  std::free(_serializedNodeArray);
  _serializedNodeArray = nullptr;
}

void Trie::TrieImpl::freeze() {
  _editingMode = false;
  if (_rootChildren.size() == 0) return;

  // count the size to allocate first
  uint32_t size = getBranchLength(_rootChildren, 0);

  _serializedNodeArray = (char *)std::malloc(sizeof(uint8_t) + size);
  _serializedNodeArraySize = sizeof(uint8_t) + size;
  *_serializedNodeArray = (uint8_t)_rootChildren.size();

  writeChildren(_serializedNodeArray + sizeof(uint8_t), _rootChildren);
}

uint32_t Trie::TrieImpl::addWord(const String &str, uint32_t id, bool replace) {
  if (!_editingMode) return 0;
  std::string hstr = str.toHangulString().theString;
  size_t strOffset = 0;

  TrieNode *currentNode = nullptr;
  std::vector<TrieNode> *currentNodeChildrenRef = &_rootChildren;

  while (strOffset < hstr.length()) {
    bool foundNodeToDescendTo = false;
    for (int c = 0; c < currentNodeChildrenRef->size(); c++) {
      TrieNode &n = (*currentNodeChildrenRef)[c];
      int charactersInCommon = compareHStr(hstr, n.value, strOffset);

      // exact match -> descend into the children node
      if (charactersInCommon == n.value.length() && n.value != "") {
        strOffset += charactersInCommon;

        currentNodeChildrenRef = &n.children;
        currentNode = &n;

        foundNodeToDescendTo = true;

        break;
      }

      // non-exact match:
      else if (charactersInCommon > 0) {
        // 1. shorten the current node value
        std::string originalString = n.value;
        n.value = hstr.substr(strOffset, charactersInCommon);
        strOffset += charactersInCommon;

        // 2. create a new branch for the existing nodes
        std::vector<TrieNode> existingChildren = std::move(n.children);
        n.children.clear();

        TrieNode existingNodes;
        existingNodes.value = originalString.substr(charactersInCommon);
        existingNodes.children = std::move(existingChildren);
        existingNodes.id = n.id;

        // 3. create a new branch for the string that is being inserted
        TrieNode newBranch;
        newBranch.value = hstr.substr(strOffset);
        newBranch.id = id;

        // 4. push them back and exit
        n.children.push_back(std::move(newBranch));
        n.children.push_back(std::move(existingNodes));

        return id;
      }
    }

    // create a new node and exit if we couldn't descend
    if (!foundNodeToDescendTo) {
      //  create two (preserve the original) if current node is final
      if (currentNodeChildrenRef->size() == 0 &&
          currentNodeChildrenRef != &_rootChildren) {
        std::vector<TrieNode> oldChildren = std::move(*currentNodeChildrenRef);
        currentNodeChildrenRef->clear();
        // zero node
        TrieNode z;
        z.value = "";
        z.children = std::move(oldChildren);
        if (z.children.size() == 0 && currentNode != nullptr)
          z.id = currentNode->id;
        currentNodeChildrenRef->push_back(std::move(z));
      }

      // new node
      TrieNode n;
      n.value = hstr.substr(strOffset);
      n.id = id;
      currentNodeChildrenRef->push_back(std::move(n));
      return id;
    }
  }

  // replace or return id if we're finished
  if (strOffset == hstr.length() && currentNode != nullptr) {
    // descend to node with value of "" if the current one is not a leaf one
    // create one if not found
    if (currentNode->children.size() > 0) {
      bool foundLeafNode = false;
      for (int c = 0; c < currentNodeChildrenRef->size(); c++) {
        TrieNode &n = (*currentNodeChildrenRef)[c];
        if (n.value == "") {
          currentNode = &n;
          foundLeafNode = true;
        }
      }
      if (!foundLeafNode) {
        TrieNode newNode;
        newNode.value = "";
        newNode.id = id;
        currentNodeChildrenRef->push_back(std::move(newNode));
        currentNode = &currentNodeChildrenRef->back();
      }
    }
    if (replace) currentNode->id = id;
    return currentNode->id;
  }
  return id;
}

uint32_t Trie::TrieImpl::findWord(const String &str) {
  if (_editingMode) return NIME_TRIE_WORD_NOT_FOUND;

  std::string hstr = str.toHangulString().theString;
  size_t strOffset = 0;

  char *currentNodePtr = _serializedNodeArray;

  while (strOffset < hstr.length()) {
    uint8_t childrenNo = getChildrenNo(currentNodePtr);
    uint32_t read = 0;
    bool foundNodeToDescendTo = false;

    // for each child of the current node
    for (int i = 0; i < childrenNo; i++) {
      char *childPointer =
          currentNodePtr +
          getChildrenOffset(currentNodePtr, _serializedNodeArray) + read;
      char *value = getValuePtr(childPointer);
      int charactersInCommon =
          compareHStr((uint8_t *)(hstr.c_str() + strOffset), (uint8_t *)value);

      // exact match -> descend into the children node
      if (charactersInCommon == std::strlen((char *)value) &&
          value[0] != '\0') {
        strOffset += charactersInCommon;
        currentNodePtr = childPointer;
        foundNodeToDescendTo = true;
        break;
      }
      read += getLenght(childPointer);
    }
    if (!foundNodeToDescendTo) {
      return NIME_TRIE_WORD_NOT_FOUND;
    }
  }

  uint8_t childrenNo = getChildrenNo(currentNodePtr);
  if (childrenNo == 0)  // we've arrived to the leaf node
    return getId(currentNodePtr);
  else {
    uint32_t read = 0;
    for (int i = 0; i < childrenNo; i++) {
      char *childPointer =
          currentNodePtr +
          getChildrenOffset(currentNodePtr, _serializedNodeArray) + read;
      uint8_t *value = (uint8_t *)getValuePtr(childPointer);
      if (value[0] == '\0') return getId(childPointer);
      read += getLenght(childPointer);
    }
  }

  return NIME_TRIE_WORD_NOT_FOUND;
}

std::vector<Trie::WordIdPair> Trie::TrieImpl::findWordPrefixes(
    const String &str) {
  std::vector<WordIdPair> prefixes;
  if (_editingMode) return prefixes;

  char *currentNodePtr = _serializedNodeArray;
  std::string hstr = str.toHangulString().theString;
  size_t strOffset = 0;
  size_t newStrOffset = 0;

  while (strOffset < hstr.length()) {
    uint8_t childrenNo = getChildrenNo(currentNodePtr);
    uint32_t read = 0;
    bool foundNodeToDescendTo = false;

    char *descendToNodePtr = nullptr;
    bool foundZeroNode = false;
    // find a node to descend to and add all nodes with value of '/0' to the
    // prefixes
    for (int i = 0; i < childrenNo; i++) {
      char *childPointer =
          currentNodePtr +
          getChildrenOffset(currentNodePtr, _serializedNodeArray) + read;
      uint8_t *value = (uint8_t *)getValuePtr(childPointer);
      int charactersInCommon =
          compareHStr((uint8_t *)(hstr.c_str() + strOffset), (uint8_t *)value);

      // exact match -> descend into the children node
      if (charactersInCommon == std::strlen((char *)value) &&
          value[0] != '\0') {
        newStrOffset = strOffset + charactersInCommon;
        descendToNodePtr = childPointer;
        foundNodeToDescendTo = true;
      } else if (value[0] == '\0') {
        WordIdPair wp;
        wp.id = getId(childPointer);
        wp.str = String(HangulString(hstr.substr(0, strOffset)));
        prefixes.push_back(wp);
        foundZeroNode = true;
      }
      read += getLenght(childPointer);
    }

    if (!foundNodeToDescendTo) {
      if (foundZeroNode) return prefixes;
      goto finalize;
    } else {
      currentNodePtr = descendToNodePtr;
      strOffset = newStrOffset;
    }
  }

finalize:
  uint8_t childrenNo = getChildrenNo(currentNodePtr);
  if (childrenNo == 0) {  // we've arrived to the leaf node
    WordIdPair wp;
    wp.id = getId(currentNodePtr);
    wp.str = String(HangulString(hstr.substr(0, strOffset)));
    prefixes.push_back(wp);
  } else {
    uint32_t read = 0;
    for (int i = 0; i < childrenNo; i++) {
      char *childPointer =
          currentNodePtr +
          getChildrenOffset(currentNodePtr, _serializedNodeArray) + read;
      uint8_t *value = (uint8_t *)getValuePtr(childPointer);
      if (value[0] == '\0') {
        WordIdPair wp;
        wp.id = getId(childPointer);
        wp.str = String(HangulString(hstr.substr(0, strOffset)));
        prefixes.push_back(wp);
      }
      read += getLenght(childPointer);
    }
  }

  return prefixes;
}

void Trie::TrieImpl::_debugSNA() {
  char *snaPtr = _serializedNodeArray;
  printf("[cn: %d]", *snaPtr);
  snaPtr++;
  while (snaPtr < _serializedNodeArray + _serializedNodeArraySize) {
    uint8_t cn = getChildrenNo(snaPtr);
    printf("[cn: %d, ", cn);
    if (cn == 0) {
      printf("id: %d, ", getId(snaPtr));
    } else {
      printf("co: %d, ", getChildrenOffset(snaPtr));
    }
    printf("v: |");
    for (uint8_t *vPtr = (uint8_t *)getValuePtr(snaPtr); *vPtr != 0; vPtr++) {
      printf("%d|", *vPtr);
    }
    uint32_t length = getLenght(snaPtr);
    printf(" (%d)]", length);
    snaPtr += length;
  }
  printf("\n");
}

void Trie::TrieImpl::writeToStream(std::ostream &s) {
  if (_editingMode) return;

  StreamBinaryWrite<uint32_t>(s, _serializedNodeArraySize);
  s.write(_serializedNodeArray, _serializedNodeArraySize);
}

void Trie::TrieImpl::loadFromStream(std::istream &s) {
  if (_editingMode) return;

  std::free(_serializedNodeArray);

  _serializedNodeArraySize = StreamBinaryRead<uint32_t>(s);
  _serializedNodeArray = (char *)std::malloc(_serializedNodeArraySize);
  s.read(_serializedNodeArray, _serializedNodeArraySize);
}

Trie::TrieImpl::~TrieImpl() {
  if (!_editingMode) {
    std::free(_serializedNodeArray);
  }
}

bool Trie::TrieImpl::editingMode() { return _editingMode; }

Trie::WordIdPair Trie::IteratorImpl::value() {
  WordIdPair wip;
  wip.id = TrieImpl::getId(_snaPointer);
  wip.str = String(HangulString(_prefix + TrieImpl::getValue(_snaPointer)));
  return wip;
}

Trie::IteratorImpl Trie::IteratorImpl::next(bool descend) {
  // if no children left and cannot descend -> ascend back to the current
  // node's parent
  if (_childrenLeft == 0 && (TrieImpl::getChildrenNo(_snaPointer) == 0)) {
  ascend:
    if (_parent == nullptr) {
      return _trie->end();
    }
    // jump to the parent
    _snaPointer = _parent->_snaPointer;
    _childrenLeft = _parent->_childrenLeft;
    _prefix = _parent->_prefix;
    _trie = _parent->_trie;
    _parent = _parent->_parent;
    // and move one to the right, no descending if there are children left
    if (_childrenLeft > 0) {
      _snaPointer += TrieImpl::getLenght(_snaPointer);
      _childrenLeft--;
    } else {  // else ascend some more
      goto ascend;
    }
  } else {
    uint8_t childrenNo = TrieImpl::getChildrenNo(_snaPointer);
    if (childrenNo > 0) {
      std::shared_ptr<IteratorImpl> newParent =
          std::shared_ptr<IteratorImpl>(new IteratorImpl());
      newParent->_snaPointer = _snaPointer;
      newParent->_childrenLeft = _childrenLeft;
      newParent->_prefix = _prefix;
      newParent->_trie = _trie;
      newParent->_parent = _parent;

      _prefix = _prefix + std::string(TrieImpl::getValue(_snaPointer));
      _snaPointer = _snaPointer + TrieImpl::getChildrenOffset(_snaPointer);
      _childrenLeft = childrenNo - 1;
      _parent = newParent;
    } else {
      _snaPointer += TrieImpl::getLenght(_snaPointer);
      _childrenLeft--;
    }
  }

  if (TrieImpl::getChildrenNo(_snaPointer) > 0) return next();
  return *this;
}

bool Trie::IteratorImpl::notEqualTo(const Trie::IteratorImpl &other) {
  return (_snaPointer !=
          other._snaPointer); /*!((_snaPointer == other._snaPointer) &&
(_childrenLeft == other._childrenLeft) &&
(_parent == other._parent) &&
(_prefix == other._prefix));*/
}

Trie::IteratorImpl Trie::TrieImpl::begin() {
  IteratorImpl it;
  // pointing to the first child of the root node
  it._snaPointer = _serializedNodeArray + sizeof(uint8_t);
  // number of root children - 1 remaining
  it._childrenLeft = getChildrenNo(_serializedNodeArray) - 1;
  // null represents the root parent
  it._parent = std::shared_ptr<IteratorImpl>(nullptr);
  it._prefix = "";
  it._trie = this;
  // find next node if the current isn't leaf
  if (getChildrenNo(it._snaPointer) > 0) it.next();
  return it;
}

Trie::IteratorImpl Trie::TrieImpl::end() {
  IteratorImpl it;
  it._snaPointer = _serializedNodeArray + _serializedNodeArraySize;
  it._childrenLeft = 0;
  it._parent = std::shared_ptr<IteratorImpl>(nullptr);
  it._prefix = "";
  it._trie = this;
  return it;
}

////
// Public method forwarding
////
Trie::Trie() : _impl(new Trie::TrieImpl()) {}
Trie::~Trie() = default;
void Trie::makeEditable() { _impl->makeEditable(); };
void Trie::freeze() { _impl->freeze(); };
uint32_t Trie::addWord(const String &str, uint32_t id, bool replace) {
  return _impl->addWord(str, id, replace);
}
uint32_t Trie::findWord(const String &str) { return _impl->findWord(str); }
std::vector<Trie::WordIdPair> Trie::findWordPrefixes(const String &str) {
  return _impl->findWordPrefixes(str);
}
void Trie::writeToStream(std::ostream &s) { _impl->writeToStream(s); }
void Trie::loadFromStream(std::istream &s) { _impl->loadFromStream(s); }
bool Trie::editingMode() { return _impl->editingMode(); }

Trie::Iterator Trie::begin() {
  Iterator it;
  it._impl = std::shared_ptr<IteratorImpl>(new IteratorImpl(_impl->begin()));
  return it;
}

Trie::Iterator Trie::end() {
  Iterator it;
  it._impl = std::shared_ptr<IteratorImpl>(new IteratorImpl(_impl->end()));
  return it;
}

Trie::Iterator::Iterator() : _impl(new Trie::IteratorImpl()) {}
Trie::Iterator::~Iterator() = default;
Trie::WordIdPair Trie::Iterator::operator*() { return _impl->value(); }
Trie::WordIdPair Trie::Iterator::operator->() { return _impl->value(); }
Trie::Iterator Trie::Iterator::operator++() {
  _impl = std::unique_ptr<IteratorImpl>(new IteratorImpl(_impl->next()));
  return *this;
}
Trie::Iterator Trie::Iterator::operator+(uint32_t moveBy) {
  for (uint32_t i = 0; i < moveBy - 1; ++i) this->operator++();
  return this->operator++();
}
bool Trie::Iterator::operator!=(const Iterator &other) {
  return _impl->notEqualTo(*other._impl);
}
}
