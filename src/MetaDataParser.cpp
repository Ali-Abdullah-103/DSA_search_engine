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


std::string MetadataParser::get_file_path(const std::string& pmcid, const std::string& sha, const std::string& license) const
{
    // Try PMC-based JSON first
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
    return "";  // No fulltext found
}
