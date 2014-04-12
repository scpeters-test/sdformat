set(TEST_TYPE "PERFORMANCE")

set(tests
)

link_directories(${PROJECT_BINARY_DIR}/test)

rml_build_tests(${tests})
