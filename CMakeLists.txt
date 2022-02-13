cmake_minimum_required (VERSION 3.8)

project ("hackri")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 一些该工程的全局定义
set(HACKRI_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory("hackri")
