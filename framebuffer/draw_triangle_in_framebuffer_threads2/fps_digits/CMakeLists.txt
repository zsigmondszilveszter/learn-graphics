add_library(FpsDigits fps_digits.cpp)
target_include_directories(FpsDigits
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
target_link_libraries(FpsDigits PUBLIC draw_triangle_in_framebuffer_threads2_compiler_flags)

