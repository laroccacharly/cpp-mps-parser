# Add the test executable
add_executable(run_tests 
    test_eigen.cpp
    test_mps.cpp
)

# Link against GoogleTest, Eigen, and our MPS library
target_link_libraries(run_tests PRIVATE 
    GTest::gtest_main 
    Eigen3::Eigen
    mps_lib
)

# Include GoogleTest in the test run
include(GoogleTest)
gtest_discover_tests(run_tests) 