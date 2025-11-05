#ifndef MESH_BASE_HPP_
#define MESH_BASE_HPP_

#include <iostream>
#include <vector>
#include <array>
#include "nlohmann/json.hpp"
#include "geoalg.h"

namespace gbmodal::mesh {

template<std::uint8_t GD, std::uint8_t NV>
class MeshBase
{
public:

#if defined(MESH_INDEX_64)
  using index_t = std::int64_t; /*< 网格索引类型*/
#else
  using index_t = std::int32_t; /*< 网格索引类型*/
#endif

using Node = std::array<double, GD>; /*< 节点类型*/ 
using Nodes = std::vector<Node>; /*< 节点容器类型*/

using Cell = std::array<index_t, NV>; /*< 单元类型*/
using Cells = std::vector<Cell>;/*< 单元容器类型*/

using Edge = std::array<index_t, 2>; /*< 边类型*/

using Vector = Node; /*< 几何向量类型, 实际和 Node 相同*/

protected:
  Nodes nodes_; /**< 节点数组*/
  Cells cells_; /**< 单元数组*/
  nlohmann::json data_; /**< json 数据结构，存储网格模型上的数据*/

public:

  /**
   * \brief 获得网格模型的点元数组
   */
  Nodes & nodes() { return nodes_;}

  /**
   * \brief 获得网格模型单元数组
   */
  Cells & cells() { return cells_;}

  Node & node(index_t i) { return nodes_[i];}

  Cell & cell(index_t i) { return cells_[i];}

  /**
   * \brief 获取网格上的 json 数据对象
   */
  nlohmann::json & data() { return data_; }

  /**
   * \brief 获取网格中点元的个数
   */
  auto number_of_nodes()
  {
    return nodes_.size();
  }

  /**
   * \brief 获取网格中单元的个数
   */
  auto number_of_cells()
  {
    return cells_.size();
  }

  auto number_of_vertices_of_cell()
  {
    return NV;
  }

  auto geo_dimension()
  {
    return GD;
  }

  /**
   * \brief 计算网格实体的测度
   */
  virtual double cell_measure(index_t c) const = 0; /*< 纯虚函数, 子类必须实现*/
  virtual double edge_measure(index_t c, int e) const = 0; 
  virtual double face_measure(index_t c, int f) const = 0;

  /**
   * \brief 设置节点集合数据
   */
  void set_node_set(
      const std::string & name,      /*< 节点集合的名字*/
      const std::vector<index_t> & nset, /*< 节点集合*/
      const bool generate)           /*< 生成式集合标记 {start, stop, step} */
  {
    data_["nset"][name]["set"] = nset;
    data_["nset"][name]["generate"] = generate;
  }

  /**
   * \brief 设置单元集合数据
   */
  void set_cell_set(
      const std::string & name,      /*< 单元集合的名字*/
      const std::vector<index_t> & cset, /*< 单元集合*/
      const bool generate)           /*< 生成式集合标记，{start, stop, step}*/
  {
    data_["cset"][name]["set"] = cset;
    data_["cset"][name]["generate"] = generate;
  }

 /**
   * \brief 设置单元集合数据
   */
  void set_face_set(
      const std::string & name,      /*< 单元集合的名字*/
      const std::vector<index_t> & fset, /*< 单元集合*/
      const bool generate)           /*< 生成式集合标记，{start, stop, step}*/
  {
    data_["fset"][name]["set"] = fset;
    data_["fset"][name]["generate"] = generate;
  }


  /*
   * \brief 设置材料集合数据
   */
  void set_material(
    const std::string & name,
    const double & density,
    const std::array<double, 2> & elastic){
    data_["material"][name]["density"] = density;      /*< 密度 kg/m^3*/
    data_["material"][name]["elastic"] = elastic;     //{4.205e+10   /*< Pa*/, 0.25 /*<泊松比*/};
  }
    
  /**
   * \brief 设置单元分区数据
   */
  void set_solid_section(
      const std::string & name,            /*< 分区的名字*/
      const std::string & cname,          /*< 对应单元集合的名字*/
      const std::string & mname,           /*< 材料的名字*/
      const std::vector<double> & geo_data /*< 对应的几何数据*/
    )
  {
    data_["section"]["solid"][name]["cset"] = cname; 
    data_["section"]["solid"][name]["material"] = mname;
    data_["section"]["solid"][name]["geo_data"]= geo_data; 
  }

