set(SRCS
    "main.cpp"
    "VoxGame.h"
    "VoxGame.cpp"
    "VoxWindow.h"
    "VoxWindow.cpp"
    "VoxInput.cpp"
    "VoxControls.cpp"
    "VoxCamera.cpp"
    "VoxRender.cpp"
    "VoxUpdate.cpp"
    "VoxGUI.cpp"
    "VoxSettings.h"
    "VoxSettings.cpp")

add_subdirectory(extern)
add_subdirectory(utils)
add_subdirectory(freetype)
add_subdirectory(Lighting)
add_subdirectory(Maths)
add_subdirectory(models)
add_subdirectory(Particles)
add_subdirectory(Renderer)
add_subdirectory(gui)
add_subdirectory(Player)
add_subdirectory(blocks)
add_subdirectory(frontend)
add_subdirectory(Skybox)
add_subdirectory(scenery)
add_subdirectory(Instance)
add_subdirectory(Inventory)
add_subdirectory(Items)
add_subdirectory(GameGUI)
add_subdirectory(Projectile)
add_subdirectory(TextEffects)
add_subdirectory(Mods)
add_subdirectory(NPC)
add_subdirectory(Enemy)
add_subdirectory(Quests)
add_subdirectory(AudioManager)

source_group("source" FILES ${SRCS})
source_group("source\\utils" FILES ${UTIL_SRCS})
source_group("source\\freetype" FILES ${FREETYPE_SRCS})
source_group("source\\lighting" FILES ${LIGHTING_SRCS})
source_group("source\\maths" FILES ${MATHS_SRCS})
source_group("source\\models" FILES ${MODELS_SRCS})
source_group("source\\particles" FILES ${PARTICLES_SRCS})
source_group("source\\renderer" FILES ${RENDERER_SRCS})
source_group("source\\gui" FILES ${GUI_SRCS})
source_group("source\\player" FILES ${PLAYER_SRCS})
source_group("source\\blocks" FILES ${BLOCKS_SRCS})
source_group("source\\frontend" FILES ${FRONTEND_SRCS})
source_group("source\\frontend\\Pages" FILES ${FRONTEND_PAGES})
source_group("source\\Skybox" FILES ${SKYBOX_SRCS})
source_group("source\\scenery" FILES ${SCENERY_SRCS})
source_group("source\\Instance" FILES ${INSTANCE_SRCS})
source_group("source\\Inventory" FILES ${INVENTORY_SRCS})
source_group("source\\Items" FILES ${ITEMS_SRCS})
source_group("source\\GameGUI" FILES ${GAMEGUI_SRCS})
source_group("source\\Projectile" FILES ${PROJECTILE_SRCS})
source_group("source\\TextEffects" FILES ${TEXTEFFECTS_SRCS})
source_group("source\\Mods" FILES ${MODS_SRCS})
source_group("source\\NPC" FILES ${NPC_SRCS})
source_group("source\\Enemy" FILES ${ENEMY_SRCS})
source_group("source\\Quests" FILES ${QUESTS_SRCS})
source_group("source\\AudioManager" FILES ${AUDIOMANAGER_SRCS})

add_executable(Vox
               ${SRCS}
               ${UTIL_SRCS}
               ${FREETYPE_SRCS}
               ${GLEW_SRCS}
               ${GLEW_HEADERS}
               ${LIGHTING_SRCS}
               ${MATHS_SRCS}
               ${MODELS_SRCS}
               ${PARTICLES_SRCS}
               ${RENDERER_SRCS}
               ${GUI_SRCS}
               ${GLM_SRCS}
               ${GLM_DETAIL_SRCS}
               ${GLM_GTC_SRCS}
               ${GLM_GTX_SRCS}
               ${PLAYER_SRCS}
               ${BLOCKS_SRCS}
               ${LUA_SRCS}
               ${SELENE_HEADERS}
               ${SELENE_SRCS}
               ${FRONTEND_SRCS}
               ${FRONTEND_PAGES}
               ${INI_SRCS}
               ${SIMPLEX_SRCS}
               ${SKYBOX_SRCS}
               ${SCENERY_SRCS}
               ${INSTANCE_SRCS}
               ${INVENTORY_SRCS}
               ${ITEMS_SRCS}
               ${GAMEGUI_SRCS}
               ${PROJECTILE_SRCS}
               ${TEXTEFFECTS_SRCS}
               ${MODS_SRCS}
               ${NPC_SRCS}
               ${ENEMY_SRCS}
               ${QUESTS_SRCS}
               ${LIBNOISE_SRCS}
               ${AUDIOMANAGER_SRCS})

