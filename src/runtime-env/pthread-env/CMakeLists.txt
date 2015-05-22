cmake_minimum_required (VERSION 2.6)
add_library (pthreadenv
    condition.cpp
    event.cpp
    file.cpp
    mutex.cpp
    platform-thread.cpp
    pthread-env.cpp
    rwlock.cpp
    semaphore.cpp
    socket.cpp
    thread-impl.cpp
    )
