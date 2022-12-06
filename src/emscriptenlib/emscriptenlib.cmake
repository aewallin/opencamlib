message(STATUS "Will build emscripten js library")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -O2 -s DISABLE_EXCEPTION_CATCHING=0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

include_directories(${Boost_INCLUDE_DIRS})

# include dirs
include_directories(${PROJECT_SOURCE_DIR}/cutters)
include_directories(${PROJECT_SOURCE_DIR}/geo)
include_directories(${PROJECT_SOURCE_DIR}/algo)
include_directories(${PROJECT_SOURCE_DIR}/dropcutter)
include_directories(${PROJECT_SOURCE_DIR}/common)
include_directories(${PROJECT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR}/emscriptenlib)

add_executable(ocl
	# SHARED
	${OCL_GEO_SRC}
	${OCL_CUTTER_SRC}
	${OCL_DROPCUTTER_SRC}
	${OCL_ALGO_SRC}
	${OCL_COMMON_SRC}
	${PROJECT_SOURCE_DIR}/emscriptenlib/emscriptenlib.cpp
)

target_link_libraries(
  ocl
  ${Boost_LIBRARIES}
)

set_target_properties(ocl PROPERTIES LINK_FLAGS "\
	-lembind \
	-s MODULARIZE=1 \
	-s EXPORT_NAME=ocl \
	-s FORCE_FILESYSTEM=1 \
	-s EXPORTED_RUNTIME_METHODS=['FS'] \
	-s ASSERTIONS=1 \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s SINGLE_FILE=1 \
	-s ENVIRONMENT=web \
	--closure 1")

install(
  TARGETS ocl
  RUNTIME DESTINATION "."
)