include (${sdf_cmake_dir}/SDFUtils.cmake)
include (${sdf_cmake_dir}/FindOS.cmake)
include (FindPkgConfig)

########################################
# Find Boost, if not specified manually
include(FindBoost)
find_package(Boost ${MIN_BOOST_VERSION} REQUIRED system filesystem program_options regex iostreams)

if (NOT Boost_FOUND)
  set (BUILD_SDF OFF CACHE INTERNAL "Build SDF" FORCE)
  BUILD_ERROR ("Boost not found. Please install thread signals system filesystem program_options regex boost version ${MIN_BOOST_VERSION} or higher.")
endif() 

#################################################
# Find tinyxml. Only debian distributions package tinyxml with a pkg-config
find_path (tinyxml_include_dir tinyxml.h ${tinyxml_include_dirs} ENV CPATH)
if (NOT tinyxml_include_dir)
  message (STATUS "Looking for tinyxml.h - not found")
  BUILD_ERROR("Missing: tinyxml")
else ()
  message (STATUS "Looking for tinyxml.h - found")
  set (tinyxml_include_dirs ${tinyxml_include_dir} CACHE STRING
    "tinyxml include paths. Use this to override automatic detection.")
  set (tinyxml_libraries "tinyxml" CACHE INTERNAL "tinyxml libraries")
endif ()

#################################################
# Macro to check for visibility capability in compiler
# Original idea from: https://gitorious.org/ferric-cmake-stuff/ 
macro (check_gcc_visibility)
  include (CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-fvisibility=hidden GCC_SUPPORTS_VISIBILITY)
endmacro()

################################################
# Find gtest
if (USE_EXTERNAL_GTEST)
  find_package(GTest)
  if (GTEST_FOUND)
    set(GTEST_LIBRARY ${GTEST_LIBRARIES})
    set(GTEST_MAIN_LIBRARY ${GTEST_MAIN_LIBRARIES})
    set(GTEST_INCLUDE_DIR ${GTEST_INCLUDE_DIRS})
  else()
    BUILD_ERROR("GTest library not found. Please install it to use USE_EXTERNAL_GTEST")
  endif()
else()
  include_directories(${PROJECT_SOURCE_DIR}/test/gtest)
  add_library(gtest STATIC "${PROJECT_SOURCE_DIR}/test/gtest/src/gtest-all.cc")
  add_library(gtest_main STATIC "${PROJECT_SOURCE_DIR}/test/gtest/src/gtest_main.cc")
  target_link_libraries(gtest_main gtest)
  set(GTEST_LIBRARY "${PROJECT_BINARY_DIR}/libgtest.a")
  set(GTEST_MAIN_LIBRARY "${PROJECT_BINARY_DIR}/libgtest_main.a")
  set(GTEST_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/test/gtest/include")
endif()
