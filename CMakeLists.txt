cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(muse_stability_test)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} $ENV{ROOTSYS}/etc/cmake/)

find_package(ROOT REQUIRED)
include(${ROOT_USE_FILE})

add_executable(muse_stability_test main.cpp dataclasses.h dataclasses.cpp)

target_link_libraries(muse_stability_test ${ROOT_LIBRARIES})
