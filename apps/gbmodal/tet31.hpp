#ifndef TET31_HPP_
#define TET31_HPP_

#include "Eigen/Dense"
#include "Eigen/Sparse"

#include "geoalg.h"
#include "linalg.h"
#include "tet_mesh.hpp"

namespace gbmodal {

namespace fem {

/**
 * \brief 三维（3）线性（1）四面体单元
 * \note 要考虑非线性的情形
 */
class Tet31
{
public:
  typedef Eigen::Triplet<double> Triplet;
  typedef Eigen::Matrix3d Matrix3d;
  typedef Eigen::SelfAdjointEigenSolver<Matrix3d>  EigenSolver; /**< 对称矩阵的特征值求解器*/

  template<size_t m, size_t n>
  using Mat = algebra::Mat<m, n>;

  template<size_t n>
  using Vec = algebra::Vec<n>;

  using Data = nlohmann::json;
  using C2D = std::array<int, 12>;

public:
  Tet31(mesh::TetMesh & mesh): m_mesh(mesh) { }

  /**
   * \brief 计算刚度矩阵
   * 
   * \param[in] cset 单元的编号
   * \param[in] params 用于计算的参数 
   * \param[out] tlist 
   */
  void construct_stiff_matrix(
      const std::vector<int> & cset,
      bool generate,
      const std::vector<double> & params, 
      std::vector<Triplet> & tlist
    ) 
  {
    if(generate) {
      for(auto i = cset[0]; i < cset[1]; i += cset[2])
        construct_stiff_matrix(i, params, tlist);
    } else {
      for(auto i : cset)
        construct_stiff_matrix(i, params, tlist);
    }
  }

  /**
   * \brief 计算第 c 个单元的刚度矩阵
   * 
   * \param[in] c 单元的编号
   * \param[in] params 用于计算的参数 
   * \param[out] tlist 三元组 
   */
  void construct_stiff_matrix(
      int i, 
      const std::vector<double> & params, 
      std::vector<Triplet> & tlist);

  /**
   * \brief 从 json 数据结构中获取计算所需的各种几何、物理等参数
   */
  void get_section_params(Data & section, std::vector<double> & params);

  /**
   * \brief 获取单元上的自由度管理数组
   */
  C2D cell_to_dof(int i)
  {
    auto & cell = m_mesh.cell(i);
    auto k = 3*cell[0]; 
    auto l = 3*cell[1];
    auto m = 3*cell[2];
    auto n = 3*cell[3];
    C2D c2d = {
      k, k+1, k+2,  
      l, l+1, l+2,
      m, m+1, m+2,
      n, n+1, n+2
    };
    return c2d;
  }

  /** 
   * \brief 计算给定单元集合上的应变和应力
   *
   * \note 注意，strain 和 stress 数组一定要置零
   */
  template<typename Vec>
  void compute_strain_and_stress(
      const std::vector<int> & cset, 
      bool generate,
      const Vec & u, 
      const std::vector<double> & params,
      Vec & strain, Vec & stress)
  {
    if(generate) {
      for(auto i = cset[0]; i < cset[1]; i+= cset[2])
        compute_strain_and_stress(i, u, params, strain, stress);
    } else {
      for(auto i : cset)
        compute_strain_and_stress(i, u, params, strain, stress);
    }
  }

  /** 
   * \brief 第 i 个单元上的应变和应力
   *
   * \note 注意，strain 和 stress 数组一定要置零
   */
  template<typename Vec>
  void compute_strain_and_stress(
      int i,
      const Vec & u,
      const std::vector<double> & params, 
      Vec & strain, Vec & stress)
  {
    auto lam = params[3];
    auto mu = params[4];
    std::array<std::array<double, 3>, 4> gphi;
    m_mesh.grad_lambda(i, gphi);
    auto c = m_mesh.cell(i);


    std::cout << i << ":" << std::endl;
    for(auto j = 0u; j < 4; j++) {
      strain[6*i    ] += u[3*c[j]    ]*gphi[j][0];
      strain[6*i + 1] += u[3*c[j] + 1]*gphi[j][1];
      strain[6*i + 2] += u[3*c[j] + 2]*gphi[j][2];

      strain[6*i + 3] += u[3*c[j] + 2]*gphi[j][1];
      strain[6*i + 3] += u[3*c[j] + 1]*gphi[j][2];

      strain[6*i + 4] += u[3*c[j] + 2]*gphi[j][0];
      strain[6*i + 4] += u[3*c[j] + 0]*gphi[j][2];

      strain[6*i + 5] += u[3*c[j] + 1]*gphi[j][0];
      strain[6*i + 5] += u[3*c[j] + 0]*gphi[j][1];
    }

    strain[6*i + 3] /= 2.0;
    strain[6*i + 4] /= 2.0;
    strain[6*i + 5] /= 2.0;


    auto val = 2*mu + lam;

    stress[6*i    ] = val*strain[6*i    ] + lam*(strain[6*i + 1] + strain[6*i + 2]);
    stress[6*i + 1] = val*strain[6*i + 1] + lam*(strain[6*i + 2] + strain[6*i + 0]);
    stress[6*i + 2] = val*strain[6*i + 2] + lam*(strain[6*i + 0] + strain[6*i + 1]);  // + 前的 strain[6*i + 1] 应该是 +2 

    stress[6*i + 3] = 2 * mu * strain[6*i + 3];
    stress[6*i + 4] = 2 * mu * strain[6*i + 4];
    stress[6*i + 5] = 2 * mu * strain[6*i + 5];

  }

