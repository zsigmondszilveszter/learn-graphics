cmake_minimum_required(VERSION 3.10)

project(sdl_framebuffer_1
    VERSION 1.0.0)

#compile commmands 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_executable(sdl_framebuffer_1
    main.cpp
    )

add_library(sdl_framebuffer_1_compiler_flags INTERFACE)
target_compile_features(sdl_framebuffer_1_compiler_flags INTERFACE cxx_std_20)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)


# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in the SDL directory
add_subdirectory(SDL EXCLUDE_FROM_ALL)

# Link to the actual SDL3 library.
target_link_libraries(sdl_framebuffer_1 PRIVATE SDL3::SDL3)
