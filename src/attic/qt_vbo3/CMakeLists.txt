cmake_minimum_required(VERSION 2.6)

project(qt_ogl_tst)


find_package(Qt4 COMPONENTS QtCore QtGui QtXml QtOpenGL REQUIRED)
include(${QT_USE_FILE})
message(STATUS "QT_USE_FILE = " ${QT_USE_FILE} )

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
  message(STATUS "found OPENGL, lib = " ${OPENGL_LIBRARIES} )
endif(OPENGL_FOUND)

find_package(GLUT REQUIRED)

if(GLUT_FOUND)
  message(STATUS "found glut, lib = " ${GLUT_LIBRARIES} )
  message(STATUS "found glut, include = " ${GLUT_INCLUDE_DIR} )
  include_directories(${GLUT_INCLUDE_DIR})
endif(GLUT_FOUND)



set (MOC_HEADERS glwidget.h randomsource.h)
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})

add_library(glwidget SHARED glwidget.cpp gldata.cpp ${MOC_OUTFILES})
target_link_libraries(glwidget  ${OPENGL_LIBRARIES})

add_executable(vbo_tst vbo_tst.cpp )
target_link_libraries(vbo_tst glwidget ${QT_LIBRARIES}  ${GLUT_LIBRARIES})
