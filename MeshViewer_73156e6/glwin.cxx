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
#include "glwin.h"

#include <iostream>
#define _USE_MATH_DEFINES 1
#include <cmath>
#include "checkgl.h"
#include <assert.h>
#include <QApplication>
#include <QFileDialog>
#include <QString>
#include <QAction>
#include <QCursor>

/// Constructor: receives program arguments in case one
///              wants to use them in the future...
glwin::glwin(const std::string &args)
{
    mainArgs = args;
    mainShaderP = 0;
    simpleShaderP = 0;
    VAOeixos = 0;
    rot = glm::mat4(1);
    VRP = glm::vec3(0.);
    dist = 1;
    updateCameraTransform();
    zoomAngle = 0;
    updateProjectionTransform();
    popup_menu = new QMenu("Menu", this); // Creates the app pop-up menu
    setup_menu();
}

void glwin::setup_menu()
{ // you may add here your new menu entries
    QAction *action = new QAction("Load Volume", this);
    connect(action, SIGNAL(triggered()), this, SLOT(loadVolume()));
    popup_menu->addAction(action);

    action = new QAction("Load Mesh", this);
    connect(action, SIGNAL(triggered()), this, SLOT(loadMesh()));
    popup_menu->addAction(action);

    action = new QAction("Add a Cube", this);
    connect(action, SIGNAL(triggered()), this, SLOT(addCube()));
    popup_menu->addAction(action);

    action = new QAction("Add a Cube with vertex Colors", this);
    connect(action, SIGNAL(triggered()), this, SLOT(addCubeVC()));
    popup_menu->addAction(action);

    slider = new QSlider(Qt::Horizontal, this);
    slider->setFocusPolicy(Qt::StrongFocus);
    slider->setTickPosition(QSlider::TicksBothSides);
    slider->setTickInterval(10);
    slider->setSingleStep(1);
}

//
// Code for slots triggered from the menu (or elsewhere...):
//
void glwin::loadMesh()
{
    QString file = QFileDialog::getOpenFileName(NULL, "Select a mesh to add:", "", "Meshes (*.obj *.ply *.stl *.off *.om);;All Files (*)");
    loadMesh(file.toStdString().c_str());
}

void glwin::loadMesh(const char *name)
{
    if (scene.load(name))
    {
        addToRender(scene.meshes().back());
        update();
    }
}

void glwin::loadVolume()
{
    QString file = QFileDialog::getOpenFileName(NULL, "Select a volume to add:", "", "Meshes (*.txt);;All Files (*)");
    loadVolume(file.toStdString().c_str());
}

void glwin::loadVolume(const char *name)
{
    int num_new_nodes = scene.loadVolume(name);
    if (scene.loadVolume(name) > 0)
    {
        for (int i = scene.meshes().size() - num_new_nodes; i < (int)scene.meshes().size(); i++)
        {
            addToRender(scene.meshes()[i]);
            update();
        }
    }
}

void glwin::addCube()
{
    scene.addCube();
    addToRender(scene.meshes().back());
    update();
}

void glwin::addCubeVC()
{
    scene.addCubeVertexcolors();
    addToRender(scene.meshes().back());
    update();
}

void glwin::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Escape:
        QCoreApplication::exit(0);
        break;
    case Qt::Key_A:
        loadMesh();
        break;
    case Qt::Key_C:
        addCube();
        break;
    case Qt::Key_V:
        addCubeVC();
        break;
    case Qt::Key_H:
        std::cout << "Quick Keys:" << std::endl
                  << " A:    Choose a mesh to add" << std::endl
                  << " C:    Add a cube (centered at origin, size 2)" << std::endl
                  << " V:    Add a cube with vertex colors" << std::endl
                  << " H:    Show this help" << std::endl
                  << " Esc:  Quit" << std::endl;
        break;
    default:
        e->ignore();
    }
}

