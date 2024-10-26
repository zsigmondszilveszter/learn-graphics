add_library(BaseGeometry base_geometry.cpp)

target_include_directories(BaseGeometry INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(BaseGeometry PRIVATE cxx_std_11)
