set(INVENTORY_SRCS
   "${CMAKE_CURRENT_SOURCE_DIR}/InventoryManager.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/InventoryManager.cpp"
   PARENT_SCOPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("Inventory" FILES ${INVENTORY_SRCS})