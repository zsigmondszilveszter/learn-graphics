cmake_minimum_required(VERSION 3.10)

project(draw_triangle_with_drm_double_buffer_vsync
    VERSION 1.0.0)

#compile commmands 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_executable(draw_triangle_with_drm_double_buffer_vsync
    main.cpp
    )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic")

#
add_library(draw_triangle_with_drm_double_buffer_vsync_compiler_flags INTERFACE)
target_compile_features(draw_triangle_with_drm_double_buffer_vsync_compiler_flags INTERFACE cxx_std_11)

# include libdrm
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules/")
find_package(Libdrm REQUIRED)
message(STATUS "libdrm_INCLUDE_DIRS: ${Libdrm_INCLUDE_DIRS}")
target_include_directories(draw_triangle_with_drm_double_buffer_vsync PRIVATE ${Libdrm_INCLUDE_DIRS})

#
add_subdirectory(drm_util)
target_link_libraries(draw_triangle_with_drm_double_buffer_vsync PUBLIC DrmUtil)

add_subdirectory(geometry)
target_link_libraries(draw_triangle_with_drm_double_buffer_vsync PUBLIC BaseGeometry Triangle)

add_subdirectory(line_drawer)
target_link_libraries(draw_triangle_with_drm_double_buffer_vsync PUBLIC LineDrawer)

add_subdirectory(fps_digits)
target_link_libraries(draw_triangle_with_drm_double_buffer_vsync PUBLIC FpsDigits)
