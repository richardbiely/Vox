set(TEXTEFFECTS_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/AnimatedText.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnimatedText.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/TextEffectsManager.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/TextEffectsManager.cpp"
    PARENT_SCOPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("TextEffects" FILES ${TEXTEFFECTS_SRCS})
