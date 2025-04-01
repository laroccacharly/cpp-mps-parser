#include <gtest/gtest.h>
#include "mps_reader.h"

TEST(MPSFileTest, CountLines) {
    const std::string filename = "../../mps_files/50v-10.mps";
    
    // The function will throw if file can't be opened
    int lineCount = mps::count_lines(filename);
    
    ASSERT_GT(lineCount, 0) << "File is empty";
    ASSERT_EQ(lineCount, 6307) << "Unexpected number of lines";
} 