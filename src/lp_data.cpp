#include "lp_data.h"

namespace mps {

LpData::LpData(int n_vars,
               const Eigen::VectorXd& c,
               const std::pair<Eigen::VectorXd, Eigen::VectorXd>& bounds,
               const Eigen::SparseMatrix<double>& A_eq,
               const Eigen::VectorXd& b_eq,
               const Eigen::SparseMatrix<double>& A_ineq,
               const Eigen::VectorXd& b_ineq,
               double obj_offset,
               const std::vector<std::string>& col_names)
    : n_vars_(n_vars)
    , c_(c)
    , lb_(bounds.first)
    , ub_(bounds.second)
    , A_eq_(A_eq)
    , b_eq_(b_eq)
    , A_ineq_(A_ineq)
    , b_ineq_(b_ineq)
    , obj_offset_(obj_offset)
    , col_names_(col_names) {
}

} // namespace mps 