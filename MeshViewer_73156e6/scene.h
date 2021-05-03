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
#ifndef __MeshViewer_scene_h_
#define __MeshViewer_scene_h_
#include <vector>
#include <utility>
#include <unordered_map>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include "utils.h"
#include "taulaMC.hpp"

#define OUT

// define traits
struct MyTraits : public OpenMesh::DefaultTraits
{
  // use double valued coordinates
  typedef OpenMesh::Vec3d Point;
  typedef OpenMesh::Vec3f Color;
  // use vertex normals and vertex colors
  VertexAttributes( OpenMesh::Attributes::Normal |
                    OpenMesh::Attributes::Color );
  // store the previous halfedge
  HalfedgeAttributes( OpenMesh::Attributes::PrevHalfedge );
  // use face normals
  FaceAttributes( OpenMesh::Attributes::Normal |
		  OpenMesh::Attributes::Color );
  // store a face handle for each vertex
};
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>  MyMesh;

class Scene {
 public:
  Scene();
  ~Scene();
  bool load(const char* name);
  int loadVolume(const char* name);
  bool computeVolumeIsosurface(const char* name);
  void addCube();
  void addCubeVertexcolors();
  void addOctahedron(OpenMesh::Vec3d position, float scale);

  void setIsovalue(double val);

  typedef enum {NONE=0, VERTEX_COLORS, FACE_COLORS} ColorInfo;
  const std::vector<std::pair<MyMesh,ColorInfo> >& meshes() {return _meshes;}
  const std::vector<std::string>& volume_names() {return _volume_names;}
  float min_value() {return _min_value;}
  float max_value() {return _max_value;}

 private:
  std::vector<std::pair<MyMesh,ColorInfo> > _meshes;
  std::vector<std::string> _volume_names;
  float* data;
  float _min_value, _max_value, cell_size, isovalue, thr;
  MCcases cases;

  // set of edges and vertices indices (in order according to taulaMC.hpp)
  std::vector<OpenMesh::Vec2i> edges = {{0, 4}, {4, 5}, {5, 1}, {1, 0}, {2, 6}, {6, 7}, {7, 3}, {3, 2}, {4, 6}, {5, 7}, {0, 2}, {1, 3}};
  std::vector<OpenMesh::Vec3i> verts = {{0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1}, {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1}};

  void initializeData(std::ifstream &volume_file, int N);
  bool parseVolume(const char* name, std::ifstream &volume_file, int &N);
  void reconstructVoxel(int &MC_config, int &N, MyMesh &m, std::unordered_map<std::pair<int, int>, MyMesh::VertexHandle, hash_pair> &edge_to_vtx_dict, int &i, int &j, int &k);
};
#endif // __MeshViewer_scene_h_
