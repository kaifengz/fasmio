cmake_minimum_required (VERSION 2.6)
add_library (fiberenv
    cooperative-condition.cpp
    cooperative-event.cpp
    cooperative-fd.cpp
    cooperative-file.cpp
    cooperative-mutex.cpp
    cooperative-rwlock.cpp
    cooperative-semaphore.cpp
    cooperative-socket.cpp
    cooperative-waitable.cpp
    epoll.cpp
    fiber-env.cpp
    fiber-impl.cpp
    log.cpp
    native-condition.cpp
    native-mutex.cpp
    native-rwlock.cpp
    platform-fiber.cpp
    platform-thread.cpp
    wait-mgr.cpp
    )
