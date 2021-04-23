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
//
// 

#include <QApplication>
#include "glwin.h"
#include <string>
#include <iostream>

int main(int argc, char ** argv)
{
  // Used to pass command line args to the plugins
  std::string args;
  for (int i=1; i<argc; ++i) {
    args+=argv[i];
    args+=" ";
  }
  QSurfaceFormat glFormat;
  // Specify an OpenGL 3.3 format using the appropriate profile.
  glFormat.setVersion( 3, 3);  // Should not be needed... In fact,
  glFormat.setProfile(QSurfaceFormat::CoreProfile); 
  QSurfaceFormat::setDefaultFormat(glFormat);
  QApplication a(argc, argv);
  // Create OpenGL window
  glwin glWidget(args);
  glWidget.show();
    
  // Print OpenGL version and profile 
  QSurfaceFormat f = glWidget.format();
  std::cout << "OpenGL Version: " << f.majorVersion() << "." << f.minorVersion() << std::endl;
  std::cout << "OpenGL Profile: " << ((f.profile()==QSurfaceFormat::CoreProfile)?"Core":"Compatibility") << std::endl;

  a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
  return a.exec();
}
