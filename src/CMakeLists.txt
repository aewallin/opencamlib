project(OpenCamLib)

cmake_minimum_required(VERSION 2.4)



# some options to set what is built:
option(BUILD_CXX_LIB
  "Build/install the pure c++ libocl library? " OFF)

option(BUILD_PY_LIB 
  "Build/install the python ocl library? " ON)

option(BUILD_DOC
  "Build/install the ocl documentation? " OFF)

if (NOT BUILD_CXX_LIB)
    MESSAGE(STATUS " Note: will NOT build pure c++ library")
endif(NOT BUILD_CXX_LIB)

if (NOT BUILD_PY_LIB)
    MESSAGE(STATUS " Note: will NOT build python library")
endif(NOT BUILD_PY_LIB)

if (NOT BUILD_DOC)
    MESSAGE(STATUS " Note: will NOT build ocl documentation")
endif(NOT BUILD_DOC)


#
# Turn compiler warnings up to 11, at least with gcc.  I dont know how to
# do this with other compilers we might support and I'm leaving it up to
# the relevant platform maintainers...
# #include'ing the boost graph-library creates deprecated warnings
# thus we use now use -Wno-deprecated here.
#
if (CMAKE_BUILD_TOOL MATCHES "make")
    MESSAGE(STATUS "setting gcc options: -Wall -Werror -Wno-deprecated -pedantic-errors")
    add_definitions(-Wall -Werror -Wno-deprecated -pedantic-errors)
endif (CMAKE_BUILD_TOOL MATCHES "make")

# find out the number of CPUs
# from: http://www.kitware.com/blog/home/post/63
if(NOT DEFINED PROCESSOR_COUNT)
  # Unknown:
  set(PROCESSOR_COUNT 0)
  # Linux:
  set(cpuinfo_file "/proc/cpuinfo")
  if(EXISTS "${cpuinfo_file}")
    file(STRINGS "${cpuinfo_file}" procs REGEX "^processor.: [0-9]+$")
    list(LENGTH procs PROCESSOR_COUNT)
  endif()
  MESSAGE(STATUS "Number of processors on this system " ${PROCESSOR_COUNT} )
  # FIXME: how do we make cmake/make use all the processors...
endif()


#
# this figures out the Python include directories and adds them to the
# header file search path
#
execute_process(
    COMMAND python-config --includes
    COMMAND sed -r "s/-I//g; s/ +/;/g"
    COMMAND tr -d '\n'
    OUTPUT_VARIABLE Python_Includes
)
include_directories(${Python_Includes})

# run get_revision.py which generates revision.h needed by ocl.revision()

execute_process(
     COMMAND python get_revision.py
)
MESSAGE(STATUS "running get_revision.py to generate revision.h")




