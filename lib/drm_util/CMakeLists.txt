add_library(DrmUtil drm_util.cpp)

target_compile_features(DrmUtil PRIVATE cxx_std_11)
target_include_directories(DrmUtil INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# add the location of a cmake module to the cmake module PATH. This module is helps to locate the system wide installed libdrm
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules/") 
find_package(Libdrm REQUIRED)
message(STATUS "libdrm_INCLUDE_DIRS: ${Libdrm_INCLUDE_DIRS}")

target_include_directories(DrmUtil PUBLIC ${Libdrm_INCLUDE_DIRS})
target_link_libraries(DrmUtil PRIVATE ${Libdrm_LIBRARIES})
