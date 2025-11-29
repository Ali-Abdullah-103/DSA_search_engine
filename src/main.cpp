#include "MetadataParser.hpp"
#include "lexicon.hpp"
#include "forward_index.hpp"
#include "inverted_index.hpp"
#include <iostream>

int main() {
    Lexicon lex;
    ForwardIndex fwd;
    InvertedIndex inv;

    MetadataParser parser;

    size_t max_docs = 100; // Try with smaller batch first
    int processed = parser.metadata_parse(lex, fwd, inv, max_docs);

    std::cout << "\n=== Indexing Complete ===\n";
    std::cout << "Documents processed: " << processed << "\n";
    lex.show_statistics();
    fwd.show_statistics();

    // ----- Save to files -----
    lex.save("D:/searchEngine/indices/lexicon.csv");
    fwd.save_to_file("D:/searchEngine/indices/forward_index.txt");
    inv.save_to_file("D:/searchEngine/indices/inverted_index.csv");

    std::cout << "\nIndexes saved successfully!\n";
    return 0;
}