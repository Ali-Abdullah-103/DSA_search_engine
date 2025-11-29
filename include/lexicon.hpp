#pragma once

#include <string>
#include <unordered_map>

class Lexicon {
private:
    //Mapping word -> wordID + frequency(in all docs)
    std::unordered_map<std::string, std::pair<size_t,size_t>> data;
    size_t next_id = 0;

public:
    size_t add(const std::string& word, size_t count = 1);

    bool present_in(const std::string& word) const;

    size_t getID(const std::string& word) const;

    size_t getFrequency(const std::string& word) const;

    size_t size() const { return data.size(); }

    void save(const std::string& path) const;

    bool load(const std::string& path);

    void show_statistics() const;

    void clear_lex();
};