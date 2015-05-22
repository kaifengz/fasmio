cmake_minimum_required (VERSION 2.6)
add_executable (ftpd
    ftp.cpp
    simple-ftp-server.cpp
    tcp-server.cpp
    )
add_executable (qotd
    qotd.cpp
    simple-qotd-server.cpp
    tcp-server.cpp
    )
include_directories (
    ../..
    )
target_link_libraries (ftpd
    fiberenv
    runtimeenv
    pthread
    )
target_link_libraries (qotd
    fiberenv
    runtimeenv
    pthread
    )
add_custom_target (vftpd valgrind --tool=memcheck --leak-check=yes --log-file=./valgrind.log ./ftpd
    DEPENDS ftpd
    )
set (dist_files
    ftp.h
    ftp.cpp
    qotd.h
    qotd.cpp
    simple-qotd-server.cpp
    simple-ftp-server.cpp
    tcp-server.cpp
    tcp-server.h
    )
set (dist_dir ../../../dist/sample/simple-tcp-server)
add_custom_command (TARGET ftpd
    POST_BUILD
    COMMAND rm -rf ${dist_dir}/
    COMMAND mkdir -p ${dist_dir}/
    COMMAND cp ${dist_files} ${dist_dir}/
    )
