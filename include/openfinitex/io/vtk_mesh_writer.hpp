#ifndef VTK_MESH_WRITER_HPP_
#define VTK_MESH_WRITER_HPP_

#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>

#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include <vtkHexagonalPrism.h>
#include <vtkHexahedron.h>
#include <vtkLine.h>
#include <vtkPentagonalPrism.h>
#include <vtkPixel.h>
#include <vtkPolyLine.h>
#include <vtkPolyVertex.h>
#include <vtkPolygon.h>
#include <vtkPyramid.h>
#include <vtkQuad.h>
#include <vtkTetra.h>
#include <vtkTriangle.h>
#include <vtkTriangleStrip.h>
#include <vtkVertex.h>
#include <vtkVoxel.h>
#include <vtkWedge.h>

#include <string>
#include <memory>

namespace openfinitex {
namespace io {

class VTKMeshWriter {
public:


public:
    VTKMeshWriter() {
        m_ugrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
        m_writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
    }

    template<typename Nodes, typename Cells>
    void set_mesh(Nodes & nodes, Cells & cells, unsigned vctype) {
      auto NN = nodes.size();
      auto points = vtkSmartPointer<vtkPoints>::New();
      points->Allocate(NN);
      auto GD = nodes[0].size(); // 几何维数

      if(GD == 3) {
        for(auto it=nodes.begin(); it != nodes.end(); it++) {
          points->InsertNextPoint((*it)[0], (*it)[1], (*it)[2]); // (*it) 括号是必须的
        }
      }
      else if(GD == 2) {
          for(auto it=nodes.begin(); it != nodes.end(); it++) {
            points->InsertNextPoint((*it)[0], (*it)[1], 0.0);
          }
      }
      m_ugrid->SetPoints(points);

      auto NC = cells.size();
      auto NV = cells[0].size();

      auto vtkcells = vtkSmartPointer<vtkCellArray>::New();
      vtkcells->AllocateExact(NC, NC*NV); 
      for(auto it = cells.begin(); it != cells.end(); ) {
        vtkcells->InsertNextCell(NV);
        for(unsigned i = 0; i < NV; i++) {
            vtkcells->InsertCellPoint(*it);
            it++;
        }
      }
      m_ugrid->SetCells(vctype, vtkcells);
    }

    template<class Mesh>
    void set_mesh(Mesh & mesh) {
      m_ugrid->Initialize(); //把网格清空
      set_nodes(mesh);
      set_cells(mesh);
    }

    template<class Mesh>
    void set_nodes(Mesh & mesh) {
      int NN = mesh.number_of_nodes();
      auto nodes = mesh.nodes();
      auto points = vtkSmartPointer<vtkPoints>::New();
      points->Allocate(NN);
      auto GD = mesh.geo_dimension();

      if(GD == 3) {
        for(auto it=nodes.begin(); it != nodes.end(); it++) {
          points->InsertNextPoint((*it)[0], (*it)[1], (*it)[2]);
        }
      }
      else if(GD == 2) {
          for(auto it=nodes.begin(); it != nodes.end(); it++) {
            points->InsertNextPoint((*it)[0], (*it)[1], 0.0);
          }
      }
      m_ugrid->SetPoints(points);
    }


    template<class Mesh>
    void set_cells(Mesh & mesh) {
      auto NC = mesh.number_of_cells();
      auto NV = mesh.number_of_vertices_of_cell();

      auto vcells = vtkSmartPointer<vtkCellArray>::New();
      vcells->AllocateExact(NC, NC*NV); 

      for(auto & cell : mesh.cells()) {
          vcells->InsertNextCell(NV);
          for(auto i = 0; i < NV; i++) {
              vcells->InsertCellPoint(cell[i]);
          }
      }
      m_ugrid->SetCells(mesh.vtk_cell_type(), vcells);
    }

