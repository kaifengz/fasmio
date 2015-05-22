cmake_minimum_required (VERSION 2.6)
add_executable (wget
    wget.cpp
    )
include_directories (
    ../..
    )
target_link_libraries (wget
    httpclient
    common
    fiberenv
    runtimeenv
    pthread
    )
#set (dist_files
#    )
#set (dist_dir ../../../dist/sample/simple-wget)
#add_custom_command (TARGET wget
#    POST_BUILD
#    COMMAND rm -rf ${dist_dir}/
#    COMMAND mkdir -p ${dist_dir}/
#    COMMAND cp ${dist_files} ${dist_dir}/
#    )
