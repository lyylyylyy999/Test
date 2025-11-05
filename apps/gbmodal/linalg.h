#ifndef LINALG_H_
#define LINALG_H_

#include <iostream>
#include <iomanip>
#include <array>
#include <vector>
#include <cmath>
#include <cassert>

#include "Eigen/Dense"

/*
// 与 Lapack 的接口
extern "C" {
    // LU decomposition
    void dgetrf_(int* dim1, int* dim2, double* a, int* lda, int* ipiv, int* info);

    // inverse of a matrix given its LU decomposition
    void dgetri_(int* n, double* a, int* lda, int* ipiv, double* work, int* lwork, int* info);
}
*/

namespace gbmodal {

namespace algebra {

/**
 * \brief 长度为 n 的向量
 *
 */
template<size_t N, typename F=double>
using Vec = std::array<F, N>;

/**
 * \brief m x n 的矩阵
 *
 */
template<size_t m, size_t n>
class Mat 
{
public:
  std::array<std::array<double, n>, m> data;
  // 默认构造函数
  Mat() : data({}) {}

  // 构造函数: 接受std::array<std::array<double, n>, m>为输入
  Mat(const std::array<std::array<double, n>, m> & input) : data(input) {}

  // std::initializer_list 构造函数
  Mat(const std::initializer_list<std::array<double, n>>& list) {
      assert(list.size() <= m && "Initializer list too large for Matrix dimensions");
      std::copy(list.begin(), list.end(), data.begin());
  }

  // 外积构造函数
  Mat(const std::array<double, m>& v0, const std::array<double, n>& v1) 
  {
    for (int i = 0; i < m; ++i) {
      for (int j = 0; j < n; ++j) {
          data[i][j] = v0[i] * v1[j];
      }
    }
  }

  template<size_t s>
  void add_sub_matrix(
      const std::array<int, s> & I, 
      const std::array<int, s> & J,
      const Mat<s, s> & mat, double coef=1.0)
  {
    for(int k=0; k < s; k++)
      for(int l=0; l < s; l++) {
        auto i = I[k];
        auto j = J[l];
        data[i][j] += coef*mat[k][l];
      }
  }

  template<size_t s>
  void add_sub_matrix_from_vector_product(
      const std::array<int, s> & I, 
      const std::array<int, s> & J,
      const std::array<double, s>& v0, const std::array<double, s>& v1, 
      double coef=1.0)
  {
    for(int k=0; k < s; k++)
      for(int l=0; l < s; l++) {
        auto i = I[k];
        auto j = J[l];
        data[i][j] += coef * v0[k] * v1[l];
      }
  }

  template<size_t s>
  void add_sub_matrix_transpose(
      const std::array<int, s> & I, 
      const std::array<int, s> & J,
      const Mat<s, s> & mat, double coef=1.0)
  {
    for(int k = 0; k < s; k++)
      for(int l = 0; l < s; l++) {
        auto i = I[k];
        auto j = J[l];
        data[i][j] += coef*mat[l][k];
      }
  }


  /*!
   * @brief 把上三角部分拷贝到下三角
   */
  void copy_upper_to_lower()
  {
    for (int i = 0; i < m; ++i) {
      for (int j = i+1; j < n; ++j) {
        data[j][i] = data[i][j];
      }
    }
  }

  /*！
   * @brief diag(T', T', ..., T')*M*diag(T, T, T, ..., T) 
   */
  template<size_t k>
  void transform( Mat<k, k> & T) {
    //TODO: 增加编译时检查的代码
    Mat<k, k> TT = transpose(T);
    for (size_t i = 0; i < m; i += k) {
      for (size_t j = 0; j < n; j += k) {
         Mat<k, k> block;
         for (size_t o = 0; o < k; ++o) {
           for (size_t p = 0; p < k; ++p) {
               block[o][p] = data[i + o][j + p];
           }
         }
         Mat<k, k> result = TT * block * T;
         for (size_t o = 0; o < k; ++o) {
           for (size_t p = 0; p < k; ++p) {
               data[i+o][j+p] = result[o][p]; 
           }
         }
      }
    }
  }

  // 标量乘法
  Mat& operator*=(double scalar) 
  {
    for (int i = 0; i < m; ++i) {
      for (int j = 0; j < n; ++j) {
        data[i][j] *= scalar;
      }
    }
    return *this;
  }

