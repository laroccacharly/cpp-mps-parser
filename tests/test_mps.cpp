#include <gtest/gtest.h>
#include "mps_reader.h"

class MPSFileTest : public ::testing::Test {
protected:
    const std::string valid_filename = "../../mps_files/50v-10.mps";
    const std::string invalid_filename = "../../mps_files/nonexistent.mps";
};

TEST_F(MPSFileTest, CountLines) {
    int lineCount = mps::count_lines(valid_filename);
    
    ASSERT_GT(lineCount, 0) << "File is empty";
    ASSERT_EQ(lineCount, 6307) << "Unexpected number of lines";
}

TEST_F(MPSFileTest, ReadProblemName) {
    std::string problemName = mps::read_problem_name(valid_filename);
    
    ASSERT_EQ(problemName, "50v-10") << "Unexpected problem name";
}

TEST_F(MPSFileTest, ReadProblemNameInvalidFile) {
    ASSERT_THROW(mps::read_problem_name(invalid_filename), std::runtime_error) 
        << "Should throw when file doesn't exist";
} 