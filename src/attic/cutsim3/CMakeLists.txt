project(ocl_cutsim3_example)

cmake_minimum_required(VERSION 2.4)

if (CMAKE_BUILD_TOOL MATCHES "make")
    add_definitions(-Wall  -Wno-deprecated )
endif (CMAKE_BUILD_TOOL MATCHES "make")
# -Werror
# -pedantic-errors

FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtXml QtOpenGL REQUIRED)
INCLUDE(${QT_USE_FILE})
 MESSAGE(STATUS "QT_USE_FILE = " ${QT_USE_FILE} )

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
    MESSAGE(STATUS "found OPENGL, lib = " ${OPENGL_LIBRARIES} )
endif(OPENGL_FOUND)


# find BOOST and boost-python
find_package( Boost )
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    MESSAGE(STATUS "found Boost: " ${Boost_LIB_VERSION})
    MESSAGE(STATUS "boost-incude dirs are: " ${Boost_INCLUDE_DIRS})
endif()



find_library(OCL_LIBRARY 
            NAMES ocl
            PATHS /usr/local/lib/opencamlib /usr/lib/opencamlib
            DOC "The opencamlib library"
)

find_library(CUTSIM_LIBRARY 
            NAMES cutsim
            PATHS /usr/local/lib/opencamlib /usr/lib/opencamlib
            DOC "The ocl cutsim library"
)

MESSAGE(STATUS "OCL_LIBRARY is now: " ${OCL_LIBRARY})
MESSAGE(STATUS "CUTSIM_LIBRARY is now: " ${CUTSIM_LIBRARY})

set (MOC_HEADERS cutsim.h )
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})


set( OCL_TST_SRC ${${PROJECT_NAME}_SOURCE_DIR}/main.cpp ${MOC_OUTFILES} )

add_executable( ${PROJECT_NAME} ${OCL_TST_SRC} )
target_link_libraries( ${PROJECT_NAME} ${OCL_LIBRARY} ${CUTSIM_LIBRARY} ${QT_LIBRARIES} ${Boost_LIBRARIES} ${OPENGL_LIBRARIES})


install( TARGETS ${PROJECT_NAME} DESTINATION bin )
