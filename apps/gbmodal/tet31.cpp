#include <map>
#include <cmath>
#include <cassert>
#include "tet31.hpp"

namespace gbmodal {
namespace fem  {

void Tet31::construct_stiff_matrix(
    int i, 
    const std::vector<double> & params,  
    std::vector<Triplet> & tlist)
{
  Mat<12, 12> mat;
  construct_cell_stiff_matrix(i, params, mat);
  auto c2d = cell_to_dof(i);
  for(int p = 0; p < 12; p++)
    for(int q = 0; q < 12; q++)
      tlist.push_back(Triplet(c2d[p], c2d[q], mat[p][q]));
}

void Tet31::construct_cell_stiff_matrix(
    int i, 
    const std::vector<double> & params,  
    Mat<12, 12> & mat)
{
  assert(params.size() == 5);
  std::array<std::array<double, 3>, 4> gphi;
  double vol = m_mesh.grad_lambda(i, gphi);
  auto lam = params[3];
  auto mu = params[4];
  for(auto m = 0; m < 4; m++)
  {
    for(auto n = 0; n < 4; n++)
    {
      double xx = gphi[m][0] * gphi[n][0];
      double yy = gphi[m][1] * gphi[n][1];
      double zz = gphi[m][2] * gphi[n][2];

      // K00
      mat[3*m  ][3*n  ] = ((2 * mu + lam) * xx + mu * yy + mu * zz) * vol;

      // K11
      mat[3*m+1][3*n+1] = (mu * xx + (2 * mu + lam) * yy + mu * zz) * vol;

      // K22
      mat[3*m+2][3*n+2] = (mu * xx + mu * yy + (2 * mu + lam) * zz) * vol;

      // K01 and K10
      mat[3*m  ][3*n+1] = (lam * gphi[m][0] * gphi[n][1] + mu*gphi[m][1] * gphi[n][0]) * vol;
      mat[3*n+1][3*m  ] = mat[3*m  ][3*n+1];

      // K12  and K21
      mat[3*m+1][3*n+2] = (lam * gphi[m][1] * gphi[n][2] + mu * gphi[m][2] * gphi[n][1]) * vol;
      mat[3*n+2][3*m+1] = mat[3*m+1][3*n+2];

      // K02 and K20
      mat[3*m  ][3*n+2] = (lam * gphi[m][0] * gphi[n][2] + mu * gphi[m][2] * gphi[n][0]) * vol;
      mat[3*n+2][3*m  ] = mat[3*m][3*n+2];
    }
  }
}

void Tet31::get_section_params(Data & section, std::vector<double> & params)
{
  params.resize(5); 
  auto & data = m_mesh.data();
  params[0] = data["material"][section["material"]]["density"];
  auto & e = data["material"][section["material"]]["elastic"];
  params[1] = e[0]; /*< 杨氏模量 E */
  params[2] = e[1]; /*< 泊松比 nu*/
  params[3] = params[1]*params[2]/(1 + params[2])/(1 - 2*params[2]); /*< 计算拉梅第一参数 lambda*/
  params[4] = params[1]/2.0/(1 + params[2]); /*< 计算拉梅第二参数, 剪切模量 G 或 mu*/
}

} // end of namespace fem
} // end of namespace gbmodal 
