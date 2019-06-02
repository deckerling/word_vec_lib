// word_vec_lib.h

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

#ifndef WORD_VEC_LIB_WORD_VEC_LIB_H_INCLUDED_
#define WORD_VEC_LIB_WORD_VEC_LIB_H_INCLUDED_

#include <algorithm>
#include <list>
#include <math.h>
#include <numeric>
#include <regex>
#include <string>
#include <utility>
#include <vector>

struct WordVec { // word vector
  std::string word;
  std::vector<double> vec;
  WordVec* next; // enables chaining of "WordVec"s in the hash table created in the class "VecStore"
  WordVec(const std::string w, const std::vector<double> v) : word(w), vec(v), next(NULL) {}
};

namespace VecPrint {
// Functions to print (word) vectors and word pairs.
  void PrintVec(const std::vector<double>& vec);
  void PrintWordVec(const WordVec* wv);
  void PrintWordVecList(std::list<WordVec*> wv_list);
  void PrintWordPair(const std::pair<std::pair<std::string, std::string>, double>& word_pair);
  void PrintWordPairList(const std::list<std::pair<std::pair<std::string, std::string>, double>>& word_pair_list);
};

namespace VecCalc {
// Functions to perform mathematical operations on (word) vectors.

  template <typename T>
  T EuclideanNorm(const std::vector<T>& vec) {
    // Calculates and returns the Euclidean norm of "vec" (needed in order to
    // calculate the cosine similarity).
    T x(0);
    for (auto& element : vec)
      x += std::pow(element, 2);
    return std::sqrt(x);
  }

  template <typename T>
  T CosineSimilarity(const std::vector<T> vec0, const std::vector<T> vec1) {
  // Calculates and returns the cosine similarity of "vec0" and "vec1".
    return (std::inner_product(vec0.begin(), vec0.end(), vec1.begin(), 0.)/(EuclideanNorm(vec0)*EuclideanNorm(vec1)));
  }

  template <typename T>
  T CosineSimilarity(const WordVec* wv0, const WordVec* wv1) {
  // Calculates and returns the cosine similarity of two (word) vectors given the
  // "WordVec"s.
    return CosineSimilarity(wv0->vec, wv1->vec);
  }

  template <typename T>
  T EuclideanDistance(const std::vector<T> vec0, const std::vector<T> vec1) {
  // Calculates and returns the Euclidean distance between "vec0" and "vec1".
    T x(0);
    for (unsigned i = 0; i < vec0.size(); ++i)
      x += std::pow((vec0[i]-vec1[i]), 2);
    return std::sqrt(x);
  }

  template <typename T>
  T EuclideanDistance(const WordVec* wv0, const WordVec* wv1) {
  // Calculates and returns the Euclidean distance between two (word) vectors
  // given the "WordVec"s.
    return EuclideanDistance(wv0->vec, wv1->vec);
  }

  template <typename T>
  std::vector<T> Add(std::vector<T> vec0, const std::vector<T> vec1) {
  // Adds two vectors and returns the resulting vector. If both vectors do not
  // got the same size an empty vector will be returned.
    if (vec0.size() != vec1.size())
      return std::vector<T>();
    std::transform(vec0.begin(), vec0.end(), vec1.begin(), vec0.begin(), std::plus<T>());
    return vec0;
  }

  template <typename T>
  std::vector<T> Add(const WordVec* wv0, const WordVec* wv1) {
  // Adds two vectors and returns the resulting vector given the "WordVec"s.
    return Add(wv0->vec, wv1->vec);
  }

  template <typename T>
  std::vector<T> Add(std::vector<std::vector<T>> vecs) {
  // Adds all vectors stored in a std::vector ("vecs") and returns the resulting
  // vector. If the vectors do not got the same size an empty vector will be
  // returned.
    const unsigned vec_size(vecs[0].size());
    for (unsigned i = 1; i < vecs.size(); ++i) {
      if (vecs[i].size() != vec_size)
        return std::vector<T>();
      for (unsigned j = 0; j < vec_size; ++j)
        vecs[0][j] += vecs[i][j];
    }
    return vecs[0];
  }

