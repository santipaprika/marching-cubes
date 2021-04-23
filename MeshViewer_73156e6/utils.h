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
#ifndef __UTILS_H__
#define __UTILS_H__
#include <iostream>
#include <glm/glm.hpp>

class BoundingBox {
 public:
  BoundingBox(double *);
  BoundingBox();
  void add(double *);
  void add(const BoundingBox &);
  const double * min() const {return _min;}
  const double * max() const {return _max;}

 private:
  double _min[3], _max[3];
};

std::ostream &operator<<(std::ostream &c, const glm::mat4& m);
std::ostream &operator<<(std::ostream &c, const glm::mat3& m);
#endif // __UTILS_H__
