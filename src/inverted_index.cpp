#include <iostream>
#include "inverted_index.hpp"
#include "forward_index.hpp"
#include "word_attributes.hpp"
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>


//Making Inverted Index using Forward index
void InvertedIndex::add_from_forward(const ForwardIndex& forward_index)
{
    size_t wordID;
    for(size_t i = 0; i < forward_index.total_documents(); i++) {
        const auto* terms = forward_index.fetch_terms(i);
        if (terms) {
            for (size_t j = 0; j < terms->size(); j++) {
                size_t wordID = (*terms)[j].word_id;
                if (inverted_index[wordID].empty() || inverted_index[wordID].back() != i) {
                    inverted_index[wordID].push_back(i);
                }
            }
        }
    }
}


const std::vector<size_t>* InvertedIndex::fetch_doc_ids(size_t word_id) const 
{
    auto target = inverted_index.find(word_id);
    if (target == inverted_index.end()) {
        return nullptr;
    }
    return &target->second;
}


void InvertedIndex::save_to_file(std::string path) 
{
    std::ofstream file(path);
    if (!file.is_open()) {
        return;
    }
    //Write total size on top of file
    file << inverted_index.size() << "\n";

    for(const auto& values : inverted_index) {
        const size_t word_id = values.first;
        //for sorting the doc_ids
        std::vector<size_t> sorted_ids = values.second;
        std::sort(sorted_ids.begin(), sorted_ids.end());

        file << word_id;
        for(const auto& curr_id : sorted_ids) {
            file << "," << curr_id;
        }
        file << "\n";
    }
    file.close();
}



bool InvertedIndex::load_from_file(std::string path) 
{
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    inverted_index.clear();
    std::string line;    //skip first line (has size of inv_index)
    std::getline(file, line);

    while(getline(file, line)) {
        std::istringstream ss(line);
        std::string token;

        std::getline(ss, token, ','); // get word_id first
        size_t word_id = std::stoull(token);

        std::vector<size_t> doc_ids;
        while (std::getline(ss, token, ',')) { // remaining tokens are doc_ids
            doc_ids.push_back(std::stoull(token));
        }

        inverted_index[word_id] = std::move(doc_ids);
    }
    return true;
}
