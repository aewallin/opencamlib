## Debian Source Package Generator
#
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
# Many modifications by Rosen Diankov <rosen.diankov@gmail.com>
#
# Creates source debian files and manages library dependencies
#
# Features:
# 
# - Automatically generates symbols and run-time dependencies from the build dependencies
# - Custom copy of source directory via CPACK_DEBIAN_PACKAGE_SOURCE_COPY
# - Simultaneous output of multiple debian source packages for each distribution
#
# Usage:
#
# set(CPACK_DEBIAN_BUILD_DEPENDS debhelper cmake)
# set(CPACK_DEBIAN_PACKAGE_PRIORITY optional)
# set(CPACK_DEBIAN_PACKAGE_SECTION devel)
# set(CPACK_DEBIAN_PACKAGE_DEPENDS mycomp0 mycomp1 some_ubuntu_package)
# set(CPACK_DEBIAN_PACKAGE_NAME mypackage)
# set(CPACK_DEBIAN_PACKAGE_REMOVE_SOURCE_FILES unnecessary_file unnecessary_dir/file0)
# set(CPACK_DEBIAN_PACKAGE_SOURCE_COPY svn export --force) # if using subversion
# set(CPACK_DEBIAN_DISTRIBUTION_NAME ubuntu)
# set(CPACK_DEBIAN_DISTRIBUTION_RELEASES karmic lucid maverick natty)
# set(CPACK_DEBIAN_CHANGELOG "  * Extra change log lines")
##

find_program(DEBUILD_EXECUTABLE debuild)
find_program(DPUT_EXECUTABLE dput)
find_program(GIT_EXECUTABLE git)

if(NOT DEBUILD_EXECUTABLE OR NOT DPUT_EXECUTABLE)
  return()
endif(NOT DEBUILD_EXECUTABLE OR NOT DPUT_EXECUTABLE)

# DEBIAN/control
# debian policy enforce lower case for package name
# Package: (mandatory)
IF(NOT CPACK_DEBIAN_PACKAGE_NAME)
  STRING(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_DEBIAN_PACKAGE_NAME)
ENDIF(NOT CPACK_DEBIAN_PACKAGE_NAME)

MESSAGE(STATUS "Debian package name: " ${CPACK_DEBIAN_PACKAGE_NAME})

# Section: (recommended)
IF(NOT CPACK_DEBIAN_PACKAGE_SECTION)
  SET(CPACK_DEBIAN_PACKAGE_SECTION "devel")
ENDIF(NOT CPACK_DEBIAN_PACKAGE_SECTION)
MESSAGE(STATUS "Debian package section: " ${CPACK_DEBIAN_PACKAGE_SECTION})

# Priority: (recommended)
IF(NOT CPACK_DEBIAN_PACKAGE_PRIORITY)
  SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
ENDIF(NOT CPACK_DEBIAN_PACKAGE_PRIORITY)
MESSAGE(STATUS "Debian package section: " ${CPACK_DEBIAN_PACKAGE_PRIORITY})

MESSAGE(STATUS "reading description file: " ${CPACK_PACKAGE_DESCRIPTION_FILE})
file(STRINGS ${CPACK_PACKAGE_DESCRIPTION_FILE} DESC_LINES)
foreach(LINE ${DESC_LINES})
  set(DEB_LONG_DESCRIPTION "${DEB_LONG_DESCRIPTION} ${LINE}\n")
endforeach(LINE ${DESC_LINES})
MESSAGE(STATUS "Debian package long description: " ${DEB_LONG_DESCRIPTION})

