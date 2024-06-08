cmake_minimum_required(VERSION 3.16)

project(test LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wvla -Werror")

find_package(PkgConfig REQUIRED)
pkg_check_modules(JANSSON REQUIRED jansson)

set(app_sources
	"test.c"
)

add_executable(test ${app_sources})
target_include_directories(test PRIVATE ${JANSSON_INCLUDE_DIRS})
target_link_directories(test PRIVATE ${JANSSON_LIBRARY_DIRS})
target_link_libraries(test ${JANSSON_LIBRARIES})
