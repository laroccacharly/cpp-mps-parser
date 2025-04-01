#include "mps_reader.h"
#include <fstream>
#include <stdexcept>
#include <string>
#include <algorithm>

namespace mps {

int count_lines(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    int lineCount = 0;
    std::string line;
    while (std::getline(file, line)) {
        lineCount++;
    }

    file.close();
    return lineCount;
}

std::string read_problem_name(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::string line;
    if (!std::getline(file, line)) {
        throw std::runtime_error("Failed to read NAME section: empty file");
    }

    // Remove carriage return if present (for Windows files)
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }

    // Check if line starts with "NAME"
    if (line.substr(0, 4) != "NAME") {
        throw std::runtime_error("Invalid MPS file: First line must start with 'NAME'");
    }

    // Extract the problem name (skip "NAME" and any whitespace)
    size_t nameStart = line.find_first_not_of(" \t", 4);
    if (nameStart == std::string::npos) {
        throw std::runtime_error("Invalid MPS file: NAME section contains no problem name");
    }

    // Trim trailing whitespace
    size_t nameEnd = line.find_last_not_of(" \t");
    return line.substr(nameStart, nameEnd - nameStart + 1);
}

} // namespace mps 