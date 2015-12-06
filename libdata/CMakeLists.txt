include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/sqlite)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SRCS
	3rdparty/sqlite/sqlite3.c
	src/dbconn.cpp
	src/sqlite3.cpp
)

set (INCS
	3rdparty/sqlite/sqlite3.h
	inc/data/dbconn.hpp
	src/sqlite3.hpp
)

add_library(data ${SRCS} ${INCS})

set_target_properties(data PROPERTIES
	CXX_STANDARD 14
	VERSION ${VERSION})

if (UNIX)

target_link_libraries(data dl)

endif (UNIX)
