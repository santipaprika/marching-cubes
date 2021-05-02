// ---------------------------------------------------------------------
//     MeshViewer
// Copyright (c) 2019, The ViRVIG resesarch group, U.P.C.
// https://www.virvig.eu
//
// This file is part of MeshViewer
// MeshViewer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
// ---------------------------------------------------------------------
#include "scene.h"

#include <OpenMesh/Core/Utils/vector_cast.hh>
#include <fstream>
#include <iostream>

#include "taulaMC.hpp"
#include "utils.h"

Scene::Scene() {
    thr = 1.0f;
}

Scene::~Scene() {}

bool Scene::load(const char *name) {
    MyMesh m;
    ColorInfo ci = NONE;
    // request desired props:
    m.request_face_normals();
    m.request_face_colors();
    m.request_vertex_normals();
    m.request_vertex_colors();
    OpenMesh::IO::Options opt(OpenMesh::IO::Options::FaceColor |
                              OpenMesh::IO::Options::VertexColor |
                              OpenMesh::IO::Options::FaceNormal |
                              OpenMesh::IO::Options::VertexNormal);
    if (not OpenMesh::IO::read_mesh(m, name, opt)) {
        std::cerr << "Error loading mesh from file " << name << std::endl;
        return false;
    }
    if (opt.check(OpenMesh::IO::Options::FaceNormal)) {
        std::cout << "File " << name << " provides face normals\n";
    } else {
        std::cout << "File " << name << " MISSING face normals\n";
    }
    // check for possible color information
    if (opt.check(OpenMesh::IO::Options::VertexColor)) {
        std::cout << "File " << name << " provides vertex colors\n";
        ci = VERTEX_COLORS;
    } else {
        std::cout << "File " << name << " MISSING vertex colors\n";
    }
    if (opt.check(OpenMesh::IO::Options::FaceColor)) {
        std::cout << "File " << name << " provides face colors\n";
        ci = FACE_COLORS;
    } else {
        std::cout << "File " << name << " MISSING face colors\n";
    }
    if (not opt.check(OpenMesh::IO::Options::FaceNormal)) {
        m.update_face_normals();
    }
    if (not opt.check(OpenMesh::IO::Options::VertexNormal)) {
        m.update_vertex_normals();
    }
    _meshes.push_back(std::pair<MyMesh, ColorInfo>(m, ci));
    return true;
}

int Scene::loadVolume(const char *name) {
    int loaded_meshes = 0;

    std::ifstream volume_file(name);
    int N = 0;

    if (volume_file.is_open()) {
        _volume_names.push_back(std::string(name));
        volume_file >> N;

        float data[N][N][N];

        float max_value = -INFINITY;
        float min_value = INFINITY;

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                for (int k = 0; k < N; k++) {
                    volume_file >> data[i][j][k];

                    max_value = std::max(max_value, data[i][j][k]);
                    min_value = std::min(min_value, data[i][j][k]);
                }
            }
        }

        volume_file.close();
        float threshold = min_value + (max_value - min_value) / thr;
        float scale_factor = 1.f / (N * N);
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                for (int k = 0; k < N; k++) {
                    if (data[i][j][k] <= threshold) {
                        loaded_meshes++;
                        addOctahedron(OpenMesh::Vec3d(i / float(N), j / float(N), k / float(N)), scale_factor);
                    }
                }
            }
        }
    }

    return loaded_meshes;
}

