#include <iostream>
#include <string>
#include "searching.hpp"
#include "auto_complete.hpp"
#include "lexicon.hpp"
#include "forward_index.hpp"
#include "inverted_index.hpp"
#include "lemmatizer.hpp"

int main()
{
    // Load lemmatizer
    load_lemmatizer("D:/searchEngine/lemmatizer/lemmatization-en.txt");

    Lexicon lex;
    ForwardIndex fwd;
    InvertedIndex inv;
    SearchEngine engine;
    AutoComplete autocomplete;

    // Load indexes
    if (!lex.load("D:/searchEngine/indices/lexicon.csv")) {
        std::cerr << "Failed to load lexicon\n";
        return 1;
    }

    if (!fwd.load_from_file("D:/searchEngine/indices/forward_index.txt")) {
        std::cerr << "Failed to load forward index\n";
        return 1;
    }

    if (!inv.load_from_file("D:/searchEngine/indices/inverted_index")) {
        std::cerr << "Failed to load inverted index\n";
        return 1;
    }

    // Load metadata (title + URL)
    if (!engine.load_metadata_urls("D:/searchEngine/data/2020-04-10/metadata.csv")) {
        std::cerr << "Failed to load metadata\n";
        return 1;
    }

    std::cout << "Search engine ready.\n";
    std::cout << "Type a query.\n";
    std::cout << "Use '?prefix' for auto-complete.\n";
    std::cout << "Type 'exit' to quit.\n\n";

    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "exit" || input.empty())
            break;

        // ---- AUTO-COMPLETE MODE ----
        // User types: ?pol
        if (input[0] == '?') {
            std::string prefix = input.substr(1);

            auto suggestions = autocomplete.suggest(prefix, lex, 10);

            if (suggestions.empty()) {
                std::cout << "No suggestions.\n\n";
                continue;
            }

            std::cout << "Suggestions:\n";
            for (const auto& s : suggestions) {
                std::cout << "  " << s << "\n";
            }
            std::cout << "\n";
            continue;
        }

        // ---- SEARCH MODE ----
        auto results = engine.search(input, lex, fwd, inv, 10);

        if (results.empty()) {
            std::cout << "No results found.\n\n";
            continue;
        }
        int rank = 1;
        for (const auto& r : results) {
            std::cout << rank++ << ". " << r.title << "\n";
            std::cout << "   URL: " << r.url << "\n";
            std::cout << "   Score: " << r.score << "\n\n";
        }
    return 0;
}