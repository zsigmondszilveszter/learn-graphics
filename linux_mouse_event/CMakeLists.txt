cmake_minimum_required(VERSION 3.10)

project(linux_mouse_event
    VERSION 1.0.1)

#compile commmands 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")


add_executable(linux_mouse_event
    main.cpp
)

# common compiler flags across the project, which can be used by libraries
add_library(linux_mouse_event_compiler_flags INTERFACE)
target_compile_features(linux_mouse_event_compiler_flags INTERFACE cxx_std_17)

#
add_subdirectory(../../cpp_libraries/cli_args_szilv cli_args_szilv)
target_link_libraries(linux_mouse_event PRIVATE CliArgsSzilv)

#
add_subdirectory(../../cpp_libraries/linux_mouse_event_reader linux_mouse_event_reader)
target_link_libraries(linux_mouse_event PUBLIC MouseEventReader)

