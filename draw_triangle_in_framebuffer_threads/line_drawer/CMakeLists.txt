add_library(LineDrawer line_drawer.cpp)
target_include_directories(LineDrawer
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
target_link_libraries(LineDrawer PUBLIC draw_triangle_in_framebuffer_threads_compiler_flags)

target_link_libraries(LineDrawer PUBLIC BaseGeometry)
target_link_libraries(LineDrawer PUBLIC Triangle)

