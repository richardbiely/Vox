set(PROJECTILE_SRCS
   "${CMAKE_CURRENT_SOURCE_DIR}/Projectile.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/Projectile.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/ProjectileManager.h"
   "${CMAKE_CURRENT_SOURCE_DIR}/ProjectileManager.cpp"
   PARENT_SCOPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("Projectile" FILES ${PROJECTILE_SRCS})