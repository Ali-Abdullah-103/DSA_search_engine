#include "semantic_search.hpp"
#include "text_processing.hpp"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <iostream>

SemanticSearch::SemanticSearch() 
    : embeddings_loaded(false), embedding_dim(300) 
{
}

bool SemanticSearch::load_glove_embeddings(const std::string& glove_file_path) {
    std::ifstream file(glove_file_path);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open GloVe file: " << glove_file_path << std::endl;
        return false;
    }

    word_embeddings.clear();
    std::string line;
    std::size_t count = 0;

    std::cout << "Loading GloVe embeddings..." << std::flush;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string word;
        iss >> word;

        std::vector<float> embedding;
        embedding.reserve(embedding_dim);
        
        float value;
        while (iss >> value) {
            embedding.push_back(value);
        }

        if (embedding.size() != embedding_dim) {
            std::cerr << "\nWarning: Word '" << word << "' has " 
                      << embedding.size() << " dimensions, expected " 
                      << embedding_dim << std::endl;
            continue;
        }

        // Normalize the embedding vector for faster cosine similarity
        normalize_vector(embedding);
        word_embeddings[word] = std::move(embedding);
        
        count++;
        if (count % 10000 == 0) {
            std::cout << "." << std::flush;
        }
    }

    file.close();
    embeddings_loaded = true;
    
    std::cout << "\nLoaded " << word_embeddings.size() 
              << " word embeddings successfully!" << std::endl;
    
    return true;
}

bool SemanticSearch::save_embeddings_binary(const std::string& binary_file_path) const {
    if (!embeddings_loaded) {
        std::cerr << "Error: No embeddings to save!" << std::endl;
        return false;
    }

    std::ofstream file(binary_file_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create binary file: " << binary_file_path << std::endl;
        return false;
    }

    std::cout << "Saving embeddings to binary file..." << std::flush;

    // Write header: number of words and embedding dimension
    std::size_t num_words = word_embeddings.size();
    file.write(reinterpret_cast<const char*>(&num_words), sizeof(num_words));
    file.write(reinterpret_cast<const char*>(&embedding_dim), sizeof(embedding_dim));

    // Write each word and its embedding
    for (const auto& entry : word_embeddings) {
        const std::string& word = entry.first;
        const std::vector<float>& embedding = entry.second;

        // Write word length and word
        std::size_t word_len = word.size();
        file.write(reinterpret_cast<const char*>(&word_len), sizeof(word_len));
        file.write(word.c_str(), word_len);

        // Write embedding vector
        file.write(reinterpret_cast<const char*>(embedding.data()), 
                   embedding_dim * sizeof(float));
    }

    file.close();
    std::cout << " Done! Saved " << num_words << " word embeddings.\n";
    return true;
}

bool SemanticSearch::load_embeddings_binary(const std::string& binary_file_path) {
    std::ifstream file(binary_file_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open binary file: " << binary_file_path << std::endl;
        return false;
    }

    std::cout << "Loading embeddings from binary file..." << std::flush;

    word_embeddings.clear();

    // Read header
    std::size_t num_words;
    std::size_t saved_dim;
    file.read(reinterpret_cast<char*>(&num_words), sizeof(num_words));
    file.read(reinterpret_cast<char*>(&saved_dim), sizeof(saved_dim));

    if (saved_dim != embedding_dim) {
        std::cerr << "\nError: Dimension mismatch! Expected " << embedding_dim 
                  << " but file has " << saved_dim << std::endl;
        return false;
    }

    // Read each word and embedding
    for (std::size_t i = 0; i < num_words; ++i) {
        // Read word
        std::size_t word_len;
        file.read(reinterpret_cast<char*>(&word_len), sizeof(word_len));
        
        std::string word(word_len, '\0');
        file.read(&word[0], word_len);

        // Read embedding
        std::vector<float> embedding(embedding_dim);
        file.read(reinterpret_cast<char*>(embedding.data()), 
                  embedding_dim * sizeof(float));

        word_embeddings[word] = std::move(embedding);

        if ((i + 1) % 50000 == 0) {
            std::cout << "." << std::flush;
        }
    }

    file.close();
    embeddings_loaded = true;

    std::cout << "\nLoaded " << word_embeddings.size() 
              << " word embeddings from binary file! (Fast!)\n";
    return true;
}

