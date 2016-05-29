set(SKYBOX_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Skybox.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Skybox.cpp"
    PARENT_SCOPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("Skybox" FILES ${SKYBOX_SRCS})
