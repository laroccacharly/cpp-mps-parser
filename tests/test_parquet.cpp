#include <gtest/gtest.h>
#include <filesystem>
#include <arrow/io/api.h>
#include <arrow/result.h>
#include <parquet/arrow/reader.h>
#include <parquet/exception.h>
#include <arrow/testing/gtest_util.h>
#include "parquet_writer.h"
#include "mps_parser.h"

namespace fs = std::filesystem;

class ParquetWriterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple LpData instance for testing
        int n_vars = 3;
        Eigen::VectorXd c(n_vars);
        c << 1.0, 2.0, 3.0;

        Eigen::VectorXd lb(n_vars), ub(n_vars);
        lb << 0.0, 0.0, 0.0;
        ub << 1.0, 1.0, 1.0;

        // Create a sparse equality constraint matrix
        Eigen::SparseMatrix<double> A_eq(2, n_vars);
        std::vector<Eigen::Triplet<double>> triplets;
        triplets.emplace_back(0, 0, 1.0);
        triplets.emplace_back(0, 1, 1.0);
        triplets.emplace_back(1, 1, 1.0);
        triplets.emplace_back(1, 2, 1.0);
        A_eq.setFromTriplets(triplets.begin(), triplets.end());

        Eigen::VectorXd b_eq(2);
        b_eq << 1.0, 1.0;

        // Create a sparse inequality constraint matrix
        Eigen::SparseMatrix<double> A_ineq(1, n_vars);
        triplets.clear();
        triplets.emplace_back(0, 0, 1.0);
        triplets.emplace_back(0, 2, 1.0);
        A_ineq.setFromTriplets(triplets.begin(), triplets.end());

        Eigen::VectorXd b_ineq(1);
        b_ineq << 2.0;

        std::vector<std::string> col_names = {"x1", "x2", "x3"};
        
        test_data = std::make_unique<mps::LpData>(
            n_vars, c, std::make_pair(lb, ub),
            A_eq, b_eq, A_ineq, b_ineq,
            0.0, col_names
        );

        // Create test directory
        test_dir = fs::temp_directory_path() / "parquet_test";
        fs::create_directories(test_dir);
    }

    void TearDown() override {
        // Clean up test directory
        fs::remove_all(test_dir);
    }

    // Helper function to read and verify a vector parquet file
    void verify_vector_file(const std::string& filename, const Eigen::VectorXd& expected, const std::string& col_name) {
        ASSERT_OK_AND_ASSIGN(auto infile,
            arrow::io::ReadableFile::Open(filename));

        std::unique_ptr<parquet::arrow::FileReader> reader;
        PARQUET_ASSIGN_OR_THROW(reader,
            parquet::arrow::OpenFile(infile, arrow::default_memory_pool()));

        std::shared_ptr<arrow::Table> table;
        ASSERT_OK(reader->ReadTable(&table));

        ASSERT_EQ(table->num_rows(), expected.size());
        ASSERT_EQ(table->num_columns(), 1);
        ASSERT_EQ(table->schema()->field(0)->name(), col_name);

        auto array = std::static_pointer_cast<arrow::DoubleArray>(table->column(0)->chunk(0));
        for (int i = 0; i < expected.size(); ++i) {
            ASSERT_DOUBLE_EQ(array->Value(i), expected[i]);
        }
    }

    // Helper function to verify a sparse matrix parquet file
    void verify_coo_matrix_file(const std::string& filename, const Eigen::SparseMatrix<double>& expected) {
        ASSERT_OK_AND_ASSIGN(auto infile,
            arrow::io::ReadableFile::Open(filename));

        std::unique_ptr<parquet::arrow::FileReader> reader;
        PARQUET_ASSIGN_OR_THROW(reader,
            parquet::arrow::OpenFile(infile, arrow::default_memory_pool()));

        std::shared_ptr<arrow::Table> table;
        ASSERT_OK(reader->ReadTable(&table));

        ASSERT_EQ(table->num_rows(), expected.nonZeros());
        ASSERT_EQ(table->num_columns(), 3);

        auto row_array = std::static_pointer_cast<arrow::Int64Array>(table->column(0)->chunk(0));
        auto col_array = std::static_pointer_cast<arrow::Int64Array>(table->column(1)->chunk(0));
        auto data_array = std::static_pointer_cast<arrow::DoubleArray>(table->column(2)->chunk(0));

        // Convert back to sparse matrix and compare
        Eigen::SparseMatrix<double> read_matrix(expected.rows(), expected.cols());
        std::vector<Eigen::Triplet<double>> triplets;
        for (int64_t i = 0; i < table->num_rows(); ++i) {
            triplets.emplace_back(row_array->Value(i), col_array->Value(i), data_array->Value(i));
        }
        read_matrix.setFromTriplets(triplets.begin(), triplets.end());

        // Compare matrices
        ASSERT_EQ(read_matrix.nonZeros(), expected.nonZeros());
        for (int k = 0; k < read_matrix.outerSize(); ++k) {
            for (Eigen::SparseMatrix<double>::InnerIterator it(read_matrix, k); it; ++it) {
                ASSERT_DOUBLE_EQ(it.value(), expected.coeff(it.row(), it.col()));
            }
        }
    }

    std::unique_ptr<mps::LpData> test_data;
    fs::path test_dir;
};

TEST_F(ParquetWriterTest, SaveVector) {
    const auto& c = test_data->get_c();
    std::string filename = (test_dir / "c.parquet").string();
    
    auto result = mps::save_vector(c, "c", filename);
    ASSERT_OK(result);
    verify_vector_file(filename, c, "c");
}

