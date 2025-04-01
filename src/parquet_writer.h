#ifndef PARQUET_WRITER_H
#define PARQUET_WRITER_H

#include "lp_data.h"
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <arrow/result.h>
#include <parquet/arrow/writer.h>
#include <string>
#include <chrono>
#include <filesystem>
#include <tuple>

namespace mps {

// Helper function to save a sparse matrix in COO format to parquet
arrow::Status save_coo_matrix(const Eigen::SparseMatrix<double>& matrix,
                                           const std::string& filename);

// Helper function to save a vector to parquet
arrow::Status save_vector(const Eigen::VectorXd& vec,
                                       const std::string& name,
                                       const std::string& filename);

// Function to save LpData to parquet files
// Returns {output_directory_path, save_time_in_seconds}
std::tuple<std::string, double> save_lp_to_parquet(const LpData& lp_data, const std::string& instance_name);

} // namespace mps

#endif // PARQUET_WRITER_H 