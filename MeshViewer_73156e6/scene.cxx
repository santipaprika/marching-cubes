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
#include <iostream>
#include <fstream>
#include <OpenMesh/Core/Utils/vector_cast.hh>

Scene::Scene() {}

Scene::~Scene() {}

bool Scene::load(const char *name)
{
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
    if (not OpenMesh::IO::read_mesh(m, name, opt))
    {
        std::cerr << "Error loading mesh from file " << name << std::endl;
        return false;
    }
    if (opt.check(OpenMesh::IO::Options::FaceNormal))
    {
        std::cout << "File " << name << " provides face normals\n";
    }
    else
    {
        std::cout << "File " << name << " MISSING face normals\n";
    }
    // check for possible color information
    if (opt.check(OpenMesh::IO::Options::VertexColor))
    {
        std::cout << "File " << name << " provides vertex colors\n";
        ci = VERTEX_COLORS;
    }
    else
    {
        std::cout << "File " << name << " MISSING vertex colors\n";
    }
    if (opt.check(OpenMesh::IO::Options::FaceColor))
    {
        std::cout << "File " << name << " provides face colors\n";
        ci = FACE_COLORS;
    }
    else
    {
        std::cout << "File " << name << " MISSING face colors\n";
    }
    if (not opt.check(OpenMesh::IO::Options::FaceNormal))
    {
        m.update_face_normals();
    }
    if (not opt.check(OpenMesh::IO::Options::VertexNormal))
    {
        m.update_vertex_normals();
    }
    _meshes.push_back(std::pair<MyMesh, ColorInfo>(m, ci));
    return true;
}

bool Scene::loadVolume(const char *name)
{
    addCube();
    std::ifstream volume_file(name);
    if (volume_file.is_open())
    {
        int N = 0;
        volume_file >> N;

        float data[N][N][N];

        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                for (int k = 0; k < N; k++)
                {
                    volume_file >> data[i][j][k];
                }
            }
        }
        //volume_file >> data[int(i/(N*N))][int(i/N)][i%N];
    }
    volume_file.close();

    return true;
}

void Scene::addCube()
{
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
    face_vhandles.clear();               //front
    face_vhandles.push_back(vhandle[0]); // (-1, -1,  1)
    face_vhandles.push_back(vhandle[1]); // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[2]); // ( 1,  1,  1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[2]); // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[3]); // (-1,  1,  1)
    face_vhandles.push_back(vhandle[0]); // (-1, -1,  1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();               // back
    face_vhandles.push_back(vhandle[7]); // (-1,  1, -1)
    face_vhandles.push_back(vhandle[6]); // ( 1,  1, -1)
    face_vhandles.push_back(vhandle[5]); // ( 1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[7]); // (-1,  1, -1)
    face_vhandles.push_back(vhandle[5]); // ( 1, -1, -1)
    face_vhandles.push_back(vhandle[4]); // (-1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();               // top
    face_vhandles.push_back(vhandle[3]); // (-1,  1,  1)
    face_vhandles.push_back(vhandle[2]); // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[6]); // ( 1,  1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 1., 0.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[3]); // (-1,  1,  1)
    face_vhandles.push_back(vhandle[6]); // ( 1,  1, -1)
    face_vhandles.push_back(vhandle[7]); // (-1,  1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 1., 0.));
    face_vhandles.clear();               // bottom
    face_vhandles.push_back(vhandle[1]); // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[0]); // (-1, -1,  1)
    face_vhandles.push_back(vhandle[4]); // (-1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 1., 0.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[1]); // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[4]); // (-1, -1, -1)
    face_vhandles.push_back(vhandle[5]); // ( 1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 1., 0.));
    face_vhandles.clear();               // left
    face_vhandles.push_back(vhandle[0]); // (-1, -1,  1)
    face_vhandles.push_back(vhandle[3]); // (-1,  1,  1)
    face_vhandles.push_back(vhandle[7]); // (-1,  1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(1., 0., 0.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[0]); // (-1, -1,  1)
    face_vhandles.push_back(vhandle[7]); // (-1,  1, -1)
    face_vhandles.push_back(vhandle[4]); // (-1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(1., 0., 0.));
    face_vhandles.clear();               // right
    face_vhandles.push_back(vhandle[2]); // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[1]); // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[5]); // ( 1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(1., 0., 0.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[2]); // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[5]); // ( 1, -1, -1)
    face_vhandles.push_back(vhandle[6]); // ( 1,  1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(1., 0., 0.));
    m.update_normals();
    _meshes.push_back(std::pair<MyMesh, ColorInfo>(std::move(m), FACE_COLORS));
}
void Scene::addCubeVertexcolors()
{
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
    face_vhandles.clear();               //front
    face_vhandles.push_back(vhandle[0]); // (-1, -1,  1)
    face_vhandles.push_back(vhandle[1]); // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[2]); // ( 1,  1,  1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[2]); // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[3]); // (-1,  1,  1)
    face_vhandles.push_back(vhandle[0]); // (-1, -1,  1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();               // back
    face_vhandles.push_back(vhandle[7]); // (-1,  1, -1)
    face_vhandles.push_back(vhandle[6]); // ( 1,  1, -1)
    face_vhandles.push_back(vhandle[5]); // ( 1, -1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[7]); // (-1,  1, -1)
    face_vhandles.push_back(vhandle[5]); // ( 1, -1, -1)
    face_vhandles.push_back(vhandle[4]); // (-1, -1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();               // top
    face_vhandles.push_back(vhandle[3]); // (-1,  1,  1)
    face_vhandles.push_back(vhandle[2]); // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[6]); // ( 1,  1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[3]); // (-1,  1,  1)
    face_vhandles.push_back(vhandle[6]); // ( 1,  1, -1)
    face_vhandles.push_back(vhandle[7]); // (-1,  1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();               // bottom
    face_vhandles.push_back(vhandle[1]); // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[0]); // (-1, -1,  1)
    face_vhandles.push_back(vhandle[4]); // (-1, -1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[1]); // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[4]); // (-1, -1, -1)
    face_vhandles.push_back(vhandle[5]); // ( 1, -1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();               // left
    face_vhandles.push_back(vhandle[0]); // (-1, -1,  1)
    face_vhandles.push_back(vhandle[3]); // (-1,  1,  1)
    face_vhandles.push_back(vhandle[7]); // (-1,  1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[0]); // (-1, -1,  1)
    face_vhandles.push_back(vhandle[7]); // (-1,  1, -1)
    face_vhandles.push_back(vhandle[4]); // (-1, -1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();               // right
    face_vhandles.push_back(vhandle[2]); // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[1]); // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[5]); // ( 1, -1, -1)
    face = m.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[2]); // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[5]); // ( 1, -1, -1)
    face_vhandles.push_back(vhandle[6]); // ( 1,  1, -1)
    face = m.add_face(face_vhandles);
    _meshes.push_back(std::pair<MyMesh, ColorInfo>(std::move(m), VERTEX_COLORS));
}

