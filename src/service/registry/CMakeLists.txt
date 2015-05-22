set (plugin_name registry)
set (plugin_sources
    data-types.xsd.cpp
    registry.cpp
    )
include (../CMakeLists.plugin NO_POLICY_SCOPE)
add_custom_command (OUTPUT data-types.xsd.cpp data-types.xsd.h
    COMMAND ${XSDC} data-types.ini
    MAIN_DEPENDENCY data-types.ini
    DEPENDS ${XSDC}
    )
