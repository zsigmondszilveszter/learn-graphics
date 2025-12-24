cmake_minimum_required(VERSION 3.12)

project(draw_triangle_in_framebuffer_threads2
    VERSION 1.0.0)

add_executable(draw_triangle_in_framebuffer_threads2
    main.cpp
    )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic")

#
add_library(draw_triangle_in_framebuffer_threads2_compiler_flags INTERFACE)
target_compile_features(draw_triangle_in_framebuffer_threads2_compiler_flags INTERFACE cxx_std_20)

add_subdirectory(geometry)
target_link_libraries(draw_triangle_in_framebuffer_threads2 PUBLIC BaseGeometry Triangle)

add_subdirectory(line_drawer)
target_link_libraries(draw_triangle_in_framebuffer_threads2 PUBLIC LineDrawer)

add_subdirectory(fps_digits)
target_link_libraries(draw_triangle_in_framebuffer_threads2 PUBLIC FpsDigits)
