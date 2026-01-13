cmake_minimum_required(VERSION 3.10)

project(sdl_framebuffer_triangle
    VERSION 1.0.0)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in the SDL directory
add_subdirectory(SDL EXCLUDE_FROM_ALL)

#compile commmands 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_executable(sdl_framebuffer_triangle
    main.cpp
    )

target_compile_features(sdl_framebuffer_triangle PRIVATE cxx_std_17)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

# Link to the actual SDL3 library.
target_link_libraries(sdl_framebuffer_triangle PRIVATE SDL3::SDL3)

add_subdirectory($ENV{HOME}/prog/practice/cpp_libraries/cli_args_szilv cli_args_szilv)
target_link_libraries(sdl_framebuffer_triangle PRIVATE CliArgsSzilv)

add_subdirectory(../../lib/base_geometry  base_geometry)
target_link_libraries(sdl_framebuffer_triangle PRIVATE BaseGeometry)

add_subdirectory(../../lib/2D_triangle  2D_triangle)
target_link_libraries(sdl_framebuffer_triangle PRIVATE 2D_triangle)

add_subdirectory(../../lib/2D_line_drawer  2D_line_drawer)
target_link_libraries(sdl_framebuffer_triangle PRIVATE 2D_line_drawer)
