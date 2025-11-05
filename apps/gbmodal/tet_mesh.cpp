#include <map>
#include <cmath>
#include "tet_mesh.hpp"

namespace gbmodal::mesh {

std::uint8_t TetMesh::m_localface[4][3] = {
    {1, 2, 3}, {0, 3, 2}, {0, 1, 3}, {0, 2, 1}
};

std::uint8_t TetMesh::m_localedge[6][2] = {
    {0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}
};

void TetMesh::from_box( std::array<double, 6> &box, int nx, int ny, int nz) {
    double hx = (box[1] - box[0]) / nx;
    double hy = (box[3] - box[2]) / ny;
    double hz = (box[5] - box[4]) / nz;

    auto NN = (nx + 1) * (ny + 1) * (nz + 1);

    nodes_.resize(NN);
    index_t idx = 0;
    for (int i = 0; i < nx + 1; i++) {
        for (int j = 0; j < ny + 1; j++) {
            for (int k = 0; k < nz + 1; k++) {
                nodes_[idx][0] = i * hx;
                nodes_[idx][1] = j * hy;
                nodes_[idx][2] = k * hz;
                idx++;
            }
        }
    }

    idx = 0; // 计数器置 0
    std::vector<std::array<index_t, 8>> hexcell(nx * ny * nz);
    for (index_t i = 0; i < nx; i++) {
        for (index_t j = 0; j < ny; j++) {
            for (index_t k = 0; k < nz; k++) {
                hexcell[idx][0] = i * (ny + 1) * (nz + 1) + j * (nz + 1) + k;
                hexcell[idx][1] = hexcell[idx][0] + (ny + 1) * (nz + 1);
                hexcell[idx][2] = hexcell[idx][1] + nz + 1;
                hexcell[idx][3] = hexcell[idx][0] + nz + 1;

                hexcell[idx][4] = hexcell[idx][0] + 1;
                hexcell[idx][5] = hexcell[idx][4] + (ny + 1) * (nz + 1);
                hexcell[idx][6] = hexcell[idx][5] + nz + 1;
                hexcell[idx][7] = hexcell[idx][4] + nz + 1;
                idx++; }
        } 
    }

    std::vector<std::array<int, 4>> localCell;
    localCell.push_back({0, 1, 2, 6});
    localCell.push_back({0, 5, 1, 6});
    localCell.push_back({0, 4, 5, 6});
    localCell.push_back({0, 7, 4, 6});
    localCell.push_back({0, 3, 7, 6});
    localCell.push_back({0, 2, 3, 6});

    cells_.resize(6 * hexcell.size());

    idx = 0; // 计数器置 0
    for (auto &c : hexcell) {
        for (int i = 0; i < 6; i++) {
            cells_[idx][0] = c[localCell[i][0]];
            cells_[idx][1] = c[localCell[i][1]];
            cells_[idx][2] = c[localCell[i][2]];
            cells_[idx][3] = c[localCell[i][3]];
          idx++;
        }
    }

}

} // end of namespace gbmodal::mesh
