// example.cc

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

#include "word_vec_lib/word_vec_lib.h"

using namespace std;

int main() {
// Some example operations using the "wordVecLib.h" on the word vectors in
// "example_data/example_word_vecs.txt".

  // Create a "VecStore" (using the default parameters for "case_sensitive" and
  // "percentage").
  VecStore my_vecs("example_data/example_word_vecs.txt");

  // Print basic information about the "VecStore" we have created.
  my_vecs.PrintInfo();

  // Get and print the vector to the word "und".
  cout << "The vector to the word \"und\" is:" << '\n';
  PrintVec(my_vecs.GetVec("und"));

  cout << endl;

  // Calculate the cosine similarity of 2 word vectors and the Euclidean
  // distance between them.
  double cosine_similarity = my_vecs.GetSimilarity({"Mann", "Frau"});
  // alternative:
    cosine_similarity = CosineSimilarity(my_vecs.GetVec("Mann"), my_vecs.GetVec("Frau"));
  double euclidean_distance = my_vecs.GetSimilarity({"Mann", "Frau"}, "eucldist");
  // alternative:
    euclidean_distance = EuclideanDistance(my_vecs.GetVec("Mann"), my_vecs.GetVec("Frau"));
  cout << "The cosine similarity of \"Mann\" and \"Frau\" = " << cosine_similarity << '\n' << "The Euclidean distance between \"Mann\" and \"Frau\" = " << euclidean_distance << endl;

  cout << endl;

  // Finds the closest "WordVec" to "Haus" (with respect to the Euclidean
  // distance).
  WordVec* closest_word_vec = my_vecs.ClosestWordVec("Haus");
  cout << "The closest word vector to the word \"Haus\" is:" << '\n';
  PrintWordVec(closest_word_vec);

  cout << endl;

  // Finds the 5 closest "WordVec"s to "Haus" (with respect to the Euclidean
  // distance).
  WordVecList closest_word_vecs = my_vecs.KClosestWordVecs("Haus", 5); // the type "WordVecList" equals "std::list<WordVec*>"
  cout << "The five closest word vectors to the word \"Haus\" are:" << '\n';
  PrintWordVecList(closest_word_vecs);

  cout << endl;

  // Finds the most distant "WordVec" to "und" (with respect to the Euclidean
  // distance).
  WordVec* most_distant_word_vec = my_vecs.MostDistantWordVec("und");
  cout << "The most distant word vector to the word \"und\" is:" << '\n';
  PrintWordVec(most_distant_word_vec);

  cout << endl;

  // Calculate an average vector to the word vectors of "kalt" and "warm" and
  // find the 5 closest word vectors to it.
  vector<double> vec_of_interest = GetAverageVec(my_vecs.GetVec("kalt"), my_vecs.GetVec("warm"));
  closest_word_vecs = my_vecs.KClosestWordVecs(vec_of_interest, 7);
  cout << "The seven closest word vectors to an average vector of the vectors of \"kalt\" and \"warm\" are:" << '\n';
  PrintWordVecList(closest_word_vecs);

  cout << endl;

  // Examples regarding a word vector that does not exist in "my_vecs".
  cout << "Examples regarding a word vector that does not exist in our \"VecStore\":" << '\n';
  cosine_similarity = CosineSimilarity(my_vecs.GetVec("Hund"), my_vecs.GetVec("Frau")); // "Frau" is stored, "Hund" is not
  cout << cosine_similarity << endl;
  PrintVec(my_vecs.GetVec("Hund"));
  PrintVec(GetAverageVec(my_vecs.GetVec("Frau"), my_vecs.GetVec("Hund")));
  PrintWordVecList(my_vecs.KClosestWordVecs(my_vecs.GetVec("Hund"), 5));
  PrintVec(Add(my_vecs.GetVec("Frau"), my_vecs.GetVec("Hund")));

  cout << endl;

  // Create a "VecStore" using the customized parameters for "case_sensitive"
  // and "percentage" (all words will be handled without case sensitivity and
  // only the first 50% of the word vectors stored in the file
  // ("example_data/example_word_vecs.txt") will be stored in our "VecStore");
  VecStore my_vecs2("example_data/example_word_vecs.txt", false, 0.5);

  // Print basic information about the new "VecStore" we have just created.
  my_vecs2.PrintInfo();

  // Calculate the cosine similarity of 2 word vectors and the Euclidean
  // distance between them showing that "my_vecs2" is case insensitive.
  cosine_similarity = my_vecs2.GetSimilarity({"Mann", "frau"});
  euclidean_distance = my_vecs2.GetSimilarity({"mann", "FrAU"}, "eucldist");
  cout << "The cosine similarity of \"mann\" and \"frau\" = " << cosine_similarity << '\n' << "The Euclidean distance between \"mann\" and \"frau\" = " << euclidean_distance << endl;

  cout << endl;

  // Create a "VecSimTable" using a regex mattern (i.e. only word vectors with
  // the "word" matching this pattern will be stored in the "VecSimTable"). In
  // this case all words ending with the German suffix "-haft" (or one of its
  // derivatives) will be stored.
  VecSimTable vst("example_data/example_word_vecs.txt", (regex) ".+[^(sc)-]haft(e([mnrs])?)?");

  // Print basic information about the "VecSimTable" we have created.
  vst.PrintInfo();

  // Calculate the cosine similarity of 2 word vectors and the Euclidean
  // distance between them.
  cosine_similarity = vst.GetCosSim("grauenhaft", "grauenhafte");
  euclidean_distance = vst.GetEuclDist("grauenhaft", "grauenhafte");
  cout << "The cosine similarity of \"grauenhaft\" and \"grauenhafte\" = " << cosine_similarity << '\n' << "The Euclidean distance between \"grauenhaft\" and \"grauenhafte\" = " << euclidean_distance << endl;

  cout << endl;

  // Finds the 3 most similar pairs to the pair "grauenhaft"/"grauenhafte" with
  // respect to their cosine similarity.
  WordPairList most_similar_pairs = vst.MostSimilarPairs("grauenhaft", "grauenhafte", "cos_sim", 3); // the type "WordPairList" equals "std::list<std::pair<std::pair<std::string, std::string>, double>>"
  cout << "The three most similar word pairs to the word pair \"grauenhaft\" / \"grauenhafte\" are:" << '\n';
  PrintWordPairList(most_similar_pairs);

  cout << endl;

  // Finds the most similar pair to all the word pairs in "most_similar_pairs"
  // with respect to their cosine similarity.
  for (auto word_pair_with_value : most_similar_pairs)
    PrintWordPair((vst.MostSimilarPairs(word_pair_with_value.first, "cos_sim", 1)).front());

  cout << endl;

  // Finds similar pairs in a range of 0.1 to the pair "grauenhaft"/
  // "grauenhafte" with respect to their Euclidean distance.
  cout << "All pairs with a Euclidean distance +/-0.1 to the Euclidean distance of the word pair \"grauenhaft\" / \"grauenhafte\" are:" << '\n';
  WordPairList similar_pairs_in_range = vst.SimilarPairs("grauenhaft", "grauenhafte", "eucl_dist", 0.1);
  PrintWordPairList(similar_pairs_in_range);

  cout << endl;

  // Create a "VecSimTable" (using the case sensitivity and storing 100% of the
  // word vectors in the file (no regex pattern is used)).
  VecSimTable vst2("example_data/example_word_vecs.txt", true, 1);

  // Print basic information about the "VecSimTable" we have created.
  vst2.PrintInfo();

  // Examples regarding a word vector that does not exist in "vsm2".
  cout << "Examples regarding a word vector that does not exist in our \"VecSimTable\":" << '\n';
  cosine_similarity = vst2.GetEuclDist("Frau", "Hund"); // "Frau" is stored, "Hund" is not
  cout << cosine_similarity << endl;
  PrintWordPairList(vst2.SimilarPairs("Frau", "Hund", "eucl_dist", 0.07));
  PrintWordPairList(vst2.MostSimilarPairs("Hund", "Frau", "cos_sim", 4));

  return 0;
}
