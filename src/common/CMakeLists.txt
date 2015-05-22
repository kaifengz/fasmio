cmake_minimum_required (VERSION 2.6)
add_library (common
    ini-reader.cpp
    list-dir.cpp
    logger.cpp
    module-logger.cpp
    ranged-istream.cpp
    stream-adaptor.cpp
    string.cpp
    task-scheduler.cpp
    thread-pool.cpp
    )
include_directories (
    ..
    )
