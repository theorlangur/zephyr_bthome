cmake_minimum_required(VERSION 3.10)
project(NrfLibBTHome VERSION 1.0.0)

add_library(NrfLibBTHome INTERFACE)

target_include_directories(NrfLibBTHome INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

