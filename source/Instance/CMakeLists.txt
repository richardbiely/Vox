set(INSTANCE_SRCS
   "${CMAKE_CURRENT_SOURCE_DIR}/InstanceManager.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/InstanceManager.cpp"
   PARENT_SCOPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("Instance" FILES ${INSTANCE_SRCS})