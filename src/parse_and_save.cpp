#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include "mps_parser.h"
#include "parquet_writer.h"
#include "lp_data.h" // Include LpData definition

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_mps_file>" << std::endl;
        return 1;
    }

    std::string mps_file_path = argv[1];

    // Check if file exists
    if (!fs::exists(mps_file_path)) {
        std::cerr << "Error: MPS file not found: " << mps_file_path << std::endl;
        return 1;
    }

    try {
        std::cout << "Parsing MPS file: " << mps_file_path << std::endl;
        
        // Parse the MPS file
        std::unique_ptr<mps::LpData> lp_data = mps::parse_mps(mps_file_path);

        if (!lp_data) {
             std::cerr << "Error: Failed to parse MPS file (returned null LpData)." << std::endl;
             return 1;
        }

        std::cout << "Successfully parsed MPS file." << std::endl;
        std::cout << "Variables: " << lp_data->get_n_vars() << std::endl;
        std::cout << "Equality Constraints: " << lp_data->get_A_eq().rows() << std::endl;
        std::cout << "Inequality Constraints: " << lp_data->get_A_ineq().rows() << std::endl;

        // Extract instance name from file path
        std::string instance_name = fs::path(mps_file_path).stem().string();

        // Add debug output
        std::cout << "[DEBUG] n_vars before saving: " << lp_data->get_n_vars() << std::endl; 

        // Save the LpData to Parquet files
        std::cout << "\nSaving LP data to Parquet for instance: " << instance_name << std::endl;
        auto [output_dir, save_time] = mps::save_lp_to_parquet(*lp_data, instance_name);
        
        std::cout << "\nSuccessfully saved data to: " << output_dir << std::endl;
        std::cout << "Save time: " << save_time << " seconds" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "\nAn error occurred: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\nAn unknown error occurred." << std::endl;
        return 1;
    }

    return 0;
} 