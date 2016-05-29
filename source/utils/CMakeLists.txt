set(UTIL_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Random.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Interpolator.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Interpolator.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CountdownTimer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CountdownTimer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/TimeManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/TimeManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/FileUtils.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/FileUtils.cpp"
	PARENT_SCOPE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("utils" FILES ${UTIL_SRCS})