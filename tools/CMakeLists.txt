#set (test_sources
#  sdformat_TEST.cc
#)

gz_build_tests(${test_sources})

add_executable(sdformat sdformat.cc)
target_link_libraries(sdformat
 sdformat
 ${gflags_LIBRARIES}
)

#if (NOT WIN32)
#  roffman(sdformat 1)
#endif()

install (PROGRAMS sdformat DESTINATION ${BIN_INSTALL_DIR})

#if (NOT WIN32)
#  manpage(gzprop 1)
#endif()
