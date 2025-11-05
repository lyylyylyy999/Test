#ifndef TET_MESH_HPP_
#define TET_MESH_HPP_

#include <cassert>
#include <array>
#include <vector>
#include <cmath>
#include <unordered_map>
#include "mesh_base.hpp"


namespace gbmodal {
namespace mesh {

/**
 * \brief 三维四面体网格
 */
class TetMesh : public MeshBase<3, 4>
{
private:
  static std::uint8_t m_localface[4][3]; /*< 定义在 cpp 文件中*/
  static std::uint8_t m_localedge[6][2]; 

public:
  using index_t = typename MeshBase<3,4>::index_t;
  using Base = MeshBase<3, 4>;

  using Node = typename Base::Node;
  using Edge = typename Base::Edge;
  using Face = std::array<index_t, 3>;
  using Cell = typename Base::Cell;

  using Vector = typename Base::Vector;

public:

  Edge edge(index_t i, index_t j) const {
    return {
      cells_[i][m_localedge[j][0]], 
      cells_[i][m_localedge[j][1]]
    };
  }

  Face face(index_t i, index_t j) const {
    return {
      cells_[i][m_localface[j][0]], 
      cells_[i][m_localface[j][1]], 
      cells_[i][m_localface[j][2]]
    };
  }

  Cell & cell(index_t c) { return cells_[c]; }


  /**
   * @brief 设置网格数据
   *
   */
  template <class InpFileParser>
  void from_inp_parser( InpFileParser &parser ) {
      const auto & nodes = parser.nodes();
      std::cout << "Number of nodes = " << nodes.size() << std::endl;

      nodes_.resize(nodes.size());
      std::unordered_map<index_t, index_t> nid_map;
      nid_map.reserve(nodes.size());
      for(std::size_t i=0u; i < nodes.size(); i++) {
          nid_map[nodes[i].id] = i;
          nodes_[i] = {nodes[i].x, nodes[i].y, nodes[i].z};
      }
      const auto & ebs = parser.elements();
      assert( ebs.size() == 1 );
      std::cout << "Number of cell blocks = " << ebs.size() << std::endl;
      std::cout << "Element type = " << ebs[0].type << std::endl;
      std::cout << "Number of cells = " << ebs[0].connectivity.size() << std::endl;

      cells_.resize(ebs[0].connectivity.size());
      std::unordered_map<index_t, index_t> cid_map;
      cid_map.reserve(ebs[0].connectivity.size());
      for(std::size_t i=0u; i < ebs[0].connectivity.size(); i++) {
          auto & conn = ebs[0].connectivity[i];
          cells_[i] = {nid_map[conn[0]], nid_map[conn[1]], nid_map[conn[2]], nid_map[conn[3]]};
          cid_map[ebs[0].ids[i]] = i;          
      }

      for(const auto& [key, value] : parser.nsets()) {
          std::vector<index_t> nset;
          for(const auto id : value.nodes) {
              nset.push_back( nid_map[id] );
          }
          data_["nset"][key]["set"] = nset;
          data_["nset"][key]["generate"] = false;
      }

      for(const auto& [key, value] : parser.elsets()) {
          std::vector<index_t> cset;
          for(const auto id : value.elems) {
              cset.push_back( cid_map[id] );
          }
          data_["cset"][key]["set"] = cset;
          data_["cset"][key]["generate"] = value.generate;
      }

      for(const auto& [key, value] : parser.materials()) {
          if (!value.elastic.has_value()) {
              data_["material"][key]["elastic"] = nullptr;
          } else {
              data_["material"][key]["elastic"] = {value.elastic.value().first, value.elastic.value().second};
          } 
          data_["material"][key]["elastic"] = {value.elastic.value().first, value.elastic.value().second};
      }

      // struct Surface { std::string name; std::string type; std::string set_name; double value=1.0; };
      for(const auto& [key, value] : parser.surfaces()) {
          data_["surface"][key]["set"] = value.set_name;
          data_["surface"][key]["type"] = value.type;
          data_["surface"][key]["value"] = value.value;
      }

      // struct Coupling { std::string name; std::string ref_node_set; std::string surface_name; std::string type; };
      // std::vector<Coupling> m_couplings;
      for(const auto& value : parser.couplings()) {
          const auto & key = value.name;
          data_["coupling"][key]["ref_node_set"] = value.ref_node_set;
          data_["coupling"][key]["surface"] = value.surface_name;
          data_["coupling"][key]["type"] = value.type;
      }

      //
      // struct BoundaryCondition { std::string set_name; int dof_start{}, dof_end{}; std::optional<double> value; };
      // std::vector<BoundaryCondition> m_boundaries;
      index_t i = 0;
      for(const auto & bd : parser.boundaries()) {
          auto name = "d" + std::to_string(i);
          data_["disp_bc"][name]["set"] = bd.set_name;
          data_["disp_bc"][name]["dof_start"] = bd.dof_start;
          data_["disp_bc"][name]["dof_end"] = bd.dof_end;
          data_["disp_bc"][name]["value"] = bd.value.value_or(0.0);
          i++;
      }
      std::cout << data_ << std::endl;


  }



