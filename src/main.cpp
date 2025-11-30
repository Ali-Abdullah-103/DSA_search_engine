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

    size_t max_docs = 1000; //let's say process first 1000 docs
    int processed = parser.metadata_parse(lex, fwd, inv, max_docs);

    std::cout << "\n----- Indexing Complete -----\n";
    std::cout << "Documents processed: " << processed << "\n\n";
    lex.show_statistics();
    std::cout << std::endl;
    std::cout << "Total documents in Forward Index: " << fwd.total_documents() << "\n";

    //Save to files
    lex.save("D:/searchEngine/indices/lexicon.csv");
    fwd.save_to_file("D:/searchEngine/indices/forward_index.txt");
    inv.save_to_file("D:/searchEngine/indices/inverted_index.csv");

    std::cout << "\nIndexes saved successfully!\n";
    return 0;
}