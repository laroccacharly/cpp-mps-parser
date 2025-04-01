find_package(GTest REQUIRED)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(Arrow REQUIRED)
find_package(Parquet REQUIRED)

# Add test executable
add_executable(mps_tests
    test_mps.cpp
    test_eigen.cpp
    test_parquet.cpp
)

# Link against Google Test and our library
target_link_libraries(mps_tests
    PRIVATE
    GTest::GTest
    GTest::Main
    mps_parser
    Eigen3::Eigen
    Arrow::arrow_shared
    Parquet::parquet_shared
)

# Enable testing
enable_testing()
add_test(NAME mps_tests COMMAND mps_tests) 