include_directories(".")
include_directories("extern")
include_directories("extern\\glfw\\include")
include_directories("extern\\glew\\include")
include_directories("extern\\freetype\\include")
include_directories("extern\\lua")
include_directories("extern\\selene")
include_directories("extern\\libnoise")
include_directories("extern\\fmod\\include")

if(MSVC)
	target_link_libraries(Vox "opengl32.lib")
	target_link_libraries(Vox "winmm.lib")
	if(MSVC_VERSION >= 1900)
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\glfw\\libs\\2015\\d\\glfw3_64.lib")
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\freetype\\libs\\2015\\freetype261d_64.lib")
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\libnoise\\libs\\2015\\noise64_d.lib")
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\fmod\\libs\\fmod64_vc.lib")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\glfw\\libs\\2015\\r\\glfw3_64.lib")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\freetype\\libs\\2015\\freetype261_64.lib")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\libnoise\\libs\\2015\\noise64.lib")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\fmod\\libs\\fmod64_vc.lib")
		else()
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\glfw\\libs\\2015\\d\\glfw3.lib")
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\freetype\\libs\\2015\\freetype261d.lib")
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\libnoise\\libs\\2015\\noise_d.lib")
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\fmod\\libs\\fmod_vc.lib")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\glfw\\libs\\2015\\r\\glfw3.lib")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\freetype\\libs\\2015\\freetype261.lib")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\libnoise\\libs\\2015\\noise.lib")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}\\extern\\fmod\\libs\\fmod_vc.lib")
		endif()
	endif()
elseif(UNIX)
	target_link_libraries(Vox "GL")
	target_link_libraries(Vox "GLU")
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}/extern/freetype/libs/linux/libfreetype261_64.a")
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw/libs/linux/d/libglfw3_64.a")
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}/extern/libnoise/libs/linux/libnoise_64.a")
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}/extern/fmod/libs/libfmod64.so")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}/extern/freetype/libs/linux/libfreetype261_64.a")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw/libs/linux/r/libglfw3_64.a")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}/extern/libnoise/libs/linux/libnoise_64.a")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}/extern/fmod/libs/libfmod64.so")
	else()
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw/libs/linux/d/libglfw3.a")
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}/extern/freetype/libs/linux/libfreetype261.a")
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}/extern/libnoise/libs/linux/libnoise.a")
		target_link_libraries(Vox debug "${CMAKE_CURRENT_SOURCE_DIR}/extern/fmod/libs/libfmod.so")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}/extern/glfw/libs/linux/r/libglfw3.a")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}/extern/freetype/libs/linux/libfreetype261.a")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}/extern/libnoise/libs/linux/libnoise.a")
		target_link_libraries(Vox optimized "${CMAKE_CURRENT_SOURCE_DIR}/extern/fmod/libs/libfmod.so")
	endif()
	target_link_libraries(Vox "X11")
	target_link_libraries(Vox "Xrandr")
	target_link_libraries(Vox "Xi")
	target_link_libraries(Vox "Xxf86vm")
	target_link_libraries(Vox "Xcursor")
	target_link_libraries(Vox "Xinerama")
	target_link_libraries(Vox "pthread")
	target_link_libraries(Vox "dl")
endif()

if(MSVC)
	set_target_properties(Vox PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../../")
	set_target_properties(Vox PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../")
	set_target_properties(Vox PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../")
	
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
elseif(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -no-pie")
endif()

set(CMAKE_CONFIGURATION_TYPES Debug Release)
