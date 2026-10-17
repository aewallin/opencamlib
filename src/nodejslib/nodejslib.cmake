add_library(ocl
	SHARED
	${OCL_SOURCE_DIR}/nodejslib/point_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/triangle_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/bbox_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/cylcutter_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/line_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/arc_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/path_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/stlsurf_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/stlreader_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/waterline_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/pathdropcutter_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/adaptivepathdropcutter_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/adaptivewaterline_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/ballcutter_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/bullcutter_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/conecutter_js.cpp
	${OCL_SOURCE_DIR}/nodejslib/nodejslib.cpp
)

target_link_libraries(
  	ocl
  PRIVATE
    ocl_core ${CMAKE_JS_LIB}
)
if(USE_OPENMP)
  target_link_libraries(ocl PRIVATE OpenMP::OpenMP_CXX)
endif()

set_target_properties(ocl PROPERTIES PREFIX "" SUFFIX ".node")

target_compile_definitions(ocl PRIVATE NAPI_VERSION=3 NAPI_CPP_EXCEPTIONS)
target_include_directories(ocl PRIVATE ${CMAKE_JS_INC}
  ${OCL_SOURCE_DIR}/nodejslib/node_modules/node-addon-api)
ocl_configure_target(ocl)

if(USE_OPENMP AND APPLE)
  # copy libomp into install directory
  install(
    FILES ${OpenMP_CXX_LIBRARIES}
    DESTINATION "."
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
  )
  # fix loader path
  add_custom_command(TARGET ocl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OpenMP_CXX_LIBRARIES} $<TARGET_FILE_DIR:ocl>
    COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change `otool -L $<TARGET_FILE:ocl> | grep libomp | cut -d ' ' -f1 | xargs echo` "@loader_path/libomp.dylib" $<TARGET_FILE:ocl>
  )
endif()

install(
	TARGETS ocl
	DESTINATION "."
)
