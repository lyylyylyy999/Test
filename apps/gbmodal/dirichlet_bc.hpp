#ifndef DIRICHLET_BC_HPP_
#define DIRICHLET_BC_HPP_

#include "Eigen/Sparse"
#include "Eigen/Dense"

namespace gbmodal::fem {

class DirichletBC
{
public:
  using CSRMatrix = Eigen::SparseMatrix<double, Eigen::RowMajor>;
  using VectorXd = Eigen::VectorXd;
  using VectorXb = Eigen::Matrix<bool, -1, 1>;

  void apply(VectorXb & dflag, VectorXd & x, CSRMatrix & K,  VectorXd & b);
};

} // end of namespace gbmodal::fem

#endif // DIRICHLET_BC_HPP_