bool Scene::computeVolumeIsosurface(const char *name) {
    int loaded_meshes = 0;

    std::ifstream volume_file(name);
    int N = 0;

    if (volume_file.is_open()) {

        _volume_names.push_back(std::string(name));
        volume_file >> N;

        float data[N][N][N];
        bool bin_data[N][N][N];

        float max_value = -INFINITY;
        float min_value = INFINITY;

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                for (int k = 0; k < N; k++) {
                    volume_file >> data[i][j][k];
                    bin_data[i][j][k] = data[i][j][k] > thr;

                    max_value = std::max(max_value, data[i][j][k]);
                    min_value = std::min(min_value, data[i][j][k]);
                }
            }
        }

        volume_file.close();

        
        MCcases cases = MCcases();

        // set of edges and vertices indices (in order according to taulaMC.hpp)
        std::vector<OpenMesh::Vec2i> edges = {{0,4},{4,5},{5,1},{1,0},{2,6},{6,7},{7,3},{3,2},{4,6},{5,7},{0,2},{1,3}}; 
        std::vector<OpenMesh::Vec3i> verts = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};

        float threshold = min_value + (max_value - min_value) / thr;
        float cell_size = 1.f/N;
        for (int i = 0; i < N - 1; i++) {
            for (int j = 0; j < N - 1; j++) {
                for (int k = 0; k < N - 1; k++) {
                    
                    int MC_config = 0;
                    std::vector<OpenMesh::Vec3d> vertices;

                    // get configuration for cube (i,j,k) -> (i+1,j+1,k+1)
                    for (int n = 0; n < 8; n++) {
                        MC_config += bin_data[i+n/4][j+(n%4)/2][k+n%2] * pow(2,n);
                    }

                    // get reconstraction for given case: set of triangles using the edges at which the vertices should go
                    std::vector<std::vector<int>> recons = cases(0);
                    for (std::vector<int> edge_idx : recons) {

                        // edges in each individual triangle
                        OpenMesh::Vec2i edge_vert[3] = {edges[edge_idx[0]],edges[edge_idx[1]],edges[edge_idx[2]]};

                        
                        // first endpoint of edge 0
                        glm::vec3 endpoint_0_indices = { i + verts[edge_vert[0][0]][0] , j + verts[edge_vert[0][0]][1] , k + verts[edge_vert[0][0]][2] };
                        glm::vec3 endpoint_1_indices = { i + verts[edge_vert[0][1]][0] , j + verts[edge_vert[0][1]][1] , k + verts[edge_vert[0][1]][2] };
                        float end_point_0 = bin_data[(int)endpoint_0_indices[0]][(int)endpoint_0_indices[1]][(int)endpoint_0_indices[2]];
                        float end_point_1 = bin_data[(int)endpoint_1_indices[0]][(int)endpoint_1_indices[1]][(int)endpoint_1_indices[2]];

                        // get vertex position using linear interpolation with the threshold value
                        float alpha = (thr - end_point_0) / (end_point_1 - end_point_0);
                        glm::vec3 vertex_position = glm::mix(endpoint_0_indices*cell_size, endpoint_1_indices*cell_size, alpha);

                    }
                }
            }
        }
    }
}

void Scene::addCube() {
    MyMesh m;
    // Add vertices
    MyMesh::VertexHandle vhandle[8];
    vhandle[0] = m.add_vertex(MyMesh::Point(-1, -1, 1));
    vhandle[1] = m.add_vertex(MyMesh::Point(1, -1, 1));
    vhandle[2] = m.add_vertex(MyMesh::Point(1, 1, 1));
    vhandle[3] = m.add_vertex(MyMesh::Point(-1, 1, 1));
    vhandle[4] = m.add_vertex(MyMesh::Point(-1, -1, -1));
    vhandle[5] = m.add_vertex(MyMesh::Point(1, -1, -1));
    vhandle[6] = m.add_vertex(MyMesh::Point(1, 1, -1));
    vhandle[7] = m.add_vertex(MyMesh::Point(-1, 1, -1));
    // Add (triangular) faces:
    std::vector<MyMesh::VertexHandle> face_vhandles;
    MyMesh::FaceHandle face;
    face_vhandles.clear();                //front
    face_vhandles.push_back(vhandle[0]);  // (-1, -1,  1)
    face_vhandles.push_back(vhandle[1]);  // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[2]);  // ( 1,  1,  1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[2]);  // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[3]);  // (-1,  1,  1)
    face_vhandles.push_back(vhandle[0]);  // (-1, -1,  1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();                // back
    face_vhandles.push_back(vhandle[7]);  // (-1,  1, -1)
    face_vhandles.push_back(vhandle[6]);  // ( 1,  1, -1)
    face_vhandles.push_back(vhandle[5]);  // ( 1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[7]);  // (-1,  1, -1)
    face_vhandles.push_back(vhandle[5]);  // ( 1, -1, -1)
    face_vhandles.push_back(vhandle[4]);  // (-1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();                // top
    face_vhandles.push_back(vhandle[3]);  // (-1,  1,  1)
    face_vhandles.push_back(vhandle[2]);  // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[6]);  // ( 1,  1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 1., 0.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[3]);  // (-1,  1,  1)
    face_vhandles.push_back(vhandle[6]);  // ( 1,  1, -1)
    face_vhandles.push_back(vhandle[7]);  // (-1,  1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 1., 0.));
    face_vhandles.clear();                // bottom
    face_vhandles.push_back(vhandle[1]);  // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[0]);  // (-1, -1,  1)
    face_vhandles.push_back(vhandle[4]);  // (-1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 1., 0.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[1]);  // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[4]);  // (-1, -1, -1)
    face_vhandles.push_back(vhandle[5]);  // ( 1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 1., 0.));
    face_vhandles.clear();                // left
    face_vhandles.push_back(vhandle[0]);  // (-1, -1,  1)
    face_vhandles.push_back(vhandle[3]);  // (-1,  1,  1)
    face_vhandles.push_back(vhandle[7]);  // (-1,  1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(1., 0., 0.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[0]);  // (-1, -1,  1)
    face_vhandles.push_back(vhandle[7]);  // (-1,  1, -1)
    face_vhandles.push_back(vhandle[4]);  // (-1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(1., 0., 0.));
    face_vhandles.clear();                // right
    face_vhandles.push_back(vhandle[2]);  // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[1]);  // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[5]);  // ( 1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(1., 0., 0.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[2]);  // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[5]);  // ( 1, -1, -1)
    face_vhandles.push_back(vhandle[6]);  // ( 1,  1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(1., 0., 0.));
    m.update_normals();
    _meshes.push_back(std::pair<MyMesh, ColorInfo>(std::move(m), FACE_COLORS));
}

