add_library(2D_line_drawer 2D_line_drawer.cpp)

target_compile_features(2D_line_drawer PRIVATE cxx_std_11)
target_include_directories(2D_line_drawer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(2D_line_drawer PRIVATE BaseGeometry)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(2D_line_drawer PRIVATE Threads::Threads)