  /*!
   * \brief 获得 BOX 四面体网格
   *
   */
  void from_box(std::array<double, 6> &box, int nx, int ny, int nz);

    
  /*!
   * \brief 获得一个四面体单元的网格
   *
   */
  void from_one_tetrahedron()
  {
      nodes_.resize(4);
      nodes_[0] = {0, 0, 0};
      nodes_[1] = {0, 0, 1};
      nodes_[2] = {1, 0, 1};
      nodes_[3] = {0, 1, 1};
      
      std::vector<index_t> nset;
      index_t i = 0;
      for(const auto & node : nodes_) {
          if (node[0] == 0.0) {
              nset.push_back(i);
          }
          i++;
      }

      cells_.resize(1);
      cells_[0] = {0, 1, 2, 3};
  }


  /**
   * \brief 计算 cell 的体积
   */
  double cell_measure(index_t c) const override
  {
    Vector v1 = nodes_[cells_[c][1]] - nodes_[cells_[c][0]];
    Vector v2 = nodes_[cells_[c][2]] - nodes_[cells_[c][0]];
    Vector v3 = nodes_[cells_[c][3]] - nodes_[cells_[c][0]];
    return dot(cross(v1, v2), v3) / 6.0;
  }

  /**
   * \brief 计算 edge 的长度
   */
  double edge_measure(index_t c, index_t i) const override
  {
    auto  e = edge(c, i);
    Node p0 = nodes_[e[0]];
    Node p1 = nodes_[e[1]];
    return length(p1 - p0);
  }

  /**
   * \brief 计算 face 的面积
   */
  double face_measure(index_t c, index_t i) const override
  {
    auto f = face(c, i);
    Vector v0 = nodes_[f[1]] - nodes_[f[0]];
    Vector v1 = nodes_[f[2]] - nodes_[f[0]];
    Vector n = cross(v0, v1);
    return length(n)/2.0;
  }

  /**
   * \brief 计算第 i 个单元重心坐标函数的导函数
   *
   */
  double grad_lambda(index_t i, std::array<Vector, 4> &gphi)
  {
    double vol = cell_measure(i);
    auto & c = cell(i);
    Vector v12 = nodes_[c[2]] - nodes_[c[1]];
    Vector v13 = nodes_[c[3]] - nodes_[c[1]]; 

    gphi[0]  = cross(v13, v12);
    gphi[0] /= 6.0*vol;

    Vector v01 = nodes_[c[1]] - nodes_[c[0]];
    Vector v02 = nodes_[c[2]] - nodes_[c[0]]; 
    Vector v03 = nodes_[c[3]] - nodes_[c[0]]; 

    gphi[1]  = cross(v02, v03);
    gphi[1] /= 6.0*vol;

    gphi[2]  = cross(v03, v01);
    gphi[2] /= 6.0*vol;

    gphi[3]  = cross(v01, v02);
    gphi[3] /= 6.0*vol;

    return vol;
  }

  Node face_barycenter(index_t i, index_t j) {
    auto f = face(i, j);
    return barycenter(nodes_[f[0]], nodes_[f[1]], nodes_[f[2]]);
  }

  Node cell_barycenter(index_t i) {
    auto & c = cell(i); 
    return barycenter(
        nodes_[c[0]], 
        nodes_[c[1]], 
        nodes_[c[2]], 
        nodes_[c[3]]);
  }

  int vtk_cell_type() { return 10;}