void Scene::addCubeVertexcolors() {
    MyMesh m;
    // Add vertices
    MyMesh::VertexHandle vh, vhandle[8];
    vh = m.add_vertex(MyMesh::Point(-1, -1, 1));
    m.set_normal(vh, OpenMesh::vector_cast<OpenMesh::Vec3f>(m.point(vh)));
    m.set_color(vh, (m.normal(vh) + OpenMesh::Vec3f(1, 1, 1)) * .5);
    vhandle[0] = vh;
    vh = m.add_vertex(MyMesh::Point(1, -1, 1));
    m.set_normal(vh, OpenMesh::vector_cast<OpenMesh::Vec3f>(m.point(vh)));
    m.set_color(vh, (m.normal(vh) + OpenMesh::Vec3f(1, 1, 1)) * .5);
    vhandle[1] = vh;
    vh = m.add_vertex(MyMesh::Point(1, 1, 1));
    m.set_normal(vh, OpenMesh::vector_cast<OpenMesh::Vec3f>(m.point(vh)));
    m.set_color(vh, (m.normal(vh) + OpenMesh::Vec3f(1, 1, 1)) * .5);
    vhandle[2] = vh;
    vh = m.add_vertex(MyMesh::Point(-1, 1, 1));
    m.set_normal(vh, OpenMesh::vector_cast<OpenMesh::Vec3f>(m.point(vh)));
    m.set_color(vh, (m.normal(vh) + OpenMesh::Vec3f(1, 1, 1)) * .5);
    vhandle[3] = vh;
    vh = m.add_vertex(MyMesh::Point(-1, -1, -1));
    m.set_normal(vh, OpenMesh::vector_cast<OpenMesh::Vec3f>(m.point(vh)));
    m.set_color(vh, (m.normal(vh) + OpenMesh::Vec3f(1, 1, 1)) * .5);
    vhandle[4] = vh;
    vh = m.add_vertex(MyMesh::Point(1, -1, -1));
    m.set_normal(vh, OpenMesh::vector_cast<OpenMesh::Vec3f>(m.point(vh)));
    m.set_color(vh, (m.normal(vh) + OpenMesh::Vec3f(1, 1, 1)) * .5);
    vhandle[5] = vh;
    vh = m.add_vertex(MyMesh::Point(1, 1, -1));
    m.set_normal(vh, OpenMesh::vector_cast<OpenMesh::Vec3f>(m.point(vh)));
    m.set_color(vh, (m.normal(vh) + OpenMesh::Vec3f(1, 1, 1)) * .5);
    vhandle[6] = vh;
    vh = m.add_vertex(MyMesh::Point(-1, 1, -1));
    m.set_normal(vh, OpenMesh::vector_cast<OpenMesh::Vec3f>(m.point(vh)));
    m.set_color(vh, (m.normal(vh) + OpenMesh::Vec3f(1, 1, 1)) * .5);
    vhandle[7] = vh;

    // Add (triangular) faces:
    std::vector<MyMesh::VertexHandle> face_vhandles;
    MyMesh::FaceHandle face;
    face_vhandles.clear();                //front
    face_vhandles.push_back(vhandle[0]);  // (-1, -1,  1)
    face_vhandles.push_back(vhandle[1]);  // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[2]);  // ( 1,  1,  1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[2]);  // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[3]);  // (-1,  1,  1)
    face_vhandles.push_back(vhandle[0]);  // (-1, -1,  1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();                // back
    face_vhandles.push_back(vhandle[7]);  // (-1,  1, -1)
    face_vhandles.push_back(vhandle[6]);  // ( 1,  1, -1)
    face_vhandles.push_back(vhandle[5]);  // ( 1, -1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[7]);  // (-1,  1, -1)
    face_vhandles.push_back(vhandle[5]);  // ( 1, -1, -1)
    face_vhandles.push_back(vhandle[4]);  // (-1, -1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();                // top
    face_vhandles.push_back(vhandle[3]);  // (-1,  1,  1)
    face_vhandles.push_back(vhandle[2]);  // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[6]);  // ( 1,  1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[3]);  // (-1,  1,  1)
    face_vhandles.push_back(vhandle[6]);  // ( 1,  1, -1)
    face_vhandles.push_back(vhandle[7]);  // (-1,  1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();                // bottom
    face_vhandles.push_back(vhandle[1]);  // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[0]);  // (-1, -1,  1)
    face_vhandles.push_back(vhandle[4]);  // (-1, -1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[1]);  // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[4]);  // (-1, -1, -1)
    face_vhandles.push_back(vhandle[5]);  // ( 1, -1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();                // left
    face_vhandles.push_back(vhandle[0]);  // (-1, -1,  1)
    face_vhandles.push_back(vhandle[3]);  // (-1,  1,  1)
    face_vhandles.push_back(vhandle[7]);  // (-1,  1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[0]);  // (-1, -1,  1)
    face_vhandles.push_back(vhandle[7]);  // (-1,  1, -1)
    face_vhandles.push_back(vhandle[4]);  // (-1, -1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();                // right
    face_vhandles.push_back(vhandle[2]);  // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[1]);  // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[5]);  // ( 1, -1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[2]);  // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[5]);  // ( 1, -1, -1)
    face_vhandles.push_back(vhandle[6]);  // ( 1,  1, -1)
    face = m.add_face(face_vhandles);
    _meshes.push_back(std::pair<MyMesh, ColorInfo>(std::move(m), VERTEX_COLORS));
}

