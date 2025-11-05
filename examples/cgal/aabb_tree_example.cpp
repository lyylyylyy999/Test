#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <CGAL/Polygon_mesh_processing/locate.h>
#include <CGAL/squared_distance_3.h>
#include <iostream>
#include <vector>

using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
using Point_3 = Kernel::Point_3;
using Mesh = CGAL::Surface_mesh<Point_3>;
using AABB_tree = CGAL::AABB_tree<
    CGAL::AABB_traits<Kernel,
        CGAL::AABB_face_graph_triangle_primitive<Mesh>>>;
using Face_location = CGAL::Polygon_mesh_processing::Face_location<Mesh, Kernel::FT>;

int main() {
    // 创建一个简单的曲面三角形网格（四面体）
    Mesh mesh;
    auto v0 = mesh.add_vertex(Point_3(0, 0, 0));
    auto v1 = mesh.add_vertex(Point_3(1, 0, 0));
    auto v2 = mesh.add_vertex(Point_3(0, 1, 0));
    auto v3 = mesh.add_vertex(Point_3(0, 0, 1));

    mesh.add_face(v0, v1, v2);
    mesh.add_face(v0, v1, v3);
    mesh.add_face(v0, v2, v3);
    mesh.add_face(v1, v2, v3);

    // 构建 AABB_tree
    AABB_tree tree;
    CGAL::Polygon_mesh_processing::build_AABB_tree(mesh, tree);

    // 定义一组查询点
    std::vector<Point_3> queries = {
        Point_3(0.5, 0.5, 0.5),
        Point_3(2.0, 2.0, 2.0),
        Point_3(0.1, 0.1, 0.1)
    };

    // 对于每个查询点，查找最近的三角形
    for (const auto& query : queries) {
        Face_location loc = CGAL::Polygon_mesh_processing::locate_with_AABB_tree(query, tree, mesh);
        auto face = loc.first;
        auto bary = loc.second;

        // 计算最近点
        Point_3 closest = CGAL::Polygon_mesh_processing::construct_point(loc, mesh);

        // 计算距离
        Kernel::FT dist = CGAL::sqrt(CGAL::squared_distance(query, closest));

        std::cout << "查询点: " << query << std::endl;
        std::cout << "最近三角形 (face index): " << size_t(face) << std::endl;
        std::cout << "重心坐标: " << bary[0] << ", " << bary[1] << ", " << bary[2] << std::endl;
        std::cout << "最近点: " << closest << std::endl;
        std::cout << "距离: " << dist << std::endl;
        std::cout << "------------------------" << std::endl;

        // 要获取三角形的顶点：
        auto he = mesh.halfedge(face);
        auto v_a = mesh.target(he);
        auto v_b = mesh.target(mesh.next(he));
        auto v_c = mesh.target(mesh.next(mesh.next(he)));
        std::cout << "三角形顶点: " << mesh.point(v_a) << ", " << mesh.point(v_b) << ", " << mesh.point(v_c) << std::endl;
    }

    return 0;
}
