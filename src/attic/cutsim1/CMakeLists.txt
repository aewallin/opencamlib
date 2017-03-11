project(ocl_cutsim1_example)

message(STATUS " project_name = " ${PROJECT_NAME} )

cmake_minimum_required(VERSION 2.4)

if (UNIX)
    add_definitions(-Wall  -Wno-deprecated )
endif (UNIX)
# -Werror
# -pedantic-errors

find_package(Qt4 COMPONENTS QtCore QtGui QtXml QtOpenGL REQUIRED)
include(${QT_USE_FILE})
 message(STATUS "QT_USE_FILE = " ${QT_USE_FILE} )

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
    message(STATUS "found OPENGL, lib = " ${OPENGL_LIBRARIES} )
endif(OPENGL_FOUND)

# find BOOST 
find_package( Boost )
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    message(STATUS "found Boost: " ${Boost_LIB_VERSION})
    message(STATUS "boost-incude dirs are: " ${Boost_INCLUDE_DIRS})
endif()

find_library(OCL_LIBRARY 
            NAMES ocl
            PATHS /usr/local/lib/opencamlib /usr/lib/opencamlib
            DOC "The opencamlib library"
)
message(STATUS "OCL_LIBRARY is now: " ${OCL_LIBRARY})

find_library(OCL_CUTSIM_LIBRARY 
            NAMES cutsim
            PATHS /usr/local/lib/opencamlib /usr/lib/opencamlib
            DOC "The opencamlib CUTSIM library"
)
message(STATUS "OCL_CUTSIM_LIBRARY is now: " ${OCL_CUTSIM_LIBRARY})

set (MOC_HEADERS cutsim.hpp)
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})

set( OCL_TST_SRC ${${PROJECT_NAME}_SOURCE_DIR}/main.cpp )

add_executable( ${PROJECT_NAME} ${OCL_TST_SRC} ${MOC_OUTFILES} )
# link with ocl, ocl-cutsim, qt, boost, opengl
target_link_libraries(${PROJECT_NAME}  ${OCL_LIBRARY} ${OCL_CUTSIM_LIBRARY} ${QT_LIBRARIES} ${Boost_LIBRARIES} ${OPENGL_LIBRARIES})

install( TARGETS ${PROJECT_NAME} DESTINATION bin )
