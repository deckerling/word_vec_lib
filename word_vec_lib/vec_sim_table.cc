// vec_sim_table.cc

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

#include <fstream>
#include <iostream>
#include <sstream>

#include "word_vec_lib.h"

VecSimTable::VecSimTable(const std::string& file, const std::regex& pattern)
// Constructor of a "VecSimTable" using a regex pattern to choose the word
// vectors that shall be stored.
    : vec_size_(GetSizeOfVectors(file)),
      case_sensitive_(true) {
  word_vecs_.reserve(1000);
  vec_num_ = StoreVecsWithPattern(file, pattern);
  sim_table_.resize(vec_num_);
  unsigned j(1);
  for (int i = 0; i < vec_num_; ++i)
    sim_table_[i].reserve(vec_num_-(j++));
  CalculateSimilarities();
}

VecSimTable::VecSimTable(const std::string& file, const bool case_sensitive, const double percentage)
// Constructor of a "VecSimTable" that stores the word vectors in order of their
// occurrence in the word vector file ("file"). If percentage != 1 only a
// certain percentage of the word vectors will be stored (i.e. the first
// "percentage" percent).
    : vec_size_(GetSizeOfVectors(file)),
      case_sensitive_(case_sensitive) {
  vec_num_ = CountVectors(file)*((percentage > 1)? 1 : percentage)+0.5;
  word_vecs_.resize(vec_num_);
  sim_table_.resize(vec_num_);
  unsigned j(1);
  for (int i = 0; i < vec_num_; ++i)
    sim_table_[i].reserve(vec_num_-(j++));
  StoreWordVecs(file, case_sensitive, percentage);
  CalculateSimilarities();
}

VecSimTable::~VecSimTable() {
  for (auto& wv : word_vecs_)
    delete wv;
  for (int i = 0; i < vec_num_; ++i) {
    for (int j = 0; j < vec_num_-(i+1); ++j)
      delete sim_table_[i][j];
  }
}

const int VecSimTable::GetSizeOfVectors(const std::string& file) {
// Returns the number of dimensions of the word vectors found in "file"
// (assuming that each line of the file contains exactly one vector and that
// all the word vectors got the same number of dimensions).
  std::ifstream file_stream(file);
  if (!file_stream.is_open()) {
    std::cout << "ERROR: OPENING \"" << file << "\" FAILED!\nMake sure that the file exists and that the path is correct." << std::endl;
    return -1;
  } else if (file_stream.bad()) {
    std::cout << "ERROR: OPENING \"" << file << "\" FAILED!" << std::endl;
    return -1;
  }
  std::cout << "CREATING A \"VecSimTable\"." << '\n' << "Input file (\"word vector file\"): " << file << '\n';
  std::cout << "\tChecking the size of the word vectors..." << std::endl;
  std::string line;
  std::getline(file_stream, line);
  std::cout << "\t---Done." << '\n';
  return std::count(line.begin(), line.end(), ' ');
}

void VecSimTable::StoreWordVecs(const std::string& file, const bool case_sensitive, const double percentage) {
// Reads the word vectors from "file" in order to store them in a vector on
// memory.
  std::cout << "\tLoading data..." << std::endl;
  std::string line;
  std::ifstream vector_file_stream(file);
  unsigned remaining_vecs(vec_num_);
  unsigned i(0);
  word_vecs_.resize(remaining_vecs);
  while (std::getline(vector_file_stream, line) && remaining_vecs-- != 0)
    StoreVectors(line, i++);
  std::sort(word_vecs_.begin(), word_vecs_.end(), SortIt);
  std::cout << "\t---Completed." << std::endl;
}

const int VecSimTable::CountVectors(const std::string& file) {
// Returns the number of word vectors in "file" (assuming that each line
// of the file contains exactly one vector).
  if (vec_size_ < 1)
    return -1;
  std::ifstream file_stream(file);
  unsigned vector_num(0);
  std::string line;
  std::cout << "\tCounting the word vectors..." << std::endl;
  while (std::getline(file_stream, line))
    vector_num++; // this might cause problems if your "file" is not a valid word vector file because actually lines and not vectors are counted
  std::cout<<"\t---Done."<<std::endl;
  return vector_num;
}

void VecSimTable::StoreVectors(const std::string& line, const unsigned i) {
// Stores the word vectors in a vector ("word_vecs_") on memory.
  const std::vector<std::string> tokens(SplitLine(line));
  std::vector<double> vector(vec_size_);
  for (int j = 0; j < vec_size_; ++j)
    // Converts the (std::string) elements of "tokens" that represent the
    // values of the word vector into the type "double".
    vector[j] = atof(tokens[j+1].c_str());
  word_vecs_[i] = new WordVec(tokens[0], vector);
}

