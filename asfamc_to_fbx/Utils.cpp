//
//  Utils.cpp
//  asfamc_to_fbx
//
//  Created by Tom on 9/13/24.
//

#include "Utils.hpp"

#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>
#include <filesystem>

// Namespace for filesystem (requires C++17 or later)
namespace fs = std::filesystem;

// Function to parse a JSON file and return it as a string
std::string parse_json_file_to_string(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + file_path);
    }

    nlohmann::json json_obj;
    file >> json_obj;

    // Convert the JSON object to a formatted string (pretty print)
    return json_obj.dump(4);  // `4` specifies the indentation level for pretty printing
}

// Function to parse a JSON file into a JSON object
nlohmann::json parse_json_file(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + file_path);
    }

    nlohmann::json json_obj;
    file >> json_obj;

    return json_obj;
}

// Function to get all files in a directory (non-recursive)
std::vector<std::string> get_files_in_directory(const std::string& directory_path) {
    std::vector<std::string> file_paths;

    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (entry.is_regular_file()) {
            file_paths.push_back(entry.path().string());
        }
    }

    return file_paths;
}

// Function to get file name from path (excluding extension)
std::string get_file_name(const std::string& file_path) {
    fs::path path(file_path);
    return path.stem().string();  // stem() returns the filename without extension
}

std::string get_file_name_with_extension(const std::string& file_path) {
    fs::path path(file_path);
    return path.filename();  // stem() returns the filename without extension
}
