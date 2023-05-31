cmake_minimum_required(VERSION 3.0.0)
project(WebSever VERSION 0.1.0)

# include(CTest)
# enable_testing()
# aux_source_directory(. ALL_SRCS)
# aux_source_directory(./base BASE)
# # add_executable(WebSever ${ALL_SRCS})
# include_directories(${PROJECT_SOURCE_DIR}/base)
# add_executable(WebSever main.cpp ${BASE})
# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

# # include(CPack)
# add_subdirectory(base)
# add_subdirectory(tests)
set(SRCS
    Channel.cpp
    Epoll.cpp
    EventLoop.cpp
    EventLoopThread.cpp
    EventLoopThreadPool.cpp
    HttpData.cpp
    main.cpp
    Server.cpp
    # ThreadPool.cpp
    Timer.cpp
    Util.cpp
    
)
include_directories(${PROJECT_SOURCE_DIR}/base)


set(CMAKE_BUILD_TYPE DEBUG)
add_executable(WebServer ${SRCS})
target_link_libraries(WebServer libserver_base)

add_subdirectory(base)

# add_executable(test test.cpp)
# add_subdirectory(tests)