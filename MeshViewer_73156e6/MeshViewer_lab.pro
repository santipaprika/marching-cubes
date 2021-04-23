TEMPLATE = app
QT += opengl
QT += widgets
CONFIG += debug
CONFIG += warn_on
QMAKE_CXX = g++-5
QMAKE_CC = gcc-5
QMAKE_CXXFLAGS += -std=c++11 -D__USE_XOPEN

# Inputs:
INCLUDEPATH += .
INCLUDEPATH += /assig/sgi/include

HEADERS += ./*.h
SOURCES += ./*.cxx

LIBS += -lGLU
LIBS += -L/assig/sgi/lib -Wl,-rpath,/assig/sgi/lib/ -lOpenMeshCore -lOpenMeshTools

# Outputs:
TARGET = MeshViewer
MOC_DIR = build
OBJECTS_DIR = build
RCC_DIR = build

TARGET = MeshViewer
