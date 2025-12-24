cmake_minimum_required(VERSION 3.10)

project(learn_linux_framebuffer_device
    VERSION 1.0.0)

add_executable(learn_linux_framebuffer_device
    main.cpp
    )

add_library(learn_linux_framework_device_compiler_flags INTERFACE)
target_compile_features(learn_linux_framework_device_compiler_flags INTERFACE cxx_std_11)

