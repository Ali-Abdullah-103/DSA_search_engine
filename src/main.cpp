#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "MetaDataParser.hpp"   // only for parse_csv_line

int main() 
{

    MetadataParser m("D:/searchEngine/data/2020-04-10/metadata.csv");
    std::ifstream file("D:/searchEngine/data/2020-04-10/metadata.csv");
    if (!file.is_open()) {
        std::cerr << "Error opening metadata.csv\n";
        return 1;
    }

    std::string line;
    int line_count = 0;

    while (line_count < 4 && std::getline(file, line)) {
        std::vector<std::string> fields;
        m.parse_line(line, fields);

        std::cout << "\nLine " << line_count + 1 << ":\n";
        for (size_t i = 0; i < fields.size(); i++) {
            std::cout << "[" << i << "] " << fields[i] << "\n";
        }

        line_count++;
    }

    file.close();
    return 0;
}