cmake_minimum_required(VERSION 3.12)

project(draw_triangle_with_drm
    VERSION 1.0.0)

add_executable(draw_triangle_with_drm
    main.cpp
    )

#
add_library(draw_triangle_with_drm_compiler_flags INTERFACE)
target_compile_features(draw_triangle_with_drm_compiler_flags INTERFACE cxx_std_20)

# include libdrm
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules/")
find_package(Libdrm REQUIRED)
message(STATUS "libdrm_INCLUDE_DIRS: ${Libdrm_INCLUDE_DIRS}")
target_include_directories(draw_triangle_with_drm PRIVATE ${Libdrm_INCLUDE_DIRS})

#
add_subdirectory(drm_util)
target_link_libraries(draw_triangle_with_drm PUBLIC DrmUtil)

add_subdirectory(geometry)
target_link_libraries(draw_triangle_with_drm  PUBLIC BaseGeometry Triangle)

add_subdirectory(line_drawer)
target_link_libraries(draw_triangle_with_drm   PUBLIC LineDrawer)
