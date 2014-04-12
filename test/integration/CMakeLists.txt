set(TEST_TYPE "INTEGRATION")

set(tests
  audio.cc
)

link_directories(${PROJECT_BINARY_DIR}/test)

rml_build_tests(${tests})
