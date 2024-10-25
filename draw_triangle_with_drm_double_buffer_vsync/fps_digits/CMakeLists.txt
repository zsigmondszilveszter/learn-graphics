add_library(FpsDigits fps_digits.cpp)
target_include_directories(FpsDigits
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
target_link_libraries(FpsDigits PUBLIC draw_triangle_with_drm_double_buffer_vsync_compiler_flags)

