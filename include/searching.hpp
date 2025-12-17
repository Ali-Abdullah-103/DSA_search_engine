#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "lexicon.hpp"
#include "forward_index.hpp"
#include "inverted_index.hpp"

//result of a query
struct SearchResult {
    std::string cord_uid;
    std::string title;
    std::string url;
    std::size_t doc_id;
    double score;
};


//we have to display title with url to the document
struct DocMeta {
    std::string title;
    std::string url;
};




class SearchEngine {
public:
    //load cord_uid -> url mapping from metadata.csv
    bool load_metadata_urls(const std::string& metadata_csv_path);


    // AND logic with OR fallback, ranked by term frequency
    std::vector<SearchResult> search(
        const std::string& raw_query,
        const Lexicon& lex,
        const ForwardIndex& fwd,
        const InvertedIndex& inv,
        std::size_t top_k = 20
    ) const;

private:
    // cord_uid -> (title, url) (resolved at query time)
    std::unordered_map<std::string, DocMeta> corduid_to_meta;

    // Posting list set operations (inputs are sorted)
    static std::vector<std::size_t> intersect_sorted(
        const std::vector<std::size_t>& a,
        const std::vector<std::size_t>& b
    );

    static std::vector<std::size_t> union_sorted(
        const std::vector<std::size_t>& a,
        const std::vector<std::size_t>& b
    );

    // Term frequency lookup inside a document
    static std::size_t term_freq_doc(
        const ForwardIndex& fwd,
        std::size_t doc_id,
        std::size_t word_id
    );

    // Simple ranking: sum of TFs of query terms
    static double score_by_tf_sum(
        const ForwardIndex& fwd,
        std::size_t doc_id,
        const std::vector<std::size_t>& query_word_ids
    );
};