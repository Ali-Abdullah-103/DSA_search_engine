#include <iostream>
#include <string>
#include "searching.hpp"
#include "auto_complete.hpp"
#include "lexicon.hpp"
#include "forward_index.hpp"
#include "inverted_index.hpp"
#include "lemmatizer.hpp"
#include "semantic_search.hpp"

int main()
{
    // Load lemmatizer
    load_lemmatizer("D:/DSA Project/DSA_search_engine/lemmatizer/lemmatization-en.txt");

    Lexicon lex;
    ForwardIndex fwd;
    InvertedIndex inv;
    SearchEngine engine;
    AutoComplete autocomplete;
    SemanticSearch semantic_search;

    // Load indexes
    if (!lex.load("D:/DSA Project/DSA_search_engine/indices/lexicon.csv")) {
        std::cerr << "Failed to load lexicon\n";
        return 1;
    }

    if (!fwd.load_from_file("D:/DSA Project/DSA_search_engine/indices/forward_index.txt")) {
        std::cerr << "Failed to load forward index\n";
        return 1;
    }

    if (!inv.load_from_file("D:/DSA Project/DSA_search_engine/indices/inverted_index")) {
        std::cerr << "Failed to load inverted index\n";
        return 1;
    }

    // Load metadata (title + URL)
    if (!engine.load_metadata_urls("D:/DSA Project/DSA_search_engine/data/data/2020-04-10/metadata.csv")) {
        std::cerr << "Failed to load metadata\n";
        return 1;
    }
    semantic_search.load_metadata("D:/DSA Project/DSA_search_engine/data/data/2020-04-10/metadata.csv");

    std::cout << "\nLoading GloVe word embeddings...\n";
    if (!semantic_search.load_embeddings_binary("D:/DSA Project/DSA_search_engine/embedding/glove_embeddings.bin")) {
        std::cerr << "Error: Cannot load GloVe embeddings from bin/glove_embeddings.bin\n";
        std::cerr << "Make sure you have created this file first!\n";
        return 1;
    }
    std::cout << "Loading document embeddings...\n";
    
    if (!semantic_search.load_document_embeddings("D:/DSA Project/DSA_search_engine/embedding/doc_embeddings.bin")) {
        std::cerr << "Error: Cannot load document embeddings from bin/doc_embeddings.bin\n";
        std::cerr << "Make sure you have created this file first!\n";
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
        
        // ---- SEMANTIC SEARCH MODE ----
        auto semantic_results = semantic_search.semantic_search(input, lex, fwd, 10);
        std::cout << "\n=== SEMANTIC SEARCH RESULTS ===\n";
        for (size_t i = 0; i < semantic_results.size(); ++i) {
            std::cout << (i + 1) << ". " << semantic_results[i].title <<"\n";
            std::cout << "   Score: " << semantic_results[i].score << "\n";
            std::cout << "   URL: " << semantic_results[i].url << "\n";
    }
    return 0;
    } 
    
}