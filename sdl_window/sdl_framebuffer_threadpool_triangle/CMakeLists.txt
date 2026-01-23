cmake_minimum_required(VERSION 3.13)

project(sdl_framebuffer_threadpool_triangle
    VERSION 1.0.0)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

#compile commmands 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_executable(sdl_framebuffer_threadpool_triangle
    main.cpp )

# tell CMake to embed the shared libraries path relative to the executable
if(APPLE)
    set_target_properties(sdl_framebuffer_threadpool_triangle PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@executable_path"
    )
elseif(UNIX)
    set_target_properties(sdl_framebuffer_threadpool_triangle PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "$ORIGIN"
        BUILD_RPATH "$ORIGIN"
    )
endif()

target_compile_features(sdl_framebuffer_threadpool_triangle PRIVATE cxx_std_17)

# ---- SDL3 
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_SHARED ON CACHE BOOL "" FORCE) # (default)
add_subdirectory(../../vendor/sdl EXCLUDE_FROM_ALL sdl3)
target_link_libraries(sdl_framebuffer_threadpool_triangle PRIVATE SDL3::SDL3)

# ---- oneTBB
#find_package(TBB REQUIRED)
set(TBB_TEST OFF CACHE BOOL "" FORCE)
set(TBB_EXAMPLES OFF CACHE BOOL "" FORCE)
set(TBB_STRICT OFF CACHE BOOL "" FORCE)
set(TBB_BUILD_SHARED OFF CACHE BOOL "" FORCE) 
add_subdirectory(../../vendor/oneTBB oneTBB)
target_link_libraries(sdl_framebuffer_threadpool_triangle PRIVATE TBB::tbb)

add_subdirectory(../../../cpp_libraries/cli_args_szilv cli_args_szilv)
target_link_libraries(sdl_framebuffer_threadpool_triangle PRIVATE CliArgsSzilv)

add_subdirectory(../../lib/base_geometry  base_geometry)
target_link_libraries(sdl_framebuffer_threadpool_triangle PRIVATE BaseGeometry)

add_subdirectory(../../lib/2D_triangle 2D_triangle)
target_link_libraries(sdl_framebuffer_threadpool_triangle PRIVATE 2D_triangle)