  /**
   *
   * type 的取值：
   * XSYMM (U1=UR2 =UR3=0)
   * YSYMM (U2 =UR1 =UR3 =0)
   * ZSYMM (U3 =UR1 =UR2 =0)
   * XASYMM (U2 = U3 =UR1 =0; Abaqus/Standard only)
   * YASYMM (U1 = U3 =UR2 =0;Abaqus/Standard only)
   * ZASYMM (U1 =U2 =UR3=0;Abaqus/Standard only)
   * PINNED (U1 =U2 =U3=0)
   * ENCASTRE (U1=U2=U3 =UR1=UR2 =UR3 =0)
  */

  void set_disp_bc(
    const std::string & name,
    const std::string & bname,
    const std::string & type)
  {
    data_["disp_bc"][name]["nset"] = bname;
    data_["disp_bc"][name]["type"] = type;
  }

  /**
   * \bref disp_bc
   */
  void set_disp_bc(
    const std::string & name,
    const std::string & bname,
    const std::array<double, 6> & value)
  {
    data_["disp_bc"][name]["type"] = "VALUE";
    data_["disp_bc"][name]["nset"] = bname;
    data_["disp_bc"][name]["value"] = value;
  }

 /**
   * \bref disp_bc
   */
  void set_disp_bc(
    const std::string & name,
    const std::string & bname,
    const std::array<double, 3> & value)
  {
    data_["disp_bc"][name]["type"] = "VALUE";
    data_["disp_bc"][name]["nset"] = bname;
    data_["disp_bc"][name]["value"] = value;
  }

  /**
   * \brief 设置 Beam 类型的单元分区数据
   */
  void set_beam_section(
      const std::string & name,             /*< 分区的名字*/
      const std::string & cname,            /*< 对应单元集合的名字*/
      const std::string & mname,            /*< 材料的名字*/
      const std::string & section,          /*< 截面类型*/
      const std::vector<double> & geo_data, /*< 对应的几何数据*/
      const std::vector<double> & fnormal   /*< 第一法向数据*/
      )
  {
    data_["section"]["beam"][name]["cset"]= cname;
    data_["section"]["beam"][name]["material"]= mname;
    data_["section"]["beam"][name]["section"]= section;
    data_["section"]["beam"][name]["geo_data"] = geo_data; 
    data_["section"]["beam"][name]["first_normal"] = fnormal;
  }
  
  /**
   * \brief 设置节点载荷数据
   */
  void set_node_load(
      const std::string & name,     /*< 点元载荷名称*/
      const std::string & nname,    /*< 点元集合名称*/
      std::array<double, 3> & value /*< 点元载荷数值*/
      )
  {
    data_["load"]["node"][name]["nset"] = nname;
    data_["load"]["node"][name]["value"] = value;
  }

  /**
   * \brief 设置边载荷数据 
   *
   */
  void set_edge_load(
      const std::string & name,     /*< 线元载荷名称*/
      std::vector<index_t> & eset,      /*< 线元集合数据, {c0, e0, c1, e1, ......}*/
      std::array<double, 3> & value /*< 线元载荷数值*/
      )
  {
    data_["load"]["edge"][name]["eset"] = eset;
    data_["load"]["edge"][name]["value"] = value;
  }

  /**
   * \brief 设置面载荷数据（一般的载荷）
   *
   */
  void set_face_load(
      const std::string & name,     /*< 面元载荷名称*/
      std::vector<index_t> & fset,      /*< 面元集合数据, {c0, f0, c1, f1, ......}*/
      std::array<double, 3> & value /*< 面元载荷数值*/
      )
  {
    data_["load"]["face"][name]["type"] = "triaxial";
    data_["load"]["face"][name]["fset"] = fset;
    data_["load"]["face"][name]["value"] = value;
  }

