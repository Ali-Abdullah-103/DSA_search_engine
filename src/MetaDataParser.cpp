#include <iostream>
#include "nlohmann_json.hpp"
#include <string>
#include <vector>
#include "MetaDataParser.hpp"
#include <filesystem>

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


std::string MetadataParser::find_fulltext_file(const std::string& pmcid,
                                               const std::string& sha,
                                               const std::string& license) const
{
    // Try PMC-based JSON first
    if (!pmcid.empty()) {
        std::vector<std::string> xml_search_paths = {
            data_path + "/comm_use_subset/pmc_json/" + pmcid + ".xml.json",
            data_path + "/noncomm_use_subset/pmc_json/" + pmcid + ".xml.json",
            data_path + "/custom_license/pmc_json/" + pmcid + ".xml.json"
        };

        for (const auto& path : xml_search_paths) {
            if (std::filesystem::exists(path)) {
                return path;
            }
        }
    }

    // Try SHA-based PDF JSON
    std::string first_sha = extract_first_sha(sha);
    if (!first_sha.empty()) {
        std::vector<std::string> pdf_search_paths = {
            data_path + "/comm_use_subset/pdf_json/" + first_sha + ".json",
            data_path + "/noncomm_use_subset/pdf_json/" + first_sha + ".json",
            data_path + "/custom_license/pdf_json/" + first_sha + ".json",
            data_path + "/biorxiv_medrxiv/pdf_json/" + first_sha + ".json"
        };

        for (const auto& path : pdf_search_paths) {
            if (std::filesystem::exists(path)) {
                return path;
            }
        }
    }
    return "";  // No fulltext found
}