void glwin::paintGL(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawAxes();
    //  next the meshes:
    glUseProgram(mainShaderP);
    glm::mat4 mvp = projectionMatrix * modelViewMatrix;
    glUniformMatrix4fv(posMVP, 1, GL_FALSE, &(mvp[0][0]));
    glUniformMatrix3fv(posNormalM, 1, GL_FALSE, &((glm::mat3(rot))[0][0]));
    std::vector<GLuint>::iterator it;
    std::vector<GLsizei>::iterator itsizes;
    std::vector<DrawMethod>::iterator itmethods;
    assert(VAOS.size() == elementsSize.size());
    assert(VAOS.size() == drawMethods.size());
    for (it = VAOS.begin(), itsizes = elementsSize.begin(), itmethods = drawMethods.begin();
         it != VAOS.end(); ++it, ++itsizes, ++itmethods)
    {
        if (*itmethods == SKIP)
            continue;
        glBindVertexArray(*it);
        if (*itmethods == USE_ELEMENTS)
            glDrawElements(GL_TRIANGLES, *itsizes, GL_UNSIGNED_INT, 0);
        else /* USE_ARRAYS */
            glDrawArrays(GL_TRIANGLES, 0, *itsizes);
    }
    glBindVertexArray(0);
}

void glwin::updateCameraTransform()
{
    modelViewMatrix = glm::translate(glm::mat4(1.), glm::vec3(0., 0., -dist)) * rot;
    modelViewMatrix = glm::translate(modelViewMatrix, -VRP);
}

void glwin::updateProjectionTransform()
{
    float radi = dist / 2.;
    projectionMatrix = glm::perspective(float(M_PI / 3.) - zoomAngle * float(M_PI) / 180.f,
                                        float(width()) / height(), radi, dist + radi);
}

const double RADIUS = 0.8;
static void projectSphere(int x, int y, glm::vec3 &p_sph, int w, int h)
{
    int mdim = std::min(w, h);
    double xd = (2. * x - mdim) / mdim;
    double yd = -(2. * y - mdim) / mdim;
    double distsq = xd * xd + yd * yd;
    double radsq = RADIUS * RADIUS;
    p_sph[0] = xd;
    p_sph[1] = yd;
    if (distsq < 0.5 * radsq)
    {
        p_sph[2] = sqrt(radsq - distsq);
    }
    else
    {
        p_sph[2] = 0.5 * radsq / sqrt(distsq);
    }
}

void glwin::mousePressEvent(QMouseEvent *e)
{
    if (e->button() & Qt::LeftButton and
        not(e->modifiers() & (Qt::ShiftModifier | Qt::AltModifier | Qt::ControlModifier)))
    {
        interactionState = glwin::ROTATE;
        xprev = e->x();
        yprev = e->y();
        projectSphere(xprev, yprev, eixPrev, width(), height());
    }
    else if (e->buttons() == Qt::RightButton and
             not(e->modifiers() & (Qt::ShiftModifier | Qt::AltModifier | Qt::ControlModifier)))
    {
        popup_menu->exec(QCursor::pos());
    }
}

void glwin::mouseMoveEvent(QMouseEvent *e)
{
    if (interactionState == glwin::ROTATE)
    {
        int xcurr = e->x();
        int ycurr = e->y();
        glm::vec3 eixCurr;
        projectSphere(xcurr, ycurr, eixCurr, width(), height());
        glm::vec3 axis = glm::cross(eixPrev, eixCurr);
        if (glm::length(axis) < 1e-7)
            axis = glm::vec3(1., 0., 0.);
        else
            axis = glm::normalize(axis);
        float t = glm::distance(eixPrev, eixCurr) / (2. * RADIUS);
        eixPrev = eixCurr;
        xprev = xcurr;
        yprev = ycurr;
        float angle = 2. * asin(glm::clamp(t, -1.f, 1.f));
        addRotation(axis, angle);
        updateCameraTransform();
    }
    update();
}

void glwin::mouseReleaseEvent(QMouseEvent *)
{
    interactionState = glwin::NONE;
}

void glwin::addRotation(glm::vec3 axis, float angle)
{
    rot = glm::rotate(glm::mat4(1.), angle, axis) * rot;
}

void glwin::wheelEvent(QWheelEvent *e)
{
    QPoint numDegrees = e->angleDelta() / 8;
    zoomAngle += numDegrees.y() / 20.;
    zoomAngle = glm::clamp(zoomAngle, -140.f, 59.f);
    updateProjectionTransform();
    update();
    e->accept();
}

