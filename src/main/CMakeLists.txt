cmake_minimum_required (VERSION 2.6)
add_executable (fasmio
    main.cpp
    )
include_directories (
    ..
    )
target_link_libraries (fasmio
    fcgisvr
    httpclient
    container
    common
    fiberenv
    pthreadenv
    runtimeenv
    pthread
    uuid
    dl
    )
