cmake_minimum_required(VERSION 3.16)
project(llvm-test LANGUAGES C)

# https://www.llvm.org/docs/CMake.html#embedding-llvm-in-your-project
find_package(LLVM REQUIRED CONFIG)
message(STATUS "LLVM_PACKAGE_VERSION: ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM_DIR: ${LLVM_DIR}")
message(STATUS "LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM_DEFINITIONS: ${LLVM_DEFINITIONS}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

#llvm_map_components_to_libnames(LLVM_LIBS core target x86codegen)
set(LLVM_LIBS ${LLVM_AVAILABLE_LIBS})
message(STATUS "LLVM_LIBS: ${LLVM_LIBS}")

set(sources
	"test.c"
)

add_executable(test ${sources})
target_link_libraries(test ${LLVM_LIBS} stdc++)
