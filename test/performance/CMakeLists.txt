set(TEST_TYPE "PERFORMANCE")

link_directories(${PROJECT_BINARY_DIR}/test)

sdf_build_tests(${tests})
