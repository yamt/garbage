cmake_minimum_required(VERSION 3.16)

project(crc32 LANGUAGES C)

if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL GNU)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wvla -Werror")
endif()

add_library(crc32 crc32.c)
set_property(TARGET crc32 PROPERTY PUBLIC_HEADER crc32.h)
target_include_directories(crc32
                           INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<INSTALL_INTERFACE:include>)

install(TARGETS crc32
        EXPORT crc32
        PUBLIC_HEADER DESTINATION include)