  /**
   * \brief 计算给定单元集合 cset 上的 Mises 应力
   */
  template<typename Vec>
  void compute_mises_stress(
      const std::vector<int> & cset, 
      bool generate,
      const Vec & stress, Vec & mstress)
  {
    if(generate) {
      for(auto i = cset[0]; i < cset[1]; i+= cset[2])
        compute_mises_stress(i, stress, mstress);
    } else {
      for(auto i : cset)
        compute_mises_stress(i, stress, mstress);
    }
  }

  /**
   * \brief 计算第 i 个单元上的 Mises 应力
   */
  template<typename Vec>
  void compute_mises_stress(
      int i,
      const Vec & stress, Vec & mstress) {
    auto val = stress[6*i] - stress[6*i + 1];
    mstress[i] += val*val;

    val = stress[6*i] - stress[6*i + 2];
    mstress[i] += val*val;

    val = stress[6*i + 1] - stress[6*i + 2];
    mstress[i] += val*val;

    mstress[i] += 6*(stress[6*i + 3] * stress[6*i + 3] + 
        stress[6*i + 4] * stress[6*i + 4] + stress[6*i + 5] * stress[6*i + 5]);
    mstress[i] = std::sqrt(mstress[i]/2.0);
  }

  /**
   * \brief 计算给定单元集合 cset 上的主应力
   *
   * \note 每个单元的主应力有 3 个 
   */
  template<typename Vec>
  void compute_principal_stress(
      const std::vector<int> & cset, 
      bool generate,
      const Vec & stress, Vec & pstress)
  {
    if(generate) {
      for(auto i = cset[0]; i < cset[1]; i+= cset[2])
        compute_principal_stress(i, stress, pstress);
    } else {
      for(auto i : cset)
        compute_principal_stress(i, stress, pstress);
    }
  }

  /**
   * \brief 计算第 i 个单元上的主应力
   */
  template<typename Vec>
  void compute_principal_stress(
      int i,
      const Vec & stress, Vec & pstress)
  {
    Matrix3d A;
    A(0, 0) = stress[6*i    ];
    A(0, 1) = stress[6*i + 5];
    A(0, 2) = stress[6*i + 4];

    A(1, 0) = stress[6*i + 5];
    A(1, 1) = stress[6*i + 1];
    A(1, 2) = stress[6*i + 3];

    A(2, 0) = stress[6*i + 4];
    A(2, 1) = stress[6*i + 3];
    A(2, 2) = stress[6*i + 2];

    EigenSolver es(A);
    auto v = es.eigenvalues();
    pstress[3*i    ] = v[0];
    pstress[3*i + 1] = v[1];
    pstress[3*i + 2] = v[2];
  }

  /**
   * \brief 计算给定单元集合的最大绝对值主应力
   */
  template<typename Vec>
  void compute_max_abs_principal_stress(
      const std::vector<int> & cset, 
      bool generate,
      const Vec & pstress, Vec & mapstress)
  {
    if(generate) {
      for(auto i = cset[0]; i < cset[1]; i+= cset[2])
        compute_max_abs_principal_stress(i, pstress, mapstress);
    } else {
      for(auto i : cset)
        compute_max_abs_principal_stress(i, pstress, mapstress);
    }
  }

  /**
   * \brief 计算第 i 个单元上的最大绝对值主应力
   */
  template<typename Vec>
  void compute_max_abs_principal_stress(
      int i,
      const Vec & pstress, Vec & mapstress)
  {
    mapstress[i] = std::abs(pstress[3*i]);

    double val = std::abs(pstress[3*i+1]);
    if(mapstress[i] < val) mapstress[i] = val;

    val = std::abs(pstress[3*i+2]);
    if(mapstress[i] < val) mapstress[i] = val;
  }

  /**
   * \brief 计算给定单元集合上的 Tresca 应力
   */
  template<typename Vec>
  void compute_tresca_stress(
      const std::vector<int> & cset, 
      bool generate,
      const Vec & pstress, Vec & tstress)
  {
    if(generate) {
      for(auto i = cset[0]; i < cset[1]; i+= cset[2])
        compute_tresca_stress(i, pstress, tstress);
    } else {
      for(auto i : cset)
        compute_tresca_stress(i, pstress, tstress);
    }
  }

  /**
   * \brief 计算第 i 个单元集合上的 Tresca 应力
   */
  template<typename Vec>
  void compute_tresca_stress(
      int i,
      const Vec & pstress, Vec & tstress)
  {
    tstress[i] = pstress[3*i + 2] - pstress[3*i];
  }


private:
  /**
   * \brief 计算第 c 个单元的刚度矩阵
   * 
   * \param[in] c 单元的编号
   * \param[in] params 用于计算的参数 
   * \param[out] mat 单元刚度矩阵 
   */
  void construct_cell_stiff_matrix(
      int i, 
      const std::vector<double> & params, 
      Mat<12, 12> & mat);

  mesh::TetMesh & m_mesh;
}; // end of class Tet31

} // end of namespace fem 
} // end of namespace gbmodal 
#endif // TET31_HPP_
