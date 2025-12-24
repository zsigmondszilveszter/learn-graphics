cmake_minimum_required(VERSION 3.12)

project(draw_with_drm2
    VERSION 1.0.0)

add_executable(draw_with_drm2
    main.cpp
    )

# include libdrm
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules/")
find_package(Libdrm REQUIRED)
message(STATUS "libdrm_INCLUDE_DIRS: ${Libdrm_INCLUDE_DIRS}")
target_include_directories(draw_with_drm2 PRIVATE ${Libdrm_INCLUDE_DIRS})

#
add_subdirectory(drm_util)
target_link_libraries(draw_with_drm2 PUBLIC DrmUtil)

#
add_library(draw_with_drm2_compiler_flags INTERFACE)
target_compile_features(draw_with_drm2_compiler_flags INTERFACE cxx_std_20)

