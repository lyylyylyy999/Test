
#include "dirichlet_bc.hpp"

namespace gbmodal {
namespace fem {

void DirichletBC::apply(VectorXb & dflag, VectorXd & x, CSRMatrix & K,  VectorXd & b)
{

  auto gdof = K.rows();
  auto data = K.valuePtr(); // 非零元数组
  auto indices = K.innerIndexPtr(); // 非零元对应的列指标数组
  auto indptr = K.outerIndexPtr();  // 非零元的起始位置数组

  b -= K*x;
  for(int i = 0; i < gdof; i++) {
    if(dflag[i]) {
      b[i] = x[i];
      for(auto k = indptr[i]; k < indptr[i+1]; k++) {
        auto j = indices[k];
        if( i == j) {
          data[k] = 1.0;
        } else {
          data[k] = 0.0;
        }
      }
    } else {
      for(auto k = indptr[i]; k < indptr[i+1]; k++) {
        auto j = indices[k];
        if(dflag[j]) data[k] = 0.0; 
      }
    }
  }
}

} // end of namespace fem 
} // end of namespace gbmodal 
