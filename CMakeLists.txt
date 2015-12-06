cmake_minimum_required (VERSION 2.8)
project (yums)
set(CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/cmake
  ${CMAKE_MODULE_PATH})

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

add_subdirectory(libenv)
add_subdirectory(libdata)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libenv/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libdata/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

INCLUDE (CheckIncludeFileCXX)
check_include_file_cxx(experimental/filesystem TS_FILESYSTEM_FOUND)

if (NOT TS_FILESYSTEM_FOUND)
find_package (Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
endif (NOT TS_FILESYSTEM_FOUND)

set(SRCS
	src/yums.cpp
	src/yums_db.cpp
	src/yums_init.cpp
	src/yums_remote.cpp
	src/yums_remote_add.cpp
	src/yums_remote_rm.cpp
	src/yums_remote_show.cpp
	src/yums_update.cpp
	src/argparser.cpp
	src/filesystem.cpp
	src/repository.cpp
)

set (INCS
	src/yums_db.hpp
	src/argparser.hpp
	src/repository.hpp
)

if (WIN32)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/yums.manifest.in ${CMAKE_CURRENT_BINARY_DIR}/yums.manifest)

endif (WIN32)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/filesystem.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/filesystem.hpp)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)

add_executable(yums ${SRCS} ${INCS})

set_target_properties(yums PROPERTIES
	CXX_STANDARD 14
	VERSION ${VERSION})
target_link_libraries(yums env data)

if (UNIX)

target_link_libraries(yums pthread)

endif (UNIX)

if (NOT TS_FILESYSTEM_FOUND)
target_link_libraries(yums boost_system boost_filesystem)
endif (NOT TS_FILESYSTEM_FOUND)


install(TARGETS yums DESTINATION bin)
