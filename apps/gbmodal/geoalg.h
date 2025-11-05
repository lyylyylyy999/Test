#ifndef GEOALG_H_
#define GEOALG_H_

#include <iostream>
#include <array>
#include <vector>
#include <cmath>
#include <cassert>

namespace gbmodal {

// 对于二维向量
inline std::array<double, 2> operator - (const std::array<double, 2>& v0, const std::array<double, 2>& v1)
{
    return {v0[0] - v1[0], v0[1] - v1[1]};
}

inline std::array<double, 2> operator + (const std::array<double, 2>& v0, const std::array<double, 2>& v1)
{
    return {v0[0] + v1[0], v0[1] + v1[1]};
}

inline std::array<double, 2> operator * (double a, const std::array<double, 2>& v)
{
    return {a*v[0], a*v[1]};
}

inline std::array<double, 2> operator * (const std::array<double, 2>& v, double a)
{
    return {a*v[0], a*v[1]};
}

inline std::array<double, 2> operator / (const std::array<double, 2>& v, double a)
{
    return {v[0]/a, v[1]/a};
}

inline std::array<double, 2>& operator /= (std::array<double, 2>& v, double scalar)
{
    if(scalar == 0.0)
    {
        // 你可以选择如何处理除以零的情况，例如抛出一个异常
        throw std::runtime_error("Division by zero!");
    }
    v[0] /= scalar;
    v[1] /= scalar;
    return v;
}

inline double cross(const std::array<double, 2> & v0, const std::array<double, 2> & v1)
{
  return v0[0]*v1[1] - v0[1]*v1[0];
}

inline double dot(const std::array<double, 2> & v0, const std::array<double, 2> & v1)
{
  return v0[0]*v1[0] + v0[1]*v1[1];
}

inline double squared_length(const std::array<double, 2> & v)
{
  return v[0]*v[0] + v[1]*v[1];
}

inline double length(const std::array<double, 2> & v)
{
  return std::hypot(v[0], v[1]);
}


// 对于三维向量
inline std::array<double, 3> operator - (const std::array<double, 3>& v0, const std::array<double, 3>& v1)
{
    return {v0[0] - v1[0], v0[1] - v1[1], v0[2] - v1[2]};
}

inline std::array<double, 3> operator + (const std::array<double, 3>& v0, const std::array<double, 3>& v1)
{
    return {v0[0] + v1[0], v0[1] + v1[1], v0[2] + v1[2]};
}

inline std::array<double, 3>& operator += (std::array<double, 3>& v0, const std::array<double, 3>& v1)
{
  v0[0] += v1[0];
  v0[1] += v1[1];
  v0[2] += v1[2];
  return v0;
}

inline std::array<double, 3> operator * (double a, const std::array<double, 3>& v)
{
    return {a*v[0], a*v[1], a*v[2]};
}

inline std::array<double, 3> operator * (const std::array<double, 3>& v, double a)
{
    return {a*v[0], a*v[1], a*v[2]};
}

inline std::array<double, 3> operator / (const std::array<double, 3>& v, double a)
{
    return {v[0]/a, v[1]/a, v[2]/a};
}

inline std::array<double, 3>& operator /= (std::array<double, 3>& v, double scalar) {
    if(scalar == 0.0) {
        throw std::runtime_error("Division by zero!");
    }
    v[0] /= scalar;
    v[1] /= scalar;
    v[2] /= scalar;
    return v;
}

inline std::array<double, 3> cross(const std::array<double, 3> & v0, const std::array<double, 3> & v1)
{
  return { v0[1] * v1[2] - v0[2] * v1[1], v0[2] * v1[0] - v0[0] * v1[2], v0[0] * v1[1] - v0[1] * v1[0]};
}

inline double dot(const std::array<double, 3> & v0, const std::array<double, 3> & v1)
{
  return v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2];
}

inline double squared_length(const std::array<double, 3> & v)
{
  return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}

inline double length(const std::array<double, 3> & v)
{
  return std::hypot(v[0], v[1], v[2]);
}

inline std::array<double, 3> barycenter(
    const std::array<double, 3> & p0, 
    const std::array<double, 3> & p1)
{
  std::array<double, 3> v(p0);
  v += p1;
  v /= 2.0;
  return v;
}

inline std::array<double, 3> barycenter(
    const std::array<double, 3> & p0, 
    const std::array<double, 3> & p1,
    const std::array<double, 3> & p2)
{
  std::array<double, 3> v(p0);
  v += p1;
  v += p2;
  v /= 3.0;
  return v;
}

inline std::array<double, 3> barycenter(
    const std::array<double, 3> & p0, 
    const std::array<double, 3> & p1,
    const std::array<double, 3> & p2,
    const std::array<double, 3> & p3)
{
  std::array<double, 3> v(p0); // 不能仅仅定义 std::array<double, 3> v; 第二次调用时 v 的元素不为零
  v += p1;
  v += p2;
  v += p3;
  v /= 4.0;
  return v;
}

} // end of namespace gbmodal 

#endif
