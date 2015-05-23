cmake_minimum_required (VERSION 2.6)
set (gtest_dir "../../3rdparty/gtest")
add_executable (test
    ${gtest_dir}/src/gtest-all.cc
    common/ini-reader-test.cpp
    common/ranged-istream-test.cpp
    common/simple-xsd-test.cpp
    common/stream-adaptor-test.cpp
    container/key-value-impl-test.cpp
    container/request-impl-test.cpp
    container/task-meta-impl-test.cpp
    container/tlv-composer-test.cpp
    container/tlv-parser-test.cpp
    fiber-env/cooperative-condition-test.cpp
    fiber-env/cooperative-event-test.cpp
    fiber-env/cooperative-file-test.cpp
    fiber-env/cooperative-mutex-test.cpp
    fiber-env/cooperative-rwlock-test.cpp
    fiber-env/cooperative-semaphore-test.cpp
    fiber-env/fiber-env-test.cpp
    main.cpp
    )
set (CMAKE_CXX_FLAGS "-std=c++0x -g")
include_directories (
    ../
    ../../3rdparty
    ${gtest_dir}
    ${gtest_dir}/include
    )
target_link_libraries (test
    fcgisvr
    container
    common
    fiberenv
    runtimeenv
    pthread
    uuid
    )
add_custom_target (check ./test
    DEPENDS test
    )
add_custom_target (valgrind valgrind --tool=memcheck --leak-check=yes --log-file=./valgrind.log ./test
    DEPENDS test
    )
