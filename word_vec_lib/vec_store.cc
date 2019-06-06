// vec_store.cc

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

VecStore::VecStore(const std::string& input_file, const bool case_sensitive, const double percentage)
    : input_file_(input_file),
      vec_size_(GetSizeOfVectors()),
      vec_num_(CountVectors()*((percentage > 1)? 1 : percentage)+0.5),
      hash_table_size_((vec_num_ > 19)? vec_num_/20 : 1), // in some cases you may have to adjust the denominator in order to reduce the number of collisions
      case_sensitive_(case_sensitive) {
  std::vector<WordVec*> HT(hash_table_size_);
  hash_table_ = HT;
  ReadVectorFile();
}

VecStore::~VecStore() {
  WordVec* current_word_vector;
  for (auto& bucket : hash_table_) {
    while (bucket) {
      current_word_vector = bucket;
      bucket = bucket->next;
      delete current_word_vector;
    }
  }
}

const int VecStore::GetSizeOfVectors() {
// Returns the number of dimensions of the word vectors found in "input_file_"
// (assuming that each line of the file contains exactly one vector and that
// all the word vectors got the same number of dimensions).
  std::ifstream file_stream(input_file_);
  if (!file_stream.is_open()) {
    std::cout << "ERROR: OPENING \"" << input_file_ << "\" FAILED!\nMake sure that the file exists and that the path is correct." << std::endl;
    return -1;
  } else if (file_stream.bad()) {
    std::cout << "ERROR: OPENING \"" << input_file_ << "\" FAILED!" << std::endl;
    return -1;
  }
  std::cout << "CREATING A \"VecStore\"." << '\n' << "Input file (\"word vector file\"): " << input_file_ << '\n';
  std::cout << "\tChecking the size of the word vectors..." << std::endl;
  std::string line;
  std::getline(file_stream, line);
  std::cout << "\t---Done." << '\n';
  return std::count(line.begin(), line.end(), ' ');
}

const int VecStore::CountVectors() {
// Returns the number of word vectors in "input_file_" (assuming that each line
// of the file contains exactly one vector).
  if (vec_size_ < 1)
    return -1;
  unsigned vector_num(0);
  std::ifstream file_stream(input_file_);
  std::cout << "\tCounting the word vectors..." << std::endl;
  std::string line;
  while (std::getline(file_stream, line))
    vector_num++; // this might cause problems if your "input_file_" is not a valid word vector file because actually lines and not vectors are counted
  std::cout << "\t---Done." << std::endl;
  return vector_num;
}

void VecStore::ReadVectorFile() {
// Reads "input_file_" and passes the lines to "VecStore::StoreVectors()".
  if (!HashTableIsValid())
    return;
  std::cout << "\tLoading data..." << std::endl;
  std::string line;
  std::ifstream vector_file_stream(input_file_);
  unsigned remaining_vecs(vec_num_);
  while (std::getline(vector_file_stream, line) && remaining_vecs-- != 0)
    StoreVectors(line);
  std::cout << "\t---Completed." << std::endl;
}

void VecStore::StoreVectors(const std::string& line) {
// Stores the word vectors in the hash table on memory.
  const std::vector<std::string> tokens(SplitLine(line));
  std::vector<double> vector(vec_size_);
  const unsigned index(GetIndex(tokens[0]));
  for (int i = 0; i < vec_size_; ++i)
    // Converts the (std::string) elements of "tokens" that represent the
    // values of the word vector into the type "double".
    vector[i] = atof(tokens[i+1].c_str());
  if (hash_table_[index]) { // collisions are handled by chaining using a linked list
    WordVec* current_word_vector = hash_table_[index];
    while (current_word_vector->next)
      current_word_vector = current_word_vector->next;
    current_word_vector->next = new WordVec(tokens[0], vector);
  } else
    hash_table_[index] = new WordVec(tokens[0], vector);
}

