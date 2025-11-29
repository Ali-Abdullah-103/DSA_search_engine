#include "forward_index.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

bool compare_by_word_id(const std::pair<size_t,size_t>& a, const std::pair<size_t,size_t>& b) {
    return a.first < b.first;
}

size_t ForwardIndex::register_document(const std::string& cord_uid, const std::unordered_map<std::string, std::pair<size_t,size_t>>& word_map)
{
    size_t current_id = next_id++;  // assign numeric doc id

    std::vector<std::pair<size_t,size_t>> term_list;
    term_list.reserve(word_map.size());

    for (const auto& entry : word_map) {
        term_list.push_back(entry.second);
    }
    std::sort(term_list.begin(), term_list.end(), compare_by_word_id);
    forward_index[current_id] = std::move(term_list);
    docid_to_corduid[current_id] = cord_uid;

    return current_id;
}

const std::vector<std::pair<size_t,size_t>>* ForwardIndex::fetch_terms(size_t doc_id) const
{
    auto target = forward_index.find(doc_id);
    if (target == forward_index.end()) {
        return nullptr;
    }
    return &target->second;
}

const std::string* ForwardIndex::fetch_cord_uid(size_t doc_id) const
{
    auto meta_target = docid_to_corduid.find(doc_id);
    if (meta_target == docid_to_corduid.end()) {
        return nullptr;
    }
    return &meta_target->second;
}

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
    file << forward_index.size() << '\n';
    for (const auto& doc_pair : forward_index)
    {
        size_t doc_id = doc_pair.first;
        const auto& terms = doc_pair.second;

        auto meta_it = docid_to_corduid.find(doc_id);
        if (meta_it != docid_to_corduid.end()) {
            file << doc_id << "|" << meta_it->second << '\n';
        }

        for (const auto& term : terms) {
            file << term.first << "," << term.second << " ";
        }
        file << '\n';
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
    docid_to_corduid.clear();

    size_t stored_doc_count;
    file >> stored_doc_count;
    file.ignore();

    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        auto pos = line.find('|');
        if (pos == std::string::npos) continue;

        size_t doc_id = std::stoull(line.substr(0, pos));
        docid_to_corduid[doc_id] = line.substr(pos + 1);

        if (!std::getline(file, line)) break;

        std::istringstream ss(line);
        std::vector<std::pair<size_t,size_t>> terms;
        size_t word_id, freq;
        char comma;

        while (ss >> word_id >> comma >> freq) {
            terms.emplace_back(word_id, freq);
        }
        std::sort(terms.begin(), terms.end(), compare_by_word_id);
        forward_index[doc_id] = std::move(terms);
    }
    return true;
}