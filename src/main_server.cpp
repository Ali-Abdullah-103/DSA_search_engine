#include <iostream>
#include <string>
#include <sstream>
#include "searching.hpp"
#include "auto_complete.hpp"
#include "lexicon.hpp"
#include "forward_index.hpp"
#include "inverted_index.hpp"
#include "lemmatizer.hpp"
#include "semantic_search.hpp"

void print_json_error(const std::string& message) {
    std::cout << "{\"error\":\"" << message << "\"}" << std::endl;
}

void print_autocomplete_results(const std::vector<std::string>& suggestions) {
    std::cout << "{\"suggestions\":[";
    for (size_t i = 0; i < suggestions.size(); ++i) {
        std::cout << "\"" << suggestions[i] << "\"";
        if (i < suggestions.size() - 1) std::cout << ",";
    }
    std::cout << "]}" << std::endl;
    std::cout.flush();
}

void print_search_results(const std::vector<SemanticResult>& results) {
    std::cout << "{\"results\":[";
    for (size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];
        std::cout << "{";
        std::cout << "\"title\":\"";
        // Escape quotes in title
        for (char c : r.title) {
            if (c == '"') std::cout << "\\\"";
            else if (c == '\\') std::cout << "\\\\";
            else if (c == '\n') std::cout << "\\n";
            else if (c == '\r') std::cout << "\\r";
            else if (c == '\t') std::cout << "\\t";
            else std::cout << c;
        }
        std::cout << "\",";
        std::cout << "\"score\":" << r.score << ",";
        std::cout << "\"url\":\"";
        // Escape quotes in URL
        for (char c : r.url) {
            if (c == '"') std::cout << "\\\"";
            else if (c == '\\') std::cout << "\\\\";
            else std::cout << c;
        }
        std::cout << "\"";
        std::cout << "}";
        if (i < results.size() - 1) std::cout << ",";
    }
    std::cout << "]}" << std::endl;
    std::cout.flush();
}

int main()
{
    // Base path for all data files
    const std::string BASE_PATH = "D:/DSA Project/DSA_search_engine/";
    
    std::cerr << "Starting CORD-19 Search Server..." << std::endl;
    
    // Load everything ONCE at startup
    std::cerr << "Loading lemmatizer..." << std::endl;
    load_lemmatizer(BASE_PATH + "lemmatizer/lemmatization-en.txt");

    Lexicon lex;
    ForwardIndex fwd;
    InvertedIndex inv;
    SearchEngine engine;
    AutoComplete autocomplete;
    SemanticSearch semantic_search;

    std::cerr << "Loading lexicon..." << std::endl;
    if (!lex.load(BASE_PATH + "indices/lexicon.csv")) {
        std::cerr << "Failed to load lexicon" << std::endl;
        return 1;
    }

    std::cerr << "Loading forward index..." << std::endl;
    if (!fwd.load_from_file(BASE_PATH + "indices/forward_index.txt")) {
        std::cerr << "Failed to load forward index" << std::endl;
        return 1;
    }

    std::cerr << "Loading inverted index..." << std::endl;
    if (!inv.load_from_file(BASE_PATH + "indices/inverted_index")) {
        std::cerr << "Failed to load inverted index" << std::endl;
        return 1;
    }

    std::cerr << "Loading metadata..." << std::endl;
    if (!engine.load_metadata_urls(BASE_PATH + "data/data/2020-04-10/metadata.csv")) {
        std::cerr << "Failed to load metadata" << std::endl;
        return 1;
    }
    semantic_search.load_metadata(BASE_PATH + "data/data/2020-04-10/metadata.csv");

    std::cerr << "Loading GloVe embeddings..." << std::endl;
    if (!semantic_search.load_embeddings_binary(BASE_PATH + "embedding/glove_embeddings.bin")) {
        std::cerr << "Cannot load GloVe embeddings" << std::endl;
        return 1;
    }

    std::cerr << "Loading document embeddings..." << std::endl;
    if (!semantic_search.load_document_embeddings(BASE_PATH + "embedding/doc_embeddings.bin")) {
        std::cerr << "Cannot load document embeddings" << std::endl;
        return 1;
    }

    std::cerr << "Server ready! Waiting for queries..." << std::endl;
    std::cout << "{\"status\":\"ready\"}" << std::endl;
    std::cout.flush();

    // Main server loop - read queries from stdin
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        
        // Parse command: "SEARCH query" or "AUTOCOMPLETE query"
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        
        std::string query;
        std::getline(iss, query);
        // Trim leading space
        if (!query.empty() && query[0] == ' ') {
            query = query.substr(1);
        }

        if (command == "SEARCH") {
            auto semantic_results = semantic_search.semantic_search(query, lex, fwd, 10);
            print_search_results(semantic_results);
        } 
        else if (command == "AUTOCOMPLETE") {
            auto suggestions = autocomplete.suggest(query, lex, 10);
            print_autocomplete_results(suggestions);
        }
        else if (command == "EXIT") {
            std::cerr << "Server shutting down..." << std::endl;
            break;
        }
        else {
            print_json_error("Invalid command. Use SEARCH, AUTOCOMPLETE, or EXIT");
        }
    }

    return 0;
}