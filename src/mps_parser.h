#ifndef MPS_PARSER_H
#define MPS_PARSER_H

#include "lp_data.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>

namespace mps {

// Forward declarations
class ParserState;

// Constants
constexpr std::chrono::seconds TIMEOUT_SECONDS{1000};

// Main parsing function
std::unique_ptr<LpData> parse_mps(const std::string& path);

class ParserState {
public:
    ParserState();
    ~ParserState();

    // Getters
    const std::vector<std::string>& get_row_names() const { return row_names_; }
    const std::vector<std::string>& get_col_names() const { return col_names_; }
    const std::string& get_objective_name() const { return objective_name_; }

    // State modification methods
    void add_row(const std::string& name, char type);
    void add_column_coefficient(const std::string& col_name, const std::string& row_name, double value);
    void add_rhs_value(const std::string& row_name, double value);
    void add_bound(const std::string& type, const std::string& col_name, double value);
    void set_objective_name(const std::string& name) { objective_name_ = name; }

    // Matrix construction helpers
    void set_default_bounds();
    std::pair<Eigen::VectorXd, Eigen::VectorXd> create_bounds() const;
    void build_matrices(int& n_vars,
                       Eigen::VectorXd& c,
                       Eigen::SparseMatrix<double>& A_eq,
                       Eigen::VectorXd& b_eq,
                       Eigen::SparseMatrix<double>& A_ineq,
                       Eigen::VectorXd& b_ineq) const;

private:
    std::vector<std::string> row_names_;
    std::vector<std::string> col_names_;
    std::string objective_name_;
    std::map<std::string, std::map<std::string, double>> constraints_;  // row -> (col -> value)
    std::map<std::string, double> objective_;  // col -> value
    std::map<std::string, double> rhs_values_;  // row -> value
    std::map<std::string, std::pair<double, double>> bounds_;  // col -> (lower, upper)
    std::map<std::string, char> row_types_;  // row -> type (N, E, L, G)
};

// Section parsing functions
void parse_rows_section(const std::string& line, ParserState& state);
void parse_columns_section(const std::string& line, ParserState& state);
void parse_rhs_section(const std::string& line, ParserState& state);
void parse_bounds_section(const std::string& line, ParserState& state);

} // namespace mps

#endif // MPS_PARSER_H 