project(ocl_cutsim1_tst)

MESSAGE(STATUS " project_name = " ${PROJECT_NAME} )

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
MESSAGE(STATUS "OCL_LIBRARY is now: " ${OCL_LIBRARY})

find_library(OCL_CUTSIM_LIBRARY 
            NAMES cutsim
            PATHS /usr/local/lib/opencamlib /usr/lib/opencamlib
            DOC "The opencamlib CUTSIM library"
)
MESSAGE(STATUS "OCL_CUTSIM_LIBRARY is now: " ${OCL_CUTSIM_LIBRARY})

set (MOC_HEADERS cutsim.h)
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})

set( OCL_TST_SRC ${${PROJECT_NAME}_SOURCE_DIR}/main.cpp )

#set(OCL_TST_SRC
#    ${OCL_CUTSIM1_EXAMPLE_SOURCE_DIR}/main.cpp
#)

#add_library(glwidget SHARED glwidget.cpp gldata.cpp ${MOC_OUTFILES})
#target_link_libraries(glwidget  ${OPENGL_LIBRARIES})

add_executable( ${PROJECT_NAME} ${OCL_TST_SRC} ${MOC_OUTFILES} )
# link with ocl, qt, boost, opengl
target_link_libraries(${PROJECT_NAME}  ${OCL_LIBRARY} ${OCL_CUTSIM_LIBRARY} ${QT_LIBRARIES} ${Boost_LIBRARIES} ${OPENGL_LIBRARIES})

install( TARGETS ${PROJECT_NAME} DESTINATION bin )

#add_executable(
#    cutsim1
#    ${OCL_TST_SRC}
#)
#target_link_libraries(cutsim1 glwidget ${OCL_LIBRARY} ${QT_LIBRARIES} ${Boost_LIBRARIES} ${OPENGL_LIBRARIES})


