add_library(FpsDigits fps_digits.cpp)

target_include_directories(FpsDigits INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(FpsDigits PRIVATE cxx_std_11)

