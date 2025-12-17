#include <iostream>
#include "nlohmann_json.hpp"
#include <string>
#include <vector>
#include <unordered_set>
#include "text_processing.hpp"
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
    const std::string data_path = "D:/searchEngine/data/2020-04-10";
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
    std::ifstream csv("D:/searchEngine/data/2020-04-10/metadata.csv");
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


        std::vector<std::string> tokens = tokenize_text(full_text);

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