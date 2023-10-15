cmake_minimum_required(VERSION 3.10)

project(draw_triangle_in_framebuffer
    VERSION 1.0.0)

add_executable(draw_triangle_in_framebuffer
    main.cpp
    )

add_library(draw_triangle_in_framebuffer_compiler_flags INTERFACE)
target_compile_features(draw_triangle_in_framebuffer_compiler_flags INTERFACE cxx_std_11)

