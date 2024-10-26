cmake_minimum_required(VERSION 3.10)

project(learn_glfw_1
    VERSION 1.0.0)

#compile commmands 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_executable(learn_glfw_1
    learn_glfw_1.cpp
    glad/src/gles2.c
    )
target_include_directories(learn_glfw_1 
    PUBLIC glad/include linmath)

add_library(learn_glfw_1_compiler_flags INTERFACE)
target_compile_features(learn_glfw_1_compiler_flags INTERFACE cxx_std_11)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)


# GLFW3
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw-3.3.8)
target_link_libraries(learn_glfw_1 glfw)


