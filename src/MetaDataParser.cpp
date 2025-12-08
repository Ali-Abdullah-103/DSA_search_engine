#include <iostream>
#include "nlohmann_json.hpp"
#include <string>
#include <vector>
#include <unordered_set>
#include "MetaDataParser.hpp"
#include "lemmatizer.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

void MetadataParser::parse_line(const std::string& l, std::vector<std::string>& split_line)
{
    std::string unique_field;
    split_line.clear();
    bool quote = false;

    for(char ch : l) {
        if (ch == '"')
            quote = !quote;
        else if (ch == ',' && !quote) {
            split_line.push_back(unique_field);
            unique_field = "";
        }
        else
            unique_field += ch;
    }
    split_line.push_back(unique_field);
}

//Sha might have multiple values in a column separated by ';'
std::string MetadataParser::extract_first_sha(const std::string& sha) const {
    if (sha.empty()) return "";
    size_t pos = sha.find(';');  // Just find the first semicolon

    if (pos == std::string::npos)
        return sha;  
    return sha.substr(0, pos);
}


std::string MetadataParser::get_file_path(const std::string& pmcid, const std::string& sha) const
{
    const std::string data_path = "D:/DSA Project/DSA_search_engine/data/data/2020-04-10";
        // Try SHA-based PDF JSON
    std::string first_sha = extract_first_sha(sha);
    if (!first_sha.empty()) {
        const std::vector<std::string> pdf_folders = {
            "comm_use_subset",
            "noncomm_use_subset",
            "custom_license",
            "biorxiv_medrxiv"
        };
        for (const auto& folder : pdf_folders) {
            std::string path = data_path + "/" + folder + "/pdf_json/" + first_sha + ".json";
            if (std::filesystem::exists(path)) {
                return path;
            }
        }
    }
    
    // Try PMC-based JSON 
    if (!pmcid.empty()) {
        const std::vector<std::string> pmc_folders = {
            "comm_use_subset",
            "noncomm_use_subset",
            "custom_license"
        };
        for (const auto& folder : pmc_folders) {
            std::string path = data_path + "/" + folder + "/pmc_json/" + pmcid + ".xml.json";
            if (std::filesystem::exists(path)) {
                return path;
            }
        }
    }
    return "";  // No fulltext found
}


//for parsing with help of nlohmann/json library
std::string MetadataParser::extract_text_from_json(const std::string& file_path) const
{
    if (file_path.empty() || !std::filesystem::exists(file_path))
        return "";

    std::ifstream file(file_path);
    if (!file.is_open())
        return "";

    nlohmann::json j;     //using nlohmann_json for parsing
    file >> j;           

    std::string text;

    // pdf_json format
    if (j.contains("body_text")) {
        for (const auto& block : j["body_text"]) {
            text += block.value("text", "") + " ";
        }
    }
    return text; 
}


int MetadataParser::metadata_parse(Lexicon& lex,
                                   ForwardIndex& fwd,
                                   InvertedIndex& inv,
                                   size_t max_docs) 
{
    std::ifstream csv("D:/DSA Project/DSA_search_engine/data/data/2020-04-10/metadata.csv");
    if (!csv.is_open()) {
        std::cerr << "Error: Cannot open metadata.csv\n";
        return 0;
    }

    std::string line;
    std::getline(csv, line);  // skip header

    int processed_count = 0;

    while (std::getline(csv, line)) 
    {
        if (processed_count >= max_docs) break;

        std::vector<std::string> cols;
        parse_line(line, cols);

        if (cols.size() < 18) continue;

        const std::string& cord_uid = cols[0];
        const std::string& sha_raw  = cols[1];
        const std::string& title    = cols[3];
        const std::string& pmcid    = cols[5];
        const std::string& abstract = cols[8];

        // Build complete text
        std::string full_text = title + "\n" + abstract + "\n";

        std::string json_path = get_file_path(pmcid, sha_raw);
        if (!json_path.empty()) {
            full_text += extract_text_from_json(json_path);
        }

        //Size too small meaning we couldnt get the doc
        if (full_text.size() < 50) continue;


        std::vector<std::string> tokens = tokenize(full_text);

        //for storing freq of words for a specific doc, changes every iteration
        std::unordered_map<std::string, size_t> local_freq;
        for (const auto& token : tokens) {
            local_freq[token]++;
        }

       
        //Pushing words in word_map (changes every iteration) for forward_index
        //At the same time, pushing in lexicon
        std::unordered_map<std::string, std::pair<size_t,size_t>> word_map;

        for (const auto& entry : local_freq) {
            size_t word_id = lex.add(entry.first, entry.second);
            word_map[entry.first] = {word_id, entry.second};
        }

        // Register document in ForwardIndex
        fwd.register_document(cord_uid, word_map);
        processed_count++;
    }
    inv.add_from_forward(fwd);
    return processed_count;
}


//Some common words which dont have are not to be stored in lexicon
std::unordered_set<std::string> common_words = {
    "a", "about", "above", "after", "again", "against", "all", "am", "an",
    "and", "any", "are", "aren't", "as", "at", "be", "because", "been",
    "before", "being", "below", "between", "both", "but", "by", "can't",
    "cannot", "could", "couldn't", "did", "didn't", "do", "does", "doesn't",
    "doing", "don't", "down", "during", "each", "few", "for", "from",
    "further", "had", "hadn't", "has", "hasn't", "have", "haven't", "having",
    "he", "he'd", "he'll", "he's", "her", "here", "here's", "hers", "herself",
    "him", "himself", "his", "how", "how's", "i", "i'd", "i'll", "i'm",
    "i've", "if", "in", "into", "is", "isn't", "it", "it's", "its", "itself",
    "let's", "me", "more", "most", "mustn't", "my", "myself", "no", "nor",
    "not", "of", "off", "on", "once", "only", "or", "other", "ought", "our",
    "ours", "ourselves", "out", "over", "own", "same", "shan't", "she",
    "she'd", "she'll", "she's", "should", "shouldn't", "so", "some", "such",
    "than", "that", "that's", "the", "their", "theirs", "them", "themselves",
    "then", "there", "there's", "these", "they", "they'd", "they'll",
    "they're", "they've", "this", "those", "through", "to", "too", "under",
    "until", "up", "very", "was", "wasn't", "we", "we'd", "we'll", "we're",
    "we've", "were", "weren't", "what", "what's", "when", "when's", "where",
    "where's", "which", "while", "who", "who's", "whom", "why", "why's",
    "with", "won't", "would", "wouldn't", "you", "you'd", "you'll", "you're",
    "you've", "your", "yours", "yourself", "yourselves", "one", "two", "using", "also", "can","however", "may"};



// Basic tokenizer: lowercase, remove special chars, split, remove common words
std::vector<std::string> MetadataParser::tokenize(const std::string& text) const
{
    std::string clean;
    clean.reserve(text.size());

    //keep letters and spaces only
    for (char ch : text) {
        if (std::isalpha(static_cast<unsigned char>(ch))) {   // C++ check for alphabet
            clean += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        } else {
            clean += ' ';  // normalize everything else to space
        }
    }
    //splitting using stringstream
    std::stringstream ss(clean);
    std::string word;
    std::vector<std::string> tokens;

    while (ss >> word) {
        if (word.size() < 3) continue;              //filter tiny words
        if (common_words.count(word)) continue;      //filter common words
        tokens.push_back(lemmatize(word));
    }
    return tokens;
}