  /**
   * \brief 设置面载荷数据 
   *
   * \note std::vector<int> & fset, 是否可以更换为 std::vector<std::pair<int,
   * int>> fset? 关键是更换后，是否可以放入 json 数据结构中。
   */
  void set_face_load(
      const std::string & name,     /*< 面元载荷名称*/
      std::vector<index_t> & fset,      /*< 面元集合数据, {c0, f0, c1, f1, ......}*/
      double & value                /*< 面元载荷数值*/
      )
  {
    data_["load"]["face"][name]["type"] = "normal";
    data_["load"]["face"][name]["fset"] = fset;
    data_["load"]["face"][name]["value"] = {-value}; // 除理压力的时候，用的是外法线方向，压力这里应该为负
  }
  
  /**
   * \brief 设置单元载荷数据 
   */
  void set_cell_load(
      const std::string & name,     /*< 单元载荷名称*/
      const std::string & type,     /*< 单元载荷类型*/
      const std::string & cname,    /*< 单元集合名称*/
      std::array<double, 3> & value /*< 单元载荷数值*/
      )
  {
    data_["load"]["cell"][name]["type"] = type;
    data_["load"]["cell"][name]["cset"] = cname;
    data_["load"]["cell"][name]["value"] = value;
  }

  /**
   * \brief 设置单元载荷数据 
   */
  void set_cell_load(
      const std::string & name,     /*< 单元载荷名称*/
      const std::string & type,     /*< 单元载荷类型*/
      const std::string & cname,    /*< 单元集合名称*/
      std::array<double, 3> & d,    /*< 单元载荷方向*/
      double val                    /*< 单元载荷大小*/
      )
  {
    data_["load"]["cell"][name]["type"] = type;
    data_["load"]["cell"][name]["cset"] = cname;
    data_["load"]["cell"][name]["value"] = {d[0]*val, d[1]*val, d[2]*val};
  }


  template<typename Vec>
  void apply_node_load(Vec & b, int nd=3) {
    if(!data_.contains("load"))  return;
    if(!data_["load"].contains("node")) return;

    auto it = data_["load"]["node"].begin();
    auto end = data_["load"]["node"].end();
    for(; it != end; it++) {
      std::cout << it.key() << std::endl;
      const std::string & name = (*it)["nset"];
      const std::vector<double> & val = (*it)["value"];
      assert(val.size() == 3);

      const std::vector<int> & nset = data_["nset"][name]["set"];
      bool generate = data_["nset"][name]["generate"];

      if(generate) {
        for(auto i = nset[0]; i < nset[1]; i += nset[2]) {
          b[nd*i    ] += val[0];
          b[nd*i + 1] += val[1];
          b[nd*i + 2] += val[2];
        }
      } else {
        for(const auto i: nset) {
          b[nd*i    ] += val[0];
          b[nd*i + 1] += val[1];
          b[nd*i + 2] += val[2];
        }
      }
    }
  }

