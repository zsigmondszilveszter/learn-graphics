add_library(2D_triangle 2D_triangle.cpp)

target_compile_features(2D_triangle PRIVATE cxx_std_11)
target_include_directories(2D_triangle INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} )

target_link_libraries(2D_triangle PRIVATE BaseGeometry)
