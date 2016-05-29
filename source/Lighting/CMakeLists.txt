set(LIGHTING_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/DynamicLight.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DynamicLight.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/LightingManager.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/LightingManager.cpp"
    PARENT_SCOPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("lighting" FILES ${LIGHTING_SRCS})
