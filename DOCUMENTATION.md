# *word_vec_lib*-DOCUMENTATION

**Content:**
1. [Files](https://github.com/deckerling/word_vec_lib/new/master#1-files)
2. Organization of *word_vec_lib*
   * 2.1 [`WordVec` (struct)](https://github.com/deckerling/word_vec_lib/new/master#21-wordvec-struct)
   * 2.2 [`WordVecList` (type)](https://github.com/deckerling/word_vec_lib/new/master#22-wordveclist-type)
   * 2.3 [`WordPairList` (type)](https://github.com/deckerling/word_vec_lib/new/master#23-wordpairlist-type)
   * 2.4 [`VecStore` (class)](https://github.com/deckerling/word_vec_lib/new/master#24-vecstore-class)
   * 2.5 [`VecSimTable` (class)](https://github.com/deckerling/word_vec_lib/new/master#25-vecsimtable-class)
   * 2.6 [`VecCalc` (namespace)](https://github.com/deckerling/word_vec_lib/new/master#26-veccalc-namespace)
   * 2.7 [`VecPrint` (namespace)](https://github.com/deckerling/word_vec_lib/new/master#27-vecprint-namespace)
3. [License](https://github.com/deckerling/word_vec_lib/new/master#3-license)


## 1. Files
*word_vec_lib* consists of four files. "[*vec_store.cc*](https://github.com/deckerling/word_vec_lib/blob/master/word_vec_lib/vec_store.cc)" contains implementations for an on-memory hash table storing all your word vectors, in a similar way "[*vec_sim_table.cc*](https://github.com/deckerling/word_vec_lib/blob/master/word_vec_lib/vec_sim_table.cc)" contains implementations for an on-memory table containing the similarities between all of your word vectors easily accessible. In "[*miscellaneous_vec_functions.cc*](https://github.com/deckerling/word_vec_lib/blob/master/word_vec_lib/miscellaneous_vec_functions.cc)" you will find above all certain print-functions for your word vectors. Last but not least "[*word_vec_lib.h*](https://github.com/deckerling/word_vec_lib/blob/master/word_vec_lib/word_vec_lib.h)" holds those files together and also provides some mathematical operations you can perform on your word vectors.

## 2. Organization of *word_vec_lib*

### 2.1 `WordVec` (struct)
`WordVec` is a basic struct in *word_vec_lib*. It consists of a `std::string` ("word") and a `std::vector<double>` ("vec").

    WordVec* my_word_vec = new WordVec("dog", {0.1, 1.2, 2.3, 3.4, 4.5});
    my_word_vec->word = "cat";
    my_word_vec->vec = {5, 4, 3, 2, 1};

### 2.2 `WordVecList` (type)
A `WordVecList` equals a `std::list<WordVec*>`.

### 2.3 `WordPairList` (type)
A `WordPairList` equals a `std::list<std::pair<std::string, std::string>, double>`. Usually the two `std::string`s contain two words and the `double` a value of the similarity of their word vectors such as their cosine similarity or the Euclidean distance between them.

### 2.4 `VecStore` (class)
The `VecStore` class allows you to read your word vectors from a file into a hash table on memory making them easily accessible in order to perform certain operations on them. The word itself (as a `std::string`) will be used as key; collisions are handled by chaining, so the time complexity of find-functions is nearly O(1).

#### 2.4.1 The constructor `VecStore::VecStore(const std::string& file, const bool case_sensitive = true, const double percentage = 1)`
The constructor needs the path of a file containing your word vectors (as a `std::string`); but there are also two optional arguments that can be used:  
The case sensitivity (of type `bool`) is `true` by default. If you change it to `false` all word vectors won’t be stored case sensitive; also the words you will enter and search for later won’t be regarded case sensitively.
`percentage` refers to the percentage of word vectors of your word vector file you want to store in your `VecStore`. By default all of them will be stored; you can use a (`double`) value between 0 and 1 to set the percentage. E.g. if you use the value 0.5 the first 50% of the word vectors stored in your file will be stored in your `VecStore` object (this is why it is helpful if the word vectors in your file are saved in some kind of order (e.g. from most frequent words to less frequent (appropriate files can be created with [Standford’s *GloVe* implementation](https://github.com/stanfordnlp/GloVe) for example))).

    VecStore my_vec_store0("my_word_vecs.txt"); // constructor using the default parameters
    VecStore my_vec_store1("my_word_vecs.txt", false, 0.75); // constructor using costumized parameters

#### 2.4.2 `void VecStore::PrintInfo()` (method)
Prints the basic information about a `VecStore` object, such as the size and number of word vectors stored and regarding the created hash table its number of buckets, its load factor, its number of empty buckets, the percentage of empty buckets, the highest number of word vectors in a bucket, the percentage of word vectors in this bucket and whether the `VecStore` object works case sensitive or not.

    VecStore my_vecs("my_word_vecs.txt");
    my_vecs.PrintInfo();

#### 2.4.3 `std::vector<double> VecStore::GetVec(std::string word)` (method)
Returns the "vector" of a word vector (`WordVec`), if the "word" is stored in the `VecStore` object; otherwise an empty `std::vector<double>` will be returned and an error message will be printed.

    VecStore my_vecs("my_word_vecs.txt");
    std::vector<double> dog_vec = my_vecs.GetVec("dog");

#### 2.4.4 `double VecStore::GetSimilarity(const std::vector<std::string>& words, std::string comparison_mode = "")` (method)
Given a `std::vector<std::string>` containing two "words" this method searches for their corresponding vectors and returns either their cosine similarity or the Euclidean distance between them (depending on the value of "comparison_mode"). If at least one of the words couldn’t be found `NaN` will be returned and an error message will be printed.  
If you enter "Euclidean distance", "eucldist", "euclidean_distance" or something similar (the regex pattern "eucl(idean)?([ _-])?dist(ance)?" is used), the comparison will return the Euclidean distance between both word vectors, otherwise their cosine similarity will be returned.

    VecStore my_vecs("my_word_vecs.txt");
    double cosine_similarity, euclidean_distance;
    cosine_similarity = my_vecs.GetSimilarity({"dog", "cat"});
    euclidean_distance = my_vecs.GetSimilarity({"dog", "cat"}, "eucldist");

#### 2.4.5 `std::vector<double> VecStore::Add(std::string word0, std::string word1)` (method)
Adds the vectors of two given words and returns the resulting vector, if both are stored in the `VecStore` object; otherwise an empty `std::vector<double>` will be returned.

    VecStore my_vecs("my_word_vecs.txt");
    std::vector<double> catwoman_vec = my_vecs.Add("cat", "woman");

#### 2.4.6 `std::vector<double> VecStore::Subtract(std::string minuend_word, std::string subtrahend_word)` (method)
Subtracts the vector corresponding to the second given word ("subtrahend_word") from the first one ("minuend_word") and returns the resulting vector, if both words are stored in the `VecStore` object; otherwise an empty `std::vector<double>` will be returned.

    VecStore my_vecs("my_word_vecs.txt");
    std::vector<double> woman_vec = my_vecs.Add("catwoman", "cat");

#### 2.4.7 `WordVec* VecStore::ClosestWordVec(...)` (method)
Returns the closest word vector to a given one (with respect to their Euclidean distance), if the given one is stored in the `VecStore` object or an actual `std::vector<double>` is given – otherwise `NULL` will be returned. The given word vector can be determined by one of the following three possible arguments:
* a `std::string`,
* a (struct) `WordVec*`,
* a `std::vector<double>`.

The time complexity is O(*n*).

    VecStore my_vecs("my_word_vecs.txt");
    WordVec* closest;
    closest = my_vecs.ClosestWordVec("cat"); // using a std::string as argument
    WordVec* dog_word_vec = new WordVec("dog", {0, 1, 2, 3, 4});
    closest = my_vecs.ClosestWordVec(dog_word_vec); // using a WordVec* as argument
    closest = my_vecs.ClosestWordVec(dog_word_vec->vec); // using a std::vector<double> as argument

#### 2.4.8 `WordVecList VecStore::KClosestWordVecs(..., const unsigned k = 3)` (method)
Returns a `WordVecList` containing the *k* closest word vectors to a given one (with respect to their Euclidean distance), if the given one is stored in the `VecStore` object or an actual `std::vector<double>` is given – otherwise an empty `WordVecList` will be returned. The given word vector can be determined by one of the following three possible arguments:
* a `std::string`,
* a (struct) `WordVec*`,
* a `std::vector<double>`.

Furthermore, *k* can be defined by another argument (of type `unsigned`); by default it is 3.  
The time complexity is O(*n*).

    VecStore my_vecs("my_word_vecs.txt");
    WordVecList closest_vecs; // WordVecList == std::list<WordVec*>
    closest_vecs = my_vecs.KClosestWordVecs("cat"); // using a std::string as argument and k = 3 (default)
    WordVec* dog_word_vec = new WordVec("dog", {0, 1, 2, 3, 4});
    closest_vecs = my_vecs.KClosestWordVecs(dog_word_vec, 5); // using a WordVec* as argument and k = 5
    closest_vecs = my_vecs.KClosestWordVecs(dog_word_vec->vec); // using a std::vector<double> as argument and k = 3 (default)

#### 2.4.9 `WordVec* VecStore::MostDistantWordVec(...)` (method)
Returns the most distant word vector to a given one (with respect to their Euclidean distance), if the given one is stored in the `VecStore` object or an actual `std::vector<double>` is given – otherwise `NULL` will be returned. The given word vector can be determined by one of the following three possible arguments:
* a `std::string`,
* a (struct) `WordVec*`,
* a `std::vector<double>`.

The time complexity is O(*n*).

    VecStore my_vecs("my_word_vecs.txt");
    WordVec* most_distant;
    most_distant = my_vecs.MostDistantWordVec("cat"); // using a std::string as argument
    WordVec* dog_word_vec = new WordVec("dog", {0, 1, 2, 3, 4});
    most_distant = my_vecs.MostDistantWordVec(dog_word_vec); // using a WordVec* as argument
    most_distant = my_vecs.MostDistantWordVec(dog_word_vec->vec); // using a std::vector<double> as argument

#### 2.4.10 `WordVecList VecStore::KMostDistantWordVecs(..., const unsigned k = 3)` (method)
Returns a `WordVecList` containing the *k* most distant word vectors to a given one (with respect to their Euclidean distance), if the given one is stored in the `VecStore` object or an actual `std::vector<double>` is given – otherwise an empty `WordVecList` will be returned. The given word vector can be determined by one of the following three possible arguments:
* a `std::string`,
* a (struct) `WordVec*`,
* a `std::vector<double>`.

Furthermore, *k* can be defined by another argument (of type `unsigned`); by default it is 3.  
The time complexity is O(*n*).

    VecStore my_vecs("my_word_vecs.txt");
    WordVecList most_distant_vecs; // WordVecList == std::list<WordVec*>
    most_distant_vecs = my_vecs.KMostDistantWordVecs("cat"); // using a std::string as argument and k = 3 (default)
    WordVec* dog_word_vec = new WordVec("dog", {0, 1, 2, 3, 4});
    most_distant_vecs = my_vecs.KMostDistantWordVecs(dog_word_vec, 5); // using a WordVec* as argument and k = 5
    most_distant_vecs = my_vecs.KMostDistantWordVecs(dog_word_vec->vec); // using a std::vector<double> as argument and k = 3 (default)

#### 2.4.11 `static std::string VecStore::SetToLowerCase(std::string& string) (static method)
Given a `std::string` all of its letters will be set to lower case letters and the adjusted string will be returned. This method is static, so no `VecStore` object is needed to use it.

    std::string old_string("Peter R."), new_string;
    new_string = VecStore::SetToLowerCase(old_string); // new_string = "peter r."

### 2.5 `VecSimTable` (class)
The `VecSimTable` class allows you to read word vectors from a file into a similarity table on memory, calculating and storing the cosine similarity and the Euclidean distance for every word vector pair. This makes those similarity measures easily accessible. Finding a word vector consumes a time complexity of O(log(*n*)); when both words were found their similarity can be accessed in O(1).

#### 2.5.1 The constructor `VecSimTable::VecSimTable(const std::string& file, ...)`
There are two different constructors for `VecSimTable` objects. Both needs the path of a file containing your word vectors (as a `std::string`) as an argument.
The first constructor also needs a `std::regex` pattern – only those word vectors in your word vector file that match this pattern will be stored in the `VecSimTable` object. This is especially helpful if you are interested in certain derivations like all words with the suffix "-less".
The second constructor allows you to specify whether you want to work case sensitive with the `VecSimTable` object or not by adding an `bool` value; it is `true` by default. If you change it to `false` all word vectors won’t be stored case sensitive; also the words you will enter and search for later won’t be regarded case sensitively. As third argument you can enter a `double` value between 0 and 1 representing the percentage of word vectors from your file you want to store. By default the value is 0.1, i.e. the first ten percent of the word vectors of your file will be stored. This is why it is helpful if the word vectors in your file are saved in some kind of order (e.g. from most frequent words to less frequent (appropriate files can be created with [Standford’s *GloVe* implementation](https://github.com/stanfordnlp/GloVe) for example)). If you have got big word vector files, it is discouraged to store all your word vectors in a single `VecSimTable` because the memory space needed to store them plus the cosine similarity and Euclidean distance of every possible pair is quite big.

    VecSimTable my_vst0("my_word_vecs.txt", "for.+"); // (first) constructor for a "VecSimTable" using a regex pattern (all words starting with the prefix "for-" will be stored) 
    VecSimTable my_vst1("my_word_vecs.txt"); // (second) constructor using the default parameters (i.e. case_sensitive == true and percentage == 0.1)
    VecSimTable my_vst2("my_word_vecs.txt", false, 0.25); // (second) constructor using costumized parameters

#### 2.5.2 `void VecSimTable::PrintInfo()` (method)
Prints the basic information about a `VecSimTable` object, such as the size and number of word vectors stored and whether the `VecSimTable` object works case sensitive or not.

    VecStore my_vst("my_word_vecs.txt");
    my_vst.PrintInfo();

#### 2.5.3 `std::vector<double> VecSimTable::GetVec(std::string word)` (method)
Returns the "vector" of a word vector (`WordVec`), if the "word" is stored in the `VecSimTable` object; otherwise an empty `std::vector<double>` will be returned and an error message will be printed.

    VecSimTable my_vst("my_word_vecs.txt");
    std::vector<double> dog_vec = my_vecs.GetVec("dog");

#### 2.5.4 `double VecSimTable::GetCosSim(...)` (method)
Given a word pair by passing either two `std::string`s or a `std::pair<std::string, std::string>` this method searches for their corresponding vectors and returns their cosine similarity from the `VecSimTable` object, if both words are stored. If at least one of the words couldn’t be found `NaN` will be returned and an error message will be printed.

    VecSimTable my_vst("my_word_vecs.txt");
    double cosine_similarity = my_vst.GetCosSim("dog", "cat");
    std::pair<std::string, std::string> word_pair = std::make_pair("dog", "cat");
    cosine_similarity = my_vst.GetCosSim(word_pair);

#### 2.5.5 `double VecSimTable::GetEuclDist(...)` (method)
Given a word pair by passing either two `std::string`s or a `std::pair<std::string, std::string>` this method searches for their corresponding vectors and returns the Euclidean distance between them, if both words are stored in the `VecSimTable` object. If at least one of the words couldn’t be found `NaN` will be returned and an error message will be printed.

    VecSimTable my_vst("my_word_vecs.txt");
    double euclidean_distance = my_vst.GetEuclDist("dog", "cat");
    std::pair<std::string, std::string> word_pair = std::make_pair("dog", "cat");
    euclidean_distance = my_vst.GetEuclDist(word_pair);

#### 2.5.6 `WordPairList VecSimTable::SimilarPairs(..., std::string comparison_mode, const double range = 0.1)` (method)
Returns a `WordPairList` containing word pairs similar to a given word pair with respect to either the cosine similarity or the Euclidean distance using a range: E.g. if "range == 0.1" and the chosen criteria is the cosine similarity that would be "0.5" for the given word pair, all word pairs with a cosine similarity between 0.4 and 0.6 will be returned. If the words of the given word pair are not stored in the `VecSimTable` object an empty list will be returned and an error message will be printed.  
The given word pair can be determined by one of the following two possible arguments:
* two `std::string`s ("word0" and "word1"),
* a `std::pair<std::string, std::string>`.

Furthermore, it is possible to pass a `double` instead, representing a similarity value of interest.  
If you enter "Euclidean distance", "eucldist", "euclidean_distance" or something similar (the regex pattern "eucl(idean)?([ _-])?dist(ance)?" is used) as the argument `comparison_mode`, the method will look for the Euclidean distances, otherwise for the cosine similarity.  
The default value of `range` is 0.1.  
The returned `WordPairList` consists of the word pairs (as `std::string`s) and a similarity value (a `double` representing either the cosine similarity or the Euclidean distance of the word pair).

    VecSimTable my_vst("my_word_vecs.txt");
    WordPairList similar_words;
    similar_words = my_vst.SimilarPairs("dog", "cat"); // returns all word pairs with a cosine similarity in a range of +/-0.1 to the cosine similarity of the word pair "dog"/"cat" (for 0.1 is the default value of "range" and the cosine similarity the default similarity measure)
    std::pair<std::string, std::string> word_pair = std::make_pair("dog", "cat");
    similar_words = my_vst.SimilarPairs(word_pair, "eucldist", 0.075); // returns all word pairs with a Euclidean distance in a range of +/-0.075 to the Euclidean distance of the word pair "dog"/"cat"
    similar_words = my_vst.SimilarPairs(1.2, "eucldist", 0.05); // returns all word pairs with a Euclidean distance between 1.15 and 1.25

#### 2.5.7 `WordPairList VecSimTable::MostSimilarPairs(..., std::string comparison_mode, const unsigned k =  3)` (method)
Returns a `WordPairList` containing the *k* most similar word pairs to a given word pair with respect to either the cosine similarity or the Euclidean distance. If the words of the given word pair are not stored in the `VecSimTable` object an empty list will be returned and an error message will be printed.  
The given word pair can be determined by one of the following two possible arguments:
* two `std::string`s ("word0" and "word1"),
* a `std::pair<std::string, std::string>`.

Furthermore, it is possible to pass a `double` instead, representing a similarity value of interest.  
If you enter "Euclidean distance", "eucldist", "euclidean_distance" or something similar (the regex pattern "eucl(idean)?([ _-])?dist(ance)?" is used) as the argument `comparison_mode`, the method will look for the Euclidean distances, otherwise for the cosine similarity.  
The default value of *k* is 3.  
The returned `WordPairList` consists of the word pairs (as `std::string`s) and a similarity value (a `double` representing either the cosine similarity or the Euclidean distance of the word pair).

    VecSimTable my_vst("my_word_vecs.txt");
    WordPairList most_similar_words;
    most_similar_words = my_vst.SimilarPairs("dog", "cat"); // returns the 3 most similar word pairs (with respect to the cosine similarity) to the word pair "dog"/"cat" (for 3 is the default value of "k" and the cosine similarity the default similarity measure)
    std::pair<std::string, std::string> word_pair = std::make_pair("dog", "cat");
    most_similar_words = my_vst.SimilarPairs(word_pair, "eucldist", 5); // returns the 5 most similar word pairs (with respect to the Euclidean distance) to the word pair "dog"/"cat"
    most_similar_words = my_vst.SimilarPairs(1.2, "eucldist", 10); // returns the 10 word pairs with a Euclidean distance closest to 1.2

### 2.6 `VecCalc` (namespace)
The namespace `VecCalc` provides several functions to perform mathematical operations on (word) vectors.  
Notice that the header "*word_vec_lib.h*" of the *word_vec_lib* is already `using namespace VecCalc;`, so you usually won’t need to write `VecCalc::` in front of the functions you use.

#### 2.6.1 `double VecCalc::EuclideanNorm(const std::vector<T>& vec)` (function)
A template for `std::vector`s containing elements of numeric data types. Given such a vector the function returns the Euclidean Norm of this vector.

    std::vector<int> vec({-1, 0, -2, 3, 5});
    double euclidean_norm = VecCalc::EuclideanNorm(vec);

#### 2.6.2 `double VecCalc::CosineSimilarity(...)` (function)
A template for `std::vector`s containing elements of numeric data types. Given two vectors the function returns their cosine similarity. You can either pass two `std::vector`s or two `WordVec*`s.

    std::vector<int> vec0({-1, 0, -2, 3, 5});
    std::vector<int> vec1({2, -3, 3, 1, 0});
    double cosine_similarity = VecCalc::CosineSimilarity(vec0, vec1);
    WordVec* word_vec0 = new WordVec("word0", vec0);
    WordVec* word_vec1 = new WordVec("word1", vec1);
    cosine_similarity = VecCalc::CosineSimilarity(word_vec0, word_vec1);

#### 2.6.3 `double VecCalc::EuclideanDistance(...)` (function)
A template for `std::vector`s containing elements of numeric data types. Given two vectors the function returns the Euclidean distance between them. You can either pass two `std::vector`s or two `WordVec*`s.

    std::vector<int> vec0({-1, 0, -2, 3, 5});
    std::vector<int> vec1({2, -3, 3, 1, 0});
    double euclidean_distance = VecCalc::EuclideanDistance(vec0, vec1);
    WordVec* word_vec0 = new WordVec("word0", vec0);
    WordVec* word_vec1 = new WordVec("word1", vec1);
    euclidean_distance = VecCalc::EuclideanDistance(word_vec0, word_vec1);

#### 2.6.4 `std::vector<T> VecCalc::Add(...)` (function)
A template for `std::vector`s containing elements of numeric data types. This function adds all given vectors and returns the resulting one. You can 
1. either pass two `std::vector`s or two `WordVec*`s, **or**
2. either a `std::vector<WordVec*> or s `WordVecList`.

If you choose one of the options of "2.", the `vec`s of all `WordVec*`s in the container will be added.

    std::vector<int> vec0({-1, 0, -2, 3, 5});
    std::vector<int> vec1({2, -3, 3, 1, 0});
    std::vector<int> vec2({-2, 3, -3, -1, 0});
    std::vector<int> vec3({0, 1, 2, 3, 4});
    std::vector<int> result;
    WordVec* word_vec0 = new WordVec("word0", vec0);
    WordVec* word_vec1 = new WordVec("word1", vec1);
    WordVec* word_vec2 = new WordVec("word2", vec2);
    WordVec* word_vec3 = new WordVec("word3", vec3);
    std::vector<WordVec*> vecs = {word_vec0, word_vec1, word_vec2, word_vec3};
    WordVecList word_vec_list;
    word_vec_list.push_back(word_vec0);
    word_vec_list.push_back(word_vec1);
    word_vec_list.push_back(word_vec2);
    word_vec_list.push_back(word_vec3);
    result = VecCalc::Add(vec0, vec1);
    result = VecCalc::Add(word_vec0, word_vec1);
    result = VecCalc::Add(vecs); // adding the "vec"s of all "WordVec*"s in "vecs"
    result = VecCalc::Add(word_vec_list); // adding the "vec"s of all "WordVec*"s in "word_vec_list"

#### 2.6.5 `std::vector<T> VecCalc::Subtract(...)` (function)
A template for `std::vector`s containing elements of numeric data types. This function subtracts the second of two given vectors from the first one and returns the resulting one. You can either pass two `std::vector`s or two `WordVec*`s.

    std::vector<int> vec0({-1, 0, -2, 3, 5});
    std::vector<int> vec1({2, -3, 3, 1, 0});
    std::vector<int> result;
    WordVec* word_vec0 = new WordVec("word0", vec0);
    WordVec* word_vec1 = new WordVec("word1", vec1);
    result = VecCalc::Subtract(vec0, vec1); // result = vec0 - vec1
    result = VecCalc::Subtract(word_vec0, word_vec1); // result = word_vec0->vec - word_vec1->vec

#### 2.6.6 `std::vector<T> VecCalc::GetAverageVec(...)` (function)
A template for `std::vector`s containing elements of numeric data types. This function adds all given vectors and divides the resulting vector by the number of given vectors, than returns the resulting vector. You can 
1. either pass two `std::vector`s or two `WordVec*`s, **or**
2. either a `std::vector<WordVec*> or s `WordVecList`.

If you choose one of the options of "2.", an average vector of the `vec`s of all `WordVec*`s in the container will be returned.

    std::vector<int> vec0({-1, 0, -2, 3, 5});
    std::vector<int> vec1({2, -3, 3, 1, 0});
    std::vector<int> vec2({-2, 3, -3, -1, 0});
    std::vector<int> vec3({0, 1, 2, 3, 4});
    std::vector<int> average_vec;
    WordVec* word_vec0 = new WordVec("word0", vec0);
    WordVec* word_vec1 = new WordVec("word1", vec1);
    WordVec* word_vec2 = new WordVec("word2", vec2);
    WordVec* word_vec3 = new WordVec("word3", vec3);
    std::vector<WordVec*> vecs = {word_vec0, word_vec1, word_vec2, word_vec3};
    WordVecList word_vec_list;
    word_vec_list.push_back(word_vec0);
    word_vec_list.push_back(word_vec1);
    word_vec_list.push_back(word_vec2);
    word_vec_list.push_back(word_vec3);
    average_vec = VecCalc::GetAverageVec(vec0, vec1);
    average_vec = VecCalc::GetAverageVec(word_vec0, word_vec1);
    average_vec = VecCalc::GetAverageVec(vecs);
    average_vec = VecCalc::GetAverageVec(word_vec_list);

### 2.7 `VecPrint` (namespace)
The namespace `VecPrint` provides several functions to print vectors, word vectors (`WordVec`s) and relevant `std::list`s (such as `WordPairList`s).  
Notice that the header "*word_vec_lib.h*" of the *word_vec_lib* is already `using namespace VecPrint;`, so you usually won’t need to write `VecPrint::` in front of the functions you use.

#### 2.7.1 `void VecPrint::PrintVec(const std::vector<T>& vec)` (function)
A template for (one dimensional) `std::vector`s. This function prints the whole given `std::vector`. If the `vec` is empty, "[]" will be printed.

    std::vector<int> vec({-1, 0, -2, 3, 5});
    VecPrint::PrintVec(vec); // prints: [-1 0 -2 3 5]

#### 2.7.2 `void VecPrint::PrintWordVec(const WordVec* wv)` (function)
Prints a `WordVec*`. If `wv == NULL`, "NULL" will be printed.

    WordVec* word_vec = new WordVec("cat", {-1, 0, -2, 3, 5});
    VecPrint::PrintWordVec(word_vec); // prints: cat [-1 0 -2 3 5]

#### 2.7.3 `void VecPrint::PrintWordVecList(WordVecList wv_list)` (function)
Prints a `WordVecList`. If the `wv_list` is empty, "NULL" will be printed.

    WordVec* word_vec0 = new WordVec("cat", {-1, 0, -2, 3, 5});
    WordVec* word_vec1 = new WordVec("wolf", {2, -3, 3, 1, 0});
    WordVec* word_vec2 = new WordVec("dog", {-2, 3, -3, -1, 0});
    WordVec* word_vec3 = new WordVec("bee", {0, 1, 2, 3, 4});
    WordVecList word_vec_list;
    word_vec_list.push_back(word_vec0);
    word_vec_list.push_back(word_vec1);
    word_vec_list.push_back(word_vec2);
    word_vec_list.push_back(word_vec3);
    VecPrint::PrintWordVecList(word_vec_list);
    /* Prints:
    --- BEGIN WordVecList: ---
    cat [-1 0 -2 3 5]
    wolf [2 -3 3 1 0]
    dog [-2 3 -3 -1 0]
    bee [0 1 2 3 4]
    --- END WordVecList.   ---
    */

#### 2.7.4 `void VecPrint::PrintWordPair(std::pair<std::pair<std:string, std::string>, double> & word_pair)` (function)
Prints a word pair (`std::pair<std::pair<std:string, std::string>, double>`, which represents two words and their similarity value (either their cosine similarity or the Euclidean distance between them)).

    std::pair<std::pair<std:string, std::string>, double> word_pair(std::make_pair(std::make_pair("cat", "dog"), 0.785));
    VecPrint::PrintWordPair(word_pair); // prints: "cat" / "dog", 0.785

#### 2.7.5 `void VecPrint::PrintWordPairList(WordPairList& word_pair_list)` (function)
Prints a `WordPairList`. If the `word_pair_list` is empty, "NULL" will be printed.

    std::pair<std::pair<std:string, std::string>, double> pair0(std::make_pair(std::make_pair("cat", "dog"), 0.785));
    std::pair<std::pair<std:string, std::string>, double> pair1(std::make_pair(std::make_pair("man", "woman"), 0.3));
    std::pair<std::pair<std:string, std::string>, double> pair2(std::make_pair(std::make_pair("woman", "women"), 0.01));
    std::pair<std::pair<std:string, std::string>, double> pair3(std::make_pair(std::make_pair("bee", "bird"), 0.5));
    WordPairList wpl;
    wpl.push_back(pair0);
    wpl.push_back(pair1);
    wpl.push_back(pair2);
    wpl.push_back(pair3);
    VecPrint::PrintWordPairList(wpl);
    /* Prints:
    --- BEGIN WordPairList: ---
    "cat" / "dog", 0.785
    "man" / "woman", 0.3
    "woman" / "women", 0.01
    "bee" / "bird", 0.5
    --- END WordPairList.   ---
    */

## 3. License
*word_vec_lib* is licensed under the [Apache License, Version 2.0](LICENSE).
