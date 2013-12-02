include_directories (
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_BINARY_DIR}/include
  ${Boost_INCLUDE_DIRS}
  ${GTEST_INCLUDE_DIR}
)

link_directories(
  ${Boost_LIBRARY_DIRS}
)

configure_file (test_config.h.in ${PROJECT_BINARY_DIR}/test_config.h)

execute_process(COMMAND cmake -E remove_directory ${CMAKE_BINARY_DIR}/test_results)
execute_process(COMMAND cmake -E make_directory ${CMAKE_BINARY_DIR}/test_results)
include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory(integration)
add_subdirectory(performance)