void Scene::addOctahedron(OpenMesh::Vec3d position, float scale) {
    MyMesh m;
    // Add vertices
    MyMesh::VertexHandle vhandle[6];
    vhandle[0] = m.add_vertex(MyMesh::Point(-1, 0, 0) * scale + position);  // left
    vhandle[1] = m.add_vertex(MyMesh::Point(0, 0, 1) * scale + position);   // front
    vhandle[2] = m.add_vertex(MyMesh::Point(1, 0, 0) * scale + position);   // right
    vhandle[3] = m.add_vertex(MyMesh::Point(0, 0, -1) * scale + position);  // back
    vhandle[4] = m.add_vertex(MyMesh::Point(0, -1, 0) * scale + position);  // bottom
    vhandle[5] = m.add_vertex(MyMesh::Point(0, 1, 0) * scale + position);   // top

    // Add (triangular) faces:
    std::vector<MyMesh::VertexHandle> face_vhandles;
    MyMesh::FaceHandle face;
    face_vhandles.clear();                // left bottom front
    face_vhandles.push_back(vhandle[0]);  // (-1, 0,  0)
    face_vhandles.push_back(vhandle[4]);  // ( 0, -1, 0)
    face_vhandles.push_back(vhandle[1]);  // ( 0, 0,  1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();                // front bottom right
    face_vhandles.push_back(vhandle[1]);  // ( 0, 0,  1)
    face_vhandles.push_back(vhandle[4]);  // ( 0, -1, 0)
    face_vhandles.push_back(vhandle[2]);  // ( 1, 0,  0)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();                // right bottom back
    face_vhandles.push_back(vhandle[2]);  // ( 1, 0,  0)
    face_vhandles.push_back(vhandle[4]);  // ( 0, -1, 0)
    face_vhandles.push_back(vhandle[3]);  // ( 0, 0, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();                // back bottom left
    face_vhandles.push_back(vhandle[3]);  // ( 0, 0, -1)
    face_vhandles.push_back(vhandle[4]);  // ( 0, -1, 0)
    face_vhandles.push_back(vhandle[0]);  // ( -1, 0, 0)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();                // left front top
    face_vhandles.push_back(vhandle[0]);  // (-1, 0,  0)
    face_vhandles.push_back(vhandle[1]);  // ( 0, 0,  1)
    face_vhandles.push_back(vhandle[5]);  // ( 0, 1,  0)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();                // front right top
    face_vhandles.push_back(vhandle[1]);  // ( 0, 0,  1)
    face_vhandles.push_back(vhandle[2]);  // ( 1, 0,  0)
    face_vhandles.push_back(vhandle[5]);  // ( 0, 1,  0)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();                // right back top
    face_vhandles.push_back(vhandle[2]);  // ( 1, 0,  0)
    face_vhandles.push_back(vhandle[3]);  // ( 0, 0, -1)
    face_vhandles.push_back(vhandle[5]);  // ( 0, 1,  0)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();                // back left top
    face_vhandles.push_back(vhandle[3]);  // ( 0, 0, -1)
    face_vhandles.push_back(vhandle[0]);  // ( -1, 0, 0)
    face_vhandles.push_back(vhandle[5]);  // ( 0,  1, 0)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));

    m.update_normals();
    _meshes.push_back(std::pair<MyMesh, ColorInfo>(std::move(m), FACE_COLORS));
}

void Scene::updateThreshold(double thr) {
    this->thr = thr;
    _meshes.clear();
}