void glwin::drawAxes()
{
    if (VAOeixos == 0)
        return;
    glUseProgram(simpleShaderP);
    glBindVertexArray(VAOeixos);
    glm::mat4 mvpaxes = projectionMatrix *
                        glm::translate(glm::mat4(1.), glm::vec3(0, 0, -dist)) * rot *
                        glm::scale(glm::mat4(1.), glm::vec3(dist));
    glUniformMatrix4fv(posMVPs, 1, GL_FALSE, &mvpaxes[0][0]);
    glUniform3f(poscolor, 1., 0., 0.);
    glDrawArrays(GL_LINES, 0, 2);
    glUniform3f(poscolor, 0., 1., 0.);
    glDrawArrays(GL_LINES, 2, 2);
    glUniform3f(poscolor, 0., 0., 1.);
    glDrawArrays(GL_LINES, 4, 2);
    glUseProgram(mainShaderP);
}

void glwin::initializeGL()
{
    initializeOpenGLFunctions();

    std::cout << "Initialized GL Version " << glGetString(GL_VERSION) << std::endl;
    std::cout << "      cappable of GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    const char *vs_src = "#version 330 core\n"
                         "uniform mat4 MVP;"
                         "uniform mat3 NormalM;"
                         "layout (location=0) in vec3 vertex;"
                         "layout (location=1) in vec3 normal;"
                         "layout (location=2) in vec3 color;"
                         "out vec3 vcolor;"
                         "void main() {"
                         "  gl_Position=MVP * vec4(vertex, 1.0);"
                         "  vcolor=color*normalize(NormalM*normal).z;"
                         "}";
    const char *fs_src = "#version 330 core\n"
                         "in vec3 vcolor;"
                         "out vec4 fragcolor;"
                         "void main(){"
                         "  fragcolor = vec4(vcolor, 1.);"
                         "}";
    const char *svs_src = "#version 330 core\n"
                          "uniform mat4 MVP;"
                          "uniform vec3 color;"
                          "layout (location= 0) in vec3 vertex;"
                          "out vec3 vcolor;"
                          "void main() {"
                          "  vcolor = color;" // to use the same FS...
                          "  gl_Position = MVP * vec4(vertex, 1.);"
                          "}";
    GLuint vs, svs, fs;
    GLint err;
    GLchar LOGS[1000];
    GLsizei len;
    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &err);
    if (err != GL_TRUE)
    {
        glGetShaderInfoLog(vs, sizeof(LOGS), &len, LOGS);
        std::cerr << "vs compilation errors:" << std::endl
                  << LOGS << std::endl;
    }
    svs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(svs, 1, &svs_src, NULL);
    glCompileShader(svs);
    glGetShaderiv(svs, GL_COMPILE_STATUS, &err);
    if (err != GL_TRUE)
    {
        glGetShaderInfoLog(svs, sizeof(LOGS), &len, LOGS);
        std::cerr << "svs compilation errors:" << std::endl
                  << LOGS << std::endl;
    }
    fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &err);
    if (err != GL_TRUE)
    {
        glGetShaderInfoLog(fs, sizeof(LOGS), &len, LOGS);
        std::cerr << "fs compilation errors:" << std::endl
                  << LOGS << std::endl;
    }
    mainShaderP = glCreateProgram();
    glAttachShader(mainShaderP, vs);
    glAttachShader(mainShaderP, fs);
    glLinkProgram(mainShaderP);
    glGetProgramiv(mainShaderP, GL_LINK_STATUS, &err);
    if (err != GL_TRUE)
    {
        glGetProgramInfoLog(mainShaderP, sizeof(LOGS), &len, LOGS);
        std::cerr << "Error linking main shader:" << std::endl
                  << LOGS << std::endl;
    }
    simpleShaderP = glCreateProgram();
    glAttachShader(simpleShaderP, svs);
    glAttachShader(simpleShaderP, fs);
    glLinkProgram(simpleShaderP);
    glGetProgramiv(simpleShaderP, GL_LINK_STATUS, &err);
    if (err != GL_TRUE)
    {
        glGetProgramInfoLog(simpleShaderP, sizeof(LOGS), &len, LOGS);
        std::cerr << "Error linking simple shader:" << std::endl
                  << LOGS << std::endl;
    }

    posMVP = glGetUniformLocation(mainShaderP, "MVP");
    posNormalM = glGetUniformLocation(mainShaderP, "NormalM");
    posMVPs = glGetUniformLocation(simpleShaderP, "MVP");
    poscolor = glGetUniformLocation(simpleShaderP, "color");

    // Set up drawing of the axes...
    glm::vec3 axes[] = {
        glm::vec3(0., 0., 0.),
        glm::vec3(1., 0., 0.),
        glm::vec3(0., 0., 0.),
        glm::vec3(0., 1., 0.),
        glm::vec3(0., 0., 0.),
        glm::vec3(0., 0., 1.)};
    glGenVertexArrays(1, &VAOeixos);
    glBindVertexArray(VAOeixos);
    GLuint VBOeixos;
    glGenBuffers(1, &VBOeixos);
    glBindBuffer(GL_ARRAY_BUFFER, VBOeixos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glClearColor(0.9, 0.9, 0.9, 1.0);
    glEnable(GL_DEPTH_TEST);
}