bool SemanticSearch::save_document_embeddings(const std::string& binary_file_path) const {
    if (doc_embeddings.empty()) {
        std::cerr << "Error: No document embeddings to save!" << std::endl;
        return false;
    }

    std::ofstream file(binary_file_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create binary file: " << binary_file_path << std::endl;
        return false;
    }

    std::cout << "Saving document embeddings to binary file..." << std::flush;

    // Write header: number of documents and embedding dimension
    std::size_t num_docs = doc_embeddings.size();
    file.write(reinterpret_cast<const char*>(&num_docs), sizeof(num_docs));
    file.write(reinterpret_cast<const char*>(&embedding_dim), sizeof(embedding_dim));

    // Write each document ID and its embedding
    for (const auto& entry : doc_embeddings) {
        std::size_t doc_id = entry.first;
        const std::vector<float>& embedding = entry.second;

        // Write doc_id
        file.write(reinterpret_cast<const char*>(&doc_id), sizeof(doc_id));

        // Write embedding vector
        file.write(reinterpret_cast<const char*>(embedding.data()), 
                   embedding_dim * sizeof(float));
    }

    file.close();
    std::cout << " Done! Saved " << num_docs << " document embeddings.\n";
    return true;
}

bool SemanticSearch::load_document_embeddings(const std::string& binary_file_path) {
    std::ifstream file(binary_file_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open binary file: " << binary_file_path << std::endl;
        return false;
    }

    std::cout << "Loading document embeddings from binary file..." << std::flush;

    doc_embeddings.clear();

    // Read header
    std::size_t num_docs;
    std::size_t saved_dim;
    file.read(reinterpret_cast<char*>(&num_docs), sizeof(num_docs));
    file.read(reinterpret_cast<char*>(&saved_dim), sizeof(saved_dim));

    if (saved_dim != embedding_dim) {
        std::cerr << "\nError: Dimension mismatch! Expected " << embedding_dim 
                  << " but file has " << saved_dim << std::endl;
        return false;
    }

    // Read each document ID and embedding
    for (std::size_t i = 0; i < num_docs; ++i) {
        // Read doc_id
        std::size_t doc_id;
        file.read(reinterpret_cast<char*>(&doc_id), sizeof(doc_id));

        // Read embedding
        std::vector<float> embedding(embedding_dim);
        file.read(reinterpret_cast<char*>(embedding.data()), 
                  embedding_dim * sizeof(float));

        doc_embeddings[doc_id] = std::move(embedding);

        if ((i + 1) % 1000 == 0) {
            std::cout << "." << std::flush;
        }
    }

    file.close();

    std::cout << "\nLoaded " << doc_embeddings.size() 
              << " document embeddings from binary file! (Fast!)\n";
    return true;
}

