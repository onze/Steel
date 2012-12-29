cmake_minimum_required(VERSION 2.8)

project(steel)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug)
    ADD_DEFINITIONS(-DDEBUG=1)
ENDIF(NOT CMAKE_BUILD_TYPE)
message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

ADD_DEFINITIONS("-std=c++0x")

set(LIBRARY_OUTPUT_PATH lib/${CMAKE_BUILD_TYPE})
message("LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")

set(CMAKE_CXX_FLAGS "-Wall")
set (CMAKE_CXX_FLAGS_DEBUG "-g -rdynamic")
set (CMAKE_CXX_FLAGS_RELEASE "-O2")

file(
    GLOB_RECURSE
    source_files
    Steel/*
)

include_directories(
    Steel/include
    /usr/include/jsoncpp
    /usr/include/ois
    /media/a0/cpp/libs/ogre/OgreMain/include
    /media/a0/cpp/libs/ogre/include
    /media/a0/cpp/libs/ogre/Components/Terrain/include
    /media/a0/cpp/libs/ogre/Components/Paging/include
#      custom fork
    /media/a0/cpp/libs/libRocket.github.onze/Include
#     stable version
#     /media/a0/cpp/libs/librocket_generic-source-1.2.1/libRocket/Include
)

link_directories(
    /media/a0/cpp/libs/ogre/lib
    /usr/lib/x86_64-linux-gnu/
#      custom fork
    /media/a0/cpp/libs/libRocket.github.onze/Build
#     stable version
#     /media/a0/cpp/libs/librocket_generic-source-1.2.1/libRocket/Build
)

add_library(steel SHARED ${source_files})

target_link_libraries(
    steel
    OgreMain
    OgreTerrain
    OgrePaging
    OIS
    RocketCore
    RocketDebugger
    RocketControls
)