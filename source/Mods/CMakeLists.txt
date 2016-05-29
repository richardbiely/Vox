set(MODS_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/ModsManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ModsManager.h"
    PARENT_SCOPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("Mods" FILES ${MODS_SRCS})
