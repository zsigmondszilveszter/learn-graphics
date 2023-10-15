add_library(Triangle triangle.cpp)
target_include_directories(Triangle
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
target_link_libraries(Triangle PUBLIC draw_triangle_with_drm_compiler_flags)

target_link_libraries(Triangle PUBLIC BaseGeometry)