std::vector<std::string> VecStore::SplitLine(const std::string& line) {
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
    tokens[0] = SetToLowerCase(tokens[0]);
  return tokens;
}

unsigned VecStore::GetIndex(const std::string& key) { // hash function
// Returns the "index" of the bucket of the hash table the "key" corresponds to.
  unsigned hash(0), j(1), k(0);
  const std::vector<int> primes({179, 181, 191, 193, 197, 199, 211, 223, 227, 229});
  for (unsigned i = 0; i < key.length(); ++i) {
    if (i == (primes.size()*j)) {
      k = 0;
      j++;
    }
    hash += (int) key[i]*primes[k]; // multiplies each ASCII-value of "key" with an element of "primes" (i.e. a prime number)
    k++;
  }
  const unsigned index(hash%hash_table_size_);
  if (index < 0)
    return 0;
  if (index > (unsigned)(hash_table_size_-1))
    return hash_table_size_-1;
  return index;
}

void VecStore::PrintInfo() {
// Calculates some of the numeric information of the created hash and prints
// them.
  int num_of_empty_buckets = 0, highest_num_of_nodes_in_a_bucket = 0, word_vector_num;
  for (int i = 0; i < hash_table_size_; ++i) {
    word_vector_num = GetNumOfWordVecs(i);
    if (word_vector_num == 0)
      num_of_empty_buckets++;
    if (highest_num_of_nodes_in_a_bucket < word_vector_num)
      highest_num_of_nodes_in_a_bucket = word_vector_num;
  }
  std::cout << "Basic information about the \"VecStore\":" << '\n';
  // Prints the most important information regarding the created hash table.
  std::cout << "\tSize of vectors = " << vec_size_ << '\n';
  std::cout << "\tNumber of stored word vectors = " << vec_num_ << '\n';
  std::cout << "\tNumber of buckets = " << hash_table_size_ << '\n';
  std::cout << "\tLoad factor = " << (double) vec_num_/hash_table_size_ << '\n';
  std::cout << "\tNumber of empty buckets = " << num_of_empty_buckets << '\n';
  std::cout << "\tPercentage of empty buckets = " << 100*((double) num_of_empty_buckets/hash_table_size_) << " %\n";
  std::cout << "\tHighest number of word vectors in a bucket = " << highest_num_of_nodes_in_a_bucket << '\n';
  std::cout << "\tPercentage of vectors in mostly filled bucket = " << 100*((double) highest_num_of_nodes_in_a_bucket/vec_num_) << '\n';
  std::cout << "\tThis \"VecStore\" works " << ((case_sensitive_)? "case sensitive." : "case insensitive.") << std::endl;
}

unsigned VecStore::GetNumOfWordVecs(const unsigned index) {
// Counts the number of word vectors (nodes) in a bucket of the
// "HashTableOnMemory" and returns this number.
  unsigned count(0);
  if (!hash_table_[index])
    return 0;
  else {
    for (WordVec* it = hash_table_[index]; it; it = it->next)
      count++;
  }
  return count;
}

double VecStore::GetSimilarity(const std::vector<std::string>& words, std::string comparison_mode) {
// Starts searching for the word vectors corresponding to the "words" by
// passing the "words" to "GetVector()". If a word cannot be found in the
// "hash_table_", "GetVector()" returns an empty vector. If so, the
// method stops by returning NaN and printing an error message. If both word
// vectors are found, their cosine similarity or Euclidean distance will be
// returned (depending on the "comparison_mode"; by default it is the cosine
// similarity).
  std::vector<std::vector<double>> vectors(2, std::vector<double>(vec_size_));
  for (unsigned i = 0; i < words.size(); ++i) {
    vectors[i] = GetVec(words[i]);
    if (vectors[i].empty()) {
      std::cout << "ERROR in GetSimilarity(): \"" << words[i] << "\" couldn't be found." << std::endl;
      return std::numeric_limits<double>::quiet_NaN();
    }
  }
  if (std::regex_match(SetToLowerCase(comparison_mode), (std::regex) "eucl(idean)?([ _-])?dist(ance)?"))
    return VecCalc::EuclideanDistance(vectors[0], vectors[1]);
  else
    return VecCalc::CosineSimilarity(vectors[0], vectors[1]);
}

