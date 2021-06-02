add_library(imglib SHARED
        imadb.h imadb.cpp
        activity.h activity.cpp
        direc_discovery.h direc_discovery.cpp
        event_sync.h event_sync.cpp
        types.h evt.cpp
        )

find_package(Boost REQUIRED)
find_package(FSWATCH REQUIRED)

target_link_libraries(imglib ${FSWATCH_LIBRARIES} ${Boost_LIBRARIES})
target_include_directories(imglib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${FSWATCH_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

#message("Boost is ${Boost_INCLUDE_DIRS}")
