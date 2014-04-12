cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

if(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
  CMAKE_POLICY(SET CMP0004 NEW)
endif(COMMAND CMAKE_POLICY)
enable_testing()

# with -fPIC
if(UNIX AND NOT WIN32)
  set (CMAKE_INSTALL_PREFIX "/usr" CACHE STRING "Install Prefix")
 find_program(CMAKE_UNAME uname /bin /usr/bin /usr/local/bin )
 if(CMAKE_UNAME)
   exec_program(uname ARGS -m OUTPUT_VARIABLE CMAKE_SYSTEM_PROCESSOR)
   set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR} CACHE INTERNAL
     "processor type (i386 and x86_64)")
   if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
     ADD_DEFINITIONS(-fPIC)
   endif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
 endif(CMAKE_UNAME)
endif()

project (RML)
string (TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

set (RML_MAJOR_VERSION 0)
set (RML_MINOR_VERSION 1)
set (RML_PATCH_VERSION 0)

set (RML_VERSION ${RML_MAJOR_VERSION}.${RML_MINOR_VERSION})
set (RML_VERSION_FULL ${RML_MAJOR_VERSION}.${RML_MINOR_VERSION}.${RML_PATCH_VERSION})

message (STATUS "${PROJECT_NAME} version ${RML_VERSION_FULL}")
set (CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

# Include GNUInstallDirs to get canonical paths
include(GNUInstallDirs)

# Default test type for test all over source
set(TEST_TYPE "UNIT")

# developer's option to cache PKG_CONFIG_PATH and
# LD_LIBRARY_PATH for local installs
if(PKG_CONFIG_PATH)
  set (ENV{PKG_CONFIG_PATH} ${PKG_CONFIG_PATH}:$ENV{PKG_CONFIG_PATH})
endif()
if(LD_LIBRARY_PATH)
  set (ENV{LD_LIBRARY_PATH} ${LD_LIBRARY_PATH}:$ENV{LD_LIBRARY_PATH})
endif()

set (INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/rml-${RML_VERSION}/rml") 
set (LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR} CACHE STRING "Installation directory for libraries (relative to CMAKE_INSTALL_PREFIX)") 
set (BIN_INSTALL_DIR ${CMAKE_INSTALL_BINDIR} CACHE STRING "Installation directory for binaries (relative to CMAKE_INSTALL_PREFIX)") 

set (USE_FULL_RPATH OFF CACHE BOOL "Set to true to enable full rpath")


if (USE_FULL_RPATH)
  # use, i.e. don't skip the full RPATH for the build tree
  set(CMAKE_SKIP_BUILD_RPATH  FALSE)
  
  # when building, don't use the install RPATH already
  # (but later on when installing)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
  
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}")
  
  # add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  
  # the RPATH to be used when installing, but only if its not a system directory
  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}" isSystemDir)
  if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}")
  endif("${isSystemDir}" STREQUAL "-1")
endif()

set (BUILD_RML ON CACHE INTERNAL "Build RML" FORCE)
set (build_errors "" CACHE INTERNAL "build errors" FORCE)
set (build_warnings "" CACHE INTERNAL "build warnings" FORCE)

set (rml_cmake_dir ${PROJECT_SOURCE_DIR}/cmake CACHE PATH
     "Location of CMake scripts")

 message (STATUS "\n\n====== Finding 3rd Party Packages ======")
include (${rml_cmake_dir}/SearchForStuff.cmake)
message (STATUS "----------------------------------------\n")

#####################################
# Build type cflags
set (CMAKE_C_FLAGS_ALL " -Wall -Wextra -Wno-long-long -Wno-unused-value -Wno-unused-value -Wno-unused-value -Wno-unused-value -Wfloat-equal -Wshadow -Winit-self -Wswitch-default -Wmissing-include-dirs -pedantic" CACHE INTERNAL "General C Flags")

# Check and add visibility hidden by default
if (UNIX)
  check_gcc_visibility()
  if (GCC_SUPPORTS_VISIBILITY)
         SET(CMAKE_C_FLAGS_ALL "${CMAKE_C_FLAGS_ALL} -fvisibility=hidden")
  endif()
endif()

#####################################
# Build type link flags

