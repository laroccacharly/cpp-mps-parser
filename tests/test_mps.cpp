#include <gtest/gtest.h>
#include "mps_parser.h"
#include "mps_reader.h"
#include <memory>
#include <stdexcept>
#include <set>
#include <cstdlib>

class MPSParserTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        const char* mps_dir = std::getenv("MPS_FILES_DIR");
        if (!mps_dir) {
            throw std::runtime_error("Environment variable MPS_FILES_DIR not set");
        }
        std::string dir_path(mps_dir);
        if (!dir_path.empty() && dir_path.back() != '/') {
            dir_path += '/';
        }
        valid_filename = dir_path + "50v-10.mps";
        invalid_filename = dir_path + "nonexistent.mps";

        // Parse the file once for all tests
        try {
            lp_data = mps::parse_mps(valid_filename);
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to parse MPS file in test setup: " + std::string(e.what()));
        }
    }

    static std::string valid_filename;
    static std::string invalid_filename;
    static std::unique_ptr<mps::LpData> lp_data;
};

// Initialize static members
std::string MPSParserTest::valid_filename;
std::string MPSParserTest::invalid_filename;
std::unique_ptr<mps::LpData> MPSParserTest::lp_data;

TEST_F(MPSParserTest, CountLines) {
    int lineCount = mps::count_lines(valid_filename);
    ASSERT_GT(lineCount, 0) << "File is empty";
    ASSERT_EQ(lineCount, 6307) << "Unexpected number of lines";
}

TEST_F(MPSParserTest, ReadProblemName) {
    std::string problemName = mps::read_problem_name(valid_filename);
    ASSERT_EQ(problemName, "50v-10") << "Unexpected problem name";
}

TEST_F(MPSParserTest, ParseValidFile) {
    ASSERT_NE(lp_data, nullptr) << "LpData object is null";
    ASSERT_GT(lp_data->get_n_vars(), 0) << "No variables found";
    ASSERT_EQ(lp_data->get_c().size(), lp_data->get_n_vars()) << "Objective vector size mismatch";
    ASSERT_EQ(lp_data->get_lb().size(), lp_data->get_n_vars()) << "Lower bounds vector size mismatch";
    ASSERT_EQ(lp_data->get_ub().size(), lp_data->get_n_vars()) << "Upper bounds vector size mismatch";
}

TEST_F(MPSParserTest, ParseInvalidFile) {
    ASSERT_THROW(mps::parse_mps(invalid_filename), std::runtime_error)
        << "Should throw when file doesn't exist";
}

TEST_F(MPSParserTest, CheckConstraintMatrices) {
    ASSERT_NE(lp_data, nullptr) << "LpData object is null";
    
    // Check equality constraints
    if (lp_data->get_A_eq().rows() > 0) {
        ASSERT_EQ(lp_data->get_A_eq().cols(), lp_data->get_n_vars())
            << "Equality constraint matrix has wrong number of columns";
        ASSERT_EQ(lp_data->get_A_eq().rows(), lp_data->get_b_eq().size())
            << "Equality constraint matrix and RHS vector size mismatch";
    }

    // Check inequality constraints
    if (lp_data->get_A_ineq().rows() > 0) {
        ASSERT_EQ(lp_data->get_A_ineq().cols(), lp_data->get_n_vars())
            << "Inequality constraint matrix has wrong number of columns";
        ASSERT_EQ(lp_data->get_A_ineq().rows(), lp_data->get_b_ineq().size())
            << "Inequality constraint matrix and RHS vector size mismatch";
    }
}

TEST_F(MPSParserTest, CheckBounds) {
    ASSERT_NE(lp_data, nullptr) << "LpData object is null";
    
    // Check that all variables have bounds
    ASSERT_EQ(lp_data->get_lb().size(), lp_data->get_n_vars())
        << "Lower bounds vector size mismatch";
    ASSERT_EQ(lp_data->get_ub().size(), lp_data->get_n_vars())
        << "Upper bounds vector size mismatch";

    // Check that lower bounds are less than or equal to upper bounds
    for (int i = 0; i < lp_data->get_n_vars(); ++i) {
        ASSERT_LE(lp_data->get_lb()(i), lp_data->get_ub()(i))
            << "Lower bound greater than upper bound for variable " << i;
    }
}

TEST_F(MPSParserTest, CheckVariableNames) {
    ASSERT_NE(lp_data, nullptr) << "LpData object is null";
    
    const auto& col_names = lp_data->get_col_names();
    ASSERT_EQ(col_names.size(), lp_data->get_n_vars())
        << "Number of variable names doesn't match number of variables";

    // Check that all names are unique
    std::set<std::string> unique_names(col_names.begin(), col_names.end());
    ASSERT_EQ(unique_names.size(), col_names.size())
        << "Variable names are not unique";
}

TEST_F(MPSParserTest, CheckParseTime) {
    ASSERT_NE(lp_data, nullptr) << "LpData object is null";

    double parse_time = lp_data->get_parse_time_seconds();

    // Check that parse time is positive and within a reasonable range.
    ASSERT_GT(parse_time, 0.0)
        << "Parse time should be positive after parsing.";
    ASSERT_GE(parse_time, 1e-9)
        << "Parse time is unexpectedly close to zero."; 
    ASSERT_LT(parse_time, 10.0)
        << "Parse time seems excessively long (\"> 10s\")";
} 