std::vector<double> VecStore::GetVec(std::string word) {
// Given a word (std::string) this method returns the corresponding vector if
// the word and its vector are stored in the "VecStore" object; if not, an
// empty vector will be returned, and an error message will be printed.
  if (!case_sensitive_)
    word = SetToLowerCase(word);
  const int index(GetIndex(word));
  if (hash_table_[index]) {
    for (WordVec* it = hash_table_[index]; it; it = it->next)
      if (it->word == word) return it->vec;
  }
  std::cout << "ERROR in GetVec(): \"" << word << "\" couldn't be found in your data; returned an empty vector." << std::endl;
  return std::vector<double>();
}

WordVec* VecStore::ClosestWordVec(const std::vector<double>& vec, const std::string& word) {
// Finds the closest vector to a given word vector (with regard to the
// Euclidean distance). If a vector ("vec") is given, the closest vector in the
// hash table ("VecStore") to this given vector will be returned. If only a
// "word" is given it will be checked whether a corresponding word vector is
// stored - if so the closest vector to this word vector will be returned
// (otherwise "NULL" will be returned).
  if (vec.empty())
    return NULL; // if no vector corresponding to the "word" is stored in the hash table NULL will be returned
  WordVec* cur, *closest_vec; // current bucket and closest word vector
  double min_distance(9999999), cur_distance;
  for (auto& bucket : hash_table_) {
    cur = bucket;
    while (cur) {
      if (cur->word != word && min_distance > (cur_distance = VecCalc::EuclideanDistance(vec, cur->vec))) {
        min_distance = cur_distance;
        closest_vec = cur;
      }
      cur = cur->next;
    }
  }
  if (closest_vec->word.empty() || closest_vec->vec.empty())
    return NULL;
  else
    return closest_vec;
}

std::list<WordVec*> VecStore::KClosestWordVecs(const std::vector<double>& vec, const unsigned k, const std::string& word) {
// Finds the k closest vectors to a given word vector (with regard to the
// Euclidean distance). If a vector ("vec") is given, the k closest vectors in
// the hash table ("VecStore") to this given vector will be returned in a
// std::list<WordVec*>. If only a "word" is given it will be checked whether a
// corresponding word vector is stored - if so the k closest vectors to this
// word vector will be returned (otherwise an empty list will be returned).
  if (vec.empty())
    return std::list<WordVec*>(); // if no vector corresponding to the "word" is stored in the hash table an empty list will be returned
  std::list<CloseWordVec*> closest;
  std::list<WordVec*> kClosest;
  double distance;
  WordVec* cur;
  for (auto& bucket : hash_table_) {
    cur = bucket;
    while (cur) {
      if (cur->word != word) {
        distance = VecCalc::EuclideanDistance(vec, cur->vec);
        if (closest.size() < k)
          closest.push_back(new CloseWordVec(cur, distance));
        else if (distance < closest.front()->distance) {
          closest.front()->word_vec = cur;
          closest.front()->distance = distance;
          closest.sort([](CloseWordVec* x, CloseWordVec* y) {return (x->distance > y->distance);}); // makes sure that "closest.front()" contains the WordVec with the largest distance to "vec" (within "closest")
        }
        if (closest.size() == k)
          closest.sort([](CloseWordVec* x, CloseWordVec* y) {return (x->distance > y->distance);});
      }
      cur = cur->next;
    }
  }
  if (closest.size() < k)
    closest.sort([](CloseWordVec* x, CloseWordVec* y) {return (x->distance > y->distance);});
  // Reverses the sorted "closest" into "kClosest" so that the first
  // vector will be the most distant one and deletes "closest".
  for (auto it : closest)
    kClosest.push_front(it->word_vec);
  DeleteList(closest);
  return kClosest;
}

