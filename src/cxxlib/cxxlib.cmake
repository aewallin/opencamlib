add_library(
  libocl
  SHARED
  ${OCL_GEO_SRC}
  ${OCL_CUTTER_SRC}
  ${OCL_ALGO_SRC}
  ${OCL_DROPCUTTER_SRC}
  ${OCL_COMMON_SRC}
  )
set_target_properties(libocl PROPERTIES PREFIX "") 
set_target_properties(libocl PROPERTIES VERSION ${MY_VERSION}) 
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