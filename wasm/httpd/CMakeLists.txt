cmake_minimum_required(VERSION 3.16)

project(https LANGUAGES C)

option(USE_THREADS "use threads" OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wvla -Werror")

set(sources
	"main.c"
)

add_executable(httpd ${sources})
if(USE_THREADS)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
target_compile_definitions(httpd PRIVATE THREADS)
endif()
#target_link_libraries(httpd )
