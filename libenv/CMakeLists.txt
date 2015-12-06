include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
link_directories(${CURL_LIBRARIES})

find_package(EXPAT REQUIRED)
include_directories(${EXPAT_INCLUDE_DIRS})
link_directories(${EXPAT_LIBRARIES})

set(SRCS
	env/utf8.cpp
	http/uri.cpp
	http/curl_http.cpp
	http/xhr.cpp
	dom/dom.cpp
	dom/dom_xpath.cpp
	dom/nodes/document_fragment.cpp
	dom/nodes/element.cpp
	dom/nodes/document.cpp
	dom/nodes/nodelist.cpp
	dom/parsers/xml_parser.cpp
	dom/parsers/parser.cpp
)

set (INCS
	inc/env/utf8.hpp
	inc/http/xhr.hpp
	inc/http/http_logger.hpp
	inc/http/uri.hpp
	inc/dom/dom_xpath.hpp
	inc/dom/dom.hpp
	inc/dom/domfwd.hpp
	inc/dom/nodes/nodelist.hpp
	inc/dom/nodes/document_fragment.hpp
	inc/dom/nodes/element.hpp
	inc/dom/nodes/attribute.hpp
	inc/dom/nodes/text.hpp
	inc/dom/nodes/node.hpp
	inc/dom/nodes/document.hpp
	inc/dom/parsers/xml.hpp
	inc/dom/parsers/parser.hpp
	inc/dom/range.hpp
	http/curl_http.hpp
	dom/nodes/nodelist.hpp
	dom/nodes/document_fragment.hpp
	dom/nodes/parent_node_impl.hpp
	dom/nodes/element.hpp
	dom/nodes/attribute.hpp
	dom/nodes/child_node_impl.hpp
	dom/nodes/node_impl.hpp
	dom/nodes/text.hpp
	dom/nodes/document.hpp
	dom/parsers/expat.hpp
)

if (WIN32)

list(APPEND SRCS
	http/win32_http.cpp
)
endif (WIN32)

if (UNIX)

list(APPEND SRCS
	http/posix_http.cpp
)

endif (UNIX)

add_library(env ${SRCS} ${INCS})

set_target_properties(env PROPERTIES
	CXX_STANDARD 14
	VERSION ${VERSION})
target_link_libraries(env ${CURL_LIBRARY} ${EXPAT_LIBRARY})