  /**
   * \brief 获取位移边界条件
   */
  template<typename Vec0, typename Vec1>
  void get_disp_bc(Vec0 & disp, Vec1 & flag, unsigned nd=3) {
    assert(data_.contains("disp_bc")); //     
    nlohmann::json::iterator it = data_["disp_bc"].begin();
    nlohmann::json::iterator end = data_["disp_bc"].end();
    for(; it != end; it++)
    {
        const std::string & type = (*it)["type"]; // 获取类型
        std::cout << "type" << type << std::endl;

        const std::string & name = (*it)["nset"];
        const std::vector<index_t> & nset = data_["nset"][name]["set"];
        bool generate = data_["nset"][name]["generate"];
        if (type == "VALUE") {
            const std::vector<double> & val = (*it)["value"];
            if(generate) {
                for(auto i = nset[0]; i < nset[1]; i += nset[2])
                    for(auto j = 0u; j < val.size(); j++) {
                        disp[nd*i + j] = val[j];
                        flag[nd*i + j] = true;
                    }
            } else {
                 for(const auto i: nset) 
                    for(auto j = 0u; j < val.size(); j++) {
                        disp[nd*i + j] = val[j];
                        flag[nd*i + j] = true;
                    }
            }
        }else if (type == "XSYMM"){
          if (generate){ 
            for(auto i = nset[0]; i < nset[1]; i += nset[2]){
              disp[nd*i + 0] = 0;
              flag[nd*i + 0] = true;
              disp[nd*i + 4] = 0;
              flag[nd*i + 4] = true;
              disp[nd*i + 5] = 0;
              flag[nd*i + 5] = true;
            }
          }else {
            for(const auto i: nset) {
              disp[nd*i + 0] = 0;
              flag[nd*i + 0] = true; 
              disp[nd*i + 4] = 0;
              flag[nd*i + 4] = true;
              disp[nd*i + 5] = 0;
              flag[nd*i + 5] = true;
            }
          }
        }else if (type == "YSYMM"){
          if (generate){ 
            for(auto i = nset[0]; i < nset[1]; i += nset[2]){
              disp[nd*i + 1] = 0;
              flag[nd*i + 1] = true; 
              disp[nd*i + 3] = 0;
              flag[nd*i + 3] = true;
              disp[nd*i + 5] = 0;
              flag[nd*i + 5] = true;
            }
          }else {
            for(const auto i: nset) {
              disp[nd*i + 1] = 0;
              flag[nd*i + 1] = true;
              disp[nd*i + 3] = 0;
              flag[nd*i + 3] = true;
              disp[nd*i + 5] = 0;
              flag[nd*i + 5] = true;
            }
          }
        } else if (type == "ZSYMM"){
          if (generate){ 
            for(auto i = nset[0]; i < nset[1]; i += nset[2]){
              disp[nd*i + 2] = 0;
              flag[nd*i + 2] = true;
              disp[nd*i + 3] = 0;
              flag[nd*i + 3] = true;
              disp[nd*i + 4] = 0;
              flag[nd*i + 4] = true;
            }
          }else {
            for(const auto i: nset) {
              disp[nd*i + 2] = 0;
              flag[nd*i + 2] = true;
              disp[nd*i + 3] = 0;
              flag[nd*i + 3] = true;
              disp[nd*i + 4] = 0;
              flag[nd*i + 4] = true;
            }
          }
        }else if (type == "XASYMM"){
          if (generate){ 
            for(auto i = nset[0]; i < nset[1]; i += nset[2]){
              disp[nd*i + 1] = 0;
              flag[nd*i + 1] = true;
              disp[nd*i + 2] = 0;
              flag[nd*i + 2] = true;
              disp[nd*i + 3] = 0;
              flag[nd*i + 3] = true;
            }
          }else {
            for(const auto i: nset) {
              disp[nd*i + 1] = 0;
              flag[nd*i + 1] = true;
              disp[nd*i + 2] = 0;
              flag[nd*i + 2] = true;
              disp[nd*i + 3] = 0;
              flag[nd*i + 3] = true;
            }
          }
        }else if (type == "YASYMM") {
          if (generate){ 
            for(auto i = nset[0]; i < nset[1]; i += nset[2]){
              disp[nd*i + 0] = 0;
              flag[nd*i + 0] = true;
              disp[nd*i + 2] = 0;
              flag[nd*i + 2] = true;
              disp[nd*i + 4] = 0;
              flag[nd*i + 4] = true;
            }
          }else {
            for(const auto i: nset) {
              disp[nd*i + 0] = 0;
              flag[nd*i + 0] = true;  
              disp[nd*i + 2] = 0;
              flag[nd*i + 2] = true;
              disp[nd*i + 4] = 0;
              flag[nd*i + 4] = true;
            }
          }
        } else if (type == "ZASYMM") {
          if (generate){ 
            for(auto i = nset[0]; i < nset[1]; i += nset[2]){
              disp[nd*i + 0] = 0;
              flag[nd*i + 0] = true;
              disp[nd*i + 1] = 0;
              flag[nd*i + 1] = true;
              disp[nd*i + 5] = 0;
              flag[nd*i + 5] = true;
            }
          }else {
            for(const auto i: nset) {
              disp[nd*i + 0] = 0;
              flag[nd*i + 0] = true;
              disp[nd*i + 1] = 0;
              flag[nd*i + 1] = true;
              disp[nd*i + 5] = 0;
              flag[nd*i + 5] = true;
            }
          }
        } else if (type == "PINNED"){
          if (generate){ 
            for(auto i = nset[0]; i < nset[1]; i += nset[2]){
              disp[nd*i + 0] = 0;
              flag[nd*i + 0] = true;
              disp[nd*i + 1] = 0;
              flag[nd*i + 1] = true;
              disp[nd*i + 2] = 0;
              flag[nd*i + 2] = true;
            }
          }else {
            for(const auto i: nset) {
              disp[nd*i + 0] = 0;
              flag[nd*i + 0] = true;  
              disp[nd*i + 1] = 0;
              flag[nd*i + 1] = true;
              disp[nd*i + 2] = 0;
              flag[nd*i + 2] = true;
            }
          }
        }else if (type == "ENCASTRE"){
          if (generate){ 
            for(auto i = nset[0]; i < nset[1]; i += nset[2]){ 
              for (auto j =0; j <6; j++){
                disp[nd*i + j] = 0;
                flag[nd*i + j] = true;
              }
            }
          }else {
            for(const auto i: nset) {
              for (auto j =0; j <6; j++){
                disp[nd*i + j] = 0;
                flag[nd*i + j] = true;
              }
            }
          }
        }
      }
    }

