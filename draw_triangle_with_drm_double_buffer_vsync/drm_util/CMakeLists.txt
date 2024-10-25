add_library(DrmUtil drm_util.cpp)

target_include_directories(DrmUtil
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
target_link_libraries(DrmUtil PUBLIC draw_triangle_with_drm_double_buffer_vsync_compiler_flags )

target_include_directories(DrmUtil PRIVATE ${Libdrm_INCLUDE_DIRS})
target_link_libraries(DrmUtil PRIVATE ${Libdrm_LIBRARIES})