file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/Debian")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Debian")
set(DEBIAN_SOURCE_ORIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/Debian/${CPACK_DEBIAN_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
MESSAGE(STATUS "Debian package source-orig-dir: " ${DEBIAN_SOURCE_ORIG_DIR})

# copy of source
if( CPACK_DEBIAN_PACKAGE_SOURCE_COPY )
  execute_process(COMMAND ${CPACK_DEBIAN_PACKAGE_SOURCE_COPY} "${CMAKE_SOURCE_DIR}" "${DEBIAN_SOURCE_ORIG_DIR}.orig")
else( CPACK_DEBIAN_PACKAGE_SOURCE_COPY )
  MESSAGE(STATUS "Copying files from ${DEB_SRC_DIR} to ${DEBIAN_SOURCE_ORIG_DIR}.orig")
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${DEB_SRC_DIR} "${DEBIAN_SOURCE_ORIG_DIR}.orig")
  # create a git_tag.txt file
    MESSAGE(STATUS "Writing git-tag.txt")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags 
        RESULT_VARIABLE res_var 
        OUTPUT_VARIABLE GIT_COM_ID 
    )
    string( REPLACE "\n" "" GIT_COMMIT_ID ${GIT_COM_ID} )
    file(WRITE "${DEBIAN_SOURCE_ORIG_DIR}.orig/git-tag.txt" ${GIT_COMMIT_ID} )
  
  #execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/../.git "${DEBIAN_SOURCE_ORIG_DIR}.orig/.git")
  #MESSAGE(STATUS "Removing .git from source-orig-dir. ")
  #execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory "${DEBIAN_SOURCE_ORIG_DIR}.orig/.git")
  #execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory "${DEBIAN_SOURCE_ORIG_DIR}.orig/.svn")
endif( CPACK_DEBIAN_PACKAGE_SOURCE_COPY )

# remove unnecessary folders
foreach(REMOVE_DIR ${CPACK_DEBIAN_PACKAGE_REMOVE_SOURCE_FILES})
    MESSAGE(STATUS "Removing direcotry from source-orig-dir. " ${REMOVE_DIR})
  file(REMOVE_RECURSE ${DEBIAN_SOURCE_ORIG_DIR}.orig/${REMOVE_DIR})
endforeach()

MESSAGE(STATUS "Creating source tar " )
# create the original source tar
execute_process(COMMAND ${CMAKE_COMMAND} -E tar czf "${CPACK_DEBIAN_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}.orig.tar.gz" "${CPACK_DEBIAN_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.orig" 
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Debian)
MESSAGE(STATUS " tar created." )

