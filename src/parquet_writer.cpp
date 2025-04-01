#include "parquet_writer.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace mps {

namespace fs = std::filesystem;
using json = nlohmann::json;

// Helper function to save a sparse matrix in COO format to parquet
arrow::Status save_coo_matrix(const Eigen::SparseMatrix<double>& matrix,
                                           const std::string& filename) {
    if (matrix.nonZeros() == 0) {
        return arrow::Status::OK();
    }

    // Convert to COO format if not already
    Eigen::SparseMatrix<double> coo_mat = matrix;
    if (matrix.isCompressed()) {
        coo_mat.makeCompressed();
    }

    // Create Arrow arrays for row, col, and data
    arrow::Int64Builder row_builder;
    arrow::Int64Builder col_builder;
    arrow::DoubleBuilder data_builder;

    // Reserve space
    ARROW_RETURN_NOT_OK(row_builder.Reserve(coo_mat.nonZeros()));
    ARROW_RETURN_NOT_OK(col_builder.Reserve(coo_mat.nonZeros()));
    ARROW_RETURN_NOT_OK(data_builder.Reserve(coo_mat.nonZeros()));

    // Fill arrays
    for (int k = 0; k < coo_mat.outerSize(); ++k) {
        for (Eigen::SparseMatrix<double>::InnerIterator it(coo_mat, k); it; ++it) {
            ARROW_RETURN_NOT_OK(row_builder.Append(it.row()));
            ARROW_RETURN_NOT_OK(col_builder.Append(it.col()));
            ARROW_RETURN_NOT_OK(data_builder.Append(it.value()));
        }
    }

    // Finish building arrays
    ARROW_ASSIGN_OR_RAISE(auto row_array, row_builder.Finish());
    ARROW_ASSIGN_OR_RAISE(auto col_array, col_builder.Finish());
    ARROW_ASSIGN_OR_RAISE(auto data_array, data_builder.Finish());

    // Create table
    auto schema = arrow::schema({
        arrow::field("row", arrow::int64()),
        arrow::field("col", arrow::int64()),
        arrow::field("data", arrow::float64())
    });

    auto table = arrow::Table::Make(schema, {row_array, col_array, data_array});

    // Write to Parquet file
    ARROW_ASSIGN_OR_RAISE(auto outfile, arrow::io::FileOutputStream::Open(filename));
    
    PARQUET_THROW_NOT_OK(
        parquet::arrow::WriteTable(*table, arrow::default_memory_pool(), outfile, 1024)
    );

    return arrow::Status::OK();
}

// Helper function to save a vector to parquet
arrow::Status save_vector(const Eigen::VectorXd& vec,
                                       const std::string& name,
                                       const std::string& filename) {
    if (vec.size() == 0) {
        return arrow::Status::OK();
    }

    arrow::DoubleBuilder builder;
    ARROW_RETURN_NOT_OK(builder.Reserve(vec.size()));
    
    for (int i = 0; i < vec.size(); ++i) {
        ARROW_RETURN_NOT_OK(builder.Append(vec[i]));
    }

    ARROW_ASSIGN_OR_RAISE(auto array, builder.Finish());

    auto schema = arrow::schema({arrow::field(name, arrow::float64())});
    auto table = arrow::Table::Make(schema, {array});

    ARROW_ASSIGN_OR_RAISE(auto outfile, arrow::io::FileOutputStream::Open(filename));
    
    PARQUET_THROW_NOT_OK(
        parquet::arrow::WriteTable(*table, arrow::default_memory_pool(), outfile, 1024)
    );

    return arrow::Status::OK();
}

