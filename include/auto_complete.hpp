#pragma once
#include <string>
#include <vector>
#include "lexicon.hpp"

class AutoComplete {
public:
    //suggest top-k words starting with prefix
    std::vector<std::string> suggest( const std::string& raw_prefix,
        const Lexicon& lex,
        std::size_t top_k = 10
    ) const;
};