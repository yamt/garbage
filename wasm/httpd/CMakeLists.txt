cmake_minimum_required(VERSION 3.16)

project(https LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wvla -Werror")

set(sources
	"main.c"
)

add_executable(httpd ${sources})
#target_link_libraries(httpd )