void SemanticSearch::build_document_embeddings(const ForwardIndex& fwd, 
                                                const Lexicon& lex) {
    if (!embeddings_loaded) {
        std::cerr << "Error: GloVe embeddings not loaded yet!" << std::endl;
        return;
    }

    doc_embeddings.clear();
    std::cout << "Building document embeddings..." << std::flush;

    std::size_t total_docs = fwd.total_documents();
    const auto& lexicon_data = lex.get_data();
    
    // Build reverse lookup: word_id -> word
    std::unordered_map<std::size_t, std::string> id_to_word;
    for (const auto& entry : lexicon_data) {
        id_to_word[entry.second.first] = entry.first;
    }

    for (std::size_t doc_id = 0; doc_id < total_docs; ++doc_id) {
        const auto* terms = fwd.fetch_terms(doc_id);
        if (!terms || terms->empty()) continue;

        // Collect words with their frequencies
        std::vector<std::string> doc_words;
        std::vector<float> doc_weights;

        for (const auto& term : *terms) {
            std::size_t word_id = term.first;
            std::size_t freq = term.second;
            
            auto it = id_to_word.find(word_id);
            if (it == id_to_word.end()) continue;
            
            const std::string& word = it->second;
            
            // Check if word has embedding
            if (word_embeddings.find(word) != word_embeddings.end()) {
                doc_words.push_back(word);
                doc_weights.push_back(static_cast<float>(freq));
            }
        }

        if (doc_words.empty()) continue;

        // Compute weighted average embedding
        std::vector<float> doc_embedding(embedding_dim, 0.0f);
        float total_weight = 0.0f;

        for (std::size_t i = 0; i < doc_words.size(); ++i) {
            const auto& word_emb = word_embeddings[doc_words[i]];
            float weight = doc_weights[i];
            
            for (std::size_t j = 0; j < embedding_dim; ++j) {
                doc_embedding[j] += word_emb[j] * weight;
            }
            total_weight += weight;
        }

        // Average by total weight
        if (total_weight > 0.0f) {
            for (std::size_t j = 0; j < embedding_dim; ++j) {
                doc_embedding[j] /= total_weight;
            }
        }

        // Normalize for cosine similarity
        normalize_vector(doc_embedding);
        doc_embeddings[doc_id] = std::move(doc_embedding);

        if ((doc_id + 1) % 100 == 0) {
            std::cout << "." << std::flush;
        }
    }

    std::cout << "\nBuilt embeddings for " << doc_embeddings.size() 
              << " documents!" << std::endl;
}

std::vector<SemanticResult> SemanticSearch::semantic_search(
    const std::string& raw_query,
    const Lexicon& lex,
    const ForwardIndex& fwd,
    std::size_t top_k)
{
    std::vector<SemanticResult> results;

    if (!embeddings_loaded) {
        std::cerr << "Error: Embeddings not loaded!" << std::endl;
        return results;
    }

    if (doc_embeddings.empty()) {
        std::cerr << "Error: Document embeddings not built!" << std::endl;
        return results;
    }

    // Tokenize query
    std::vector<std::string> query_tokens = tokenize_text(raw_query);
    if (query_tokens.empty()) return results;

    // Compute query embedding (average of word embeddings)
    std::vector<float> query_embedding = compute_average_embedding(query_tokens);
    
    // Check if query has valid embedding
    bool has_valid_embedding = false;
    for (float val : query_embedding) {
        if (val != 0.0f) {
            has_valid_embedding = true;
            break;
        }
    }

    if (!has_valid_embedding) {
        std::cerr << "Warning: Query has no valid embeddings!" << std::endl;
        return results;
    }

    // Compute similarity with all documents
    for (const auto& doc_pair : doc_embeddings) {
        std::size_t doc_id = doc_pair.first;
        const auto& doc_emb = doc_pair.second;

        double similarity = cosine_similarity(query_embedding, doc_emb);
        
        if (similarity <= 0.0) continue;  // Skip irrelevant documents

        const std::string* cord_uid = fwd.fetch_cord_uid(doc_id);
        if (!cord_uid) continue;

        SemanticResult result;
        result.doc_id = doc_id;
        result.cord_uid = *cord_uid;
        result.score = similarity;

        // Attach metadata if available
        auto meta_it = corduid_to_meta.find(*cord_uid);
        if (meta_it != corduid_to_meta.end()) {
            result.title = meta_it->second.title;
            result.url = meta_it->second.url;
        }

        results.push_back(std::move(result));
    }

    // Sort by similarity score (descending)
    std::sort(results.begin(), results.end(),
              [](const SemanticResult& a, const SemanticResult& b) {
                  return a.score > b.score;
              });

    // Keep only top-k results
    if (results.size() > top_k) {
        results.resize(top_k);
    }

    return results;
}

