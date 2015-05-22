cmake_minimum_required (VERSION 2.6)
add_library (fasmiosdk
    plugin.cpp
    service-base.cpp
    service-factory-impl.cpp
    task-base.cpp
    )
include_directories (
    ../..
    )
