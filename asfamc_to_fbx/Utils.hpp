//
//  Utils.hpp
//  asfamc_to_fbx
//
//  Created by Tom on 9/13/24.
//

#ifndef Utils_hpp
#define Utils_hpp

#include <stdio.h>
#include <vector>
#include <nlohmann/json.hpp>

// Function to parse a JSON file and return it as a string
std::string parse_json_file_to_string(const std::string& file_path);

// Function to parse a JSON file into a JSON object
nlohmann::json parse_json_file(const std::string& file_path);

// Function to get all files in a directory (non-recursive)
std::vector<std::string> get_files_in_directory(const std::string& directory_path);
// Function to get file name from path (excluding extension)
std::string get_file_name(const std::string& file_path);

// Function to get file name from path (excluding extension)
std::string get_file_name_with_extension(const std::string& file_path);

#endif /* Utils_hpp */
