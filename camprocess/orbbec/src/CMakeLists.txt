cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(cam_process)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(/usr/local/include/opencv4)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(cam_process cam_process.cpp)

target_link_libraries(cam_process Threads::Threads ${OpenCV_LIBS} ${PROJECT_SOURCE_DIR}/lib/libOrbbecSDK.so ${PROJECT_SOURCE_DIR}/lib/libdepthengine.so)

set_property(TARGET cam_process PROPERTY CXX_STANDARD 14)
