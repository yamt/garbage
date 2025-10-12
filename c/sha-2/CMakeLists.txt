cmake_minimum_required(VERSION 3.16)

project(sha-2 LANGUAGES C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED NO)
set(CMAKE_C_EXTENSIONS NO)

add_compile_options(-Wall -Wvla -Wdouble-promotion -Werror)

#add_compile_options(-fsanitize=undefined)
#add_link_options(-fsanitize=undefined)

add_library(sha256 sha256.c)
add_executable(sha256_cli sha256_cli.c sha256)
add_executable(sha256_test sha256_test.c sha256)

add_library(sha512 sha512.c)
add_executable(sha512_cli sha512_cli.c sha512)
add_executable(sha512_test sha512_test.c sha512)
