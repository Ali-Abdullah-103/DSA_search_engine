#pragma once

#include <string>
#include "lexicon.hpp"
#include "forward_index.hpp"
#include "inverted_index.hpp"

class MetadataParser 
{
private:
    //There can be multiple sha for one document, we use only 1 for identification
    std::string extract_first_sha(const std::string& sha) const;

        //parse a CSV line into fields (handles quoted commas)
    void parse_line(const std::string&, std::vector<std::string>&);

    //finds fulltext JSON file path (checks pmcid first, then sha)
    std::string get_file_path(const std::string& pmcid,
                                   const std::string& sha) const;

    // Extract raw body text from JSON file
    std::string extract_text_from_json(const std::string& file_path) const;

    //For tokenizing the text of the document to put in lexicon
    std::vector<std::string> tokenize(const std::string& text) const;
public:
    // Main parsing function: populates Lexicon, ForwardIndex, and InvertedIndex
    int metadata_parse(Lexicon& lex, ForwardIndex& fwd, InvertedIndex& inv, size_t max_docs);
};