cmake_minimum_required (VERSION 2.6)
add_library (container
    container-impl.cpp
    data-types.xsd.cpp
    key-value-impl.cpp
    plugin-impl.cpp
    queue-impl.cpp
    request-impl.cpp
    service-agent-impl.cpp
    stream-impl.cpp
    slot/delegated-query-handler.cpp
    slot/delegated-update-handler.cpp
    slot/delete-slot-handler.cpp
    slot/pop-slot-handler.cpp
    slot/push-slot-handler.cpp
    slot/query-slot-handler.cpp
    slot/slot-handler.cpp
    slot/update-slot-handler.cpp
    task-meta-impl.cpp
    tlv-composer.cpp
    tlv-parser.cpp
    ../service/registry/data-types.xsd.cpp
    )
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTLV_ASCII_ONLY")
include_directories (
    ..
    ../../3rdparty
    )
add_custom_command (OUTPUT data-types.xsd.cpp data-types.xsd.h
    COMMAND ${XSDC} data-types.ini
    MAIN_DEPENDENCY data-types.ini
    DEPENDS ${XSDC}
    )