# find BOOST and boost-python
find_package( Boost COMPONENTS python REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    MESSAGE(STATUS "found Boost: " ${Boost_LIB_VERSION})
    MESSAGE(STATUS "boost-incude dirs are: " ${Boost_INCLUDE_DIRS})
    MESSAGE(STATUS "boost-python lib is: " ${Boost_PYTHON_LIBRARY})
endif()

#
# find OpenMP
#

find_package( OpenMP REQUIRED )

IF (OPENMP_FOUND)
    MESSAGE(STATUS "found OpenMP, compiling with flags: " ${OpenMP_CXX_FLAGS} )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
ENDIF(OPENMP_FOUND)

# this defines the source-files

set(OCL_GEO_SRC
    ${OpenCamLib_SOURCE_DIR}/geo/arc.cpp
    ${OpenCamLib_SOURCE_DIR}/geo/bbox.cpp
    ${OpenCamLib_SOURCE_DIR}/geo/ccpoint.cpp
    ${OpenCamLib_SOURCE_DIR}/geo/clpoint.cpp
    ${OpenCamLib_SOURCE_DIR}/geo/line.cpp
    ${OpenCamLib_SOURCE_DIR}/geo/path.cpp
    ${OpenCamLib_SOURCE_DIR}/geo/point.cpp
    ${OpenCamLib_SOURCE_DIR}/geo/stlreader.cpp
    ${OpenCamLib_SOURCE_DIR}/geo/stlsurf.cpp
    ${OpenCamLib_SOURCE_DIR}/geo/triangle.cpp
)

set(OCL_CUTTER_SRC
    ${OpenCamLib_SOURCE_DIR}/cutters/ballcutter.cpp
    ${OpenCamLib_SOURCE_DIR}/cutters/bullcutter.cpp
    ${OpenCamLib_SOURCE_DIR}/cutters/compositecutter.cpp
    ${OpenCamLib_SOURCE_DIR}/cutters/conecutter.cpp
    ${OpenCamLib_SOURCE_DIR}/cutters/millingcutter.cpp
    ${OpenCamLib_SOURCE_DIR}/cutters/cylcutter.cpp
    ${OpenCamLib_SOURCE_DIR}/cutters/ellipse.cpp
    ${OpenCamLib_SOURCE_DIR}/cutters/ellipseposition.cpp
)

set(OCL_ALGO_SRC
    ${OpenCamLib_SOURCE_DIR}/algo/batchdropcutter.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/pointdropcutter.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/batchpushcutter.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/fiberpushcutter.cpp
    
    ${OpenCamLib_SOURCE_DIR}/algo/lineclfilter.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/pathdropcutter.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/adaptivepathdropcutter.cpp
    
    ${OpenCamLib_SOURCE_DIR}/algo/interval.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/fiber.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/waterline.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/adaptivewaterline.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/weave.cpp
    
    # ${OpenCamLib_SOURCE_DIR}/algo/halfedgediagram.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/voronoidiagram.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/facegrid.cpp
    
    ${OpenCamLib_SOURCE_DIR}/algo/numeric.cpp

)

set( OCL_CUTSIM_SRC
    ${OpenCamLib_SOURCE_DIR}/cutsim/volume.cpp
    ${OpenCamLib_SOURCE_DIR}/cutsim/octnode.cpp
    ${OpenCamLib_SOURCE_DIR}/cutsim/octree.cpp
)

set( OCL_INCLUDE_FILES
    ${OpenCamLib_SOURCE_DIR}/revision.h
    
    ${OpenCamLib_SOURCE_DIR}/geo/arc.h
    ${OpenCamLib_SOURCE_DIR}/geo/bbox.h
    ${OpenCamLib_SOURCE_DIR}/geo/ccpoint.h
    ${OpenCamLib_SOURCE_DIR}/geo/clpoint.h
    ${OpenCamLib_SOURCE_DIR}/geo/line.h
    ${OpenCamLib_SOURCE_DIR}/geo/path.h
    ${OpenCamLib_SOURCE_DIR}/geo/stlreader.h
    ${OpenCamLib_SOURCE_DIR}/geo/stlsurf.h
    ${OpenCamLib_SOURCE_DIR}/geo/triangle.h
    ${OpenCamLib_SOURCE_DIR}/geo/point.h
    
    ${OpenCamLib_SOURCE_DIR}/cutters/ballcutter.h
    ${OpenCamLib_SOURCE_DIR}/cutters/bullcutter.h
    ${OpenCamLib_SOURCE_DIR}/cutters/compositecutter.h
    ${OpenCamLib_SOURCE_DIR}/cutters/conecutter.h
    ${OpenCamLib_SOURCE_DIR}/cutters/cylcutter.h
    ${OpenCamLib_SOURCE_DIR}/cutters/ellipseposition.h
    ${OpenCamLib_SOURCE_DIR}/cutters/millingcutter.h
    ${OpenCamLib_SOURCE_DIR}/cutters/ellipse.h
    
    #${OpenCamLib_SOURCE_DIR}/cutsim/mc_tables.h
    ${OpenCamLib_SOURCE_DIR}/cutsim/octnode.h
    ${OpenCamLib_SOURCE_DIR}/cutsim/octree.h
    ${OpenCamLib_SOURCE_DIR}/cutsim/volume.h
    ${OpenCamLib_SOURCE_DIR}/cutsim/marching_cubes.h
    
    ${OpenCamLib_SOURCE_DIR}/algo/operation.h
    ${OpenCamLib_SOURCE_DIR}/algo/batchdropcutter.h
    ${OpenCamLib_SOURCE_DIR}/algo/batchpushcutter.h
    ${OpenCamLib_SOURCE_DIR}/algo/fiberpushcutter.h
    ${OpenCamLib_SOURCE_DIR}/algo/brent_zero.h
    ${OpenCamLib_SOURCE_DIR}/algo/clfilter.h
    ${OpenCamLib_SOURCE_DIR}/algo/fiber.h
    ${OpenCamLib_SOURCE_DIR}/algo/interval.h
    ${OpenCamLib_SOURCE_DIR}/algo/kdnode3.h
    ${OpenCamLib_SOURCE_DIR}/algo/kdtree3.h
    ${OpenCamLib_SOURCE_DIR}/algo/lineclfilter.h
    ${OpenCamLib_SOURCE_DIR}/algo/numeric.h
    ${OpenCamLib_SOURCE_DIR}/algo/pathdropcutter.h
    ${OpenCamLib_SOURCE_DIR}/algo/waterline.h
    ${OpenCamLib_SOURCE_DIR}/algo/adaptivewaterline.h
    ${OpenCamLib_SOURCE_DIR}/algo/weave.h
    ${OpenCamLib_SOURCE_DIR}/algo/weave_typedef.h
    ${OpenCamLib_SOURCE_DIR}/algo/voronoidiagram.h
    # ${OpenCamLib_SOURCE_DIR}/algo/halfedgediagram.h
    ${OpenCamLib_SOURCE_DIR}/algo/halfedgediagram2.h
    ${OpenCamLib_SOURCE_DIR}/algo/facegrid.h
)

# this branches into the dirs and compiles stuff there
ADD_SUBDIRECTORY( ${OpenCamLib_SOURCE_DIR}/cutters  )
ADD_SUBDIRECTORY( ${OpenCamLib_SOURCE_DIR}/geo  )
ADD_SUBDIRECTORY( ${OpenCamLib_SOURCE_DIR}/algo  )
ADD_SUBDIRECTORY( ${OpenCamLib_SOURCE_DIR}/cutsim  )

# include dirs
include_directories( ${OpenCamLib_SOURCE_DIR}/cutters )
include_directories( ${OpenCamLib_SOURCE_DIR}/geo )
include_directories( ${OpenCamLib_SOURCE_DIR}/algo )
include_directories( ${OpenCamLib_SOURCE_DIR}/cutsim )
include_directories( ${OpenCamLib_SOURCE_DIR} )

if (BUILD_PY_LIB)
    # this makes the ocl Python module
    add_library(
        ocl 
        MODULE
        ocl_cutters.cpp
        ocl_geometry.cpp
        ocl_cutsim.cpp
        ocl_algo.cpp
        ocl.cpp
    )
endif (BUILD_PY_LIB)

MESSAGE(STATUS "linking ocl.so with boost: " ${Boost_LIBRARIES})
target_link_libraries(ocl ocl_cutters ocl_geo ocl_algo ocl_cutsim ${Boost_LIBRARIES})
# this makes the lib name ocl.so and not libocl.so
set_target_properties(ocl PROPERTIES PREFIX "") 



# this is the C++ shared library
if (BUILD_CXX_LIB)
    add_library(
        libocl
        SHARED
        ${OCL_GEO_SRC}
        ${OCL_CUTTER_SRC}
        ${OCL_ALGO_SRC}
        ${OCL_CUTSIM_SRC}
    )
    set_target_properties(libocl PROPERTIES PREFIX "") 
endif (BUILD_CXX_LIB)

#
# this figures out where to install the Python modules
#

execute_process(
    COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()"
    OUTPUT_VARIABLE Python_site_packages
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
MESSAGE(STATUS "Python libraries will be installed to: " ${Python_site_packages})

if (BUILD_PY_LIB)
    # this installs the python library
    install(
        TARGETS ocl
        LIBRARY DESTINATION ${Python_site_packages}
    )


    # these are the python helper lib-files such as camvtk.py 
    install(
        DIRECTORY ../lib/
        DESTINATION ${Python_site_packages}
        PATTERN .svn EXCLUDE
    )
endif (BUILD_PY_LIB)

# this installs the c++ library
if (BUILD_CXX_LIB)
    install(
        TARGETS libocl
        LIBRARY 
        DESTINATION lib/opencamlib
        ARCHIVE DESTINATION lib/opencamlib
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )

    # this installs the c++ include headers
    install(
        FILES ${OCL_INCLUDE_FILES}
        DESTINATION include/opencamlib
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
    )
endif (BUILD_CXX_LIB)




#
# this installs the examples
#

# install(
#     DIRECTORY ../scripts/
#    DESTINATION share/doc/python-opencam/examples
#    PATTERN .svn EXCLUDE
# )

# the example STL files
# install(
#     DIRECTORY ../stl/
#     DESTINATION share/doc/python-opencam/examples/stl
#     PATTERN .svn EXCLUDE
# )


#
# build & install documentation (if Doxygen is available)
#

if (BUILD_DOC)

FIND_PACKAGE(Doxygen)
IF (DOXYGEN_FOUND)
    MESSAGE(STATUS "Found doxygen. ")
    
    # this works around a bug in cmake 2.4 (Ubuntu Hardy)
    execute_process(
        COMMAND mkdir -p doc/html doc/latex
    )

    FIND_PACKAGE(LATEX)
    IF    (NOT LATEX_COMPILER)
        MESSAGE(STATUS "latex command LATEX_COMPILER not found but usually required. You will probably get warnings and user inetraction on doxy run.")
    ENDIF (NOT LATEX_COMPILER)
    IF    (NOT MAKEINDEX_COMPILER)
        MESSAGE(STATUS "makeindex command MAKEINDEX_COMPILER not found but usually required.")
    ENDIF (NOT MAKEINDEX_COMPILER)
    IF    (NOT DVIPS_CONVERTER)
        MESSAGE(STATUS "dvips command DVIPS_CONVERTER not found but usually required.")
    ENDIF (NOT DVIPS_CONVERTER)
    
    if (EXISTS Doxyfile)
        set(DOXY_CONFIG Doxyfile)
    endif (EXISTS Doxyfile)

    add_custom_command(
        OUTPUT
            doc/latex/index.tex
            doc/html/index.html
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG}
        COMMENT building LaTex & HTML docs
    )

    add_custom_target(
        doc
        DEPENDS doc/latex/index.tex
    )


    IF (EXISTS ${PDFLATEX_COMPILER})
        add_custom_command(
            OUTPUT doc/latex/refman.pdf
            DEPENDS doc/latex/index.tex
            WORKING_DIRECTORY doc/latex
            COMMAND make pdf
            COMMENT building PDF docs
            COMMAND mv refman.pdf ../ocl-manual.pdf
        )

        add_custom_target(
            doc-pdf
            DEPENDS doc/latex/refman.pdf
        )

        add_dependencies(doc doc-pdf)
    ELSE (EXISTS ${PDFLATEX_COMPILER}) 
       message(STATUS "pdflatex compiler not found, PDF docs will not be built")
   ENDIF (EXISTS ${PDFLATEX_COMPILER})


    add_custom_target(
        doc-latex
        DEPENDS doc/latex/index.tex
    )

    #install(
    #    DIRECTORY doc/latex/
    #    DESTINATION share/doc/python-opencam/pdf
    #    FILES_MATCHING PATTERN *.pdf
    #)

    #install(
    #    FILES doc/ocl-manual.pdf
    #    DESTINATION share/doc/python-opencam/pdf
    #)

    #install(
    #    DIRECTORY doc/html
    #    DESTINATION share/doc/python-opencam/
    #)

ENDIF(DOXYGEN_FOUND)
endif (BUILD_DOC)




