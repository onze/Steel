cmake_minimum_required(VERSION 2.8)

project(steel)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug)
ENDIF(NOT CMAKE_BUILD_TYPE)
message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(LIBRARY_OUTPUT_PATH lib/${CMAKE_BUILD_TYPE})
message("LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")

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
)

link_directories(
    /media/a0/cpp/libs/ogre/lib
    /usr/lib/x86_64-linux-gnu/
)

add_library(steel SHARED ${source_files})

target_link_libraries(
    steel
    OgreMain
    OIS
)