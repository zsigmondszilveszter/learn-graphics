add_library(LineDrawer line_drawer.cpp)
target_include_directories(LineDrawer
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
target_link_libraries(LineDrawer PUBLIC draw_triangle_with_drm_double_buffer_compiler_flags)

target_include_directories(LineDrawer PRIVATE ${Libdrm_INCLUDE_DIRS})

target_link_libraries(LineDrawer PUBLIC BaseGeometry)
target_link_libraries(LineDrawer PUBLIC Triangle)
target_link_libraries(LineDrawer PUBLIC DrmUtil)

