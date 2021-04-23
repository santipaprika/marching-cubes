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
#ifndef __MeshViewer_glwin_h_
#define __MeshViewer_glwin_h_
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QKeyEvent>
#include <QString>
#include <QFileDialog>
#include <QTimer>
#include <QMenu>
#include <string>
#include <vector>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "scene.h"
#include "utils.h"


class glwin : public QOpenGLWidget, public QOpenGLFunctions_3_3_Core
{
  Q_OBJECT
  
 public:
  glwin(const std::string& args);
  void loadMesh(const char *name);
  void loadVolume(const char *name);

 private slots:
  void loadVolume();
  void loadMesh();
  void addCube();
  void addCubeVC();
  
 private:
  Scene scene;
  void setup_menu();
  void drawAxes();
  void addRotation(glm::vec3 axis, float angle);
  void updateCameraTransform();
  void updateProjectionTransform();
  void addToRender(const std::pair<MyMesh,Scene::ColorInfo> &mesh_);

  virtual void initializeGL() Q_DECL_OVERRIDE;
  virtual void paintGL( void ) Q_DECL_OVERRIDE;
  virtual void resizeGL (int width, int height) Q_DECL_OVERRIDE;

  virtual void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
  virtual void mousePressEvent( QMouseEvent *e) Q_DECL_OVERRIDE;
  virtual void mouseReleaseEvent( QMouseEvent *) Q_DECL_OVERRIDE;
  virtual void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
  virtual void wheelEvent ( QWheelEvent *e) Q_DECL_OVERRIDE;

  std::string mainArgs;
  GLuint mainShaderP, simpleShaderP;
  GLint posMVP, posMVPs, posNormalM, poscolor;
  GLuint VAOeixos;
  typedef enum {SKIP=0, USE_ARRAYS, USE_ELEMENTS} DrawMethod;
  std::vector<GLuint> VAOS;
  std::vector<GLsizei> elementsSize;
  std::vector<DrawMethod> drawMethods;
  BoundingBox bb;
  std::vector<BoundingBox> boxes;
  
  glm::mat4 modelViewMatrix;
  glm::vec3 VRP;
  float dist;
  glm::mat4 rot;
  float zoomAngle;
  glm::mat4 projectionMatrix;
  glm::mat3 NormalMatrix;

  //interaction state:
  typedef enum {NONE=0, ROTATE, ZOOM, PAN} InteractionState;
  InteractionState interactionState;
  int xprev, yprev;
  glm::vec3 eixPrev;
  QMenu *popup_menu;
};
#endif // __MeshViewer_glwin_h_
