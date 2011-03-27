cmake_minimum_required(VERSION 2.6)
 
PROJECT(qt_ogl_tst)
 

FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtXml QtOpenGL REQUIRED)
INCLUDE(${QT_USE_FILE})
 MESSAGE(STATUS "QT_USE_FILE = " ${QT_USE_FILE} )

set (MOC_HEADERS glwidget.h)
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})

add_library(glwidget SHARED glwidget.cpp ${MOC_OUTFILES})

ADD_EXECUTABLE(vbo_tst vbo_tst.cpp )
target_link_libraries(vbo_tst glwidget ${QT_LIBRARIES}  )