void Scene::addOctagon() 
{
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
    face_vhandles.clear();               //front
    face_vhandles.push_back(vhandle[0]); // (-1, -1,  1)
    face_vhandles.push_back(vhandle[1]); // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[2]); // ( 1,  1,  1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[2]); // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[3]); // (-1,  1,  1)
    face_vhandles.push_back(vhandle[0]); // (-1, -1,  1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();               // back
    face_vhandles.push_back(vhandle[7]); // (-1,  1, -1)
    face_vhandles.push_back(vhandle[6]); // ( 1,  1, -1)
    face_vhandles.push_back(vhandle[5]); // ( 1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[7]); // (-1,  1, -1)
    face_vhandles.push_back(vhandle[5]); // ( 1, -1, -1)
    face_vhandles.push_back(vhandle[4]); // (-1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 0., 1.));
    face_vhandles.clear();               // top
    face_vhandles.push_back(vhandle[3]); // (-1,  1,  1)
    face_vhandles.push_back(vhandle[2]); // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[6]); // ( 1,  1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 1., 0.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[3]); // (-1,  1,  1)
    face_vhandles.push_back(vhandle[6]); // ( 1,  1, -1)
    face_vhandles.push_back(vhandle[7]); // (-1,  1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 1., 0.));
    face_vhandles.clear();               // bottom
    face_vhandles.push_back(vhandle[1]); // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[0]); // (-1, -1,  1)
    face_vhandles.push_back(vhandle[4]); // (-1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 1., 0.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[1]); // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[4]); // (-1, -1, -1)
    face_vhandles.push_back(vhandle[5]); // ( 1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(0., 1., 0.));
    face_vhandles.clear();               // left
    face_vhandles.push_back(vhandle[0]); // (-1, -1,  1)
    face_vhandles.push_back(vhandle[3]); // (-1,  1,  1)
    face_vhandles.push_back(vhandle[7]); // (-1,  1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(1., 0., 0.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[0]); // (-1, -1,  1)
    face_vhandles.push_back(vhandle[7]); // (-1,  1, -1)
    face_vhandles.push_back(vhandle[4]); // (-1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(1., 0., 0.));
    face_vhandles.clear();               // right
    face_vhandles.push_back(vhandle[2]); // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[1]); // ( 1, -1,  1)
    face_vhandles.push_back(vhandle[5]); // ( 1, -1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(1., 0., 0.));
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[2]); // ( 1,  1,  1)
    face_vhandles.push_back(vhandle[5]); // ( 1, -1, -1)
    face_vhandles.push_back(vhandle[6]); // ( 1,  1, -1)
    face = m.add_face(face_vhandles);
    m.set_color(face, MyMesh::Color(1., 0., 0.));
    m.update_normals();
    _meshes.push_back(std::pair<MyMesh, ColorInfo>(std::move(m), FACE_COLORS));    
}
