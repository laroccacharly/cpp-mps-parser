#ifndef LP_DATA_H
#define LP_DATA_H

#include <Eigen/Sparse>
#include <vector>
#include <string>

namespace mps {

class LpData {
public:
    LpData(int n_vars,
           const Eigen::VectorXd& c,
           const std::pair<Eigen::VectorXd, Eigen::VectorXd>& bounds,
           const Eigen::SparseMatrix<double>& A_eq,
           const Eigen::VectorXd& b_eq,
           const Eigen::SparseMatrix<double>& A_ineq,
           const Eigen::VectorXd& b_ineq,
           double obj_offset,
           const std::vector<std::string>& col_names,
           double parse_time_seconds = 0.0);

    // Getters
    int get_n_vars() const { return n_vars_; }
    const Eigen::VectorXd& get_c() const { return c_; }
    const Eigen::VectorXd& get_lb() const { return lb_; }
    const Eigen::VectorXd& get_ub() const { return ub_; }
    const Eigen::SparseMatrix<double>& get_A_eq() const { return A_eq_; }
    const Eigen::VectorXd& get_b_eq() const { return b_eq_; }
    const Eigen::SparseMatrix<double>& get_A_ineq() const { return A_ineq_; }
    const Eigen::VectorXd& get_b_ineq() const { return b_ineq_; }
    double get_obj_offset() const { return obj_offset_; }
    const std::vector<std::string>& get_col_names() const { return col_names_; }
    double get_parse_time_seconds() const { return parse_time_seconds_; }

private:
    int n_vars_;
    Eigen::VectorXd c_;              // Objective coefficients
    Eigen::VectorXd lb_, ub_;        // Lower and upper bounds
    Eigen::SparseMatrix<double> A_eq_;    // Equality constraints matrix
    Eigen::VectorXd b_eq_;           // Equality constraints RHS
    Eigen::SparseMatrix<double> A_ineq_;  // Inequality constraints matrix
    Eigen::VectorXd b_ineq_;         // Inequality constraints RHS
    double obj_offset_;              // Objective function offset
    std::vector<std::string> col_names_;  // Variable names
    double parse_time_seconds_;      // Added parse time member
};

} // namespace mps

#endif // LP_DATA_H 