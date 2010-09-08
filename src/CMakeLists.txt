project(OpenCamLib)

cmake_minimum_required(VERSION 2.4)

#
# Turn compiler warnings up to 11, at least with gcc.  I dont know how to
# do this with other compilers we might support and I'm leaving it up to
# the relevant platform maintainers...
# #include'ing the boost graph-library creates deprecated warnings
# thus we use now use -Wno-deprecated here.
#

if (CMAKE_BUILD_TOOL MATCHES "make")
    add_definitions(-Wall -Werror -Wno-deprecated)
endif (CMAKE_BUILD_TOOL MATCHES "make")

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
    ${OpenCamLib_SOURCE_DIR}/cutters/compoundcutter.cpp
    ${OpenCamLib_SOURCE_DIR}/cutters/conecutter.cpp
    ${OpenCamLib_SOURCE_DIR}/cutters/millingcutter.cpp
    ${OpenCamLib_SOURCE_DIR}/cutters/cylcutter.cpp
    ${OpenCamLib_SOURCE_DIR}/cutters/oellipse.cpp
    ${OpenCamLib_SOURCE_DIR}/cutters/epos.cpp
)

set(OCL_ALGO_SRC
    ${OpenCamLib_SOURCE_DIR}/algo/batchdropcutter.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/batchpushcutter.cpp
    
    ${OpenCamLib_SOURCE_DIR}/algo/lineclfilter.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/pathdropcutter.cpp
    
    ${OpenCamLib_SOURCE_DIR}/algo/fiber.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/waterline.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/weave.cpp
    
    ${OpenCamLib_SOURCE_DIR}/algo/numeric.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/kdtree.cpp
    ${OpenCamLib_SOURCE_DIR}/algo/kdtree2.cpp
)

set( OCL_CUTSIM_SRC
    ${OpenCamLib_SOURCE_DIR}/cutsim/volume.cpp
    ${OpenCamLib_SOURCE_DIR}/cutsim/octnode.cpp
    ${OpenCamLib_SOURCE_DIR}/cutsim/octree.cpp
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

#
# this makes the ocl Python module
#

add_library(
    ocl 
    MODULE
    ocl_cutters.cpp
    ocl_geometry.cpp
    ocl_cutsim.cpp
    ocl_algo.cpp
    ocl.cpp
)
MESSAGE(STATUS "linking ocl.so with boost: " ${Boost_LIBRARIES})
target_link_libraries(ocl ocl_cutters ocl_geo ocl_algo ocl_cutsim ${Boost_LIBRARIES})
# this makes the lib name ocl.so and not libocl.so
set_target_properties(ocl PROPERTIES PREFIX "") 


# this is the C++ shared library
add_library(
    libocl
    SHARED
    ${OCL_GEO_SRC}
    ${OCL_CUTTER_SRC}
    ${OCL_ALGO_SRC}
    # ocl_cutsim
)
# MESSAGE(STATUS "linking libocl.so with boost: " ${Boost_LIBRARIES})
# target_link_libraries(libocl ${Boost_LIBRARIES})
set_target_properties(libocl PROPERTIES PREFIX "") 

#
# this figures out where to install the Python modules
#

execute_process(
    COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()"
    OUTPUT_VARIABLE Python_site_packages
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
MESSAGE(STATUS "Python libraries will be installed to: " ${Python_site_packages})

install(
    TARGETS ocl
    LIBRARY DESTINATION ${Python_site_packages}
)

install(
    DIRECTORY ../lib/
    DESTINATION ${Python_site_packages}
    PATTERN .svn EXCLUDE
)


#
# this installs the examples
#

# install(
#     DIRECTORY ../scripts/
#    DESTINATION share/doc/python-opencam/examples
#    PATTERN .svn EXCLUDE
# )

# install(
#     DIRECTORY ../stl/
#     DESTINATION share/doc/python-opencam/examples/stl
#     PATTERN .svn EXCLUDE
# )


#
# build & install documentation (if Doxygen is available)
#

FIND_PACKAGE(Doxygen)
IF (DOXYGEN_FOUND)
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


#    IF (EXISTS ${PDFLATEX_COMPILER})
#        add_custom_command(
#            OUTPUT doc/latex/refman.pdf
#            DEPENDS doc/latex/index.tex
#            WORKING_DIRECTORY doc/latex
#            COMMAND make pdf
#            COMMENT building PDF docs
#            COMMAND mv refman.pdf ../ocl-manual.pdf
#        )
#
#        add_custom_target(
#            doc-pdf
#            DEPENDS doc/latex/refman.pdf
#        )
#
#        add_dependencies(doc doc-pdf)
#    ELSE (EXISTS ${PDFLATEX_COMPILER}) 
 #       message(STATUS "pdflatex compiler not found, PDF docs will not be built")
 #   ENDIF (EXISTS ${PDFLATEX_COMPILER})


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

