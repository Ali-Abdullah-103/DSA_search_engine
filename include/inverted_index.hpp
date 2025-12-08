#pragma once
#include <iostream>
#include <unordered_map>
#include "forward_index.hpp"
#include <string>

class InvertedIndex {

private:
//Mapping word_id -> doc_id (from forward_index)
static const size_t BARREL_SIZE = 3000;

std::unordered_map<size_t, std::unordered_map<size_t, std::vector<size_t>>> barrels;

public:

size_t get_barrel_id(size_t word_id) const {
        return word_id / BARREL_SIZE;
    }

void add_from_forward(const ForwardIndex&);

const std::vector<size_t>* fetch_doc_ids(size_t word_id) const;

void save_to_file(std::string path);

bool load_from_file(std::string path);

size_t size();

const std::unordered_map<size_t, std::unordered_map<size_t, std::vector<size_t>>> &get_inv_index() const {
    return barrels;
}

};