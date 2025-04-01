# Add the test executable
add_executable(run_tests test_eigen.cpp)

# Link against GoogleTest and Eigen
target_link_libraries(run_tests PRIVATE GTest::gtest_main Eigen3::Eigen)

# Include GoogleTest in the test run
include(GoogleTest)
gtest_discover_tests(run_tests) 