  std::vector<double> Add(const std::list<WordVec*> wvs);

  template <typename T>
  std::vector<T> Subtract(std::vector<T> minuend_vec, const std::vector<T> substrahend_vec) {
  // Subtracts the second given vector from a first one and returns the
  // resulting vector. If both vectors do not got the same size an empty vector
  // will be returned.
    if (minuend_vec.size() != substrahend_vec.size())
      return std::vector<T>();
    std::transform(minuend_vec.begin(), minuend_vec.end(), substrahend_vec.begin(), minuend_vec.begin(), std::minus<T>());
    return minuend_vec;
  }

  template <typename T>
  std::vector<T> Subtract(const WordVec* minuend_wv, const WordVec* substrahend_wv) {
  // Given two "WordVec"s the second given vector will be subtracted from a
  // first one and returns the resulting vector.
    return Subtract(minuend_wv->vec, substrahend_wv->vec);
  }

  template <typename T>
  std::vector<T> GetAverageVec(std::vector<T> vec0, const std::vector<T> vec1) {
  // Calculates an average vector of two vectors by adding the vectors and
  // dividing every element of the resulting vector by 2, then returns this
  // resulting vector.
    vec0 = Add(vec0, vec1);
    for (auto& element : vec0)
      element /= 2;
    return vec0;
  }

  template <typename T>
  std::vector<T> GetAverageVec(std::vector<std::vector<T>> vecs) {
  // Calculates an average vector of all vectors stored in "vecs" by adding these
  // vectors and dividing every element of the resulting vector by the number of
  // vectors in "vecs", then returns this resulting vector.
    vecs[0] = Add(vecs);
    const unsigned num_of_vecs(vecs.size());
    for (auto& element : vecs[0])
      element /= num_of_vecs;
    return vecs[0];
  }

  template <typename T>
  std::vector<T> GetAverageVec(const WordVec* wv0, const WordVec* wv1) {
  // Calculates an average vector of the vectors of two given "WordVec"s by
  // adding the vectors and dividing every element of the resulting vector by 2,
  // then returns this resulting vector.
    return GetAverageVec(wv0->vec, wv1->vec);
  }

  template <typename T>
  std::vector<T> GetAverageVec(const std::list<WordVec*> wvs) {
  // Calculates an average vector of the vectors of the given "WordVec"s stored
  // in "wvs" by adding these vectors and dividing every element of the resulting
  // vector by the numer of "WordVec"s in "wvs", then returns this resulting
  // vector.
    std::vector<T> average_vec(Add(wvs));
    const unsigned num_of_vecs(wvs.size());
    for (auto& element : average_vec)
      element /= num_of_vecs;
    return average_vec;
  }
};

using namespace VecPrint;
using namespace VecCalc;

typedef std::list<WordVec*> WordVecList;
typedef std::list<std::pair<std::pair<std::string, std::string>, double>> WordPairList;

class VecStore {
// Class to store word vectors read from a file in a hash table on memory.
 public:
  VecStore(const std::string& file, const bool case_sensitive = true, const double percentage = 1.);
  ~VecStore();

  void PrintInfo();

  double GetSimilarity(const std::vector<std::string>& words, std::string comparison_mode = "");

  std::vector<double> GetVec(std::string word);

  std::vector<double> Add(std::string word0, std::string word1) {
  // Adds two (word) vectors given the "words".
    if (!case_sensitive_) {
      SetToLowerCase(word0);
      SetToLowerCase(word1);
    }
    return VecCalc::Add(GetVec(word0), GetVec(word1));
  }

  std::vector<double> Subtract(std::string minuend_word, std::string substrahend_word) {
  // Subtracts two (word) vectors given the "words".
    if (!case_sensitive_) {
      SetToLowerCase(minuend_word);
      SetToLowerCase(substrahend_word);
    }
    return VecCalc::Subtract(GetVec(minuend_word), GetVec(substrahend_word));
  }