int VecSimTable::StoreVecsWithPattern(const std::string& file, const std::regex& pattern) {
// Returns the number of word vectors in "file" that match the regex "pattern"
// (assuming that each line of the file contains exactly one vector) and stores
// the word vectors in a vector ("word_vecs_") on memory.
  if (vec_size_ < 1)
    return -1;
  std::cout << "\tLoading data..." << std::endl;
  std::string line;
  std::ifstream vector_file_stream(file);
  int vec_count(0);
  std::vector<std::string> tokens;
  tokens.reserve(vec_size_+1);
  std::vector<double> vector(vec_size_);
  while (std::getline(vector_file_stream, line)) {
    tokens = SplitLine(line);
    if (!std::regex_match(tokens[0], pattern))
      continue;
    for (int i = 0; i < vec_size_; ++i)
      // Converts the (std::string) elements of "tokens" that represent the
      // values of the word vector into the type "double".
      vector[i] = atof(tokens[i+1].c_str());
    word_vecs_.push_back(new WordVec(tokens[0], vector));
    vec_count++;
  }
  std::sort(word_vecs_.begin(), word_vecs_.end(), SortIt);
  std::cout << "\t---Completed." << std::endl;
  return vec_count;
}

std::vector<std::string> VecSimTable::SplitLine(const std::string& line) {
// Splits the lines (strings) of the vector file into their tokens and returns
// all of those tokens in a std::vector. (The tokens within a "line" should be
// separated by whitespaces.)
  std::stringstream stream(line);
  std::string item;
  std::vector<std::string> tokens(vec_size_+1);
  for (auto& token : tokens) {
    std::getline(stream, item, ' ');
    token = item;
  }
  if (!case_sensitive_)
    tokens[0] = VecStore::SetToLowerCase(tokens[0]);
  return tokens;
}

void VecSimTable::CalculateSimilarities() {
// Calulates and stores the cosine similarities and Euclidean distances of all
// word pairs provided by "word_vecs_".
  std::cout << "\tCalculating similarities..." << std::endl;
  std::vector<double> cur_vec;
  cur_vec.reserve(vec_size_);
  for (int i = 0; i < vec_num_; ++i) {
    cur_vec = word_vecs_[i]->vec;
    sim_table_.push_back(std::vector<SimMeasures*>());
    for (int j = i+1; j < vec_num_; ++j)
      sim_table_[i].push_back(new SimMeasures(CosineSimilarity(cur_vec, word_vecs_[j]->vec), EuclideanDistance(cur_vec, word_vecs_[j]->vec)));
  }
  std::cout << "\t---Completed." << std::endl;
}

void VecSimTable::PrintInfo() {
  // Prints the most important information regarding the VecSimTable.
  std::cout << "Basic information about the \"VecSimTable\":" << '\n';
  std::cout << "\tSize of vectors = " << vec_size_ << '\n';
  std::cout << "\tNumber of stored word vectors = " << vec_num_ << '\n';
  std::cout << "\tThis \"VecSimTable\" works " << ((case_sensitive_)? "case sensitive." : "case insensitive.") << std::endl;
}

std::vector<double> VecSimTable::GetVec(std::string word) {
  const int index(GetIndex(word));
  if (index < 0) {
    std::cout << "ERROR in GetVec(): \"" << word << "\" couldn't be found in your data; returned an empty vector." << std::endl;
    return std::vector<double>();
  }
  return word_vecs_[index]->vec;
}

double VecSimTable::GetCosSim(std::string word0, std::string word1) {
// Searches for the cosine similarity of a word pair ("word0", "word1") in the
// sim_table_ and returnes it.
  if (!case_sensitive_) {
    word0 = VecStore::SetToLowerCase(word0);
    word1 = VecStore::SetToLowerCase(word1);
  }
  if (word0 == word1) return 1;
  const int i(GetIndex(word0));
  if (i == -1) {
    std::cout << "ERROR in GetCosSim(): \"" << word0 << "\" couldn't be found, 0 returned." << std::endl;
    return 0;
  }
  const int j(GetIndex(word1));
  if (j == -1) {
    std::cout << "ERROR in GetCosSim(): \"" << word1 << "\" couldn't be found, 0 returned." << std::endl;
    return 0;
  }
  const std::pair<int, int> sim_table_indices(GetSimTableIndices(i, j));
  return sim_table_[sim_table_indices.first][sim_table_indices.second]->cos_sim;
}

