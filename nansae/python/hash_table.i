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
#include "nansae/core/hash_table.h"
%}


%rename("HashTableUInt32_Entry") NSL::HashTable<uint32_t>::Entry;
%rename("HashTableUInt64_Entry") NSL::HashTable<uint64_t>::Entry;
%rename("HashTableUInt32_Iterator") NSL::HashTable<uint32_t>::Iterator;
%rename("HashTableUInt64_Iterator") NSL::HashTable<uint64_t>::Iterator;

%include "nansae/core/hash_table.h"

%template(HashTableUInt32) NSL::HashTable<uint32_t>;
%template(HashTableUInt64) NSL::HashTable<uint64_t>;


%pythoncode %{
HashTableUInt32_Iterator.entry = property(lambda self: self.__deref__())
HashTableUInt64_Iterator.entry = property(lambda self: self.__deref__())
%}
