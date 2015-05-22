cmake_minimum_required (VERSION 2.6)
add_executable (plugin-test-loader
    plugin-test-loader.cpp
    )
target_link_libraries (plugin-test-loader
    dl
    )
