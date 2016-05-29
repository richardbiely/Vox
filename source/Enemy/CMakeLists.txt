set(ENEMY_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Enemy.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Enemy.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/EnemyManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/EnemyManager.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/EnemySpawner.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/EnemySpawner.h"
    PARENT_SCOPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("Enemy" FILES ${ENEMY_SRCS})