  /**
   * \brief 获取位移边界条件
   */
  template<typename Vec>
  void get_disp_bc_node_flag(Vec & flag)
  {
    assert(data_.contains("disp_bc"));
    nlohmann::json::iterator it = data_["disp_bc"].begin();
    nlohmann::json::iterator end = data_["disp_bc"].end();
    for(; it != end; it++)
    {
      std::cout << it.key() << std::endl;
      const std::string & name = (*it)["nset"];
      const std::vector<double> & val = (*it)["value"];

      const std::vector<index_t> & nset = data_["nset"][name]["set"];
      bool generate = data_["nset"][name]["generate"];

      if(generate) {
        for(auto i = nset[0]; i < nset[1]; i += nset[2]) {
          flag[i] = true;
        }
      } else {
        for(const auto i: nset) {
          flag[i] = true;
        }
      }
    }
  }

  /**
   * \brief 单元数据转化为节点数据
   * \note 这里用简单平均，可以考虑用更复杂的平均方法
   */
  std::vector<double> cell_data_to_node_data(
      const std::vector<double> & data, 
      index_t nc)
  {
    auto NN = number_of_nodes();
    auto NC = number_of_cells();
    std::vector<double> nd(nc*NN, 0.0);
    std::vector<index_t> nn(NN, 0);

    for(auto i = 0u; i < NC; i++) {
      for(auto j = 0u; j < NV; j++) {
        auto m = cells_[i][j];
        for(auto k = 0; k < nc; k++) {
          nd[nc*m + k] += data[nc*i + k];
        }
        nn[m] += 1;
      }
    }

    for(auto i = 0u; i < NN; i++)
    {
      if(nn[i] > 0)
      {
        for(auto j = 0u; j < nc; j++)
        {
          nd[nc*i + j] /= nn[i];
        }
      }
    }
    return nd;
  }

  /**
   * \brief 打印网格信息，用于调试程序
   */
  template<std::uint8_t gd, std::uint8_t nv>
  friend std::ostream& operator<<(std::ostream& os, MeshBase<gd, nv>& mesh);

};

template<std::uint8_t GD, std::uint8_t NV>
std::ostream& operator<<(std::ostream& os, MeshBase<GD, NV>& mesh)
{
  // 打印节点数组
  os << "Nodes:\n";
  auto & nodes = mesh.nodes_;
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    os << i << ": (";
    for (size_t j = 0; j < GD; ++j)
    {
      os << nodes[i][j];
      if (j < GD - 1) os << ", ";
    }
    os << ")\n";
  }

  // 打印单元数组
  os << "\nCells:\n";
  auto & cells = mesh.cells_;
  for (size_t i = 0; i < cells.size(); ++i)
  {
    os << i << ": (";
    for (size_t j = 0; j < NV; ++j)
    {
      os << cells[i][j];
      if (j < NV - 1) os << ", ";
    }
    os << ")\n";
  }

  os << "\n" << mesh.data_ << std::endl;

  return os;
}

} // end of namespace gbmodal::mesh

#endif // MESH_HPP_
