cmake_minimum_required (VERSION 2.6)
project (fasmio)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g -Wall -Werror -fvisibility=hidden")
set (XSDC "${CMAKE_SOURCE_DIR}/scripts/simple-xsd-compiler.py")

add_subdirectory (interface)
add_subdirectory (runtime-env)
add_subdirectory (common)
add_subdirectory (service)
add_subdirectory (net-client)
add_subdirectory (net-server)
add_subdirectory (container)
add_subdirectory (main)
add_subdirectory (sample)
add_subdirectory (test)

add_custom_target (pack
    COMMAND scripts/pack.sh
    )
add_custom_target (wc
    COMMAND find . -name '*.cpp' -o -name '*.h' -o -name '*.sh' -o -name '*.py' | grep -v './gtest\\|CMake\\|rapidxml' | xargs wc | sort -n
    )
add_custom_target (build
    COMMAND scripts/build.sh
    )
