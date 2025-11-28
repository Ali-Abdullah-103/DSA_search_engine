#include <iostream>
#include <vector>
#include <unordered_map>
#include "lexicon.hpp"
#include "forward_index.hpp"
#include "inverted_index.hpp"

// Utility: build word-map from tokens for ForwardIndex
std::unordered_map<std::string, WordData> build_word_map(
    Lexicon& lexicon, const std::vector<std::string>& tokens)
{
    std::unordered_map<std::string, WordData> word_map;
    
    for (const auto& token : tokens) {
        size_t id = lexicon.add(token); // Assign global ID

        auto& wd = word_map[token];
        wd.word_id = id;
        wd.frequency++;  // Count frequency in this document
    }
    return word_map;
}

int main() {
    Lexicon lex;
    ForwardIndex fwd;
    InvertedIndex inv;

    // === Register Documents ===
    std::vector<std::string> doc0 = {"machine", "learning", "is", "machine", "powerful"};
    auto map0 = build_word_map(lex, doc0);
    fwd.register_document("UID-A1", map0);

    std::vector<std::string> doc1 = {"deep", "learning", "drives", "AI"};
    auto map1 = build_word_map(lex, doc1);
    fwd.register_document("UID-A2", map1);

    std::vector<std::string> doc2 = {"machine", "intelligence", "is", "growing"};
    auto map2 = build_word_map(lex, doc2);
    fwd.register_document("UID-A3", map2);

    std::cout << "\n=== ForwardIndex Statistics ===\n";
    fwd.show_statistics();

    std::cout << "\n=== Terms in Document 0 ===\n";
    if (auto terms = fwd.fetch_terms(0)) {
        for (const auto& t : *terms)
            std::cout << "WordID: " << t.word_id << " | Freq: " << t.frequency << "\n";
    }

    // === Build Inverted Index ===
    inv.add_from_forward(fwd);

    std::cout << "\n=== InvertedIndex Contents ===\n";
    for (const auto& pair : inv.get_inv_index()) {
        std::cout << "WordID " << pair.first << " -> Docs: ";
        for (auto d : pair.second) std::cout << d << " ";
        std::cout << "\n";
    }

    // === Testing Search-like Behavior ===
    std::string query = "machine";
    size_t qid = lex.getID(query);
    auto results = inv.fetch_doc_ids(qid);

    std::cout << "\nSearch Results for word '" << query 
              << "' (ID=" << qid << "): ";
    if (results) {
        for (auto d : *results) std::cout << d << " ";
    }
    std::cout << "\n";

    // === Save to File ===
    inv.save_to_file("D:/inverted.csv");
    fwd.save_to_file("D:/forward.txt");

    std::cout << "\nIndexes saved successfully!\n";

    // === Reload and Test Again ===
    InvertedIndex inv2;
    ForwardIndex fwd2;

    inv2.load_from_file("D:/inverted.csv");
    fwd2.load_from_file("D:/forward.txt");

    std::cout << "\n=== Reloaded InvertedIndex ===\n";
    for (const auto& pair : inv2.get_inv_index()) {
        std::cout << "WordID " << pair.first << " -> ";
        for (auto doc : pair.second) std::cout << doc << " ";
        std::cout << "\n";
    }

    std::cout << "\n=== Reloaded ForwardIndex Stats ===\n";
    fwd2.show_statistics();

    return 0;
}