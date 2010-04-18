
project(OpenCamLib)

cmake_minimum_required(VERSION 2.4)


#
# Turn compiler warnings up to 11, at least with gcc.  I dont know how to
# do this with other compilers we might support and I'm leaving it up to
# the relevant platform maintainers...
#

if (CMAKE_BUILD_TOOL MATCHES "make")
    add_definitions(-Wall -Werror)
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


#
# this finds the boost python library (it's installed under different names
# on Ubuntu Hardy through Karmic vs Lucid)
#

find_library(
    LIB_BOOST_PYTHON
    NAMES boost_python-mt boost_python
)


#
# this makes the ocl Python module
#

add_library(
    ocl 
    MODULE
    arc.cpp
    ballcutter.cpp
    bullcutter.cpp
    conecutter.cpp
    cutter.cpp
    cylcutter.cpp
    kdtree.cpp
    line.cpp
    numeric.cpp
    ocl.cpp
    octree.cpp
    oellipse.cpp
    path.cpp
    pathfinish.cpp
    pfinish.cpp
    point.cpp
    stlsurf.cpp
    triangle.cpp
    volume.cpp
)
target_link_libraries(ocl ${LIB_BOOST_PYTHON})
set_target_properties(ocl PROPERTIES PREFIX "")

#
# this makes a shared library for use by c/c++ code
#
# FIXME: make this work
#
# add_library(
#    ocl10.4
#    SHARED
#    arc.cpp
#    ballcutter.cpp
#    bullcutter.cpp
#    conecutter.cpp
#    cutter.cpp
#    cylcutter.cpp
#    kdtree.cpp
#    line.cpp
#    numeric.cpp
#    #ocl.cpp
#    octree.cpp
#    oellipse.cpp
#    path.cpp
#    pathfinish.cpp
#    pfinish.cpp
#    point.cpp
#    stlsurf.cpp
#    triangle.cpp
#    volume.cpp
# )
# target_link_libraries(ocl "")

#
# this figures out where to install the Python modules
#

execute_process(
    COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()"
    OUTPUT_VARIABLE Python_site_packages
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

install(
    TARGETS ocl
    LIBRARY DESTINATION ${Python_site_packages}
)

install(
    FILES ../lib/camvtk.py
    DESTINATION ${Python_site_packages}
    # PATTERN .svn EXCLUDE
)


#
# this installs the examples
#

# install(
#    DIRECTORY ../scripts/
#    FILES
#        drop_cutter_tst_1.py
#        drop_cutter_tst_2.py
#        drop_cutter_tst_3.py
#        drop_cutter_tst_4.py
#        drop_cutter_tst_5.py
#        kdtree_movie1.py
#        kdtree_movie2.py
#        kdtree_tst_1.py
#        kdtree_tst.py
#        ocl_tst.py
#        pfinish_tst_1.py
#        stl2ocl_tst.py
#        stlsurf_tst.py
#    DESTINATION
#        share/doc/python-opencam/examples
# )

# install(
#    DIRECTORY ../stl/
#    DESTINATION share/doc/python-opencam/examples/stl
#    PATTERN .svn EXCLUDE
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
    
    IF   (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
        SET(DOXY_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
    ENDIF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")

    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_SOURCE_DIR}/doc/latex/index.tex
            ${CMAKE_CURRENT_SOURCE_DIR}/doc/html/index.html
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG}
        COMMENT building LaTex & HTML docs
    )

    add_custom_target(
        doc
        #ALL
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/doc/latex/index.tex
    )


    IF (EXISTS ${PDFLATEX_COMPILER})
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/doc/latex/refman.pdf
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/doc/latex/index.tex
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc/latex
            COMMAND make pdf
            COMMENT building PDF docs
            COMMAND mv refman.pdf ../ocl-manual.pdf
        )

        add_custom_target(
            doc-pdf
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/doc/latex/refman.pdf
        )

        add_dependencies(doc doc-pdf)
    ELSE (EXISTS ${PDFLATEX_COMPILER}) 
        message(STATUS "pdflatex compiler not found, PDF docs will not be built")
    ENDIF (EXISTS ${PDFLATEX_COMPILER})


    add_custom_target(
        doc-latex
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/doc/latex/index.tex
    )

    install(
        DIRECTORY doc/latex
        DESTINATION share/doc/python-opencam/pdf
        PATTERN *.pdf
    )

    install(
        DIRECTORY doc/html
        DESTINATION share/doc/python-opencam/
    )

ENDIF(DOXYGEN_FOUND)

