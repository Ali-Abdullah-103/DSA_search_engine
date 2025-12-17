#include "searching.hpp"         
#include "MetadataParser.hpp"  
#include "lemmatizer.hpp"      
#include "text_processing.hpp"
#include <fstream>          
#include <sstream>             
#include <algorithm>           
#include <cctype>     


std::vector<SearchResult> SearchEngine::search(const std::string& raw_query,
                     const Lexicon& lex,
                     const ForwardIndex& fwd,
                     const InvertedIndex& inv,
                     std::size_t top_k) const
{
    std::vector<SearchResult> results;

    //tokenize query
    std::vector<std::string> query_tokens = tokenize_text(raw_query);
    if (query_tokens.empty())
        return results;

    //convert query tokens → word_ids
    std::vector<std::size_t> query_word_ids;
    for (const auto& token : query_tokens) {
        if (lex.present_in(token)) {
            query_word_ids.push_back(lex.getID(token));
        }
    }

    if (query_word_ids.empty())
        return results;

    //fetch posting lists for each query word
    std::vector<std::vector<std::size_t>> postings;
    for (std::size_t word_id : query_word_ids) {
        const auto* docs = inv.fetch_doc_ids(word_id);
        if (docs && !docs->empty()) {
            postings.push_back(*docs);
        }
    }

    if (postings.empty())
        return results;

    //AND logic: intersect all posting lists
    std::vector<std::size_t> candidate_docs = postings[0];
    for (std::size_t i = 1; i < postings.size(); ++i) {
        candidate_docs = intersect_sorted(candidate_docs, postings[i]);
        if (candidate_docs.empty())
            break;
    }

    //OR fallback if AND result is empty
    if (candidate_docs.empty()) {
        candidate_docs = postings[0];
        for (std::size_t i = 1; i < postings.size(); ++i) {
            candidate_docs = union_sorted(candidate_docs, postings[i]);
        }
    }

    //Score each candidate document
    for (std::size_t doc_id : candidate_docs) {
        double score = score_by_tf_sum(fwd, doc_id, query_word_ids);
        if (score == 0.0)
            continue;

        const std::string* cord_uid = fwd.fetch_cord_uid(doc_id);
        if (!cord_uid)
            continue;

        SearchResult r;
        r.doc_id = doc_id;
        r.cord_uid = *cord_uid;
        r.score = score;

        //attach metadata (title + url)
        auto it = corduid_to_meta.find(*cord_uid);
        if (it != corduid_to_meta.end()) {
            r.title = it->second.title;
            r.url   = it->second.url;
        }

        results.push_back(std::move(r));
    }

    //sort by score (descending)
    std::sort(results.begin(), results.end(),
              [](const SearchResult& a, const SearchResult& b) {
                  return a.score > b.score;
              });

    //Keep only top-k
    if (results.size() > top_k)
        results.resize(top_k);
    return results;
}


bool SearchEngine::load_metadata_urls(const std::string& metadata_csv_path)
{
    std::ifstream file(metadata_csv_path);
    if (!file.is_open())
        return false;

    // Clear any old data
    corduid_to_meta.clear();

    MetadataParser parser;          // reuse existing CSV parsing logic
    std::string line;
    std::vector<std::string> cols;

    // Skip CSV header
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // Parse CSV row safely (handles quoted commas)
        parser.parse_line(line, cols);

        // We need at least cord_uid. title and url
        if (cols.size() < 4) continue;

        const std::string& cord_uid = cols[0];
        const std::string& title    = cols[3];
        const std::string& url      = cols.back();

        if (!cord_uid.empty())
            corduid_to_meta[cord_uid] = { title, url };
    }
    return true;
}


//for query word present in both docs (AND operation)
std::vector<std::size_t> SearchEngine::intersect_sorted(const std::vector<std::size_t>& a,
        const std::vector<std::size_t>& b)
{
    std::vector<std::size_t> result;
    result.reserve(std::min(a.size(), b.size()));

    std::size_t i = 0;
    std::size_t j = 0;

    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            result.push_back(a[i]);
            ++i;
            ++j;
        }
        else if (a[i] < b[j]) {
            ++i;
        }
        else {
            ++j;
        }
    }
    std::sort(result.begin(), result.end());

    return result;
}


//for query word present in either docs (OR operation)
std::vector<std::size_t> SearchEngine::union_sorted(const std::vector<std::size_t>& a,
                           const std::vector<std::size_t>& b)
{
    std::vector<std::size_t> result;
    result.reserve(a.size() + b.size());

    std::size_t i = 0;
    std::size_t j = 0;

    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            result.push_back(a[i]);
            ++i;
            ++j;
        }
        else if (a[i] < b[j]) {
            result.push_back(a[i]);
            ++i;
        }
        else {
            result.push_back(b[j]);
            ++j;
        }
    }

    // Append remaining elements
    while (i < a.size()) {
        result.push_back(a[i++]);
    }

    while (j < b.size()) {
        result.push_back(b[j++]);
    }

    //remove duplicates (just in case)
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    return result;
}


//finding term frequency using binary search
std::size_t SearchEngine::term_freq_doc(const ForwardIndex& fwd, std::size_t doc_id, std::size_t word_id)
{
    const auto* terms = fwd.fetch_terms(doc_id);
    if (!terms) return 0;

    std::size_t left = 0;
    std::size_t right = terms->size();

    while (left < right) {
        std::size_t mid = (left + right) / 2;

        if ((*terms)[mid].first == word_id)
            return (*terms)[mid].second;
        else if ((*terms)[mid].first < word_id)
            left = mid + 1;
        else
            right = mid;
    }
    return 0;
}

//score of each document (one call scores one document only)
double SearchEngine::score_by_tf_sum(const ForwardIndex& fwd, std::size_t doc_id, const std::vector<std::size_t>& query_word_ids)
{
    double score = 0.0;
    for (std::size_t word_id : query_word_ids) {
        score += term_freq_doc(fwd, doc_id, word_id);
    }
    return score;
}