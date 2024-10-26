cmake_minimum_required(VERSION 3.10)

project(linux_mouse_event
    VERSION 1.0.0)

#compile commmands 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")


add_executable(linux_mouse_event
    main.cpp
)

# common compiler flags across the project, which can be used by libraries
add_library(linux_mouse_event_compiler_flags INTERFACE)
target_compile_features(linux_mouse_event_compiler_flags INTERFACE cxx_std_11)

# cxxopts argument parser library
add_subdirectory(../libraries_external/cxxopts cxxopts)
target_link_libraries(linux_mouse_event PUBLIC cxxopts)

#
add_subdirectory(../lib/mouse_event_reader mouse_event_reader)
target_link_libraries(linux_mouse_event PUBLIC MouseEventReader)

