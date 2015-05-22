cmake_minimum_required (VERSION 2.6)
add_library (runtimeenv
    abs-time.cpp
    interlocked.cpp
    )
include_directories (
    ..
    )
add_subdirectory (fiber-env)
add_subdirectory (pthread-env)
