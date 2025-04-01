find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(Arrow REQUIRED)
find_package(Parquet REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(mps_parser
    mps_reader.cpp
    mps_reader.h
    mps_parser.cpp
    mps_parser.h
    lp_data.cpp
    lp_data.h
    parquet_writer.cpp
    parquet_writer.h
)

target_link_libraries(mps_parser 
    PUBLIC 
        Eigen3::Eigen
        Arrow::arrow_shared
        Parquet::parquet_shared
        nlohmann_json::nlohmann_json
)
target_include_directories(mps_parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}) 