set(QUESTS_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Quest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Quest.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/QuestJournal.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/QuestJournal.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/QuestManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/QuestManager.h"
    PARENT_SCOPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("Quests" FILES ${QUESTS_SRCS})
