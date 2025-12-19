#pragma once
#include <iostream>
#include <unordered_map>
#include "forward_index.hpp"
#include <string>
#include<unordered_set>

class InvertedIndex {

private:
//each barrel has 30k words
static const size_t BARREL_SIZE = 30000;

//(barrel_id -> (word_id -> vector of (doc_ids))
std::unordered_map<size_t, std::unordered_map<size_t, std::vector<size_t>>> barrels;


std::unordered_set<size_t> barrel_ids;

public:

size_t get_barrel_id(size_t word_id) const {
        return word_id / BARREL_SIZE;
    }

void add_from_forward(const ForwardIndex&);

bool load_from_file(std::string basePath); 

bool load_barrel(size_t barrel_id, const std::string& basePath);

const std::vector<size_t>* fetch_doc_ids(size_t word_id) const;

void save_to_file(std::string path);

size_t size();

const std::unordered_map<size_t, std::unordered_map<size_t, std::vector<size_t>>> &get_inv_index() const {
    return barrels;
}

};