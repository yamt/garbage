cmake_minimum_required(VERSION 3.16)

project(base64 LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wvla -Werror")

add_library(base64decode base64decode.c)
add_library(base64encode base64encode.c)
