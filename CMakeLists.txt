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

# adding -Wl,-z,defs will warn for undef references in the output shared object.
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Werror -Winvalid-pch ")
set(CMAKE_CXX_FLAGS_DEBUG "-g -rdynamic") #-D_GLIBCXX_DEBUG
set(CMAKE_CXX_FLAGS_RELEASE "-O2")


IF(CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(RocketLibSuffix "_d")
    SET(GwenLibSuffix "d")
#     SET(GwenRendererLibSuffix "D")
ELSE(CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(RocketLibSuffix "")
    SET(GwenLibSuffix "")
ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")

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
    /media/a0/cpp/libs/ogre/Components/Terrain/include
    /media/a0/cpp/libs/ogre/Components/Paging/include
    /media/a0/cpp/libs/ogre/build/include
    /media/a0/cpp/libs/libRocket.github.onze/Include
    /usr/include/bullet
    /media/a0/cpp/libs/btogre/include
)

link_directories(
    /media/a0/cpp/libs/ogre/build/lib
    /usr/lib/x86_64-linux-gnu/
    /media/a0/cpp/libs/libRocket.github.onze/Build
    /usr/local/lib
    /media/a0/cpp/libs/btogre/build
)


add_library(
    steel SHARED ${source_files}
)
            
target_link_libraries(
    steel
    OgreMain
    OgreTerrain
    OgrePaging
    OIS
    jsoncpp
    RocketCore${RocketLibSuffix}
    RocketDebugger${RocketLibSuffix}
    RocketControls${RocketLibSuffix}
    BulletDynamics
    BulletCollision
    LinearMath
    BtOgre
)
