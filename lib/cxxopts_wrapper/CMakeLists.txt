add_library(CxxOptsWrapper cxxopts_wrapper.cpp)

target_include_directories(CxxOptsWrapper INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(CxxOptsWrapper PRIVATE cxx_std_11)

# cxxopts argument parser library
add_subdirectory(../../libraries_external/cxxopts  cxxopts)
target_link_libraries(CxxOptsWrapper PRIVATE cxxopts)
