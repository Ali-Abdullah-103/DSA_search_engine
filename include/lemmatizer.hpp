#pragma once
#include <unordered_map>
#include <string>

extern std::unordered_map<std::string, std::string> lemma_dict;

void load_lemmatizer(const std::string& filename);
std::string lemmatize(const std::string& word);
