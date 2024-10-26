add_library(LineDrawer line_drawer.cpp)

target_compile_features(LineDrawer PRIVATE cxx_std_11)
target_include_directories(LineDrawer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(LineDrawer PRIVATE BaseGeometry)
target_link_libraries(LineDrawer PRIVATE Triangle)
target_link_libraries(LineDrawer PRIVATE DrmUtil)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(LineDrawer PRIVATE Threads::Threads)
