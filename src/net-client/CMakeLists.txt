cmake_minimum_required (VERSION 2.6)
add_library (httpclient
    conn-base.cpp
    fcgi-client.cpp
    fcgi-conn.cpp
    http-client.cpp
    http-conn.cpp
    http-header-parser.cpp
    )
include_directories (
    ..
    )
