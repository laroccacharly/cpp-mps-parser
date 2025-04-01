cmake_minimum_required(VERSION 3.14)
project(mps_parser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Eigen
find_package(Eigen3 3.3 REQUIRED CONFIG) # Assumes Eigen is installed or discoverable

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Add tests subdirectory
add_subdirectory(tests)

# Link Eigen to targets that need it (example in tests/CMakeLists.txt)
# target_link_libraries(your_target PRIVATE Eigen3::Eigen GTest::gtest_main) 