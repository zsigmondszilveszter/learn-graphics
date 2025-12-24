cmake_minimum_required(VERSION 3.10)

project(draw_triangle_with_drm_mouse_input
    VERSION 1.0.0)

#compile commmands 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_executable(draw_triangle_with_drm_mouse_input
    main.cpp
)
target_compile_features(draw_triangle_with_drm_mouse_input PRIVATE cxx_std_17)


add_subdirectory($ENV{HOME}/prog/practice/cpp_libraries/cli_args_szilv cli_args_szilv)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE CliArgsSzilv)

#
add_subdirectory($ENV{HOME}/prog/practice/cpp_libraries/linux_mouse_event_reader  mouse_event_reader)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE MouseEventReader)

#
add_subdirectory(../../lib/drm_util  drm_util)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE DrmUtil)

add_subdirectory(../../lib/base_geometry  base_geometry)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE BaseGeometry)

add_subdirectory(../../lib/2D_triangle  2D_triangle)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE 2D_triangle)

add_subdirectory(../../lib/2D_line_drawer  2D_line_drawer)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE 2D_line_drawer)

add_subdirectory(../../lib/fps_digits  fps_digits)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE FpsDigits)

add_subdirectory(../../lib/tools tools)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE Tools)

