cmake_minimum_required(VERSION 2.6)
 
PROJECT(qt_ogl_tst)
 

FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtXml QtOpenGL REQUIRED)
INCLUDE(${QT_USE_FILE})
 MESSAGE(STATUS "QT_USE_FILE = " ${QT_USE_FILE} )
# ADD_DEFINITIONS( ${QT_DEFINITIONS} )

set(QT_USE_OPENGL TRUE)

set (MOC_HEADERS nehewidget.h)
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})


# INCLUDE( nehewidget.h )
add_library(nehewidget SHARED nehewidget.cpp ${MOC_OUTFILES})
target_link_libraries(nehewidget ${QT_LIBRARIES}  )
ADD_EXECUTABLE(qt_ogl_tst qt_ogl_tst.cpp )
target_link_libraries(qt_ogl_tst nehewidget ${QT_LIBRARIES}  )

