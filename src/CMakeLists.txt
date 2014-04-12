include (${rml_cmake_dir}/RMLUtils.cmake)

link_directories(
  ${PROJECT_BINARY_DIR}/test
)

set (sources
  RMLHelper.cc
)

set (gtest_sources
)
rml_build_tests(${gtest_sources})

# This variable should contain the name of subdirectory that will be
# used to generate the C++ DOM files.
set(DOM_DIR 1.5)

#################################################
# The following code generates the C++ DOM files and library.

set (SDF_SRCS)
set (SDF_HDRS)
set (SDF_ROOT ${CMAKE_SOURCE_DIR}/rml)

file(GLOB files "${SDF_ROOT}/${DOM_DIR}/*.sdf")

foreach(FIL ${files})
  get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
  get_filename_component(FIL_WE ${FIL} NAME_WE)

  list(APPEND SDF_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.cc")
  list(APPEND SDF_HDRS "${FIL_WE}.hh")
  list(APPEND SDF_HDRS_FINAL "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.hh")

  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.cc"
           "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.hh"
    COMMAND ${SDFormat_SDFC_EXECUTABLE}
    ARGS -s ${SDF_ROOT}/${DOM_DIR} -i ${ABS_FIL} -o ${CMAKE_CURRENT_BINARY_DIR} -n "rml"
    DEPENDS ${ABS_FIL}
    COMMENT "Running C++ SDF compiler on ${FIL}"
    VERBATIM)
endforeach()

set_source_files_properties(${SDF_SRCS} ${SDF_HDRS} PROPERTIES GENERATED TRUE)

rml_add_library(rml ${sources} ${SDF_SRCS})
target_link_libraries(rml
  ${SDFormat_LIBRARIES}
  ${Boost_LIBRARIES}
)

rml_install_library(rml)

#################################################
# Configure the rml.hh file.
set (rml_headers "")
foreach (hdr ${SDF_HDRS})
  APPEND_TO_CACHED_STRING(rml_headers "SDF Headers" "#include <rml/${hdr}>\n")
endforeach()

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/rml.hh.in
  ${CMAKE_CURRENT_BINARY_DIR}/rml.hh)

rml_install_includes("" ${SDF_HDRS_FINAL} RMLHelper.hh
  ${CMAKE_CURRENT_BINARY_DIR}/rml.hh)
