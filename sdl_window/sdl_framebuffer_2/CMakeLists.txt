cmake_minimum_required(VERSION 3.10)

project(sdl_framebuffer_2
    VERSION 1.0.0)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in the SDL directory
add_subdirectory(SDL EXCLUDE_FROM_ALL)

#compile commmands 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_executable(sdl_framebuffer_2
    main.cpp
    )

# tell CMake to embed the shared libraries path relative to the executable
if(APPLE)
    set_target_properties(sdl_framebuffer_2 PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@executable_path"
    )
elseif(UNIX)
    set_target_properties(sdl_framebuffer_2 PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "$ORIGIN"
    )
endif()

add_library(sdl_framebuffer_2_compiler_flags INTERFACE)
target_compile_features(sdl_framebuffer_2 PRIVATE cxx_std_17)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

# Link to the actual SDL3 library.
target_link_libraries(sdl_framebuffer_2 PRIVATE SDL3::SDL3)
