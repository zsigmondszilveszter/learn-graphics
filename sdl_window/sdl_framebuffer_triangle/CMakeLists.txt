cmake_minimum_required(VERSION 3.13)

project(sdl_framebuffer_triangle
    VERSION 1.0.0)

set(SDL_VIDEO ON CACHE BOOL "" FORCE)
set(SDL_EVENTS ON CACHE BOOL "" FORCE)
set(SDL_TIMER ON CACHE BOOL "" FORCE)
set(SDL_RENDER ON CACHE BOOL "" FORCE)

set(SDL_AUDIO OFF CACHE BOOL "" FORCE)
set(SDL_CAMERA OFF CACHE BOOL "" FORCE)
set(SDL_JOYSTICK OFF CACHE BOOL "" FORCE)
set(SDL_HAPTIC OFF CACHE BOOL "" FORCE)
set(SDL_SENSOR OFF CACHE BOOL "" FORCE)
set(SDL_POWER OFF CACHE BOOL "" FORCE)
set(SDL_LOCALE OFF CACHE BOOL "" FORCE)
set(SDL_FILESYSTEM OFF CACHE BOOL "" FORCE)

set(SDL_GPU OFF CACHE BOOL "" FORCE)
set(SDL_METAL OFF CACHE BOOL "" FORCE)
set(SDL_VULKAN OFF CACHE BOOL "" FORCE)
set(SDL_OPENGL OFF CACHE BOOL "" FORCE)

set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in the SDL directory
add_subdirectory(SDL EXCLUDE_FROM_ALL)

#compile commmands 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_executable(sdl_framebuffer_triangle
    main.cpp
    )

# tell CMake to embed the shared libraries path relative to the executable
#set_target_properties(sdl_framebuffer_triangle PROPERTIES
#    BUILD_WITH_INSTALL_RPATH TRUE
#    INSTALL_RPATH "$ORIGIN"
#    INSTALL_RPATH_USE_LINK_PATH TRUE
#)
if(APPLE)
    set_target_properties(sdl_framebuffer_triangle PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@executable_path"
    )
elseif(UNIX)
    set_target_properties(sdl_framebuffer_triangle PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "$ORIGIN"
    )
endif()

target_compile_features(sdl_framebuffer_triangle PRIVATE cxx_std_17)

# Link to libraries.
target_link_libraries(sdl_framebuffer_triangle PRIVATE SDL3::SDL3)

add_subdirectory($ENV{HOME}/prog/practice/cpp_libraries/cli_args_szilv cli_args_szilv)
target_link_libraries(sdl_framebuffer_triangle PRIVATE CliArgsSzilv)

add_subdirectory(../../lib/base_geometry  base_geometry)
target_link_libraries(sdl_framebuffer_triangle PRIVATE BaseGeometry)

add_subdirectory(../../lib/2D_triangle 2D_triangle)
target_link_libraries(sdl_framebuffer_triangle PRIVATE 2D_triangle)

add_subdirectory(../../lib/2D_line_drawer 2D_line_drawer)
target_link_libraries(sdl_framebuffer_triangle PRIVATE 2D_line_drawer)