  template<typename Vec>
  void apply_edge_load(Vec & b) {

    if(!data_.contains("load"))  return;
    if(!data_["load"].contains("edge")) return;

    auto it = data_["load"]["edge"].begin();
    auto end = data_["load"]["edge"].end();
    for(; it != end; it++)
    {
      std::cout << it.key() << std::endl;
      const std::vector<index_t> & eset = (*it)["eset"];
      const std::vector<double> & val = (*it)["value"]; /*< 施加在一组边的力，单位长度上的受力, 长度为 3*/
      auto NE = eset.size()/2;
      for(auto k = 0u; k < NE; k++)
      {
        auto i = eset[2*k  ];
        auto j = eset[2*k+1];
        auto l = edge_measure(i, j);

        Vector force = {l*val[0]/2.0, l*val[1]/2.0, l*val[2]/2.0};

        auto e = edge(i, j);
        b[3*e[0]    ] += force[0];
        b[3*e[0] + 1] += force[1];
        b[3*e[0] + 2] += force[2];

        b[3*e[1]    ] += force[0];
        b[3*e[1] + 1] += force[1];
        b[3*e[1] + 2] += force[2];
      }
    }
  }

  template<typename Vec>
  void apply_face_load(Vec & b) {

    if(!data_.contains("load"))  return;
    if(!data_["load"].contains("face")) return;

    auto it = data_["load"]["face"].begin();
    auto end = data_["load"]["face"].end();
    for(; it != end; it++)
    {
      std::cout << it.key() << std::endl;
      std::cout << (*it)["fset"] << " " << (*it) << std::endl;
      //const std::string name = it.key(); 
      const std::vector<double> & val = (*it)["value"];
      const std::string & type = (*it)["type"];
      
      const std::vector<index_t> & fset = (*it)["fset"];
      //bool generate = data_["nset"][name]["generate"]; /*< 目前不支持生成式集合*/

    
      auto NF = fset.size()/2;
    
      for(auto k = 0u; k < NF; k++) {
        auto i = fset[2*k  ];
        auto j = fset[2*k+1];

        auto f = face(i, j);

        Vector force;
        auto v1 = nodes_[f[1]] - nodes_[f[0]];
        auto v2 = nodes_[f[2]] - nodes_[f[0]];
        auto n = cross(v1, v2); /*< 垂直于三角形面的外法线方向, 长度为面积的 2 倍*/
        if( type == "normal") {

          force = {
            n[0]*val[0]/6.0, 
            n[1]*val[0]/6.0,
            n[2]*val[0]/6.0
          };
        } else { // triaxial
          double a2 = length(n);
          force = {
            a2*val[0]/6.0, 
            a2*val[1]/6.0,
            a2*val[2]/6.0
          };
        }

        b[3*f[0]    ] += force[0];
        b[3*f[0] + 1] += force[1];
        b[3*f[0] + 2] += force[2];

        b[3*f[1]    ] += force[0];
        b[3*f[1] + 1] += force[1];
        b[3*f[1] + 2] += force[2];

        b[3*f[2]    ] += force[0];
        b[3*f[2] + 1] += force[1];
        b[3*f[2] + 2] += force[2];
      }
    }
  }

  /**
   * \brief 施加单元载荷，如重力、电磁力等
   */
  template<typename Vec>
  void apply_cell_load(Vec & b) {

    if(!data_.contains("load"))  return;
    if(!data_["load"].contains("cell")) return;

    auto apply_force = [&](const auto k, const std::vector<double>& f) {
      auto c = cell(k);
      auto vol = cell_measure(k)/4.0;
      Vector force = { f[0]*vol, f[1]*vol, f[2]*vol };

      b[3*c[0]    ] += force[0];
      b[3*c[0] + 1] += force[1];
      b[3*c[0] + 2] += force[2];

      b[3*c[1]    ] += force[0];
      b[3*c[1] + 1] += force[1];
      b[3*c[1] + 2] += force[2];

      b[3*c[2]    ] += force[0];
      b[3*c[2] + 1] += force[1];
      b[3*c[2] + 2] += force[2];

      b[3*c[3]    ] += force[0];
      b[3*c[3] + 1] += force[1];
      b[3*c[3] + 2] += force[2];
    };

    auto it = data_["load"]["cell"].begin();
    auto end = data_["load"]["cell"].end();
    for(; it != end; it++) {
      std::cout << it.key() << std::endl;
      const std::string  cname = (*it)["cset"];
      const std::vector<double>  f = (*it)["value"];
      
      const std::vector<index_t> & cset = data_["cset"][cname]["set"];
      bool generate = data_["cset"][cname]["generate"];
     
      if(generate) {
        for(auto k = cset[0]; k < cset[1]; k+=cset[2]) {
            apply_force(k, f);
        }
      } else {
        for(auto & k : cset){
            apply_force(k, f);
        }
      }
    }
  }
  
};


} // end of namespace Mesh

} // end of namespace GBMODAL 
#endif // TET_MESH_HPP_
