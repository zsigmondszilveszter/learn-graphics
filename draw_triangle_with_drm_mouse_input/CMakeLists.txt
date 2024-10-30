cmake_minimum_required(VERSION 3.10)

project(draw_triangle_with_drm_mouse_input
    VERSION 1.0.0)

#compile commmands 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_executable(draw_triangle_with_drm_mouse_input
    main.cpp
)
target_compile_features(draw_triangle_with_drm_mouse_input PRIVATE cxx_std_17)


add_subdirectory(../lib/cxxopts_wrapper cxxopts_wrapper)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE CxxOptsWrapper)

#
add_subdirectory(../lib/mouse_event_reader  mouse_event_reader)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE MouseEventReader)

#
add_subdirectory(../lib/drm_util  drm_util)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE DrmUtil)

add_subdirectory(../lib/base_geometry  base_geometry)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE BaseGeometry)

add_subdirectory(../lib/triangle  triangle)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE Triangle)

add_subdirectory(../lib/line_drawer  line_drawer)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE LineDrawer)

add_subdirectory(../lib/fps_digits  fps_digits)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE FpsDigits)

add_subdirectory(../lib/tools tools)
target_link_libraries(draw_triangle_with_drm_mouse_input PRIVATE Tools)

