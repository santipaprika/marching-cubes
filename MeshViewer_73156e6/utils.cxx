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
#include "utils.h"
#include <string.h>
#include <cmath>
#include <values.h>
#define min(a, b) ((a)<(b))?a:b
#define max(a, b) ((a)>(b))?a:b

BoundingBox::BoundingBox(double *p) {
  memmove(_min, p, 3*sizeof(double));
  memmove(_max, p, 3*sizeof(double));
}

BoundingBox::BoundingBox() {
  _min[0] = _min[1] = _min[2] = MAXFLOAT;
  _max[0] = _max[1] = _max[2] = -MAXFLOAT;
}

void BoundingBox::add(double *p) {
  for (unsigned int i=0; i<3; ++i) {
    double curr = p[i];
    if (curr > _max[i]) _max[i] = curr;
    if (curr < _min[i]) _min[i] = curr;
  }
}

void BoundingBox::add(const BoundingBox &bb) {
  for (unsigned int i=0; i<3; ++i) {
    _min[i] = min(_min[i], bb._min[i]);
    _max[i] = max(_max[i], bb._max[i]);
  }
}

std::ostream &operator<<(std::ostream &c, const glm::mat4& m) {
  for (unsigned int i=0; i<4; ++i) {
    for (unsigned int j=0; j<4; ++j)   c << m[i][j]<< "\t";
    c << std::endl;
  }
  return c;
}

std::ostream &operator<<(std::ostream &c, const glm::mat3& m) {
  for (unsigned int i=0; i<3; ++i) {
    for (unsigned int j=0; j<3; ++j)   c << m[i][j]<< "\t";
    c << std::endl;
  }
  return c;
}
