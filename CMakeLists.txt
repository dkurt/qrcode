cmake_minimum_required(VERSION 2.8)

project(qrcode)

# OpenCV's flags
set(BUILD_LIST "core,videoio,highgui,calib3d" CACHE STRING "")
set(WITH_WEBP OFF CACHE BOOL "")
set(WITH_ZLIB OFF CACHE BOOL "")
set(WITH_OPENEXR OFF CACHE BOOL "")
add_subdirectory(3rdparty/opencv)

include_directories(
  "3rdparty/opencv/include"
  "3rdparty/quirc/lib"
)

file(GLOB quirc_sources "3rdparty/quirc/lib/*")
add_library(quirc SHARED ${quirc_sources})

add_executable(${CMAKE_PROJECT_NAME} main.cpp qrcode.hpp test.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME}
  opencv_core
  opencv_highgui
  opencv_calib3d
  opencv_videoio
  quirc
)
