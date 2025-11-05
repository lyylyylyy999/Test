#include <array>

#include <Eigen/Sparse>
#include <Eigen/Dense>

#include "openfinitex/io/inp_file_parser.hpp"
#include "openfinitex/io/vtk_mesh_writer.hpp"

#include "gbmodal_interface.hpp"
#include "tet_mesh.hpp"
#include "tet31.hpp"


struct GBModalInterface::Impl {

    // Type aliases
    using InpFileParser = openfinitex::io::InpFileParser;
    using Writer = openfinitex::io::VTKMeshWriter;

    using TetMesh = gbmodal::mesh::TetMesh;
    using Tet31 = gbmodal::fem::Tet31;

    using VectorXb = Eigen::Matrix<bool, -1, 1>;
    using CSRMatrix = Eigen::SparseMatrix<double, Eigen::RowMajor>;
    using VectorXd  = Eigen::VectorXd;

    using index_t = typename gbmodal::mesh::TetMesh::index_t;

    // The tetrahedral mesh
    TetMesh mesh_;

    void read_gearbox_modal_data(const char *fname) {
        std::string name(fname);
        InpFileParser parser;
        parser.from_file(name);
        mesh_.from_inp_parser(parser);
    }

    void write_to_vtu(const char* fname) {
        Writer writer;
        writer.set_mesh(mesh_);
        writer.write(fname);
    }

    void build_cantilever_modal_model(
            double L, double W,
            int nx, int ny, int nz){
        std::array<double, 6> box{
            0.0, L,
            0.0, W,
            0.0, W
        };
        mesh_.from_box(box, nx, ny, nz);
        //auto delta = W / L;
        //auto g = 0.4 * delta * delta;

        auto NC = static_cast<index_t>(mesh_.number_of_cells());
        mesh_.set_cell_set("c0", {0, NC, 1}, true);
        
        // 设置右端面节点集
        std::vector<index_t> nset;
        auto nn =  static_cast<index_t>((ny+1)*(nz+1));
        nset.reserve(nn);
        constexpr double eps = 1e-8;
        std::size_t i = 0;
        for(const auto & node : mesh_.nodes()) {
            const double x = node[0];
            if (x < eps) {
                nset.push_back(i);
            }
            ++i;
        }
        mesh_.set_node_set("n0", nset, false);
        double lam = 1.25;
        double mu = 1.0;
        double E = mu * (3 * lam + 2 * mu) / (lam + mu);
        double nu = lam / (2 * (lam + mu));

        mesh_.set_material("m0", 1.0, {E, nu});
        mesh_.set_solid_section("s0", "c0", "m0", {1.0, 0.0, 0.0, 0.0});
        mesh_.set_disp_bc("d0", "n0", "PINNED");
    }


};

GBModalInterface::GBModalInterface() : pimpl_(std::make_unique<Impl>()) {}
GBModalInterface::~GBModalInterface() = default;

void GBModalInterface::read_gearbox_modal_data(const char *fname) {
    pimpl_->read_gearbox_modal_data(fname);
}

void GBModalInterface::write_to_vtu(const char* fname) const {
    pimpl_->write_to_vtu(fname);
}