TEST_F(ParquetWriterTest, SaveSparseMatrix) {
    const auto& A_eq = test_data->get_A_eq();
    std::string filename = (test_dir / "A_eq.parquet").string();
    
    auto result = mps::save_coo_matrix(A_eq, filename);
    ASSERT_OK(result);
    verify_coo_matrix_file(filename, A_eq);
}

TEST_F(ParquetWriterTest, SaveFullLpData) {
    auto [output_dir, save_time] = mps::save_lp_to_parquet(*test_data, "test_instance");
    
    // Verify directory exists
    ASSERT_TRUE(fs::exists(output_dir));
    ASSERT_TRUE(fs::is_directory(output_dir));

    // Verify all files exist
    ASSERT_TRUE(fs::exists(fs::path(output_dir) / "c.parquet"));
    ASSERT_TRUE(fs::exists(fs::path(output_dir) / "bounds.parquet"));
    ASSERT_TRUE(fs::exists(fs::path(output_dir) / "A_eq_coo.parquet"));
    ASSERT_TRUE(fs::exists(fs::path(output_dir) / "b_eq.parquet"));
    ASSERT_TRUE(fs::exists(fs::path(output_dir) / "A_ineq_coo.parquet"));
    ASSERT_TRUE(fs::exists(fs::path(output_dir) / "b_ineq.parquet"));
    ASSERT_TRUE(fs::exists(fs::path(output_dir) / "metadata.json"));

    // Verify vector contents
    verify_vector_file((fs::path(output_dir) / "c.parquet").string(), test_data->get_c(), "c");
    verify_vector_file((fs::path(output_dir) / "b_eq.parquet").string(), test_data->get_b_eq(), "b_eq");
    verify_vector_file((fs::path(output_dir) / "b_ineq.parquet").string(), test_data->get_b_ineq(), "b_ineq");

    // Verify matrix contents
    verify_coo_matrix_file((fs::path(output_dir) / "A_eq_coo.parquet").string(), test_data->get_A_eq());
    verify_coo_matrix_file((fs::path(output_dir) / "A_ineq_coo.parquet").string(), test_data->get_A_ineq());

    // Verify bounds file
    ASSERT_OK_AND_ASSIGN(auto bounds_file,
        arrow::io::ReadableFile::Open((fs::path(output_dir) / "bounds.parquet").string()));

    std::unique_ptr<parquet::arrow::FileReader> bounds_reader;
    PARQUET_ASSIGN_OR_THROW(bounds_reader,
        parquet::arrow::OpenFile(bounds_file, arrow::default_memory_pool()));

    std::shared_ptr<arrow::Table> bounds_table;
    ASSERT_OK(bounds_reader->ReadTable(&bounds_table));

    ASSERT_EQ(bounds_table->num_columns(), 2);
    ASSERT_EQ(bounds_table->schema()->field(0)->name(), "lb");
    ASSERT_EQ(bounds_table->schema()->field(1)->name(), "ub");

    auto lb_array = std::static_pointer_cast<arrow::DoubleArray>(bounds_table->column(0)->chunk(0));
    auto ub_array = std::static_pointer_cast<arrow::DoubleArray>(bounds_table->column(1)->chunk(0));

    const auto& lb = test_data->get_lb();
    const auto& ub = test_data->get_ub();
    for (int i = 0; i < lb.size(); ++i) {
        ASSERT_DOUBLE_EQ(lb_array->Value(i), lb[i]);
        ASSERT_DOUBLE_EQ(ub_array->Value(i), ub[i]);
    }

    // Clean up test directory
    fs::remove_all(output_dir);
}

TEST_F(ParquetWriterTest, SaveEmptyMatrices) {
    // Create an LP with no constraints
    int n_vars = 2;
    Eigen::VectorXd c(n_vars);
    c << 1.0, 1.0;

    Eigen::VectorXd lb(n_vars), ub(n_vars);
    lb << 0.0, 0.0;
    ub << 1.0, 1.0;

    Eigen::SparseMatrix<double> A_eq(0, n_vars);
    Eigen::VectorXd b_eq(0);
    Eigen::SparseMatrix<double> A_ineq(0, n_vars);
    Eigen::VectorXd b_ineq(0);

    std::vector<std::string> col_names = {"x1", "x2"};

    mps::LpData empty_data(n_vars, c, std::make_pair(lb, ub),
                          A_eq, b_eq, A_ineq, b_ineq,
                          0.0, col_names);

    auto [output_dir, save_time] = mps::save_lp_to_parquet(empty_data, "empty_test");

    // Verify only necessary files exist
    ASSERT_TRUE(fs::exists(fs::path(output_dir) / "c.parquet"));
    ASSERT_TRUE(fs::exists(fs::path(output_dir) / "bounds.parquet"));
    ASSERT_TRUE(fs::exists(fs::path(output_dir) / "metadata.json"));

    // Verify constraint files don't exist
    ASSERT_FALSE(fs::exists(fs::path(output_dir) / "A_eq_coo.parquet"));
    ASSERT_FALSE(fs::exists(fs::path(output_dir) / "b_eq.parquet"));
    ASSERT_FALSE(fs::exists(fs::path(output_dir) / "A_ineq_coo.parquet"));
    ASSERT_FALSE(fs::exists(fs::path(output_dir) / "b_ineq.parquet"));

    // Clean up test directory
    fs::remove_all(output_dir);
} 