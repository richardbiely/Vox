set(MODELS_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/BoundingBox.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/BoundingBox.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/modelloader.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/MS3DAnimator.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/MS3DAnimator.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/MS3DModel.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/MS3DModel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/OBJModel.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/objmodel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/QubicleBinary.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/QubicleBinary.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/QubicleBinaryManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/QubicleBinaryManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/VoxelCharacter.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/VoxelCharacter.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/VoxelObject.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/VoxelObject.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/VoxelWeapon.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/VoxelWeapon.cpp"
	PARENT_SCOPE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
source_group("models" FILES ${MODELS_SRCS})
