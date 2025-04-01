find_package(GTest REQUIRED)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# Add test executable
add_executable(mps_tests
    test_mps.cpp
    test_eigen.cpp
)

# Link against Google Test and our library
target_link_libraries(mps_tests
    PRIVATE
    GTest::GTest
    GTest::Main
    mps_parser
    Eigen3::Eigen
)

# Enable testing
enable_testing()
add_test(NAME mps_tests COMMAND mps_tests) 