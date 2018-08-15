cmake_minimum_required(VERSION 3.1)

project(qrcode)


set(CMAKE_CXX_STANDARD 14)

# OpenCV's flags
set(BUILD_LIST "core,videoio,highgui,calib3d" CACHE STRING "")
set(WITH_WEBP OFF CACHE BOOL "")
set(WITH_ZLIB OFF CACHE BOOL "")
set(WITH_OPENEXR OFF CACHE BOOL "")
set(WITH_PROTOBUF OFF CACHE BOOL "")
set(WITH_FFMPEG OFF CACHE BOOL "")
set(WITH_IPP OFF CACHE BOOL "")
set(WITH_JASPER OFF CACHE BOOL "")
set(BUILD_ZLIB OFF CACHE BOOL "")
set(BUILD_TIFF OFF CACHE BOOL "")
set(BUILD_JASPER OFF CACHE BOOL "")
add_subdirectory(3rdparty/opencv)

include_directories(
  "3rdparty/opencv/include"
  "3rdparty/opencv/modules/core/include"
  "3rdparty/opencv/modules/calib3d/include"
  "3rdparty/opencv/modules/features2d/include"
  "3rdparty/opencv/modules/highgui/include"
  "3rdparty/opencv/modules/imgcodecs/include"
  "3rdparty/opencv/modules/videoio/include"
  "3rdparty/opencv/modules/imgproc/include"
  "${CMAKE_BINARY_DIR}"
  "3rdparty/quirc/lib"
)

file(GLOB quirc_sources "3rdparty/quirc/lib/*")
add_library(quirc STATIC ${quirc_sources})

add_executable(${CMAKE_PROJECT_NAME} main.cpp qrcode.hpp test.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME}
  opencv_core
  opencv_highgui
  opencv_calib3d
  opencv_videoio
  quirc
)
