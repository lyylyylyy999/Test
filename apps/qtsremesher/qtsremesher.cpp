#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/boost/graph/iterator.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>

#include <vector>
#include <stdexcept>
#include <limits>
#include <cmath>
#include <cstddef>

namespace py = pybind11;
using NodesArray = py::array_t<double, py::array::c_style | py::array::forcecast>;
using CellsArray = py::array_t<std::int32_t, py::array::c_style | py::array::forcecast>;

using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
using Point  = Kernel::Point_3;
using Vector = Kernel::Vector_3;
using Mesh   = CGAL::Surface_mesh<Point>;
using FT     = Kernel::FT;

using Primitive = CGAL::AABB_face_graph_triangle_primitive<Mesh>;
using Traits    = CGAL::AABB_traits<Kernel, Primitive>;
using Tree      = CGAL::AABB_tree<Traits>;

class QTSRemesher {
    public:
        QTSRemesher(NodesArray nodes, CellsArray cells) {
            if (nodes.ndim() != 2 || nodes.shape(1) != 3) {
                throw std::runtime_error("Nodes array must be of shape (NN, 3) and dtype=float64");
            }
            if (cells.ndim() != 2 || cells.shape(1) != 6) {
                throw std::runtime_error("Cells array must be of shape (NC, 6) and dtype=int32");
            }
            const size_t NN = nodes.shape(0);
            const size_t NC = cells.shape(0);
            if (NN < 6 || NC < 1)
                throw std::runtime_error("invalid sizes: need at least 6 nodes and 1 cell.");


            auto nbuf = nodes.unchecked<2>();
            std::vector<Mesh::Vertex_index> vmap;
            vmap.reserve(NN);
            for (size_t i = 0; i < NN; ++i) {
                vmap.push_back(m_mesh.add_vertex(
                            Point(nbuf(i, 0), nbuf(i, 1), nbuf(i, 2))
                            ));
            }

            // 给 face 一个 property-map 来保存“原始 cell 行号”
            m_fidx_map = m_mesh.add_property_map<Mesh::Face_index, std::ptrdiff_t>("f:cell_index", -1).first;

            std::vector<Mesh::Vertex_index> cvertex(NN); // trianlge corner vertex marker
            auto cbuf = cells.unchecked<2>();
            for (size_t i = 0; i < NC; ++i) {
                std::int32_t p0 = cbuf(i, 0);
                std::int32_t p1 = cbuf(i, 1);
                std::int32_t p2 = cbuf(i, 2);
                std::int32_t p3 = cbuf(i, 3);
                std::int32_t p4 = cbuf(i, 4);
                std::int32_t p5 = cbuf(i, 5);

                cvertex[p0] = vmap[p0];
                cvertex[p3] = vmap[p3];
                cvertex[p5] = vmap[p5];

                Mesh::Face_index f0 = m_mesh.add_face(vmap[p0], vmap[p1], vmap[p2]);
                Mesh::Face_index f1 = m_mesh.add_face(vmap[p3], vmap[p4], vmap[p1]);
                Mesh::Face_index f2 = m_mesh.add_face(vmap[p5], vmap[p2], vmap[p4]);
                Mesh::Face_index f3 = m_mesh.add_face(vmap[p4], vmap[p2], vmap[p1]);

                if (f0 == Mesh::null_face() || 
                    f1 == Mesh::null_face() ||
                    f2 == Mesh::null_face() ||
                    f3 == Mesh::null_face()) {
                    throw std::runtime_error("mesh.add_face failed at row " + std::to_string(i) +
                                             " (maybe degenerate or reversed indices).");
                }
                put(m_fidx_map, f0, (std::ptrdiff_t)i);
                put(m_fidx_map, f1, (std::ptrdiff_t)i);
                put(m_fidx_map, f2, (std::ptrdiff_t)i);
                put(m_fidx_map, f3, (std::ptrdiff_t)i);
            }

            if (!CGAL::is_triangle_mesh(m_mesh)) {
                throw std::runtime_error("The constructed mesh is not a valid triangle mesh.");
            }

            for(size_t i = 0; i < NN; ++i) {
                if (cvertex[i]) {
                    m_opt_mesh.add_vertex(m_mesh.point(vmap[i]));
                }
            }

            m_tree = std::make_unique<Tree>(faces(m_mesh).first, faces(m_mesh).second, m_mesh);
            m_tree->accelerate_distance_queries();
        }
        
    private:
        Mesh m_mesh;
        Mesh m_opt_mesh;
        std::unique_ptr<Tree> m_tree;
        Mesh::Property_map<Mesh::Face_index, std::ptrdiff_t> m_fidx_map;
};

PYBIND11_MODULE(qtsremesher, m) {
    py::class_<QTSRemesher>(m, "QTSRemesher")
        .def(py::init<NodesArray, CellsArray>(), py::arg("node"), py::arg("cell"),
                R"pbdoc( 
Parameters:
    node (numpy.ndarray): Array of shape (NN, 3) with dtype float64 representing node coordinates.
    cell (numpy.ndarray): Array of shape (NC, 6) with dtype int32 representing cell connectivity.
                )pbdoc");
}
