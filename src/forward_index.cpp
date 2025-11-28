#include "forward_index.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>


bool compare_by_word_id(const WordData& a, const WordData& b) {
    return a.word_id < b.word_id;
}


size_t ForwardIndex::register_document(const std::string& cord_uid, const std::unordered_map<std::string, WordData>& word_map)
{
    size_t current_id = next_id++;  // assign numeric doc id

    std::vector<WordData> term_list;
    term_list.reserve(word_map.size());

    // We ignore the string-key (actual word text) here…
    // We only store WordData (which already contains word_id + freq)
    for (const auto& entry : word_map) {
        term_list.push_back(entry.second);
    }
    std::sort(term_list.begin(), term_list.end(), compare_by_word_id);
    forward_index[current_id] = std::move(term_list);  // move, don’t copy
    doc_metadata[current_id] = cord_uid;              // store metadata

    return current_id;
}

//get the wordIDs related to frequencies of a certain document
const std::vector<WordData>* ForwardIndex::fetch_terms(size_t doc_id) const
{
    auto target = forward_index.find(doc_id);
    if (target == forward_index.end()) {
        return nullptr;
    }
    return &target->second;
}

//get the cord_uid of a document 
const std::string* ForwardIndex::fetch_cord_uid(size_t doc_id) const
{
    auto meta_target = doc_metadata.find(doc_id);
    if (meta_target == doc_metadata.end()) {
        return nullptr;
    }
    return &meta_target->second;
}

//total terms in the forward_index as a whole
size_t ForwardIndex::total_terms() const
{
    size_t count = 0;
    for (const auto& entry : forward_index) {
        count += entry.second.size();
    }
    return count;
}

void ForwardIndex::show_statistics() const
{
    std::cout << "Total documents: " << total_documents() << '\n';
    std::cout << "Total term entries: " << total_terms() << '\n';
}

void ForwardIndex::save_to_file(const std::string& output_path) const
{
    std::ofstream file(output_path);
    if (!file.is_open()) {
        return;
    }
    // Write total number of documents
    file << forward_index.size() << '\n';
    for (const auto& doc_pair : forward_index)
    {
        size_t doc_id = doc_pair.first;
        const auto& terms = doc_pair.second;

        // Write doc_id and cord_uid (metadata)
        auto meta_it = doc_metadata.find(doc_id);
        if (meta_it != doc_metadata.end()) {
            file << doc_id << "|" << meta_it->second << '\n';
        }

        // Write all terms: "word_id,frequency" white-space separated
        for (const auto& term : terms) {
            file << term.word_id << "," << term.frequency << " ";
        }
        file << '\n';  // end of doc block
    }
    file.close();
}



bool ForwardIndex::load_from_file(const std::string& input_path)
{
    std::ifstream file(input_path);
    if (!file.is_open()) {
        return false;
    }

    forward_index.clear();
    doc_metadata.clear();

    //skip the first line: doc count
    size_t stored_doc_count;
    file >> stored_doc_count;
    file.ignore();

    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        auto pos = line.find('|'); //format is doc_id|cord_uid
        if (pos == std::string::npos) continue;

        size_t doc_id = std::stoull(line.substr(0, pos));
        doc_metadata[doc_id] = line.substr(pos + 1);

        if (!std::getline(file, line)) break;

        //for converting string to stream..easy comparisons
        std::istringstream ss(line);
        std::vector<WordData> terms;
        size_t word_id, freq;
        char comma;

        while (ss >> word_id >> comma >> freq) {
            WordData data;
            data.word_id = word_id;
            data.frequency = freq;
            terms.push_back(data);
        }
        std::sort(terms.begin(), terms.end(), compare_by_word_id);
        forward_index[doc_id] = std::move(terms);
    }

    return true;
}