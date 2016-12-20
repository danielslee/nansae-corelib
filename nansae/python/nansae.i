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

%define %ignoreall %ignore ""; %enddef
%define %unignore %rename("%s") %enddef
%define %unignoreall %rename("%s") ""; %enddef

%feature ("flatnested");

//%include <std_except.i>
%include <std_string.i>
%include <stdint.i>

%include "nansae/python/character.i"
%include "nansae/python/string.i"
%include "nansae/python/hash_table.i"
%include "nansae/python/trie.i"
%include "nansae/python/segmentations.i"
