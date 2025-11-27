#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "word_attributes.hpp"

class ForwardIndex {
private:
    // Mapping: doc numeric ID → list of (word_id, frequency)
    std::unordered_map<size_t, std::vector<WordData>> forward_index;

    // Mapping: doc numeric ID → cord_uid (or other metadata)
    //cord_uid is a unique identifier of doc, specified in dataset
    std::unordered_map<size_t, std::string> doc_metadata;

    size_t next_id = 0;

public:
    // Insert a new document with its word-frequency data
    size_t register_document(const std::string& cord_uid, const std::unordered_map<std::string, WordData>& word_map);

    // Access terms (words) of a document
    const std::vector<WordData>* fetch_terms(size_t doc_id) const;

    // Get original cord_uid
    const std::string* fetch_cord_uid(size_t doc_id) const;

    // Save index to disk
    void save_to_file(const std::string& file_path) const;

    // Load index from disk
    bool load_from_file(const std::string& file_path);

    // Stats
    size_t total_documents() const { return forward_index.size(); }
    size_t total_terms() const;

    void show_statistics() const;
};