double VecSimTable::GetEuclDist(std::string word0, std::string word1) {
// Searches for the Euclidean distance between two word vectors (of "word0",
// "word1") in the sim_table_ and returnes it.
  if (!case_sensitive_) {
    word0 = VecStore::SetToLowerCase(word0);
    word1 = VecStore::SetToLowerCase(word1);
  }
  if (word0 == word1) return 0;
  const int i(GetIndex(word0));
  if (i < 0) {
    std::cout << "ERROR in GetEuclDist(): \"" << word0 << "\" couldn't be found." << std::endl;
    return std::numeric_limits<double>::quiet_NaN();;
  }
  const int j(GetIndex(word1));
  if (j < 0) {
    std::cout << "ERROR in GetEuclDist(): \"" << word1 << "\" couldn't be found." << std::endl;
    return std::numeric_limits<double>::quiet_NaN();;
  }
  const std::pair<int, int> sim_table_indices(GetSimTableIndices(i, j));
  return sim_table_[sim_table_indices.first][sim_table_indices.second]->eucl_dist;
}

std::list<std::pair<std::pair<std::string, std::string>, double>> VecSimTable::SimilarPairs(std::string word0, std::string word1, std::string comparison_mode, const double range) {
// Returns a list of similar word pairs to a given word pair ("word0", "word1")
// with respect to either the cosine similarity or the Euclidean distance using
// a range: E.g. if "range == 0.1" and the chosen criteria is the cosine
// similarity that would be "0.5" for the given word pair, all word pairs with
// a cosine similarity between 0.4 and 0.6 will be returned. If the words of
// the given word pair are not stored in the VecSimTable an empty list will be
// returned.
  if (!case_sensitive_) {
    word0 = VecStore::SetToLowerCase(word0);
    word1 = VecStore::SetToLowerCase(word1);
  }
  if (word0 == word1) {
    std::cout << "ERROR in SimilarPairs():: No real word pair selected (both words were \"" << word0 << "\"); returned an empty list." << std::endl;
    return std::list<std::pair<std::pair<std::string, std::string>, double>>();
  }
  const int k(GetIndex(word0));
  if (k < 0) {
    std::cout << "ERROR in SimilarPairs(): \"" << word0 << "\" couldn't be found; returned an empty list." << std::endl;
    return std::list<std::pair<std::pair<std::string, std::string>, double>>();
  }
  const int l(GetIndex(word1));
  if (l < 0) {
    std::cout << "ERROR in SimilarPairs(): \"" << word1 << "\" couldn't be found; returned an empty list." << std::endl;
    return std::list<std::pair<std::pair<std::string, std::string>, double>>();
  }
  const std::pair<int, int> sim_table_indices(GetSimTableIndices(k, l));
  std::pair<int, int> word_vecs_indices;
  const bool cos_sim((std::regex_match(VecStore::SetToLowerCase(comparison_mode), (std::regex) "eucl(idean)?([ _-])?dist(ance)?"))? false : true);
  const double value_min(((cos_sim)? sim_table_[sim_table_indices.first][sim_table_indices.second]->cos_sim : sim_table_[sim_table_indices.first][sim_table_indices.second]->eucl_dist)-range);
  const double value_max(((cos_sim)? sim_table_[sim_table_indices.first][sim_table_indices.second]->cos_sim : sim_table_[sim_table_indices.first][sim_table_indices.second]->eucl_dist)+range);
  std::list<std::pair<std::pair<std::string, std::string>, double>> list_of_pairs;
  for (int i = 0; i < vec_num_; ++i) {
    for (int j = 0; j < vec_num_-(i+1); ++j) {
      if (i == sim_table_indices.first && j == sim_table_indices.second)
        continue; // skips the original word pair in question ("word0", "word1")
      if (cos_sim && sim_table_[i][j]->cos_sim >= value_min && sim_table_[i][j]->cos_sim <= value_max) {
        // Makes a pair of the word pair and its cosine similarity and pushs it
        // back to "list_of_pairs".
        word_vecs_indices = GetWordVecsIndices(i, j);
        list_of_pairs.push_back(std::make_pair(std::make_pair(word_vecs_[word_vecs_indices.first]->word, word_vecs_[word_vecs_indices.second]->word), sim_table_[i][j]->cos_sim));
      } else if (sim_table_[i][j]->eucl_dist >= value_min && sim_table_[i][j]->eucl_dist <= value_max) {
        // Makes a pair of the word pair and its Euclidean distance and pushs
        // it back to "list_of_pairs".
        word_vecs_indices = GetWordVecsIndices(i, j);
        list_of_pairs.push_back(std::make_pair(std::make_pair(word_vecs_[word_vecs_indices.first]->word, word_vecs_[word_vecs_indices.second]->word), sim_table_[i][j]->eucl_dist));
      }
    }
  }
  return list_of_pairs;
}

