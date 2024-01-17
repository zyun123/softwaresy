cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(cam_process)

set(DEPENDENCIES k4a)
find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(/usr/local/include/opencv4)

add_executable(cam_process cam_process.cpp)

target_link_libraries(cam_process "${DEPENDENCIES}" Threads::Threads ${OpenCV_LIBS})

set_property(TARGET cam_process PROPERTY CXX_STANDARD 14)