  WordVec* ClosestWordVec(std::string word) {
    if (!case_sensitive_)
      word = SetToLowerCase(word);
    return ClosestWordVec(GetVec(word), word);
  }

  WordVec* ClosestWordVec(WordVec* wv) {
    if ((int)wv->vec.size() == vec_size_)
      return ClosestWordVec(wv->vec, ((!case_sensitive_)? SetToLowerCase(wv->word) : wv->word));
    return NULL;
  }

  WordVec* ClosestWordVec(const std::vector<double>& vec, const std::string& word = "");

  std::list<WordVec*> KClosestWordVecs(std::string word, const unsigned k = 3) {
    if (!case_sensitive_)
      word = SetToLowerCase(word);
    return KClosestWordVecs(GetVec(word), k, word);
  }

  std::list<WordVec*> KClosestWordVecs(WordVec* wv, const unsigned k = 3) {
    if ((int)wv->vec.size() == vec_size_)
      return KClosestWordVecs(wv->vec, k, ((!case_sensitive_)? SetToLowerCase(wv->word) : wv->word));
    return std::list<WordVec*>();
  }

  std::list<WordVec*> KClosestWordVecs(const std::vector<double>& vec, const unsigned k = 3, const std::string& word = "");

  WordVec* MostDistantWordVec(std::string word) {
    if (!case_sensitive_)
      word = SetToLowerCase(word);
    return MostDistantWordVec(GetVec(word), word);
  }

  WordVec* MostDistantWordVec(WordVec* wv) {
    if ((int)wv->vec.size() == vec_size_)
      return MostDistantWordVec(wv->vec, ((!case_sensitive_)? SetToLowerCase(wv->word) : wv->word));
    return NULL;
  }

  WordVec* MostDistantWordVec(const std::vector<double>& vec, const std::string& word = "");

  std::list<WordVec*> KMostDistantWordVecs(std::string word, const unsigned k = 3) {
  // Returns the k most distant WordVecs to a given word (if there is a vector
  // corresponding to this word stored).
    if (!case_sensitive_)
      word = SetToLowerCase(word);
    return SearchForMostDistantWordVecs(word, {}, k);
  }

  std::list<WordVec*> KMostDistantWordVecs(WordVec* wv, const unsigned k = 3) {
    if ((int)wv->vec.size() == vec_size_)
      return SearchForMostDistantWordVecs(((!case_sensitive_)? SetToLowerCase(wv->word) : wv->word), wv->vec, k);
    return std::list<WordVec*>();
  }

  std::list<WordVec*> KMostDistantWordVecs(const std::vector<double>& vec, const unsigned k = 3) {
  // Returns the k most distant WordVecs to a given vector.
    return SearchForMostDistantWordVecs("", vec, k);
  }

  static std::string SetToLowerCase(std::string& string);

 private:
  struct CloseWordVec {
    WordVec* word_vec;
    double distance;
    CloseWordVec(WordVec* wv, const double dist) : word_vec(wv), distance(dist) {}
  };
  std::vector<WordVec*> hash_table_;
  const std::string input_file_;
  const int vec_size_, vec_num_, hash_table_size_;
  const bool case_sensitive_; // if "false" all chars of all "words" ("std::string"s) will be set to lower case

  const int GetSizeOfVectors();

  const int CountVectors();

  bool HashTableIsValid() {
  // Returns "false" if no or only empty vectors were found and "true"
  // otherwise.
    return (vec_size_ >= 1 && vec_num_ >= 1);
  }

  void ReadVectorFile();

  void StoreVectors(const std::string& line);

  unsigned GetIndex(const std::string& key); // hash function

  std::vector<std::string> SplitLine(const std::string& line);

  unsigned GetNumOfWordVecs(const unsigned index);

  std::list<WordVec*> SearchForMostDistantWordVecs(const std::string& word, std::vector<double> vec, const unsigned k);

  void DeleteList(std::list<CloseWordVec*>& list);
};

