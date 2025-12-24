add_library(BaseGeometry base_geometry.cpp)
target_include_directories(BaseGeometry
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
target_link_libraries(BaseGeometry PUBLIC draw_triangle_with_drm_double_buffer_compiler_flags)