WordVec* VecStore::MostDistantWordVec(const std::vector<double>& vec, const std::string& word) {
// Finds the most distant vector to a given word vector (with regard to the
// Euclidean distance). If a vector ("vec") is given, the most distant vector
// in the hash table ("VecStore") to this given vector will be returned. If
// only a "word" is given it will be checked whether a corresponding word
// vector is stored - if so the most distant vector to this word vector will be
// returned (otherwise "NULL" will be returned).
  if (vec.empty())
    return NULL; // if no vector corresponding to the "word" is stored in the hash table NULL will be returned
  WordVec* cur, *most_distant_vec; // current bucket and most distant word vector
  double max_distance(0), cur_distance;
  for (auto& bucket : hash_table_) {
    cur = bucket;
    while (cur) {
      if (cur->word != word && max_distance < (cur_distance = VecCalc::EuclideanDistance(vec, cur->vec))) {
        max_distance = cur_distance;
        most_distant_vec = cur;
      }
      cur = cur->next;
    }
  }
  if (most_distant_vec->word.empty() || most_distant_vec->vec.empty())
    return NULL;
  else
    return most_distant_vec;
}

std::list<WordVec*> VecStore::SearchForMostDistantWordVecs(const std::string& word, std::vector<double> vec, const unsigned k) {
// Finds the k most distant vectors to a given word vector (with regard to the
// Euclidean distance). If a vector ("vec") is given, the k most distant
// vectors in the hash table ("VecStore") to this given vector will be returned
// in a std::list<WordVec*>. If only a "word" is given it will be checked
// whether a corresponding word vector is stored - if so the k most distant
// vectors to this word vector will be returned (otherwise an empty list will
// be returned).
  if (vec.empty()) {
    vec = GetVec(word);
    if (vec.empty()) return std::list<WordVec*>(); // if no vector corresponding to the "word" is stored in the hash table an empty list will be returned
  }
  std::list<CloseWordVec*> most_distant;
  std::list<WordVec*> kMostDistant;
  double distance;
  WordVec* cur;
  for (auto& bucket : hash_table_) {
    cur = bucket;
    while (cur) {
      if (cur->word != word) {
        distance = VecCalc::EuclideanDistance(vec, cur->vec);
        if (most_distant.size() < k)
          most_distant.push_back(new CloseWordVec(cur, distance));
        else if (distance > most_distant.front()->distance) {
          most_distant.front()->word_vec = cur;
          most_distant.front()->distance = distance;
          most_distant.sort([](CloseWordVec* x, CloseWordVec* y) {return (x->distance < y->distance);}); // makes sure that "most_distant.front()" contains the WordVec with the smallest distance to "vec" (within "most_distant")
        }
        if (most_distant.size() == k)
          most_distant.sort([](CloseWordVec* x, CloseWordVec* y) {return (x->distance < y->distance);});
      }
      cur = cur->next;
    }
  }
  if (most_distant.size() < k)
    most_distant.sort([](CloseWordVec* x, CloseWordVec* y) {return (x->distance < y->distance);});
  // Reverses the sorted "most_distant" into "kMostDistant" so that the first
  // vector will be the most distant one and deletes "most_distant".
  for (auto it : most_distant)
    kMostDistant.push_front(it->word_vec);
  DeleteList(most_distant);
  return kMostDistant;
}

std::string VecStore::SetToLowerCase(std::string& string) {
// Sets every character of a string to lower case and returns the string as a
// whole.
  for (auto& character : string)
    character = std::tolower(character);
  return string;
}

void VecStore::DeleteList(std::list<CloseWordVec*>& list) {
// Destructs "std::list"s containing pointers.
  while (!list.empty()) {
    delete list.front();
    list.pop_front();
  }
}
