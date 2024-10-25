add_library(DrmUtil drm_util.cpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

target_include_directories(DrmUtil
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
target_link_libraries(DrmUtil PUBLIC draw_triangle_with_drm_compiler_flags )

target_include_directories(DrmUtil PRIVATE ${Libdrm_INCLUDE_DIRS})
target_link_libraries(DrmUtil PRIVATE ${Libdrm_LIBRARIES})
