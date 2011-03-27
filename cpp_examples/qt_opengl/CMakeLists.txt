cmake_minimum_required(VERSION 2.6)
 
PROJECT(qt_ogl_tst)
 

FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtXml QtOpenGL REQUIRED)
INCLUDE(${QT_USE_FILE})
 MESSAGE(STATUS "QT_USE_FILE = " ${QT_USE_FILE} )

find_package(GLUT REQUIRED)

if(GLUT_FOUND)
    MESSAGE(STATUS "found glut, lib = " ${GLUT_LIBRARIES} )
    MESSAGE(STATUS "found glut, include = " ${GLUT_INCLUDE_DIR} )
    include_directories(${GLUT_INCLUDE_DIR})
endif(GLUT_FOUND)

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
    MESSAGE(STATUS "found OPENGL, lib = " ${OPENGL_LIBRARIES} )
    #MESSAGE(STATUS "found gOPENGL, include = " ${GLUT_INCLUDE_DIR} )
    #include_directories(${GLUT_INCLUDE_DIR})
endif(OPENGL_FOUND)

set (MOC_HEADERS nehewidget.h)
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})


add_library(nehewidget SHARED nehewidget.cpp ${MOC_OUTFILES})
target_link_libraries(nehewidget ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES})


ADD_EXECUTABLE(qt_ogl_tst qt_ogl_tst.cpp )
target_link_libraries(qt_ogl_tst nehewidget ${QT_LIBRARIES} ${GLUT_LIBRARIES} )