bool SemanticSearch::load_metadata(const std::string& metadata_csv_path) {
    std::ifstream file(metadata_csv_path);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open metadata file: " << metadata_csv_path << std::endl;
        return false;
    }

    corduid_to_meta.clear();
    std::string line;
    std::vector<std::string> cols;

    // Skip header
    std::getline(file, line);

    int loaded_count = 0;
    int line_num = 1;

    while (std::getline(file, line)) {
        line_num++;
        if (line.empty()) continue;

        parse_csv_line(line, cols);

        // Check minimum columns needed
        if (cols.size() < 18) {
            continue;
        }

        const std::string& cord_uid = cols[0];
        const std::string& title = cols[3];
        const std::string& url = cols[17];  // URL is typically at index 16

        if (!cord_uid.empty()) {
            // Trim whitespace from cord_uid
            std::string trimmed_uid = cord_uid;
            trimmed_uid.erase(0, trimmed_uid.find_first_not_of(" \t\r\n"));
            trimmed_uid.erase(trimmed_uid.find_last_not_of(" \t\r\n") + 1);
            
            corduid_to_meta[trimmed_uid] = {title, url};
            loaded_count++;
            
            /* Debug first few entries
            if (loaded_count <= 3) {
                std::cout << "  Sample " << loaded_count << ": cord_uid='" 
                          << trimmed_uid << "', title='" << title << "'\n";
            }
            */
        }
    }

    std::cout << "Loaded metadata for " << corduid_to_meta.size() 
              << " documents (from " << line_num << " lines)!" << std::endl;
    
    return corduid_to_meta.size() > 0;
}

const std::vector<float>* SemanticSearch::get_word_embedding(
    const std::string& word) const 
{
    auto it = word_embeddings.find(word);
    if (it != word_embeddings.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<float> SemanticSearch::compute_average_embedding(
    const std::vector<std::string>& words) const 
{
    std::vector<float> avg_embedding(embedding_dim, 0.0f);
    std::size_t count = 0;

    for (const auto& word : words) {
        auto it = word_embeddings.find(word);
        if (it != word_embeddings.end()) {
            const auto& emb = it->second;
            for (std::size_t i = 0; i < embedding_dim; ++i) {
                avg_embedding[i] += emb[i];
            }
            count++;
        }
    }

    if (count > 0) {
        for (std::size_t i = 0; i < embedding_dim; ++i) {
            avg_embedding[i] /= static_cast<float>(count);
        }
        normalize_vector(avg_embedding);
    }

    return avg_embedding;
}

double SemanticSearch::cosine_similarity(const std::vector<float>& a,
                                         const std::vector<float>& b) 
{
    if (a.size() != b.size()) return 0.0;

    double dot_product = 0.0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        dot_product += static_cast<double>(a[i]) * static_cast<double>(b[i]);
    }

    // Vectors are already normalized, so dot product = cosine similarity
    return dot_product;
}

void SemanticSearch::normalize_vector(std::vector<float>& vec) {
    double magnitude = 0.0;
    for (float val : vec) {
        magnitude += static_cast<double>(val) * static_cast<double>(val);
    }
    magnitude = std::sqrt(magnitude);

    if (magnitude > 1e-10) {
        for (float& val : vec) {
            val /= static_cast<float>(magnitude);
        }
    }
}

void SemanticSearch::parse_csv_line(const std::string& line,
                                     std::vector<std::string>& fields) 
{
    fields.clear();
    std::string field;
    bool in_quotes = false;

    for (char ch : line) {
        if (ch == '"') {
            in_quotes = !in_quotes;
        } else if (ch == ',' && !in_quotes) {
            fields.push_back(field);
            field.clear();
        } else {
            field += ch;
        }
    }
    fields.push_back(field);
}