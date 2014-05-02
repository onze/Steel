cmake_minimum_required(VERSION 2.8)

project(steel)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug)
    ADD_DEFINITIONS(-DDEBUG=1)
ENDIF(NOT CMAKE_BUILD_TYPE)
message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(LIBRARY_OUTPUT_PATH lib/${CMAKE_BUILD_TYPE})
message("LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")

# adding -Wl,-z,defs will warn for undef references in the output shared object.
# for when there precompiled header is setup  -Winvalid-pch
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Werror ")
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
    /media/a0/cpp/libs/jsoncpp/jsoncpp/include
    /usr/include/ois
    /media/a0/cpp/libs/ogre/OgreMain/include
    /media/a0/cpp/libs/ogre/Components/Terrain/include
    /media/a0/cpp/libs/ogre/Components/Paging/include
    /media/a0/cpp/libs/ogre/build/include
    /media/a0/cpp/libs/btogre/include
    /media/a0/cpp/libs/libRocket.github.onze/Include
    /media/a0/cpp/libs/mygui/MyGUIEngine/include
    /media/a0/cpp/libs/mygui/Platforms/Ogre/OgrePlatform/include
)

link_directories(
    /media/a0/cpp/libs/ogre/build/lib
    /media/a0/cpp/libs/jsoncpp/build/lib
    /usr/lib/x86_64-linux-gnu/
    /media/a0/cpp/libs/libRocket.github.onze/Build
    /usr/local/lib
    /media/a0/cpp/libs/btogre/build
    /media/a0/cpp/libs/mygui/build/lib
)


add_library(
    steel SHARED ${source_files}
)

# skip gcc warning for unused variables in external libraries by declaring them as system:
# http://stackoverflow.com/questions/15053776/how-do-you-disable-the-unused-variable-warnings-coming-out-of-gcc
# http://stackoverflow.com/questions/3371127/use-isystem-instead-of-i-with-cmake
target_include_directories(steel SYSTEM PUBLIC 
    /usr/include/bullet
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
    MyGUIEngine
    MyGUI.OgrePlatform
)