std::tuple<std::string, double> save_lp_to_parquet(const LpData& lp_data,
                                                  const std::string& instance_name) {
    auto start_time = std::chrono::high_resolution_clock::now();

    // Create output directory
    fs::path base_data_dir("data");
    fs::path output_dir = base_data_dir / (instance_name + "_parquet");
    fs::create_directories(output_dir);

    std::cout << "Saving data to directory: " << output_dir << std::endl;

    // Save vectors
    auto c_result = save_vector(lp_data.get_c(), "c", 
                              (output_dir / "c.parquet").string());
    if (!c_result.ok()) {
        throw std::runtime_error("Failed to save c vector: " + c_result.ToString());
    }

    // Save bounds
    arrow::DoubleBuilder lb_builder, ub_builder;
    const auto& lb = lp_data.get_lb();
    const auto& ub = lp_data.get_ub();
    
    PARQUET_THROW_NOT_OK(lb_builder.Reserve(lb.size()));
    PARQUET_THROW_NOT_OK(ub_builder.Reserve(ub.size()));
    
    for (int i = 0; i < lb.size(); ++i) {
        PARQUET_THROW_NOT_OK(lb_builder.Append(lb[i]));
        PARQUET_THROW_NOT_OK(ub_builder.Append(ub[i]));
    }

    auto lb_result = lb_builder.Finish();
    auto ub_result = ub_builder.Finish();
    if (!lb_result.ok() || !ub_result.ok()) {
        throw std::runtime_error("Failed to build bounds arrays");
    }

    auto bounds_schema = arrow::schema({
        arrow::field("lb", arrow::float64()),
        arrow::field("ub", arrow::float64())
    });
    auto bounds_table = arrow::Table::Make(bounds_schema, {*lb_result, *ub_result});

    auto bounds_file_result = arrow::io::FileOutputStream::Open((output_dir / "bounds.parquet").string());
    if (!bounds_file_result.ok()) {
        throw std::runtime_error("Failed to create bounds file: " + bounds_file_result.status().ToString());
    }

    PARQUET_THROW_NOT_OK(
        parquet::arrow::WriteTable(*bounds_table, arrow::default_memory_pool(), *bounds_file_result, 1024)
    );

    // Save equality constraints
    if (lp_data.get_b_eq().size() > 0) {
        auto b_eq_result = save_vector(lp_data.get_b_eq(), "b_eq",
                                     (output_dir / "b_eq.parquet").string());
        if (!b_eq_result.ok()) {
            throw std::runtime_error("Failed to save b_eq vector: " + b_eq_result.ToString());
        }

        auto A_eq_result = save_coo_matrix(lp_data.get_A_eq(),
                                         (output_dir / "A_eq_coo.parquet").string());
        if (!A_eq_result.ok()) {
            throw std::runtime_error("Failed to save A_eq matrix: " + A_eq_result.ToString());
        }
    }

    // Save inequality constraints
    if (lp_data.get_b_ineq().size() > 0) {
        auto b_ineq_result = save_vector(lp_data.get_b_ineq(), "b_ineq",
                                       (output_dir / "b_ineq.parquet").string());
        if (!b_ineq_result.ok()) {
            throw std::runtime_error("Failed to save b_ineq vector: " + b_ineq_result.ToString());
        }

        auto A_ineq_result = save_coo_matrix(lp_data.get_A_ineq(),
                                           (output_dir / "A_ineq_coo.parquet").string());
        if (!A_ineq_result.ok()) {
            throw std::runtime_error("Failed to save A_ineq matrix: " + A_ineq_result.ToString());
        }
    }

    // Calculate save time
    auto end_time = std::chrono::high_resolution_clock::now();
    double save_parquet_time = std::chrono::duration<double>(end_time - start_time).count();

    std::cout << "Finished saving to Parquet in " << save_parquet_time << " seconds" << std::endl;

    // Save metadata
    json metadata = {
        {"n_vars", lp_data.get_n_vars()},
        {"obj_offset", lp_data.get_obj_offset()},
        {"parse_time_seconds", lp_data.get_parse_time_seconds()},
        {"save_parquet_time_seconds", save_parquet_time}
    };

    std::ofstream metadata_file(output_dir / "metadata.json");
    metadata_file << metadata.dump(4);
    metadata_file.close();

    return {output_dir.string(), save_parquet_time};
}

} // namespace mps 