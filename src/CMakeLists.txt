find_package(Eigen3 3.3 REQUIRED NO_MODULE)

add_library(mps_parser
    mps_reader.cpp
    mps_reader.h
    mps_parser.cpp
    mps_parser.h
    lp_data.cpp
    lp_data.h
)

target_link_libraries(mps_parser PUBLIC Eigen3::Eigen)
target_include_directories(mps_parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}) 