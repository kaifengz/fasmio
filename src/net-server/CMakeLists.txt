cmake_minimum_required (VERSION 2.6)
add_library (fcgisvr
    base-net-server.cpp
    fcgi-conn.cpp
    fcgi-server.cpp
    )
include_directories (
    ..
    )
