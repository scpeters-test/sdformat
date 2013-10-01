cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include (FindPkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(SDFORMAT sdformat)
endif()

include_directories(${SDFORMAT_INCLUDE_DIRS})
link_directories(${SDFORMAT_LIBRARY_DIRS})

add_executable(parse_file main.cc)
target_link_libraries(parse_file ${SDFORMAT_LIBRARIES})
