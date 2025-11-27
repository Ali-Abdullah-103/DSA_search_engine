#include "lexicon.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

//comparator for sorting by frequency, used is savefile
bool freq_compare(const std::pair<std::string, WordData>& a, const std::pair<std::string, WordData>& b) {
    return a.second.frequency > b.second.frequency;
}

size_t Lexicon::add(const std::string& word, size_t count) {
    auto target = data.find(word);

    if (target != data.end()) {
        target->second.frequency += count;   // increase only frequency
        return target->second.word_id;       // return existing ID
    }

    // new word: assign word_id and set frequency
    data[word] = { next_id, count };
    return next_id++;
}

//checks if a word is present in the lexicon or not
bool Lexicon::present_in(const std::string& word) const {
    return data.find(word) != data.end();
}


size_t Lexicon::getID(const std::string& word) const {
    auto target = data.find(word);
    if (target != data.end()) {
        return target->second.word_id;
    }
    return static_cast<size_t>(-1);  // word not found
}


size_t Lexicon::getFrequency(const std::string& word) const {
    auto it = data.find(word);
    if (it != data.end()) {
        return it->second.frequency;
    }
    return 0;   // word not found
}


void Lexicon::save(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file " << path << std::endl;
        return;
    }
    //we convert hashmap to vector for sorting by frequency.
    std::vector<std::pair<std::string, WordData>> vec(data.begin(), data.end());
    std::sort(vec.begin(), vec.end(), freq_compare);

    for (const auto& entry : vec) {
        file << entry.first << " "
             << entry.second.word_id << " "
             << entry.second.frequency << "\n";
    }
    file.close();
}


#include <fstream>
#include <sstream>

bool Lexicon::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file " << path << std::endl;
        return false;
    }

    clear_lex();


    std::string line;
    while (std::getline(file, line)) {

        //convert line to stream to extract indivisual attributes
        std::stringstream ss(line);
        std::string word;
        size_t id, freq;

        if (ss >> word >> id >> freq) {
            data[word] = { id, freq };

            //track highest assigned ID to reset next_id safely
            if (id >= next_id) {
                next_id = id + 1;
            }
        }
    }
    file.close();
    return true;
}


void Lexicon::print_top_words(int n) const {
    if (data.empty()) {
        std::cout << "Lexicon is empty.\n";
        return;
    }
    std::vector<std::pair<std::string, WordData>> vec(data.begin(), data.end());
    std::sort(vec.begin(), vec.end(), freq_compare);

    if (static_cast<int>(vec.size()) < n) {
        std::cout << "Only " << vec.size() << " words available (requested " << n << ").\n";
        return;
    }

    int count = 0;
    for (const auto& entry : vec) {
        if (count++ >= n) break;
        std::cout << entry.first << " (ID: " << entry.second.word_id << ", Frequency: " << entry.second.frequency << ")\n";
    }
}

void Lexicon::clear_lex() 
{
    data.clear();
    next_id = 0;
}