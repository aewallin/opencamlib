cmake_minimum_required(VERSION 2.6)
 
PROJECT(qt_opengl)
 
FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtXml QtOpenGL REQUIRED)
INCLUDE(${QT_USE_FILE})
 MESSAGE(STATUS "QT_USE_FILE = " ${QT_USE_FILE} )

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
    MESSAGE(STATUS "found OPENGL, lib = " ${OPENGL_LIBRARIES} )
endif(OPENGL_FOUND)

set (MOC_HEADERS nehewidget.h)
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})


add_library(nehewidget SHARED nehewidget.cpp ${MOC_OUTFILES})
target_link_libraries(nehewidget  ${OPENGL_LIBRARIES})


ADD_EXECUTABLE(qt_opengl qt_ogl_tst.cpp )
target_link_libraries(qt_opengl nehewidget ${QT_LIBRARIES} )