std::list<std::pair<std::pair<std::string, std::string>, double>> VecSimTable::SimilarPairs(const double similarity, std::string comparison_mode, const double range) {
// Returns a list of word pairs with a similar similarity value to a given one
// (either the cosine similarity or the Euclidean distance) using a range: E.g.
// if "range == 0.1" and the chosen criteria is the cosine similarity that
// would be "0.5", all word pairs with a cosine similarity between 0.4 and 0.6
// will be returned.
  const bool cos_sim((std::regex_match(VecStore::SetToLowerCase(comparison_mode), (std::regex) "eucl(idean)?([ _-])?dist(ance)?"))? false : true);
  const double value_min(similarity-range);
  const double value_max(similarity+range);
  std::list<std::pair<std::pair<std::string, std::string>, double>> list_of_pairs;
  for (int i = 0; i < vec_num_; ++i) {
    for (int j = 0; j < vec_num_-(i+1); ++j) {
      if (cos_sim && sim_table_[i][j]->cos_sim >= value_min && sim_table_[i][j]->cos_sim <= value_max)
        list_of_pairs.push_back(std::make_pair(std::make_pair(word_vecs_[i]->word, word_vecs_[j]->word), sim_table_[i][j]->cos_sim));
      else if (sim_table_[i][j]->eucl_dist >= value_min && sim_table_[i][j]->eucl_dist <= value_max)
        list_of_pairs.push_back(std::make_pair(std::make_pair(word_vecs_[i]->word, word_vecs_[j]->word), sim_table_[i][j]->eucl_dist));
    }
  }
  return list_of_pairs;
}

std::list<std::pair<std::pair<std::string, std::string>, double>> VecSimTable::MostSimilarPairs(std::string word0, std::string word1, std::string comparison_mode, const unsigned k) {
// Returns a list of the k most similar word pairs to a given word pair ("word0",
// "word1") with respect to either the cosine similarity or the Euclidean
// distance. If the words of the given word pair are not stored in the
// VecSimTable an empty list will be returned.
  if (!case_sensitive_) {
    word0 = VecStore::SetToLowerCase(word0);
    word1 = VecStore::SetToLowerCase(word1);
  }
  if (word0 == word1) {
    std::cout << "ERROR in MostSimilarPairs():: No real word pair selected (both words were \"" << word0 << "\"); returned an empty list." << std::endl;
    return std::list<std::pair<std::pair<std::string, std::string>, double>>();
  }
  const int l(GetIndex(word0));
  if (l < 0) {
    std::cout << "ERROR in MostSimilarPairs(): \"" << word0 << "\" couldn't be found; returned an empty list." << std::endl;
    return std::list<std::pair<std::pair<std::string, std::string>, double>>();
  }
  const int m(GetIndex(word1));
  if (m < 0) {
    std::cout << "ERROR in MostSimilarPairs(): \"" << word1 << "\" couldn't be found; returned an empty list." << std::endl;
    return std::list<std::pair<std::pair<std::string, std::string>, double>>();
  }
  const std::pair<int, int> sim_table_indices(GetSimTableIndices(l, m));
  std::pair<int, int> word_vecs_indices;
  const bool cos_sim((std::regex_match(VecStore::SetToLowerCase(comparison_mode), (std::regex) "eucl(idean)?([ _-])?dist(ance)?"))? false : true);
  const double central_value(((cos_sim)? sim_table_[sim_table_indices.first][sim_table_indices.second]->cos_sim : sim_table_[sim_table_indices.first][sim_table_indices.second]->eucl_dist));
  std::list<std::pair<std::pair<std::string, std::string>, double>> list_of_pairs;
  for (int i = 0; i < vec_num_; ++i) {
    for (int j = 0; j < vec_num_-(i+1); ++j) {
      if (i == sim_table_indices.first && j == sim_table_indices.second)
        continue; // skips the original word pair in question ("word0", "word1")
      if (list_of_pairs.size() < k) {
        // Makes a pair of the word pair and its cosine similarity or
        // Euclidean distance (depending on whether "cos_sim" is "true" or
        // not) and pushs it back to "list_of_pairs".
        word_vecs_indices = GetWordVecsIndices(i, j);
        list_of_pairs.push_back(std::make_pair(std::make_pair(word_vecs_[word_vecs_indices.first]->word, word_vecs_[word_vecs_indices.second]->word), ((cos_sim)? sim_table_[i][j]->cos_sim : sim_table_[i][j]->eucl_dist)));
      } else if (std::abs(central_value-((cos_sim)? sim_table_[i][j]->cos_sim : sim_table_[i][j]->eucl_dist)) < std::abs(central_value-list_of_pairs.front().second)) {
      // If the current value (in "sim_table_[i][j]") is closer to the
      // "central_value" than the value of "list_of_pairs.front()", this first
      // element of the list will be replaced with the current word pair and
      // its value.
        list_of_pairs.pop_front();
        word_vecs_indices = GetWordVecsIndices(i, j);
        list_of_pairs.push_front(std::make_pair(std::make_pair(word_vecs_[word_vecs_indices.first]->word, word_vecs_[word_vecs_indices.second]->word), ((cos_sim)? sim_table_[i][j]->cos_sim : sim_table_[i][j]->eucl_dist)));
        SortListOfWordPairs(list_of_pairs, central_value);
      }
      if (list_of_pairs.size() == k)
        SortListOfWordPairs(list_of_pairs, central_value);
    }
  }
  if (list_of_pairs.size() < k)
    SortListOfWordPairs(list_of_pairs, central_value);
  std::reverse(list_of_pairs.begin(), list_of_pairs.end()); // makes sure that the word pair with the most similar value to the word pair in question ("word0", "word1") is the first in the list
  return list_of_pairs;
}

