set(SIMPLEX_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/simplexnoise.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/simplexnoise.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/simplextextures.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/simplextextures.cpp"
	PARENT_SCOPE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("simplex" FILES ${SIMPLEX_SRCS})