    void set_node_data(std::vector<int> & data, int ncomponents, 
        const std::string name) {
        int n = data.size()/ncomponents;
        vtkNew<vtkIntArray> vtkdata;
        vtkdata->SetNumberOfComponents(ncomponents);
        vtkdata->SetNumberOfTuples(n);
        vtkdata->SetName(name.c_str());
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < ncomponents; j ++)
                vtkdata->SetComponent(i, j, data[i*ncomponents + j]);
        }
        m_ugrid->GetPointData()->AddArray(vtkdata);
    }

    void set_node_data(std::vector<bool> & data, int ncomponents, const std::string name)
    {
        int n = data.size()/ncomponents;
        vtkNew<vtkIntArray> vtkdata;
        vtkdata->SetNumberOfComponents(ncomponents);
        vtkdata->SetNumberOfTuples(n);
        vtkdata->SetName(name.c_str());
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < ncomponents; j ++) {
              int val = data[i*ncomponents + j]?1:0;
              vtkdata->SetComponent(i, j, val);
            }
        }
        m_ugrid->GetPointData()->AddArray(vtkdata);
    }

    void set_cell_data(std::vector<bool> & data, int ncomponents, const std::string name)
    {
        int n = data.size()/ncomponents;
        vtkNew<vtkIntArray> vtkdata;
        vtkdata->SetNumberOfComponents(ncomponents);
        vtkdata->SetNumberOfTuples(n);
        vtkdata->SetName(name.c_str());
        for(int i = 0u; i < n; i++) {
            for(int j = 0; j < ncomponents; j ++) {
              int val = data[i*ncomponents + j]?1:0;
              vtkdata->SetComponent(i, j, val);
            }
        }
        m_ugrid->GetCellData()->AddArray(vtkdata);
    }

    void set_cell_data(std::vector<int> & data, int ncomponents, const std::string name)
    {
        int n = data.size()/ncomponents;
        vtkNew<vtkIntArray> vtkdata;
        vtkdata->SetNumberOfComponents(ncomponents);
        vtkdata->SetNumberOfTuples(n);
        vtkdata->SetName(name.c_str());
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < ncomponents; j ++)
                vtkdata->SetComponent(i, j, data[i*ncomponents + j]);
        }
        m_ugrid->GetCellData()->AddArray(vtkdata);
    }

    void set_node_data(std::vector<double> & data, int ncomponents, const std::string name)
    {
        int n = data.size()/ncomponents;
        vtkNew<vtkDoubleArray> vtkdata;
        vtkdata->SetNumberOfComponents(ncomponents);
        vtkdata->SetNumberOfTuples(n);
        vtkdata->SetName(name.c_str());
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < ncomponents; j ++)
                vtkdata->SetComponent(i, j, data[i*ncomponents + j]);
        }
        m_ugrid->GetPointData()->AddArray(vtkdata);
    }

    void set_cell_data(std::vector<double> & data, int ncomponents, const std::string name)
    {
        int n = data.size()/ncomponents;
        vtkNew<vtkDoubleArray> vtkdata;
        vtkdata->SetName(name.c_str());
        vtkdata->SetNumberOfComponents(ncomponents);
        vtkdata->SetNumberOfTuples(n);
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < ncomponents; j ++)
                vtkdata->SetComponent(i, j, data[i*ncomponents + j]);
        }
        m_ugrid->GetCellData()->AddArray(vtkdata);
    }


    void write(const std::string & fname)
    {
        m_writer->SetFileName(fname.c_str());
        m_writer->SetInputData(m_ugrid);
        m_writer->Write();
    }

    void write(const char* fname)
    {
        m_writer->SetFileName(fname);
        m_writer->SetInputData(m_ugrid);
        m_writer->Write();
    }
private:
  vtkSmartPointer<vtkUnstructuredGrid> m_ugrid;
  vtkSmartPointer<vtkXMLUnstructuredGridWriter> m_writer;
};

} // end of namespace io 

} // end of namespace openfinitex

#endif // VTK_MESH_WRITER_HPP_

