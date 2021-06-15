cmake_minimum_required(VERSION 3.0)
project(FFS)

set(CMAKE_CXX_FLAGS "-D_FILE_OFFSET_BITS=64")
#set(CMAKE_CXX_FLAGS "-D_FILE_OFFSET_BITS=64 -lulockmgr")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall --pedantic -g")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

add_subdirectory(img)
add_subdirectory(src)

INCLUDE(CPack)