# USE_UPSTREAM_CFLAGS (default TRUE)
# Use our own defined flags for building types
if(NOT DEFINED USE_UPSTREAM_CFLAGS OR USE_UPSTREAM_CFLAGS)
  message(STATUS "Using upstream CFlags")
  if (NOT APPLE)
    # -s doesn't work with default osx compiler clang, alternative:
    # http://stackoverflow.com/questions/6085491/gcc-vs-clang-symbol-strippingu
    set (CMAKE_C_FLAGS_RELEASE " -s ")
  endif()
  set (CMAKE_C_FLAGS_RELEASE " ${CMAKE_C_FLAGS_RELEASE} -O2 -DNDEBUG"
    CACHE INTERNAL "C Flags for release" FORCE)
  set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
  
  set (CMAKE_C_FLAGS_DEBUG " -ggdb3"
    CACHE INTERNAL "C Flags for debug" FORCE)
  set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
endif()

#####################################
# Set the default build type
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "RELEASE" CACHE STRING 
    "Choose the type of build, options are: debug release profile check" FORCE)
endif (NOT CMAKE_BUILD_TYPE)
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)

#####################################
# Set all the global build flags
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}} ${CMAKE_C_FLAGS_ALL}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS_ALL}")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
  set (BUILD_TYPE_RELEASE TRUE)
else()
  set (BUILD_TYPE_RELEASE FALSE)
endif()

if ( "${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
  set (BUILD_TYPE_DEBUG TRUE)
else()
  set (BUILD_TYPE_DEBUG FALSE)
endif()


#################################################
# OS Specific initialization
if (UNIX)
  rml_setup_unix()
else (WIN32)
  rml_setup_windows()
else (APPLE)
  rml_setup_apple()
endif()

#################################################
# Print warnings and errors
if ( build_warnings )
  message(STATUS "BUILD WARNINGS")
  foreach (msg ${build_warnings})
    message(STATUS ${msg})
  endforeach ()
  message(STATUS "END BUILD WARNINGS\n")
endif (build_warnings)

########### Add uninstall target ###############
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake")

if (build_errors)
  message(STATUS "BUILD ERRORS: These must be resolved before compiling.")
  foreach (msg ${build_errors})
    message(STATUS ${msg})
  endforeach ()
  message(STATUS "END BUILD ERRORS\n")
  message (FATAL_ERROR "Errors encountered in build. Please see the BUILD ERRORS above.")

else (buid_errors)

  ########################################
  # Write the config.h file
  configure_file (${rml_cmake_dir}/rml_config.h.in
    ${PROJECT_BINARY_DIR}/src/rml_config.h)
  rml_install_includes("" ${PROJECT_BINARY_DIR}/src/rml_config.h)

  message (STATUS "C Flags:${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")
  message (STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
  message (STATUS "Install path: ${CMAKE_INSTALL_PREFIX}")
 
  if (BUILD_RML)
    include_directories(include
      ${PROJECT_BINARY_DIR}
      ${SDFormat_INCLUDE_DIRS}
      )

    link_directories(${SDFormat_LIBRARY_DIRS})

    add_subdirectory(test)
    add_subdirectory(src)
    #add_subdirectory(include/rml)
    add_subdirectory(rml)
    add_subdirectory(doc)
  endif(BUILD_RML)

  ########################################
  # Make the package config file
  configure_file(${CMAKE_SOURCE_DIR}/cmake/rml_pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/rml.pc @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/rml.pc DESTINATION
    ${LIB_INSTALL_DIR}/pkgconfig COMPONENT pkgconfig)

  ########################################
  # Make the cmake config files
  set(PKG_NAME ${PROJECT_NAME})
  set(PKG_LIBRARIES rml)
  set(PKG_DEPENDS Boost)
  set(cmake_conf_file "cmake/rml_config.cmake")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${cmake_conf_file}.in" "${CMAKE_CURRENT_BINARY_DIR}/RMLConfig.cmake" @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/RMLConfig.cmake DESTINATION ${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME_LOWER}/ COMPONENT cmake)

  ########################################
  # Package Creation: 
  include (${rml_cmake_dir}/rml_cpack.cmake)
  set (CPACK_PACKAGE_VERSION "${RML_VERSION_FULL}")
  set (CPACK_PACKAGE_VERSION_MAJOR "${RML_MAJOR_VERSION}")
  set (CPACK_PACKAGE_VERSION_MINOR "${RML_MINOR_VERSION}")
  set (CPACK_PACKAGE_VERSION_PATCH "${RML_PATCH_VERSION}")

  if (CPACK_GENERATOR)
    message(STATUS "Found CPack generators: ${CPACK_GENERATOR}")

    configure_file("${rml_cmake_dir}/cpack_options.cmake.in"
                   ${RML_CPACK_CFG_FILE} @ONLY)
    set(CPACK_PROJECT_CONFIG_FILE ${RML_CPACK_CFG_FILE})
    include (CPack)
  endif()

  message(STATUS "Configuration successful. Type make to compile RM")
endif(build_errors)

