cmake_minimum_required(VERSION 3.16)

project(base64 LANGUAGES C)

if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL GNU)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wvla -Werror")
endif()

add_library(base64decode base64decode.c)
set_property(TARGET base64decode PROPERTY PUBLIC_HEADER base64decode.h)
target_include_directories(base64decode
                           INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<INSTALL_INTERFACE:include>)

install(TARGETS base64decode
        EXPORT base64decode
        PUBLIC_HEADER DESTINATION include)

add_library(base64encode base64encode.c)
set_property(TARGET base64encode PROPERTY PUBLIC_HEADER base64encode.h)
target_include_directories(base64encode
                           INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<INSTALL_INTERFACE:include>)

install(TARGETS base64encode
        EXPORT base64encode
        PUBLIC_HEADER DESTINATION include)
