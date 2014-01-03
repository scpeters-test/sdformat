include (${sdf_cmake_dir}/SDFUtils.cmake)

link_directories(
  ${PROJECT_BINARY_DIR}/test
)

set (sources
  Assert.cc
  Console.cc
  Converter.cc
  Exception.cc
  parser.cc
  Param.cc
  SDF.cc
  SDFHelper.cc
)

set (gtest_sources
  SDF_TEST.cc
  Console_TEST.cc
  Converter_TEST.cc
)
sdf_build_tests(${gtest_sources})

sdf_add_library(sdformat ${sources})
target_link_libraries(sdformat
  ${tinyxml_libraries}
  ${Boost_LIBRARIES}
)

sdf_install_library(sdformat)