std::list<std::pair<std::pair<std::string, std::string>, double>> VecSimTable::MostSimilarPairs(const double similarity, std::string comparison_mode, const unsigned k) {
// Returns a list of the k word pairs with the most similar similarity value to
// a given one (either the cosine similarity or the Euclidean distance).
  const bool cos_sim((std::regex_match(VecStore::SetToLowerCase(comparison_mode), (std::regex) "eucl(idean)?([ _-])?dist(ance)?"))? false : true);
  std::list<std::pair<std::pair<std::string, std::string>, double>> list_of_pairs;
  for (int i = 0; i < vec_num_; ++i) {
    for (int j = 0; j < vec_num_-(i+1); ++j) {
      if (list_of_pairs.size() < k)
        list_of_pairs.push_back(std::make_pair(std::make_pair(word_vecs_[i]->word, word_vecs_[j]->word), ((cos_sim)? sim_table_[i][j]->cos_sim : sim_table_[i][j]->eucl_dist)));
      else if (std::abs(similarity-((cos_sim)? sim_table_[i][j]->cos_sim : sim_table_[i][j]->eucl_dist)) < std::abs(similarity-list_of_pairs.front().second)) {
        list_of_pairs.pop_front();
        list_of_pairs.push_front(std::make_pair(std::make_pair(word_vecs_[i]->word, word_vecs_[j]->word), ((cos_sim)? sim_table_[i][j]->cos_sim : sim_table_[i][j]->eucl_dist)));
        SortListOfWordPairs(list_of_pairs, similarity);
      }
      if (list_of_pairs.size() == k)
        SortListOfWordPairs(list_of_pairs, similarity);
    }
  }
  if (list_of_pairs.size() < k)
    SortListOfWordPairs(list_of_pairs, similarity);
  std::reverse(list_of_pairs.begin(), list_of_pairs.end());
  return list_of_pairs;
}

void VecSimTable::SortListOfWordPairs(std::list<std::pair<std::pair<std::string, std::string>, double>>& list_of_pairs, const double central_value) {
// Sorts a list of word pairs and their corresponding values so that the pair
// whose value is the closest to "central_value" becomes the last element of
// the list.
  list_of_pairs.sort([&central_value](std::pair<std::pair<std::string, std::string>, double> x, std::pair<std::pair<std::string, std::string>, double> y) {return std::abs(central_value-x.second) > std::abs(central_value-y.second);});
}

int VecSimTable::GetIndex(const std::string& word) {
// Checks whether "word" is stored (using binary search for "word_vecs_" is
// sorted) and returns -1 if not and otherwise its index.
  int index, start(0), end(vec_num_-1);
  while (start <= end) {
    index = start+(end-start)/2;
    if (word_vecs_[index]->word == word)
      return index;
    else if (word.compare(word_vecs_[index]->word) < 0)
      end = --index;
    else
      start = ++index;
  }
  return -1;
}
