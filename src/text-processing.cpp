#include "text_processing.hpp"
#include "lemmatizer.hpp"
#include <unordered_set>
#include <sstream>

//Some common words which dont have are not to be stored in lexicon
std::unordered_set<std::string> common_words = {
    "a", "about", "above", "after", "again", "against", "all", "am", "an",
    "and", "any", "are", "aren't", "as", "at", "be", "because", "been",
    "before", "being", "below", "between", "both", "but", "by", "can't",
    "cannot", "could", "couldn't", "did", "didn't", "do", "does", "doesn't",
    "doing", "don't", "down", "during", "each", "few", "for", "from",
    "further", "had", "hadn't", "has", "hasn't", "have", "haven't", "having",
    "he", "he'd", "he'll", "he's", "her", "here", "here's", "hers", "herself",
    "him", "himself", "his", "how", "how's", "i", "i'd", "i'll", "i'm",
    "i've", "if", "in", "into", "is", "isn't", "it", "it's", "its", "itself",
    "let's", "me", "more", "most", "mustn't", "my", "myself", "no", "nor",
    "not", "of", "off", "on", "once", "only", "or", "other", "ought", "our",
    "ours", "ourselves", "out", "over", "own", "same", "shan't", "she",
    "she'd", "she'll", "she's", "should", "shouldn't", "so", "some", "such",
    "than", "that", "that's", "the", "their", "theirs", "them", "themselves",
    "then", "there", "there's", "these", "they", "they'd", "they'll",
    "they're", "they've", "this", "those", "through", "to", "too", "under",
    "until", "up", "very", "was", "wasn't", "we", "we'd", "we'll", "we're",
    "we've", "were", "weren't", "what", "what's", "when", "when's", "where",
    "where's", "which", "while", "who", "who's", "whom", "why", "why's",
    "with", "won't", "would", "wouldn't", "you", "you'd", "you'll", "you're",
    "you've", "your", "yours", "yourself", "yourselves", "one", "two", "using", "also", "can","however", "may"};



// Basic tokenizer: lowercase, remove special chars, split, remove common words
std::vector<std::string> tokenize_text(const std::string& text)
{
    std::string clean;
    clean.reserve(text.size());

    //keep letters and spaces only
    for (char ch : text) {
        if (std::isalpha(static_cast<unsigned char>(ch))) {   // C++ check for alphabet
            clean += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        } else {
            clean += ' ';  // normalize everything else to space
        }
    }
    //splitting using stringstream
    std::stringstream ss(clean);
    std::string word;
    std::vector<std::string> tokens;

    while (ss >> word) {
        if (word.size() < 3) continue;              //filter tiny words
        if (common_words.count(word)) continue;      //filter common words
        tokens.push_back(lemmatize(word));
    }
    return tokens;
}