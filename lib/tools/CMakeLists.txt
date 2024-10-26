add_library(Tools tools.cpp)

target_include_directories(Tools INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(Tools PRIVATE cxx_std_11)

