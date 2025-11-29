#include <iostream>
#include "forward_index.hpp"
#include "inverted_index.hpp"
#include <unordered_map>

int main() {
    ForwardIndex fwd;

    // Simulated documents with word_id and frequency pairs
    std::unordered_map<std::string, std::pair<size_t,size_t>> doc1 = {
        {"fever", {0, 2}}, {"cough", {1, 1}}, {"virus", {3, 4}}
    };
    std::unordered_map<std::string, std::pair<size_t,size_t>> doc2 = {
        {"cough", {1, 3}}, {"headache", {2, 2}}, {"virus", {3, 1}}
    };
    std::unordered_map<std::string, std::pair<size_t,size_t>> doc3 = {
        {"fever", {0, 1}}, {"infection", {4, 5}}
    };

    size_t id1 = fwd.register_document("DOC_A", doc1);
    size_t id2 = fwd.register_document("DOC_B", doc2);
    size_t id3 = fwd.register_document("DOC_C", doc3);

    fwd.show_statistics();

    fwd.save_to_file("D:/forward_index.txt");

    ForwardIndex fwd2;
    fwd2.load_from_file("D:/forward_index.txt");

    std::cout << "\nAfter loading from file:\n";
    fwd2.show_statistics();

    InvertedIndex inv;
    inv.add_from_forward(fwd2);

    const std::vector<size_t>* docs_for_word_1 = inv.fetch_doc_ids(1);

    std::cout << "\nDocuments containing word_id 1:\n";
    if (docs_for_word_1) {
        for (size_t doc_id : *docs_for_word_1) {
            std::cout << "Doc ID: " << doc_id << std::endl;
        }
    } else {
        std::cout << "No documents found for word_id 1\n";
    }

    inv.save_to_file("D:/inverted_index.csv");

    InvertedIndex inv2;
    inv2.load_from_file("D:/inverted_index.csv");

    std::cout << "\nAfter reloading inverted index:\n";
    const std::vector<size_t>* docs_for_word_0 = inv2.fetch_doc_ids(0);
    if (docs_for_word_0) {
        std::cout << "Docs with word_id 0: ";
        for (size_t doc_id : *docs_for_word_0) std::cout << doc_id << " ";
        std::cout << "\n";
    }

    return 0;
}