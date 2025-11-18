cmake_minimum_required(VERSION 3.16)

project(sha-2 LANGUAGES C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED NO)
set(CMAKE_C_EXTENSIONS NO)

if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID STREQUAL GNU)
add_compile_options(-Wall -Wvla -Wdouble-promotion -Werror)
endif()

#add_compile_options(-fsanitize=undefined)
#add_link_options(-fsanitize=undefined)

add_library(sha256 sha256.c)
set_property(TARGET sha256 PROPERTY PUBLIC_HEADER sha256.h)
target_include_directories(sha256
                           INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<INSTALL_INTERFACE:include>)
install(TARGETS sha256
        EXPORT sha256
        PUBLIC_HEADER DESTINATION include)

add_executable(sha256_cli sha256_cli.c sha256)
add_executable(sha256_test sha256_test.c sha256)

add_library(sha512 sha512.c)
set_property(TARGET sha512 PROPERTY PUBLIC_HEADER sha512.h)
target_include_directories(sha512
                           INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<INSTALL_INTERFACE:include>)
install(TARGETS sha512
        EXPORT sha512
        PUBLIC_HEADER DESTINATION include)

add_executable(sha512_cli sha512_cli.c sha512)
add_executable(sha512_test sha512_test.c sha512)
