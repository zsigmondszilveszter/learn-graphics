add_library(Triangle triangle.cpp)

target_compile_features(Triangle PRIVATE cxx_std_11)
target_include_directories(Triangle INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} )

target_link_libraries(Triangle PRIVATE BaseGeometry)