  // 标量加法
  Mat& operator+=(double scalar) 
  {
    for (int i = 0; i < m; ++i) {
      for (int j = 0; j < n; ++j) {
        data[i][j] += scalar;
      }
    }
    return *this;
  }

  // 自增运算符
  Mat& operator+=(const Mat& other) 
  {
    for (int i = 0; i < m; ++i) {
      for (int j = 0; j < n; ++j) {
        data[i][j] += other.data[i][j];
      }
    }
    return *this;
  }

  // 自减运算符
  Mat& operator-=(const Mat& other) {
    for (int i = 0; i < m; ++i) {
      for (int j = 0; j < n; ++j) {
        data[i][j] -= other.data[i][j];
      }
    }
    return *this;
  }

  // 用于访问和修改矩阵的元素
  std::array<double, n>& operator[](int index) {
    return data[index];
  }

  // const 版本的访问矩阵元素的操作符
  const std::array<double, n>& operator[](int index) const {
    return data[index];
  }

  // 输出运算符重载
  friend std::ostream& operator<<(std::ostream& os, const Mat& matrix) {
    for (int i = 0; i < m; ++i) {
      for (int j = 0; j < n; ++j) {
        os << std::setw(12) << matrix.data[i][j] << ' ';
      }
      os << '\n';
    }
    return os;
  }

};

// 矩阵求逆
template<int m>
Mat<m, m> inv(const Mat<m, m>& mat)
{
  Eigen::Matrix<double, m, m> eigenMat;
  for (int i = 0; i <m; ++i) {
      for (int j = 0; j < m; ++j) {
          eigenMat(i, j) = mat[i][j];
      }
  }
  Eigen::Matrix<double, m, m> eigenInvMat = eigenMat.inverse();

  Mat<m, m> invMat;

  for (int i = 0; i < eigenInvMat.rows(); ++i) {
      for (int j = 0; j < eigenInvMat.cols(); ++j) {
          invMat[i][j] = eigenInvMat(i, j);
      }
  }
  return invMat;
}

/*
template<size_t m>
Mat<m, m> inv(const Mat<m, m> & mat)
{

  int N = m;
  double a[m*m];
  for(int i = 0; i < m; i++)
  {
    for(int j = 0; j < m; j++)
    {
      a[i*m + j] = mat[j][i];
    }
  }

  int ipiv[N];
  int lwork = N*N;
  double work[lwork];
  int info;

  dgetrf_(&N, &N, a, &N, ipiv, &info);
  if (info != 0) 
  {
    std::cout << "Error during LU decomposition!" << std::endl;
  }

  // calculate the inverse
  dgetri_(&N, a, &N, ipiv, work, &lwork, &info);
  if (info != 0) 
  {
    std::cout << "Error during matrix inversion!" << std::endl;
  }

  Matrix<m, m> imat;
  for (int i = 0; i < N; i++) 
  {
    for (int j = 0; j < N; j++) 
    {
      imat[j][i] = a[i*m + j];
    }
  }
  return imat;
}
*/


template<size_t p, size_t q, size_t r>
Mat<p, r> operator*(const Mat<p, q>& A, const Mat<q, r>& B) {
  Mat<p, r> C;
  for (size_t i = 0; i < p; ++i) {
      for (size_t j = 0; j < r; ++j) {
          for (size_t k = 0; k < q; ++k) {
              C[i][j] += A[i][k] * B[k][j];
          }
      }
  }
  return C;
}

template<size_t m, size_t n>
Mat<m, n> operator-(const Mat<m, n>& A, const Mat<m, n>& B) {
  Mat<m, n> C;
  for (size_t i = 0; i < m; ++i) {
      for (size_t j = 0; j < n; ++j) {
        C[i][j] = A[i][j] - B[i][j];
      }
  }
  return C;
}

template<size_t p, size_t q>
Mat<q, p> transpose(const Mat<p, q>& A) {
  Mat<q, p> B;
  for (size_t i = 0; i < p; ++i) {
      for (size_t j = 0; j < q; ++j) {
          B[j][i] = A[i][j];
      }
  }
  return B;
}

} // end of namespace algebra

} // end of namespace gbmodal
#endif
