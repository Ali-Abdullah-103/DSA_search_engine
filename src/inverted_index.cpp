#include <iostream>
#include "inverted_index.hpp"
#include "forward_index.hpp"
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

//comparator for sorting by word_id
bool sort_by_word_id(const std::pair<size_t, std::vector<size_t>>& a, const std::pair<size_t, std::vector<size_t>>& b)
{
    return a.first < b.first;
}

//Making Inverted Index using Forward index
void InvertedIndex::add_from_forward(const ForwardIndex& forward_index)
{
    for(size_t i = 0; i < forward_index.total_documents(); i++) {
        const auto* terms = forward_index.fetch_terms(i);
        if (terms) {
            for (size_t j = 0; j < terms->size(); j++) {
                size_t wordID = (*terms)[j].first;
                size_t barrelID = get_barrel_id(wordID);
                auto &barrel = barrels[barrelID];
                if (barrel[wordID].empty() || barrel[wordID].back() != i) {
                    barrel[wordID].push_back(i);
                }
            }
        }
    }
    //After making inv_index, sort its documents list and remove suplicates
    for (auto& barrelPair : barrels){
        for (auto& [word_id, doc_list] : barrelPair.second) {
            std::sort(doc_list.begin(), doc_list.end());
            doc_list.erase(std::unique(doc_list.begin(), doc_list.end()), doc_list.end());
    }
    }
    
}


const std::vector<size_t>* InvertedIndex::fetch_doc_ids(size_t word_id) const 
{
    size_t barrelID = get_barrel_id(word_id);            

    auto barrelTarget = barrels.find(barrelID);                 
    if (barrelTarget == barrels.end()) return nullptr;           

    auto wordTarget = barrelTarget->second.find(word_id);             
    if (wordTarget == barrelTarget->second.end()) return nullptr;      

    return &wordTarget->second;
}


void InvertedIndex::save_to_file(std::string basePath) 
{
    for (const auto &barrel_pair : barrels) {
        size_t barrel_id = barrel_pair.first;
        std::string file_name = basePath + "_barrel" + std::to_string(barrel_id) + ".csv";
        std::ofstream file(file_name);
        if (!file.is_open()) continue;

        file << barrel_pair.second.size() << "\n";

        std::vector<std::pair<size_t, std::vector<size_t>>> vec(
            barrel_pair.second.begin(), barrel_pair.second.end());
             std::sort(vec.begin(), vec.end(), sort_by_word_id);

        for (const auto &p : vec) {
            file << p.first; 
            for (const auto &doc_id : p.second) {
                file << "," << doc_id;
            }
            file << "\n";
        }
    }
}



bool InvertedIndex::load_from_file(std::string basePath) 
{
    barrels.clear();  // Start fresh

    // Try loading barrel_0, barrel_1, barrel_2 ... until a file does NOT exist.
    for (size_t barrel_id = 0;; ++barrel_id) {

        std::string file_name = basePath + "_barrel" + std::to_string(barrel_id) + ".csv";
        std::ifstream file(file_name);

        // Stop if this barrel file does NOT exist
        if (!file.is_open())
            break;

        std::unordered_map<size_t, std::vector<size_t>> barrel;

        std::string line;

        // First line contains the number of word entries — we ignore it
        std::getline(file, line);

        // Each following line contains:
        // word_id,doc1,doc2,doc3,...
        while (std::getline(file, line)) {

            std::istringstream ss(line);
            std::string token;

            // First token = word_id
            std::getline(ss, token, ',');
            size_t word_id = std::stoull(token);

            // Rest are doc IDs
            std::vector<size_t> docs;
            while (std::getline(ss, token, ',')) {
                docs.push_back(std::stoull(token));
            }

            // Insert posting list into this barrel
            barrel[word_id] = std::move(docs);
        }

        // Store barrel in main structure
        barrels[barrel_id] = std::move(barrel);
    }

    // Return false if no files were loaded
    return !barrels.empty();
}