#pragma once
#ifndef GBMODAL_INTERFACE_HPP_
#define GBMODAL_INTERFACE_HPP_

#include <memory>

class GBModalInterface {
public:
    GBModalInterface();
    ~GBModalInterface();
    void read_gearbox_modal_data(const char* filename);
    void write_to_vtu(const char *filename) const;
    void build_cantilever_modal_model(
            double L, double W,
            int nx, int ny, int nz);
    void compute_stiffness_matrix();
    void compute_mass_matrix();
private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

#endif // GBMODAL_INTERFACE_HPP_
