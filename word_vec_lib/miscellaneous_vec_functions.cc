// miscellaneous_vec_functions.cc

// Copyright 2019 E. Decker
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>

#include "word_vec_lib.h"

void VecPrint::PrintVec(const std::vector<double>& vec) {
// Prints all elements of a vector (separated by whitespaces).
  if (vec.empty()) {
    std::cout << "[]" << std::endl;
    return;
  }
  const unsigned size(vec.size());
  std::cout << '[';
  for (unsigned i = 0; i < size; ++i)
    std::cout << vec[i] << ((i != size-1)? ' ' : ']');
  std::cout << std::endl;
}

void VecPrint::PrintWordVec(const WordVec* wv) {
// Prints the "word" and the "vector" of a "WordVec".
  if (!wv) {
    std::cout << "NULL" << std::endl;
    return;
  }
  std::cout << wv->word << ' ';
  PrintVec(wv->vec);
}

void VecPrint::PrintWordVecList(std::list<WordVec*> wv_list) {
// Prints all "WordVec"s in a given list.
  if (wv_list.empty()) {
    std::cout << "NULL" << std::endl;
    return;
  }
  std::cout << "--- BEGIN WordVec List: ---" << '\n';
  for (auto wv : wv_list)
    PrintWordVec(wv);
  std::cout << "--- END WordVec List.   ---" << std::endl;
}

void VecPrint::PrintWordPair(const std::pair<std::pair<std::string, std::string>, double>& word_pair) {
// Prints a "word pair".
  std::cout << '\"' << word_pair.first.first << "\" / \"" << word_pair.first.second << "\", " << word_pair.second << std::endl;
}

void VecPrint::PrintWordPairList(const std::list<std::pair<std::pair<std::string, std::string>, double>>& word_pair_list) {
// Prints all "word pairs" in a given list.
  if (word_pair_list.empty()) {
    std::cout << "NULL" << std::endl;
    return;
  }
  std::cout << "--- BEGIN WordPair List: ---" << '\n';
  for (auto& word_pair : word_pair_list)
    PrintWordPair(word_pair);
  std::cout << "--- END WordPair List.   ---" << std::endl;
}

std::vector<double> VecCalc::Add(const std::list<WordVec*> wvs) {
// Adds all vectors of the "WordVec"s stored in a std::vector ("wvs") and
// returns the resulting vector.
  std::vector<std::vector<double>> vecs;
  vecs.reserve(wvs.size());
  for (auto wv : wvs)
    vecs.push_back(wv->vec);
  return Add(vecs);
}
