#ifndef MPS_READER_H
#define MPS_READER_H

#include <string>

namespace mps {

/**
 * Counts the number of lines in an MPS file.
 * @param filename Path to the MPS file
 * @return Number of lines in the file
 * @throws std::runtime_error if the file cannot be opened
 */
int count_lines(const std::string& filename);

} // namespace mps

#endif // MPS_READER_H 