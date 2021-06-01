add_library(imglib SHARED imadb.h imadb.cpp)

find_package(Boost )

target_link_libraries(imglib)
target_include_directories(imglib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
