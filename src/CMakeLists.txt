include (${sdf_cmake_dir}/SDFUtils.cmake)

include_directories(${PROJECT_SOURCE_DIR}/src/urdf)

link_directories(
  ${PROJECT_BINARY_DIR}/test
)

set (sources
  Assert.cc
  Console.cc
  Converter.cc
  Exception.cc
  parser.cc
  parser_urdf.cc
  Param.cc
  SDF.cc
)

add_library(urdfdom_parser STATIC
  urdf/urdf_parser/model.cpp
  urdf/urdf_parser/link.cpp
  urdf/urdf_parser/joint.cpp
  urdf/urdf_parser/pose.cpp
  urdf/urdf_parser/twist.cpp
  urdf/urdf_parser/urdf_model_state.cpp
  urdf/urdf_parser/urdf_sensor.cpp
  urdf/urdf_parser/world.cpp
)

if (UNIX)
  check_gcc_visibility()
  if (GCC_SUPPORTS_VISIBILITY)
     set_target_properties(urdfdom_parser
                           PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")
  endif()
endif()

set (gtest_sources
  SDF_TEST.cc
  Console_TEST.cc
  Converter_TEST.cc
  parser_urdf_TEST.cc
)
sdf_build_tests(${gtest_sources})

sdf_add_library(sdformat ${sources})
target_link_libraries(sdformat
  urdfdom_parser
  ${tinyxml_libraries}
  ${Boost_LIBRARIES}
)
sdf_install_library(sdformat)
