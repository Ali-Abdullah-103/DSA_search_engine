#include "lemmatizer.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

std::unordered_map<std::string, std::string> lemma_dict;

void load_lemmatizer(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open lemmatizer file: " << filename << "\n";
        return;
    }

    std::string line, word, lemma;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        if (iss >> word >> lemma) {
            lemma_dict[word] = lemma;
        }
    }

    std::cout << "Lemmatizer loaded. Total entries: " << lemma_dict.size() << "\n";
}

std::string lemmatize(const std::string& word) {
    auto it = lemma_dict.find(word);
    if (it != lemma_dict.end()) return it->second;
    return word; // fallback
}