class VecSimTable { // (word) vector similarity table
// Class to store word vectors read from a file in a vector on memory as well
// as their similarities that get calculated.
 public:
  VecSimTable(const std::string& file, const std::regex& pattern);
  VecSimTable(const std::string& file, const bool case_sensitive = true, const double percentage = 0.1);
  ~VecSimTable();

  void PrintInfo();

  std::vector<double> GetVec(std::string word);

  double GetCosSim(const std::pair<std::string, std::string>& word_pair) {
  // Returnes the cosine similarity of a word pair.
    return GetCosSim(word_pair.first, word_pair.second);
  }

  double GetCosSim(std::string word0, std::string word1);

  double GetEuclDist(const std::pair<std::string, std::string>& word_pair) {
  // Returnes the Euclidean distance between the word vectors of a word pair.
    return GetEuclDist(word_pair.first, word_pair.second);
  }

  double GetEuclDist(std::string word0, std::string word1);

  std::list<std::pair<std::pair<std::string, std::string>, double>> SimilarPairs(const std::pair<std::string, std::string>& word_pair, std::string comparison_mode, const double range = 0.1) {
    return SimilarPairs(word_pair.first, word_pair.second, comparison_mode, range);
  }

  std::list<std::pair<std::pair<std::string, std::string>, double>> SimilarPairs(std::string word0, std::string word1, std::string comparison_mode, const double range = 0.1);

  std::list<std::pair<std::pair<std::string, std::string>, double>> SimilarPairs(const double similarity, std::string comparison_mode, const double range);

  std::list<std::pair<std::pair<std::string, std::string>, double>> MostSimilarPairs(const std::pair<std::string, std::string>& word_pair, std::string comparison_mode, const unsigned k = 3) {
    return MostSimilarPairs(word_pair.first, word_pair.second, comparison_mode, k);
  }

  std::list<std::pair<std::pair<std::string, std::string>, double>> MostSimilarPairs(std::string word0, std::string word1, std::string comparison_mode, const unsigned k = 3);

  std::list<std::pair<std::pair<std::string, std::string>, double>> MostSimilarPairs(const double similarity, std::string comparison_mode, const unsigned k = 3);

 private:
  struct SimMeasures {
    const double cos_sim;
    const double eucl_dist;
    SimMeasures(const double cos, const double eucl) : cos_sim(cos), eucl_dist(eucl) {}
  };
  const int vec_size_;
  const bool case_sensitive_; // if "false" all chars of all "words" ("std::string"s) will be set to lower case
  int vec_num_;
  std::vector<WordVec*> word_vecs_;
  std::vector<std::vector<SimMeasures*>> sim_table_; // "sim_table_" will be shaped like a triangle

  const int GetSizeOfVectors(const std::string& file);

  void StoreWordVecs(const std::string& file, const bool case_sensitive, const double percentage);

  static bool SortIt(WordVec* wv0, WordVec* wv1) {
    return (wv0->word < wv1->word);
  }

  const int CountVectors(const std::string& file);

  void StoreVectors(const std::string& line, const unsigned i);

  int StoreVecsWithPattern(const std::string& file, const std::regex& pattern);

  std::vector<std::string> SplitLine(const std::string& line);

  void CalculateSimilarities();

  void SortListOfWordPairs(std::list<std::pair<std::pair<std::string, std::string>, double>>& list_of_pairs, const double central_value);

  int GetIndex(const std::string& word);

  std::pair<int, int> GetSimTableIndices(int i, int j) {
  // Given the indices of two "WordVec"s in "word_vecs_" the corresponding
  // indices of the word pair in "sim_table_" will be returned.
    if (i > j)
      std::swap(i, j);
    j -= i+1;
    return std::make_pair(i, j);
  }

  std::pair<int, int> GetWordVecsIndices(int i, int j) {
  // Given the indices of a word pair in "sim_table_" the corresponding indices
  // of the word pair in "word_vecs_" will be returned.
    if (i < j)
      return std::make_pair(i, ++j);
    j += i+1;
    return std::make_pair(i, j);
  }
};

#endif // WORD_VEC_LIB_WORD_VEC_LIB_H_
