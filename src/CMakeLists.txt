# Create a library from the source files
add_library(mps_lib
    mps_reader.cpp
    mps_reader.h
)

# Make the header files available to other targets
target_include_directories(mps_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}) 