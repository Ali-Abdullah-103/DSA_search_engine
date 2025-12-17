#include "auto_complete.hpp"
#include "text_processing.hpp"
#include <algorithm>
#include <cctype>

std::vector<std::string>
AutoComplete::suggest(const std::string& raw_prefix, const Lexicon& lex, std::size_t top_k) const
{
    std::vector<std::string> results;

    //normalize prefix
    std::string prefix;
    for (char c : raw_prefix) {
        if (std::isalpha(static_cast<unsigned char>(c)))
            prefix += std::tolower(static_cast<unsigned char>(c));
    }

    if (prefix.empty())
        return results;

    //collect matches: (word, frequency)
    std::vector<std::pair<std::string, std::size_t>> matches;

    for (const auto& [word, info] : lex.get_data()) {
        if (word.rfind(prefix, 0) == 0) {  // starts_with
            matches.emplace_back(word, info.second);
        }
    }

    // Sort by global frequency (descending)
    std::sort(matches.begin(), matches.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });

    // Extract top-k
    for (std::size_t i = 0; i < matches.size() && i < top_k; ++i) {
        results.push_back(matches[i].first);
    }
    
    return results;
}