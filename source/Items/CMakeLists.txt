set(ITEMS_SRCS
   "${CMAKE_CURRENT_SOURCE_DIR}/Item.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/Item.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/ItemSpawner.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/ItemSpawner.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/ItemManager.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/ItemManager.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/StatAttribute.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/StatAttribute.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/ItemUtils.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/ItemsEnum.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/EquipmentUtils.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/EquipmentEnum.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/RandomLootManager.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/RandomLootManager.cpp"
   PARENT_SCOPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("Items" FILES ${ITEMS_SRCS})
