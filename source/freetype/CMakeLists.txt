set(FREETYPE_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/freetypefont.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/freetypefont.cpp"
	PARENT_SCOPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("freetype" FILES ${FREETYPE_SRCS})