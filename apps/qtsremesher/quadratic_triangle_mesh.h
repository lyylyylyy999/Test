#ifndef QUADRATIC_TRIANGLE_MESH_H_
#define QUADRATIC_TRIANGLE_MESH_H_

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

namespace py = pybind11;
using NodesArray = py::array_t<double, py::array::c_style | py::array::forcecast>;
using CellsArray = py::array_t<std::int32_t, py::array::c_style | py::array::forcecast>;

class QuadraticTriangleMesh {
public:
    QuadraticTriangleMesh(const NodesArray& nodes, const CellsArray& cells)
        : nodes_(nodes), cells_(cells) {
        if (nodes_.ndim() != 2 || nodes_.shape(1) != 2) {
            throw std::runtime_error("Nodes array must be of shape (N, 2)");
        }
        if (cells_.ndim() != 2 || cells_.shape(1) != 6) {
            throw std::runtime_error("Cells array must be of shape (M, 6)");
        }
    }

private:
    NodesArray & m_nodes;
    CellsArray & m_cells;

};
#endif // QUADRATIC_TRIANGLE_MESH_H_
