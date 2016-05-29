set(SCENERY_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneryManager.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneryManager.cpp"
    PARENT_SCOPE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("scenery" FILES ${SCENERY_SRCS})
