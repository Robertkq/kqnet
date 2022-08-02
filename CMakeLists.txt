# This file handles building each task as sub-project.
#
# Each task becomes an independent project in which you can experiment.
#
# Tasks are added as separate sub-projects so as compilation errors in
# one task don't affect another task.


cmake_minimum_required(VERSION 3.12)

project(kqnet)

set(CMAKE_CXX_STANDARD 17)

include_directories("include/asio-1.20.0/include")

set(SERVER_SOURCES
        include/mainServer.cpp
        )
		
set(CLIENT_SOURCES
        include/mainClient.cpp
        )

add_executable(kqnetServer ${SERVER_SOURCES})
add_executable(kqnetClient ${CLIENT_SOURCES})