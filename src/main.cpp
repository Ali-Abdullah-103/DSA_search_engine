#include <iostream>
#include <unordered_map>
#include "forward_index.hpp"
#include "lexicon.hpp"

// Utility: build per-document WordData using Lexicon
std::unordered_map<std::string, WordData> build_word_map(
    Lexicon& lexicon, const std::vector<std::string>& tokens)
{
    std::unordered_map<std::string, WordData> word_map;

    for (const auto& token : tokens) {
        size_t id = lexicon.add(token); // Assign word_id globally

        auto& wd = word_map[token];
        wd.word_id = id;
        wd.frequency++;   // Count frequency in this document
    }
    return word_map;
}

int main() {
    Lexicon lex;
    ForwardIndex fwd;

    std::cout << "\n=== Registering Documents ===\n";

    // Document 0
    std::vector<std::string> doc0 = {"deep", "learning", "is", "deep", "powerful"};
    auto map0 = build_word_map(lex, doc0);
    fwd.register_document("UID-A1", map0);

    // Document 1
    std::vector<std::string> doc1 = {"machine", "learning", "is", "popular"};
    auto map1 = build_word_map(lex, doc1);
    fwd.register_document("UID-A2", map1);

    // Document 2
    std::vector<std::string> doc2 = {"deep", "neural", "networks", "are", "powerful"};
    auto map2 = build_word_map(lex, doc2);
    fwd.register_document("UID-A3", map2);

    std::cout << "\n=== ForwardIndex Stats ===\n";
    fwd.show_statistics();  // Total docs and term counts

    std::cout << "\n=== Checking terms in Document 0 ===\n";
    if (auto terms = fwd.fetch_terms(0)) {
        for (const auto& t : *terms)
            std::cout << "WordID: " << t.word_id << ", Freq: " << t.frequency << "\n";
    }

    std::cout << "\n=== Checking Document Metadata ===\n";
    for (size_t i = 0; i < 3; i++) {
        if (auto meta = fwd.fetch_cord_uid(i)) {
            std::cout << "Doc " << i << " -> CordUID = " << *meta << "\n";
        }
    }

    // Save to file
    std::cout << "\nSaving ForwardIndex to D:/forward.txt...\n";
    fwd.save_to_file("D:/forward.txt");

    // Now load into NEW instance
    ForwardIndex fwd2;
    std::cout << "\nLoading ForwardIndex into new object...\n";
    fwd2.load_from_file("D:/forward.txt");

    std::cout << "\n=== Reloaded ForwardIndex Stats ===\n";
    fwd2.show_statistics();

    std::cout << "\n=== Checking Reloaded Terms for Document 0 ===\n";
    if (auto terms2 = fwd2.fetch_terms(0)) {
        for (const auto& t : *terms2)
            std::cout << "WordID: " << t.word_id << ", Freq: " << t.frequency << "\n";
    }

    return 0;
}