set(DEB_SOURCE_CHANGES)
foreach(RELEASE ${CPACK_DEBIAN_DISTRIBUTION_RELEASES})
    MESSAGE(STATUS " processing release: " ${RELEASE})
  set(DEBIAN_SOURCE_DIR "${DEBIAN_SOURCE_ORIG_DIR}-${CPACK_DEBIAN_DISTRIBUTION_NAME}1~${RELEASE}1")
  MESSAGE(STATUS "  release source dir: " ${DEBIAN_SOURCE_DIR})
  set(RELEASE_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}-${CPACK_DEBIAN_DISTRIBUTION_NAME}1~${RELEASE}1")
  MESSAGE(STATUS "  release package version: " ${RELEASE_PACKAGE_VERSION})
  file(MAKE_DIRECTORY ${DEBIAN_SOURCE_DIR}/debian)
  ##############################################################################
  # debian/control
  MESSAGE(STATUS "  creating debian/control file.")
  set(DEBIAN_CONTROL ${DEBIAN_SOURCE_DIR}/debian/control)
  file(WRITE ${DEBIAN_CONTROL}
    "Source: ${CPACK_DEBIAN_PACKAGE_NAME}\n"
    "Section: ${CPACK_DEBIAN_PACKAGE_SECTION}\n"
    "Priority: ${CPACK_DEBIAN_PACKAGE_PRIORITY}\n"
    "DM-Upload-Allowed: yes\n"
    "Maintainer: ${CPACK_PACKAGE_CONTACT}\n"
    "Build-Depends: "
    )

  foreach(DEP ${DEBSRC_BUILD_DEPENDS})
    MESSAGE(STATUS "   build-depency: " ${DEP})
    file(APPEND ${DEBIAN_CONTROL} "${DEP}, ")
  endforeach(DEP ${DEBSRC_BUILD_DEPENDS})

  file(APPEND ${DEBIAN_CONTROL} "\n"
    "Standards-Version: 3.9.1\n"
    "Homepage: ${CPACK_PACKAGE_VENDOR}\n"
    "\n"
    "Package: ${CPACK_DEBIAN_PACKAGE_NAME}\n"
    "Architecture: any\n"
    "Suggests: ${CPACK_DEBIAN_BUILD_SUGGESTS}\n"
    "Depends: "
    )
    set(DEBHELP_DEPENDS "\${misc:Depends}")
    file(APPEND ${DEBIAN_CONTROL} "${DEBHELP_DEPENDS}, ")

  foreach(DEP ${DEBSRC_PACKAGE_DEPENDS})
    MESSAGE(STATUS "   package-depency: " ${DEP})
    file(APPEND ${DEBIAN_CONTROL} "${DEP}, ")
  endforeach(DEP ${DEBSRC_PACKAGE_DEPENDS})  

  file(APPEND ${DEBIAN_CONTROL} "\n"
    "Description: ${CPACK_PACKAGE_DISPLAY_NAME} ${CPACK_PACKAGE_DESCRIPTION_SUMMARY}\n"
    "${DEB_LONG_DESCRIPTION}"
    )

  foreach(COMPONENT ${CPACK_COMPONENTS_ALL})
    string(TOUPPER ${COMPONENT} UPPER_COMPONENT)
    set(DEPENDS "\${shlibs:Depends}")
    foreach(DEP ${CPACK_COMPONENT_${UPPER_COMPONENT}_DEPENDS})
      set(DEPENDS "${DEPENDS}, ${DEP}")
    endforeach(DEP ${CPACK_COMPONENT_${UPPER_COMPONENT}_DEPENDS})
    file(APPEND ${DEBIAN_CONTROL} "\n"
      "Package: ${COMPONENT}\n"
      "Architecture: any\n"
      "Depends: ${DEPENDS}\n"
      "Description: ${CPACK_PACKAGE_DISPLAY_NAME} ${CPACK_COMPONENT_${UPPER_COMPONENT}_DISPLAY_NAME}\n"
      "${DEB_LONG_DESCRIPTION}"
      " .\n"
      " ${CPACK_COMPONENT_${UPPER_COMPONENT}_DESCRIPTION}\n"
      )
  endforeach(COMPONENT ${CPACK_COMPONENTS_ALL})

  MESSAGE(STATUS "  creating debian/control DONE.")
  
  ##############################################################################
  # debian/copyright
  MESSAGE(STATUS "  creating debian/coyright.")
  set(DEBIAN_COPYRIGHT ${DEBIAN_SOURCE_DIR}/debian/copyright)
  execute_process(COMMAND ${CMAKE_COMMAND} -E
    copy ${CPACK_RESOURCE_FILE_LICENSE} ${DEBIAN_COPYRIGHT}
    )

  ##############################################################################
  # debian/rules
  MESSAGE(STATUS "  creating debian/rules.")
  set(DEBIAN_RULES ${DEBIAN_SOURCE_DIR}/debian/rules)
  file(WRITE ${DEBIAN_RULES}
    "#!/usr/bin/make -f\n"
    "\n"
    "BUILDDIR = build_dir\n"
    "\n"
    "build:\n"
    "	mkdir $(BUILDDIR)\n"
    "	cd $(BUILDDIR); cmake -DCMAKE_BUILD_TYPE=Release -DOPT_BIN_SUFFIX=ON -DBASH_COMPLETION_DIR=../etc/bash_completion.d -DCMAKE_INSTALL_PREFIX=/usr ..\n"
    "	$(MAKE) -C $(BUILDDIR) preinstall\n"
    "	touch build\n"
    "\n"
    "binary: binary-indep binary-arch\n"
    "\n"
    "binary-indep: build\n"
    "\n"
    "binary-arch: build\n"
    "	cd $(BUILDDIR); cmake -DCOMPONENT=Unspecified -DCMAKE_INSTALL_PREFIX=../debian/tmp/usr -P cmake_install.cmake\n"
    "	mkdir -p debian/tmp/DEBIAN\n"
    "	dpkg-gensymbols -p${CPACK_DEBIAN_PACKAGE_NAME}\n"
    )

  foreach(COMPONENT ${CPACK_COMPONENTS_ALL})
    set(PATH debian/${COMPONENT})
    file(APPEND ${DEBIAN_RULES}
      "	cd $(BUILDDIR); cmake -DCOMPONENT=${COMPONENT} -DCMAKE_INSTALL_PREFIX=../${PATH}/usr -P cmake_install.cmake\n"
      "	mkdir -p ${PATH}/DEBIAN\n"
      "	dpkg-gensymbols -p${COMPONENT} -P${PATH}\n"
      )
  endforeach(COMPONENT ${CPACK_COMPONENTS_ALL})

  file(APPEND ${DEBIAN_RULES}
    "	dh_shlibdeps\n"
    "	dh_strip\n" # for reducing size
    "	dpkg-gencontrol -p${CPACK_DEBIAN_PACKAGE_NAME}\n"
    "	dpkg --build debian/tmp ..\n"
    )

  foreach(COMPONENT ${CPACK_COMPONENTS_ALL})
    set(PATH debian/${COMPONENT})
    file(APPEND ${DEBIAN_RULES}
      "	dpkg-gencontrol -p${COMPONENT} -P${PATH} -Tdebian/${COMPONENT}.substvars\n"
      "	dpkg --build ${PATH} ..\n"
      )
  endforeach(COMPONENT ${CPACK_COMPONENTS_ALL})

  file(APPEND ${DEBIAN_RULES}
    "\n"
    "clean:\n"
    "	rm -f build\n"
    "	rm -rf $(BUILDDIR)\n"
    "\n"
    ".PHONY: binary binary-arch binary-indep clean\n"
    )

  execute_process(COMMAND chmod +x ${DEBIAN_RULES})

  ##############################################################################
  # debian/compat
  file(WRITE ${DEBIAN_SOURCE_DIR}/debian/compat "7")

  ##############################################################################
  # debian/source/format
  file(WRITE ${DEBIAN_SOURCE_DIR}/debian/source/format "3.0 (quilt)")

  ##############################################################################
  # debian/changelog
  set(DEBIAN_CHANGELOG ${DEBIAN_SOURCE_DIR}/debian/changelog)
  execute_process(COMMAND date -R  OUTPUT_VARIABLE DATE_TIME)
  file(WRITE ${DEBIAN_CHANGELOG}
    "${CPACK_DEBIAN_PACKAGE_NAME} (${RELEASE_PACKAGE_VERSION}) ${RELEASE}; urgency=low\n\n"
    "  * Package built with CMake\n\n"
    "${CPACK_DEBIAN_CHANGELOG} \n"
    " -- ${CPACK_PACKAGE_CONTACT}  ${DATE_TIME}"
    )

  ##############################################################################
  # debuild -S
  if( DEB_SOURCE_CHANGES )
    set(DEBUILD_OPTIONS "-sd")
  else()
    set(DEBUILD_OPTIONS "-sa")
  endif()
  set(SOURCE_CHANGES_FILE "${CPACK_DEBIAN_PACKAGE_NAME}_${RELEASE_PACKAGE_VERSION}_source.changes")
  set(DEB_SOURCE_CHANGES ${DEB_SOURCE_CHANGES} "${SOURCE_CHANGES_FILE}" CACHE STRING "to ppa")
  #add_custom_command(OUTPUT "${SOURCE_CHANGES_FILE}" COMMAND ${DEBUILD_EXECUTABLE} -S ${DEBUILD_OPTIONS} WORKING_DIRECTORY ${DEBIAN_SOURCE_DIR})
    message( STATUS " running for ${DEBUILD_EXECUTABLE} -S ${DEBUILD_OPTIONS}  ")
    message( STATUS " in directory  ${DEBIAN_SOURCE_DIR} ")
    execute_process(
        COMMAND ${DEBUILD_EXECUTABLE} -S ${DEBUILD_OPTIONS} 
        WORKING_DIRECTORY ${DEBIAN_SOURCE_DIR}
        #ERROR_QUIET
        #OUTPUT_QUIET
    )
    message( STATUS "source package for ${RELEASE} generated.")
    
endforeach(RELEASE ${CPACK_DEBIAN_DISTRIBUTION_RELEASES})

##############################################################################
# dput ppa:your-lp-id/ppa <source.changes>
message( STATUS "DebSourcePPA.cmake DONE.")

