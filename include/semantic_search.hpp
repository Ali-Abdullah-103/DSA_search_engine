#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "lexicon.hpp"
#include "forward_index.hpp"
#include "inverted_index.hpp"

// Result of a semantic search query
struct SemanticResult {
    std::string cord_uid;
    std::string title;
    std::string url;
    std::size_t doc_id;
    double score;           // cosine similarity score
};

class SemanticSearch {
public:
    // Constructor
    SemanticSearch();

    // Load GloVe embeddings from file (glove.6B.300d.txt)
    bool load_glove_embeddings(const std::string& glove_file_path);

    // Save word embeddings to binary file (fast loading later)
    bool save_embeddings_binary(const std::string& binary_file_path) const;

    // Load word embeddings from binary file (much faster!)
    bool load_embeddings_binary(const std::string& binary_file_path);

    // Build document embeddings from ForwardIndex and Lexicon
    void build_document_embeddings(const ForwardIndex& fwd, const Lexicon& lex);

    // Save document embeddings to binary file
    bool save_document_embeddings(const std::string& binary_file_path) const;

    // Load document embeddings from binary file
    bool load_document_embeddings(const std::string& binary_file_path);

    // Perform semantic search using cosine similarity
    std::vector<SemanticResult> semantic_search(
        const std::string& raw_query,
        const Lexicon& lex,
        const ForwardIndex& fwd,
        std::size_t top_k = 20
    );

    // Load metadata (title, url) for display
    bool load_metadata(const std::string& metadata_csv_path);

    // Get embedding vector for a word (if it exists)
    const std::vector<float>* get_word_embedding(const std::string& word) const;

    // Check if embeddings are loaded
    bool is_loaded() const { return embeddings_loaded; }

    // Get embedding dimension
    std::size_t get_dimension() const { return embedding_dim; }

private:
    // GloVe word embeddings: word -> 300D vector
    std::unordered_map<std::string, std::vector<float>> word_embeddings;
    
    // Document embeddings: doc_id -> averaged embedding vector
    std::unordered_map<std::size_t, std::vector<float>> doc_embeddings;
    
    // Metadata: cord_uid -> (title, url)
    struct DocMeta {
        std::string title;
        std::string url;
    };
    std::unordered_map<std::string, DocMeta> corduid_to_meta;
    
    // Configuration
    std::size_t embedding_dim = 300;  // GloVe dimension
    bool embeddings_loaded = false;

    // Helper functions
    
    // Compute average embedding for a list of words
    std::vector<float> compute_average_embedding(
        const std::vector<std::string>& words
    ) const;
    
    // Compute cosine similarity between two vectors
    static double cosine_similarity(
        const std::vector<float>& a,
        const std::vector<float>& b
    );
    
    // Normalize a vector (make it unit length)
    static void normalize_vector(std::vector<float>& vec);
    
    // Parse CSV line (handles quoted commas)
    static void parse_csv_line(
        const std::string& line,
        std::vector<std::string>& fields
    );
};