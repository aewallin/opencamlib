
# from: http://stackoverflow.com/questions/435708/any-way-in-cmake-to-require-gcc-version-4

if(CMAKE_COMPILER_IS_GNUCXX)

  exec_program(
      ${CMAKE_CXX_COMPILER}
      ARGS                    --version
      OUTPUT_VARIABLE _compiler_output)
      string(REGEX REPLACE ".*([0-9]\\.[0-9]\\.[0-9]).*" "\\1"
      gcc_compiler_version 
      "${_compiler_output}"
      )
  
  message(STATUS "C++ compiler version: ${gcc_compiler_version} [${CMAKE_CXX_COMPILER}]")
  
  IF(${gcc_compiler_version} MATCHES ".*4\\.[6789]\\.[0-9].*")
    #MESSAGE("gcc >= 4.6")
    set(GCC_4_6 TRUE)
  ELSE(${gcc_compiler_version} MATCHES ".*4\\.[6789]\\.[0-9].*")
    #MESSAGE("gcc < 4.6")
    set(GCC_4_6 FALSE)
  ENDIF(${gcc_compiler_version} MATCHES ".*4\\.[6789]\\.[0-9].*")

endif(CMAKE_COMPILER_IS_GNUCXX)
