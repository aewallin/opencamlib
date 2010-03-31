
project(OpenCamLib)

cmake_minimum_required(VERSION 2.4)


#
# Turn compiler warnings up to 11, at least with gcc.  I dont know how to
# do this with other compilers we might support and I'm leaving it up to
# the relevant platform maintainers...
#

if (CMAKE_BUILD_TOOL MATCHES "make")
    add_definitions(-Wall -Werror)
endif ()


#
# this figures out the Python include directories and adds them to the
# header file search path
#

execute_process(
    COMMAND python-config --includes
    COMMAND sed -r "s/-I//g; s/ +/;/g"
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
    cutter.cpp
    cylcutter.cpp
    kdtree.cpp
    line.cpp
    numeric.cpp
    ocl.cpp
    oellipse.cpp
    path.cpp
    pathfinish.cpp
    pfinish.cpp
    point.cpp
    stlsurf.cpp
    triangle.cpp
)
target_link_libraries(ocl ${LIB_BOOST_PYTHON})
set_target_properties(ocl PROPERTIES PREFIX "")


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
    FILES camvtk.py
    DESTINATION ${Python_site_packages}
)


#
# this installs the examples
#

install(
    FILES
        drop_cutter_tst_1.py
        drop_cutter_tst_2.py
        drop_cutter_tst_3.py
        drop_cutter_tst_4.py
        drop_cutter_tst_5.py
        kdtree_movie1.py
        kdtree_movie2.py
        kdtree_tst_1.py
        kdtree_tst.py
        ocl_tst.py
        pfinish_tst_1.py
        stl2ocl_tst.py
        stlsurf_tst.py
        demo.stl
        gnu_tux_mod.stl
        sphere.stl
    DESTINATION
        share/doc/python-opencam/examples
)

