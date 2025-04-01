#include "mps_reader.h"
#include <fstream>
#include <stdexcept>

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

} // namespace mps 