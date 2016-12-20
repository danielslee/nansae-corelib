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

%{
#include "nansae/core/trie.h"
%}

%rename("Trie_WordIdPair") NSL::Trie::WordIdPair;
%rename("Trie_Iterator") NSL::Trie::Iterator;

%include "nansae/core/trie.h"

%pythoncode %{
Trie_Iterator.word_id_pair = property(lambda self: self.__deref__())
%}
