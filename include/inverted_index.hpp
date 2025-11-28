#pragma once

#include <iostream>
#include <unordered_map>
#include "word_attributes.hpp"
#include "forward_index.hpp"
#include <string>

class InvertedIndex {

private:
//Mapping word_id -> doc_id (from forward_index)
std::unordered_map<size_t, std::vector<size_t>> inverted_index;

public:

void add_from_forward(const ForwardIndex&);

const std::vector<size_t>* fetch_doc_ids(size_t word_id) const;

void save_to_file(std::string path);

bool load_from_file(std::string path);

size_t size() { return inverted_index.size(); }

const std::unordered_map<size_t, std::vector<size_t>>& get_inv_index() const {
    return inverted_index;
}

};