cmake_minimum_required(VERSION 3.12)

project(draw_with_drm
    VERSION 1.0.0)

add_executable(draw_with_drm
    main.cpp
    )

# include libdrm
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(Libdrm REQUIRED)
target_include_directories(draw_with_drm PRIVATE ${Libdrm_INCLUDE_DIRS})
target_link_libraries(draw_with_drm PRIVATE ${Libdrm_LIBRARIES})

#
add_library(draw_with_drm_compiler_flags INTERFACE)
target_compile_features(draw_with_drm_compiler_flags INTERFACE cxx_std_20)

