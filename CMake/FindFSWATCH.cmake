# Lol s/FUSE/FSWatch
# Find the FUSE includes and library
#
#  FSWATCH_INCLUDE_DIR - where to find fuse.h, etc.
#  FSWATCH_LIBRARIES   - List of libraries when using FUSE.
#  FSWATCH_FOUND       - True if FUSE lib is found.

# check if already in cache, be silent
IF (FSWATCH_INCLUDE_DIR)
    SET (FSWATCH_FIND_QUIETLY TRUE)
ENDIF (FSWATCH_INCLUDE_DIR)

# find includes
FIND_PATH (FSWATCH_C_INCLUDE_DIR libfswatch.h
        /usr/local/include/libfswatch/c
        /usr/local/include
        /usr/include/libfswatch/c
        /usr/include/libfswatch/c++
        /usr/include
        )

FIND_PATH (FSWATCH_CPP_INCLUDE_DIR monitor.hpp
        /usr/local/include
        /usr/local/include/libfswatch/c
        /usr/local/include/libfswatch/c++
        /usr/include/libfswatch/c
        /usr/include/libfswatch/c++
        /usr/include
        )

SET(FSWATCH_INCLUDE_DIRS ${FSWATCH_CPP_INCLUDE_DIR} ${FSWATCH_C_INCLUDE_DIR})

IF (APPLE)
  SET(FSWATCH_NAMES libfswatch.dylib)
ELSE (APPLE)
  SET(FSWATCH_NAMES libfswatch)
endif (APPLE)


SET(FSWATCH_NAMES fswatch)
SET(FSWATCH_ROOT_DIR /usr/local/include/libfswatch/c++)
FIND_LIBRARY(FSWATCH_LIBRARIES
	NAMES fswatch ${FSWATCH_NAMES}
        PATHS /lib64 /lib /usr/lib64 /usr/lib /usr/local/lib64 /usr/local/lib /usr/lib/x86_64-linux-gnu/libfswatch
        )

include ("FindPackageHandleStandardArgs")
find_package_handle_standard_args ("FSWATCH" DEFAULT_MSG
        FSWATCH_INCLUDE_DIRS FSWATCH_LIBRARIES)

mark_as_advanced (FSWATCH_INCLUDE_DIRS FSWATCH_LIBRARIES)
