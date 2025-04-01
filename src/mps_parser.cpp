#include "mps_parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <iostream>
#include <chrono>

namespace mps {

ParserState::ParserState() = default;
ParserState::~ParserState() = default;

void ParserState::add_row(const std::string& name, char type) {
    row_names_.push_back(name);
    row_types_[name] = type;
    if (type == 'N') {
        objective_name_ = name;
    }
}

void ParserState::add_column_coefficient(const std::string& col_name, const std::string& row_name, double value) {
    if (std::find(col_names_.begin(), col_names_.end(), col_name) == col_names_.end()) {
        col_names_.push_back(col_name);
    }

    if (row_name == objective_name_) {
        objective_[col_name] = value;
    } else {
        constraints_[row_name][col_name] = value;
    }
}

void ParserState::add_rhs_value(const std::string& row_name, double value) {
    rhs_values_[row_name] = value;
}

void ParserState::add_bound(const std::string& type, const std::string& col_name, double value) {
    if (bounds_.find(col_name) == bounds_.end()) {
        bounds_[col_name] = {0.0, std::numeric_limits<double>::infinity()};
    }

    auto& bound = bounds_[col_name];
    if (type == "LO") {
        bound.first = value;
    } else if (type == "UP") {
        bound.second = value;
    } else if (type == "FX") {
        bound.first = bound.second = value;
    } else if (type == "FR") {
        bound.first = -std::numeric_limits<double>::infinity();
        bound.second = std::numeric_limits<double>::infinity();
    } else if (type == "MI") {
        bound.first = -std::numeric_limits<double>::infinity();
    } else if (type == "PL") {
        bound.second = std::numeric_limits<double>::infinity();
    } else if (type == "BV") {
        bound.first = 0.0;
        bound.second = 1.0;
    }
}

void ParserState::set_default_bounds() {
    for (const auto& col : col_names_) {
        if (bounds_.find(col) == bounds_.end()) {
            bounds_[col] = {0.0, std::numeric_limits<double>::infinity()};
        }
    }
}

std::pair<Eigen::VectorXd, Eigen::VectorXd> ParserState::create_bounds() const {
    const int n_vars = col_names_.size();
    Eigen::VectorXd lb = Eigen::VectorXd::Zero(n_vars);
    Eigen::VectorXd ub = Eigen::VectorXd::Constant(n_vars, std::numeric_limits<double>::infinity());

    for (int i = 0; i < n_vars; ++i) {
        const auto& col = col_names_[i];
        if (bounds_.find(col) != bounds_.end()) {
            const auto& bound = bounds_.at(col);
            lb(i) = bound.first;
            ub(i) = bound.second;
        }
    }

    return {lb, ub};
}

void ParserState::build_matrices(int& n_vars,
                               Eigen::VectorXd& c,
                               Eigen::SparseMatrix<double>& A_eq,
                               Eigen::VectorXd& b_eq,
                               Eigen::SparseMatrix<double>& A_ineq,
                               Eigen::VectorXd& b_ineq) const {
    using Triplet = Eigen::Triplet<double>;
    std::vector<Triplet> eq_triplets, ineq_triplets;
    std::vector<double> eq_rhs, ineq_rhs;
    std::vector<int> eq_indices, l_indices, g_indices;

    // Count constraints by type
    for (size_t i = 0; i < row_names_.size(); ++i) {
        const auto& row = row_names_[i];
        if (row == objective_name_) continue;

        char type = row_types_.at(row);
        if (type == 'E') eq_indices.push_back(i);
        else if (type == 'L') l_indices.push_back(i);
        else if (type == 'G') g_indices.push_back(i);
    }

    // Set dimensions
    n_vars = col_names_.size();
    c = Eigen::VectorXd::Zero(n_vars);

    // Fill objective coefficients
    for (const auto& [col, value] : objective_) {
        auto it = std::find(col_names_.begin(), col_names_.end(), col);
        if (it != col_names_.end()) {
            c(std::distance(col_names_.begin(), it)) = value;
        }
    }

    // Build equality constraints
    if (!eq_indices.empty()) {
        A_eq.resize(eq_indices.size(), n_vars);
        b_eq.resize(eq_indices.size());

        for (size_t i = 0; i < eq_indices.size(); ++i) {
            const auto& row = row_names_[eq_indices[i]];
            b_eq(i) = rhs_values_.count(row) ? rhs_values_.at(row) : 0.0;

            if (constraints_.count(row)) {
                for (const auto& [col, value] : constraints_.at(row)) {
                    auto it = std::find(col_names_.begin(), col_names_.end(), col);
                    if (it != col_names_.end()) {
                        eq_triplets.emplace_back(i, std::distance(col_names_.begin(), it), value);
                    }
                }
            }
        }
        A_eq.setFromTriplets(eq_triplets.begin(), eq_triplets.end());
    }

    // Build inequality constraints
    const size_t n_ineq = l_indices.size() + g_indices.size();
    if (n_ineq > 0) {
        A_ineq.resize(n_ineq, n_vars);
        b_ineq.resize(n_ineq);

        size_t ineq_idx = 0;

        // Process L constraints
        for (int l_idx : l_indices) {
            const auto& row = row_names_[l_idx];
            b_ineq(ineq_idx) = rhs_values_.count(row) ? rhs_values_.at(row) : 0.0;

            if (constraints_.count(row)) {
                for (const auto& [col, value] : constraints_.at(row)) {
                    auto it = std::find(col_names_.begin(), col_names_.end(), col);
                    if (it != col_names_.end()) {
                        ineq_triplets.emplace_back(ineq_idx, std::distance(col_names_.begin(), it), value);
                    }
                }
            }
            ineq_idx++;
        }

        // Process G constraints (convert to ≤ form by negating)
        for (int g_idx : g_indices) {
            const auto& row = row_names_[g_idx];
            b_ineq(ineq_idx) = rhs_values_.count(row) ? -rhs_values_.at(row) : 0.0;

            if (constraints_.count(row)) {
                for (const auto& [col, value] : constraints_.at(row)) {
                    auto it = std::find(col_names_.begin(), col_names_.end(), col);
                    if (it != col_names_.end()) {
                        ineq_triplets.emplace_back(ineq_idx, std::distance(col_names_.begin(), it), -value);
                    }
                }
            }
            ineq_idx++;
        }

        A_ineq.setFromTriplets(ineq_triplets.begin(), ineq_triplets.end());
    }
}

void parse_rows_section(const std::string& line, ParserState& state) {
    std::istringstream iss(line);
    std::string type_str, name;
    iss >> type_str >> name;
    
    if (type_str.length() != 1) {
        throw std::runtime_error("Invalid row type: " + type_str);
    }
    
    state.add_row(name, type_str[0]);
}

void parse_columns_section(const std::string& line, ParserState& state) {
    std::istringstream iss(line);
    std::string col_name;
    iss >> col_name;

    if (col_name == "'MARKER'") return;

    std::string row_name;
    double value;
    while (iss >> row_name >> value) {
        state.add_column_coefficient(col_name, row_name, value);
    }
}

void parse_rhs_section(const std::string& line, ParserState& state) {
    std::istringstream iss(line);
    std::string rhs_name;  // Skip RHS name
    iss >> rhs_name;

    std::string row_name;
    double value;
    while (iss >> row_name >> value) {
        state.add_rhs_value(row_name, value);
    }
}

void parse_bounds_section(const std::string& line, ParserState& state) {
    std::istringstream iss(line);
    std::string bound_type, bound_name, col_name;
    iss >> bound_type >> bound_name >> col_name;

    double value = 0.0;
    if (!(bound_type == "FR" || bound_type == "MI" || bound_type == "PL")) {
        iss >> value;
    }

    state.add_bound(bound_type, col_name, value);
}

std::unique_ptr<LpData> parse_mps(const std::string& path) {
    const auto start_time = std::chrono::steady_clock::now();
    std::cout << "Starting MPS parsing for file: " << path << std::endl;

    ParserState state;
    std::string current_section;

    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + path);
        }

        std::string line;
        size_t line_num = 0;
        while (std::getline(file, line)) {
            // Check timeout
            if (line_num++ % 100 == 0) {
                auto current_time = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::seconds>(
                        current_time - start_time) > TIMEOUT_SECONDS) {
                    throw std::runtime_error("MPS parsing exceeded timeout");
                }
            }

            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);

            if (line.empty() || line[0] == '*') continue;

            // Check for section headers
            if (line == "NAME" || line == "ROWS" || line == "COLUMNS" || 
                line == "RHS" || line == "RANGES" || line == "BOUNDS" || 
                line == "ENDATA") {
                if (line == "ENDATA") break;
                current_section = line;
                if (line == "RANGES") {
                    std::cout << "Warning: RANGES section is not currently handled" << std::endl;
                }
                continue;
            }

            try {
                if (current_section == "ROWS") {
                    parse_rows_section(line, state);
                } else if (current_section == "COLUMNS") {
                    parse_columns_section(line, state);
                } else if (current_section == "RHS") {
                    parse_rhs_section(line, state);
                } else if (current_section == "BOUNDS") {
                    parse_bounds_section(line, state);
                }
            } catch (const std::exception& e) {
                throw std::runtime_error("Error parsing line " + 
                    std::to_string(line_num) + " in section " + 
                    current_section + ": " + e.what());
            }
        }

        std::cout << "Finished reading MPS sections in " 
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::steady_clock::now() - start_time).count() / 1000.0 
                  << " seconds" << std::endl;

        // Post-processing and matrix construction
        state.set_default_bounds();
        auto bounds = state.create_bounds();

        int n_vars;
        Eigen::VectorXd c;
        Eigen::SparseMatrix<double> A_eq, A_ineq;
        Eigen::VectorXd b_eq, b_ineq;

        state.build_matrices(n_vars, c, A_eq, b_eq, A_ineq, b_ineq);

        std::cout << "Total parsing time: " 
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::steady_clock::now() - start_time).count() / 1000.0 
                  << " seconds" << std::endl;

        return std::make_unique<LpData>(
            n_vars,
            c,
            bounds,
            A_eq,
            b_eq,
            A_ineq,
            b_ineq,
            0.0,  // obj_offset
            state.get_col_names()
        );

    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        throw;
    }
}

} // namespace mps 