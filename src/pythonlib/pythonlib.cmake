#
# Figure out how to build and where to install the Python modules
#

find_package(PythonInterp)

if (PYTHONINTERP_FOUND)
if (UNIX AND NOT APPLE)
  if (PYTHON_VERSION_MAJOR EQUAL 3)
      find_package(Boost COMPONENTS python${PYTHON_VERSION_SUFFIX})
      find_package(PythonInterp 3)
      find_package(PythonLibs 3 REQUIRED)
  else()
      find_package(Boost COMPONENTS python)
      find_package(PythonInterp)
      find_package(PythonLibs REQUIRED)
  endif()
else()  
  if (PYTHON_VERSION_MAJOR EQUAL 3)
      find_package(Boost COMPONENTS python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR})
      find_package(PythonInterp 3)
      find_package(PythonLibs 3 REQUIRED)
  else()
      find_package(Boost COMPONENTS python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR})
      find_package(PythonInterp)
      find_package(PythonLibs REQUIRED)
  endif()
endif()
else()
  message("Python not found")
endif()

execute_process(
COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print get_python_lib(0,0,\"/usr/local\")"
OUTPUT_VARIABLE Python_site_packages
OUTPUT_STRIP_TRAILING_WHITESPACE
) # on Ubuntu 11.10 this outputs: /usr/local/lib/python2.7/dist-packages

execute_process(
COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print get_python_lib(plat_specific=1,standard_lib=0,prefix=\"/usr/local\")"
OUTPUT_VARIABLE Python_arch_packages
OUTPUT_STRIP_TRAILING_WHITESPACE
)

# strip away /usr/local/  because that is what CMAKE_INSTALL_PREFIX is set to
# also, since there is no leading "/", it makes ${Python_site_packages} a relative path.
STRING(REGEX REPLACE "/usr/local/(.*)$" "\\1" Python_site_packages "${Python_site_packages}" )
STRING(REGEX REPLACE "/usr/local/(.*)$" "\\1" Python_arch_packages "${Python_arch_packages}" )

message(STATUS "Python modules will be installed to: " ${Python_site_packages})
message(STATUS "Python libraries will be installed to: " ${Python_arch_packages})


message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")
message(STATUS "PYTHON_EXECUTABLE:FILEPATH=${PYTHON_EXECUTABLE}")
message(STATUS "PYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}")
message(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_INCLUDE_DIR:FILEPATH=${PYTHON_INCLUDE_DIR}")
message(STATUS "PYTHON_FRAMEWORK_INCLUDES=${PYTHON_FRAMEWORK_INCLUDES}")
message(STATUS "PYTHONLIBS_VERSION_STRING=${PYTHONLIBS_VERSION_STRING}")
message(STATUS "Python_FRAMEWORKS=${Python_FRAMEWORKS}")
message(STATUS "PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PYTHON_INCLUDE_DIRS})

# this makes the ocl Python module
add_library(
ocl 
MODULE
pythonlib/ocl_cutters.cpp
pythonlib/ocl_geometry.cpp
pythonlib/ocl_algo.cpp
pythonlib/ocl_dropcutter.cpp
pythonlib/ocl.cpp

)

target_link_libraries(ocl ocl_common ocl_dropcutter ocl_cutters  ocl_geo ocl_algo ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})

message(STATUS "linking python binary ocl.so with boost: " ${Boost_PYTHON_LIBRARY})

# this makes the lib name ocl.so and not libocl.so
set_target_properties(ocl PROPERTIES PREFIX "") 
if (WIN32)
set_target_properties(ocl PROPERTIES VERSION ${MY_VERSION}) 
endif (WIN32)

install(
TARGETS ocl
LIBRARY DESTINATION ${Python_arch_packages}
)
# these are the python helper lib-files such as camvtk.py 
install(
DIRECTORY lib/
DESTINATION ${Python_site_packages}
#    PATTERN .svn EXCLUDE
)