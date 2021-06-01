cmake_minimum_required(VERSION 3.0)
project(FFS)

find_package(FUSE REQUIRED)

add_executable(FFS fs.cpp load_file.h load_file.cpp)

target_link_libraries(FFS LINK_PUBLIC ${FUSE_LIBRARIES} imglib)