void glwin::resizeGL(int, int)
{
    updateProjectionTransform();
}

//
// The following method does all the preparation for GL rendering...
// Notice that the vectors of VAOs, sizes and draw methods must be
// "in sync" (i.e. contain the info corresponding to the same mesh
// at each valid index).
// One may mark the draw-method as SKIP to avoid drawing (momentarily)
// a given mesh.
void glwin::addToRender(const std::pair<MyMesh, Scene::ColorInfo> &mesh_)
{
    const MyMesh &m = mesh_.first;
    Scene::ColorInfo ci = mesh_.second;
    makeCurrent();
    glUseProgram(mainShaderP);
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    if ((ci == Scene::VERTEX_COLORS) or (ci == Scene::NONE))
    {
        drawMethods.push_back(USE_ELEMENTS);
        GLuint VBOS[4];
        glGenBuffers(4, VBOS);
        glBindBuffer(GL_ARRAY_BUFFER, VBOS[0]);
        glBufferData(GL_ARRAY_BUFFER, m.n_vertices() * sizeof(typename MyMesh::Point),
                     m.points(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, VBOS[1]);
        glBufferData(GL_ARRAY_BUFFER, m.n_vertices() * sizeof(typename MyMesh::Normal),
                     m.vertex_normals(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, VBOS[2]);
        GLfloat Colors[m.n_vertices() * 3];
        if (ci == Scene::VERTEX_COLORS)
        {
            const MyMesh::Color *clrs = m.vertex_colors();
            for (unsigned int i = 0; i < m.n_vertices(); ++i)
            {
                *(Colors + 3 * i + 0) = clrs[i][0];
                *(Colors + 3 * i + 1) = clrs[i][1];
                *(Colors + 3 * i + 2) = clrs[i][2];
            }
        }
        else
        { // ci==Scene::NONE
            std::cout << "Filling colors per vertex with fabs(normal).\n";
            for (unsigned int i = 0; i < m.n_vertices(); ++i)
            {
                *(Colors + 3 * i + 0) = fabs(m.vertex_normals()[i][0]);
                *(Colors + 3 * i + 1) = fabs(m.vertex_normals()[i][1]);
                *(Colors + 3 * i + 2) = fabs(m.vertex_normals()[i][2]);
            }
        }
        glBufferData(GL_ARRAY_BUFFER, m.n_vertices() * 3 * sizeof(GL_FLOAT),
                     Colors, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
        // make the face list:
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOS[3]);
        MyMesh::ConstFaceIter f_end = m.faces_end();
        MyMesh::ConstFaceVertexIter fv_it;
        std::vector<GLuint> indices;
        for (MyMesh::ConstFaceIter f_it = m.faces_begin(); f_it != f_end; ++f_it)
        {
            for (fv_it = m.cfv_iter(*f_it); fv_it.is_valid(); ++fv_it)
            {
                indices.push_back(fv_it->idx());
            }
        }
        assert(3 * m.n_faces() == indices.size());
        elementsSize.push_back(indices.size());
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.n_faces() * 3 * sizeof(GLuint),
                     &indices[0], GL_STATIC_DRAW);
    }
    else if (ci == Scene::FACE_COLORS)
    {
        drawMethods.push_back(USE_ARRAYS);
        GLuint VBOS[3];
        glGenBuffers(3, VBOS);
        const unsigned int mida = m.n_faces() * 9;
        std::vector<GLfloat> vertexBuff;
        vertexBuff.reserve(mida);
        std::vector<GLfloat> colorBuff;
        colorBuff.reserve(mida);
        std::vector<GLfloat> normalBuff;
        normalBuff.reserve(mida);
        MyMesh::ConstFaceIter f_end = m.faces_end();
        MyMesh::ConstFaceVertexIter fv_it;
        for (MyMesh::ConstFaceIter f_it = m.faces_begin(); f_it != f_end; ++f_it)
        {
            for (fv_it = m.cfv_iter(*f_it); fv_it.is_valid(); ++fv_it)
            {
                vertexBuff.push_back(static_cast<GLfloat>(m.point(*fv_it)[0]));
                vertexBuff.push_back(static_cast<GLfloat>(m.point(*fv_it)[1]));
                vertexBuff.push_back(static_cast<GLfloat>(m.point(*fv_it)[2]));

                normalBuff.push_back(m.normal(*f_it)[0]);
                normalBuff.push_back(m.normal(*f_it)[1]);
                normalBuff.push_back(m.normal(*f_it)[2]);

                colorBuff.push_back(m.color(*f_it)[0]);
                colorBuff.push_back(m.color(*f_it)[1]);
                colorBuff.push_back(m.color(*f_it)[2]);
            }
        }
        assert(mida == vertexBuff.size());
        assert(mida == normalBuff.size());
        assert(mida == colorBuff.size());

        glBindBuffer(GL_ARRAY_BUFFER, VBOS[0]);
        glBufferData(GL_ARRAY_BUFFER, mida * sizeof(GLfloat), &vertexBuff[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, VBOS[1]);
        glBufferData(GL_ARRAY_BUFFER, mida * sizeof(GLfloat), &normalBuff[0], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, VBOS[2]);
        glBufferData(GL_ARRAY_BUFFER, mida * sizeof(GLfloat), &colorBuff[0], GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
        elementsSize.push_back(mida / 3); // number of points pushed
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    VAOS.push_back(VAO);
    // we now update a bounding box and the camera to include the new mesh:
    double *p = (double *)(m.points());
    BoundingBox bbaux(p);
    for (unsigned int i = 1; i < m.n_vertices(); ++i)
        bbaux.add(p + 3 * i);
    boxes.push_back(std::move(bbaux));
    bb.add(bbaux);
    std::cerr << "Box:   (" << bbaux.min()[0] << ", " << bbaux.min()[1] << ", " << bbaux.min()[2]
              << "),  (" << bbaux.max()[0] << ", " << bbaux.max()[1] << ", " << bbaux.max()[2]
              << ")\n";
    std::cerr << "ScBox: (" << bb.min()[0] << ", " << bb.min()[1] << ", " << bb.min()[2]
              << "),  (" << bb.max()[0] << ", " << bb.max()[1] << ", " << bb.max()[2]
              << ")\n";
    VRP = glm::vec3(bb.min()[0] + bb.max()[0],
                    bb.min()[1] + bb.max()[1],
                    bb.min()[2] + bb.max()[2]) *
          .5f;
    dist = sqrt(pow(-bb.min()[0] + bb.max()[0], 2) +
                pow(-bb.min()[1] + bb.max()[1], 2) +
                pow(-bb.min()[2] + bb.max()[2], 2));
    std::cerr << "dist = " << dist << std::endl
              << "VRP  = (" << VRP[0] << ", " << VRP[1] << ", " << VRP[2] << ")" << std::endl;
    updateCameraTransform();
    updateProjectionTransform();
    update();
}
