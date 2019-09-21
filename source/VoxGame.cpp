// ******************************************************************************
// Filename:    VoxGame.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 27/10/15
//
// Copyright (c) 2005-2015, Steven Ball
// ******************************************************************************

#include "glew/include/GL/glew.h"

#include "VoxGame.h"
#include "utils/Interpolator.h"
#include <glm/detail/func_geometric.hpp>

#ifdef __linux__
#include <sys/time.h>
#endif //__linux__

const bool VoxGame::STEAM_BUILD = true;


extern string g_soundEffectFilenames[eSoundEffect_NUM];

// Initialize the singleton instance
VoxGame *VoxGame::c_instance = 0;

VoxGame* VoxGame::GetInstance()
{
	if (c_instance == 0)
		c_instance = new VoxGame;

	return c_instance;
}

// Creation
void VoxGame::Create(VoxSettings* pVoxSettings)
{
	m_pRenderer = nullptr;
	m_pGameCamera = nullptr;
	m_pQubicleBinaryManager = nullptr;
	m_pPlayer = nullptr;
	m_pChunkManager = nullptr;
	m_pFrontendManager = nullptr;

	m_pInventoryGUI = nullptr;
	m_pCharacterGUI = nullptr;
	m_pLootGUI = nullptr;
	m_pCraftingGUI = nullptr;
	m_pQuestGUI = nullptr;
	m_pActionBar = nullptr;
	m_pHUD = nullptr;

	m_GUICreated = false;

	m_pVoxSettings = pVoxSettings;
	m_pVoxWindow = new VoxWindow(this, m_pVoxSettings);

	// Create the window
	m_pVoxWindow->Create();

	/* Setup the FPS and deltatime counters */
#ifdef _WIN32
	QueryPerformanceCounter(&m_fpsPreviousTicks);
	QueryPerformanceCounter(&m_fpsCurrentTicks);
	QueryPerformanceFrequency(&m_fpsTicksPerSecond);
#else
	struct timeval tm;
	gettimeofday(&tm, nullptr);
	m_fpsCurrentTicks = (double)tm.tv_sec + (double)tm.tv_usec / 1000000.0;
	m_fpsPreviousTicks = (double)tm.tv_sec + (double)tm.tv_usec / 1000000.0;
#endif //_WIN32
	m_deltaTime = 0.0f;
	m_fps = 0.0f;

	/* Mouse name picking */
	m_pickedObject = -1;
	m_bNamePickingSelected = false;

	/* Custom cursors */
	m_bPressedCursorDown = false;
	m_bCustomCursorOn = false;

	/* Paper doll viewport dimensions */
	m_paperdollViewportX = 0;
	m_paperdollViewportY = 0;
	m_paperdollViewportWidth = 800;
	m_paperdollViewportHeight = 800;

	/* Portrain viewport dimensions */
	m_portraitViewportX = 0;
	m_portraitViewportY = 0;
	m_portraitViewportWidth = 800;
	m_portraitViewportHeight = 800;

	/* Setup the initial starting wait timing */
	m_initialWaitTimer = 0.0f;
	m_initialWaitTime = 0.5f;
	m_initialStartWait = true;

	/* Create the renderer */
	m_windowWidth = m_pVoxWindow->GetWindowWidth();
	m_windowHeight = m_pVoxWindow->GetWindowHeight();
	m_pRenderer = new Renderer(m_windowWidth, m_windowHeight, 32, 8);

	/* Pause and quit */
	m_bGameQuit = false;
	m_bPaused = false;

	/* Interactions */
	m_pInteractItem = nullptr;

	/* Biome */
	m_currentBiome = Biome_None;

	/* Music and Audio */
	m_pMusicChannel = nullptr;
	m_pMusicSound = nullptr;
	m_currentBiomeMusic = Biome_None;

	/* Create the GUI */
	m_pGUI = new OpenGLGUI(m_pRenderer);

	/* Create cameras */
	m_pGameCamera = new Camera(m_pRenderer);
	m_pGameCamera->SetPosition(vec3(8.0f, 8.25f, 15.5f));
	m_pGameCamera->SetFakePosition(m_pGameCamera->GetPosition());
	m_pGameCamera->SetFacing(vec3(0.0f, 0.0f, -1.0f));
	m_pGameCamera->SetUp(vec3(0.0f, 1.0f, 0.0f));
	m_pGameCamera->SetRight(vec3(1.0f, 0.0f, 0.0f));

	/* Create viewports */
	m_pRenderer->CreateViewport(0, 0, m_windowWidth, m_windowHeight, 60.0f, &m_defaultViewport);
	m_pRenderer->CreateViewport(0, 0, m_windowWidth, m_windowHeight, 60.0f, &m_firstpersonViewport);
	m_pRenderer->CreateViewport(m_paperdollViewportY, m_paperdollViewportX, m_paperdollViewportWidth, m_paperdollViewportHeight, 60.0f, &m_paperdollViewport);
	m_pRenderer->CreateViewport(m_portraitViewportY, m_portraitViewportX, m_portraitViewportWidth, m_portraitViewportHeight, 60.0f, &m_portraitViewport);

	/* Create fonts */
	m_pRenderer->CreateFreeTypeFont("media/fonts/arial.ttf", 12, &m_defaultFont);

	/* Create the custom cursor textures */
	int lTextureWidth, lTextureHeight, lTextureWidth2, lTextureHeight2;
	m_pRenderer->LoadTexture("media/textures/cursors/finger_cursor_normal.tga", &lTextureWidth, &lTextureHeight, &lTextureWidth2, &lTextureHeight2, &m_customCursorNormalBuffer);
	m_pRenderer->LoadTexture("media/textures/cursors/finger_cursor_clicked.tga", &lTextureWidth, &lTextureHeight, &lTextureWidth2, &lTextureHeight2, &m_customCursorClickedBuffer);
	m_pRenderer->LoadTexture("media/textures/cursors/finger_cursor_rotate.tga", &lTextureWidth, &lTextureHeight, &lTextureWidth2, &lTextureHeight2, &m_customCursorRotateBuffer);
	m_pRenderer->LoadTexture("media/textures/cursors/finger_cursor_zoom.tga", &lTextureWidth, &lTextureHeight, &lTextureWidth2, &lTextureHeight2, &m_customCursorZoomBuffer);

	/* Create lights */
	m_defaultLightPosition = vec3(300.0f, 300.0f, 300.0f);
	m_defaultLightView = vec3(0.0f, 0.0f, 0.0f);
	vec3 lightDirection = m_defaultLightView - m_defaultLightPosition;
	m_pRenderer->CreateLight(Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f),
							 m_defaultLightPosition, lightDirection, 0.0f, 0.0f, 2.0f, 0.001f, 0.0f, true, false, &m_defaultLight);

	/* Create materials */
	m_pRenderer->CreateMaterial(Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f), 64, &m_defaultMaterial);

	/* Create the frame buffers */
	bool frameBufferCreated = false;
	frameBufferCreated = m_pRenderer->CreateFrameBuffer(-1, true, true, true, true, m_windowWidth, m_windowHeight, 1.0f, "SSAO", &m_SSAOFrameBuffer);
	frameBufferCreated = m_pRenderer->CreateFrameBuffer(-1, true, true, true, true, m_windowWidth, m_windowHeight, 5.0f, "Shadow", &m_shadowFrameBuffer);
	frameBufferCreated = m_pRenderer->CreateFrameBuffer(-1, true, true, true, true, m_windowWidth, m_windowHeight, 1.0f, "Deferred Lighting", &m_lightingFrameBuffer);
	frameBufferCreated = m_pRenderer->CreateFrameBuffer(-1, true, true, true, true, m_windowWidth, m_windowHeight, 1.0f, "Transparency", &m_transparencyFrameBuffer);
	frameBufferCreated = m_pRenderer->CreateFrameBuffer(-1, true, true, true, true, m_windowWidth, m_windowHeight, 1.0f, "Water Reflection", &m_waterReflectionFrameBuffer);
	frameBufferCreated = m_pRenderer->CreateFrameBuffer(-1, true, true, true, true, m_windowWidth, m_windowHeight, 1.0f, "FXAA", &m_FXAAFrameBuffer);
	frameBufferCreated = m_pRenderer->CreateFrameBuffer(-1, true, true, true, true, m_windowWidth, m_windowHeight, 1.0f, "FullScreen 1st Pass", &m_firstPassFullscreenBuffer);
	frameBufferCreated = m_pRenderer->CreateFrameBuffer(-1, true, true, true, true, m_windowWidth, m_windowHeight, 1.0f, "FullScreen 2nd Pass", &m_secondPassFullscreenBuffer);
	frameBufferCreated = m_pRenderer->CreateFrameBuffer(-1, true, true, true, true, 800, 800, 1.0f, "Paperdoll", &m_paperdollBuffer);
	frameBufferCreated = m_pRenderer->CreateFrameBuffer(-1, true, true, true, true, 800, 800, 1.0f, "Paperdoll SSAO Texture", &m_paperdollSSAOTextureBuffer);
	frameBufferCreated = m_pRenderer->CreateFrameBuffer(-1, true, true, true, true, 800, 800, 1.0f, "Portrait", &m_portraitBuffer);
	frameBufferCreated = m_pRenderer->CreateFrameBuffer(-1, true, true, true, true, 800, 800, 1.0f, "Portrait SSAO Texture", &m_portraitSSAOTextureBuffer);

	/* Create the shaders */
	bool shaderLoaded = false;
	m_defaultShader = -1;
	m_phongShader = -1;
	m_SSAOShader = -1;
	m_shadowShader = -1;
	m_waterShader = -1;
	m_lightingShader = -1;
	m_cubeMapShader = -1;
	m_textureShader = -1;
	m_fxaaShader = -1;
	m_blurVerticalShader = -1;
	m_blurHorizontalShader = -1;
	m_paperdollShader = -1;
	shaderLoaded = m_pRenderer->LoadGLSLShader("media/shaders/default.vertex", "media/shaders/default.pixel", &m_defaultShader);
	shaderLoaded = m_pRenderer->LoadGLSLShader("media/shaders/phong.vertex", "media/shaders/phong.pixel", &m_phongShader);
	shaderLoaded = m_pRenderer->LoadGLSLShader("media/shaders/shadow.vertex", "media/shaders/shadow.pixel", &m_shadowShader);
	shaderLoaded = m_pRenderer->LoadGLSLShader("media/shaders/water_still.vertex", "media/shaders/water_still.pixel", &m_waterShader);
	shaderLoaded = m_pRenderer->LoadGLSLShader("media/shaders/texture.vertex", "media/shaders/texture.pixel", &m_textureShader);
	shaderLoaded = m_pRenderer->LoadGLSLShader("media/shaders/fullscreen/SSAO.vertex", "media/shaders/fullscreen/SSAO.pixel", &m_SSAOShader);
	shaderLoaded = m_pRenderer->LoadGLSLShader("media/shaders/fullscreen/fxaa.vertex", "media/shaders/fullscreen/fxaa.pixel", &m_fxaaShader);
	shaderLoaded = m_pRenderer->LoadGLSLShader("media/shaders/fullscreen/lighting.vertex", "media/shaders/fullscreen/lighting.pixel", &m_lightingShader);
	shaderLoaded = m_pRenderer->LoadGLSLShader("media/shaders/cube_map.vertex", "media/shaders/cube_map.pixel", &m_cubeMapShader);
	shaderLoaded = m_pRenderer->LoadGLSLShader("media/shaders/fullscreen/blur_vertical.vertex", "media/shaders/fullscreen/blur_vertical.pixel", &m_blurVerticalShader);
	shaderLoaded = m_pRenderer->LoadGLSLShader("media/shaders/fullscreen/blur_horizontal.vertex", "media/shaders/fullscreen/blur_horizontal.pixel", &m_blurHorizontalShader);
	shaderLoaded = m_pRenderer->LoadGLSLShader("media/shaders/paperdoll.vertex", "media/shaders/paperdoll.pixel", &m_paperdollShader);

	/* Create the mods manager */
	m_pModsManager = new ModsManager();
	m_pModsManager->LoadMods();

	/* Create the audio manager */
	AudioManager::GetInstance()->Setup();

	/* Create the qubicle binary file manager */
	m_pQubicleBinaryManager = new QubicleBinaryManager(m_pRenderer);

	/* Create the chunk manager*/
	m_pChunkManager = new ChunkManager(m_pRenderer, m_pVoxSettings, m_pQubicleBinaryManager);
	m_pChunkManager->SetStepLockEnabled(m_pVoxSettings->m_stepUpdating);

	/* Create the biome manager */
	m_pBiomeManager = new BiomeManager(m_pRenderer);

	/* Create the lighting manager */
	m_pLightingManager = new LightingManager(m_pRenderer);

	/* Create the scenery manager */
	m_pSceneryManager = new SceneryManager(m_pRenderer, m_pChunkManager);

	/* Create the skybox */
	m_pSkybox = new Skybox(m_pRenderer);

	/* Create the block particle manager */
	m_pBlockParticleManager = new BlockParticleManager(m_pRenderer, m_pChunkManager);

	/* Create the text effects manager */
	m_pTextEffectsManager = new TextEffectsManager(m_pRenderer);
	m_pTextEffectsManager->SetCamera(m_pGameCamera);

	/* Create the instance manager */
	m_pInstanceManager = new InstanceManager(m_pRenderer);

	/* Create the player */
	m_pPlayer = new Player(m_pRenderer, m_pChunkManager, m_pQubicleBinaryManager, m_pLightingManager, m_pBlockParticleManager);

	/* Create the NPC manager */
	m_pNPCManager = new NPCManager(m_pRenderer, m_pChunkManager);

	/* Create the enemy manager */
	m_pEnemyManager = new EnemyManager(m_pRenderer, m_pChunkManager, m_pPlayer);

	/* Create the quest manager */
	m_pQuestManager = new QuestManager();

	/* Create the quest journal */
	m_pQuestJournal = new QuestJournal(m_pQuestManager);

	/* Create the inventory manager */
	m_pInventoryManager = new InventoryManager();

	/* Create the item manager */
	m_pItemManager = new ItemManager(m_pRenderer, m_pChunkManager, m_pPlayer);

	/* Create the random loot manager */
	m_pRandomLootManager = new RandomLootManager();

	/* Create the projectile manager */
	m_pProjectileManager = new ProjectileManager(m_pRenderer, m_pChunkManager);

	/* Create the frontend manager */
	m_pFrontendManager = new FrontendManager(m_pRenderer, m_pGUI);
	m_pFrontendManager->SetWindowDimensions(m_windowWidth, m_windowHeight);
	m_pFrontendManager->SetCamera(m_pGameCamera);

	/* Create the game GUI pages */
	m_pInventoryGUI = new InventoryGUI(m_pRenderer, m_pGUI, m_pFrontendManager, m_pChunkManager, m_pPlayer, m_pInventoryManager, m_windowWidth, m_windowHeight);
	m_pCharacterGUI = new CharacterGUI(m_pRenderer, m_pGUI, m_pFrontendManager, m_pChunkManager, m_pPlayer, m_pInventoryManager, m_windowWidth, m_windowHeight);
	m_pLootGUI = new LootGUI(m_pRenderer, m_pGUI, m_pFrontendManager, m_pChunkManager, m_pPlayer, m_pInventoryManager, m_windowWidth, m_windowHeight);
	m_pCraftingGUI = new CraftingGUI(m_pRenderer, m_pGUI, m_pFrontendManager, m_pChunkManager, m_pPlayer, m_pInventoryManager, m_windowWidth, m_windowHeight);
	m_pQuestGUI = new QuestGUI(m_pRenderer, m_pGUI, m_pFrontendManager, m_pChunkManager, m_pPlayer, m_pInventoryManager, m_windowWidth, m_windowHeight);
	m_pActionBar = new ActionBar(m_pRenderer, m_pGUI, m_pFrontendManager, m_pChunkManager, m_pPlayer, m_pInventoryManager, m_windowWidth, m_windowHeight);
	m_pHUD = new HUD(m_pRenderer, m_pGUI, m_pFrontendManager, m_pChunkManager, m_pPlayer, m_pInventoryManager, m_windowWidth, m_windowHeight);

	/* Create module and manager linkage */
	m_pChunkManager->SetPlayer(m_pPlayer);
	m_pChunkManager->SetSceneryManager(m_pSceneryManager);
	m_pChunkManager->SetBiomeManager(m_pBiomeManager);
	m_pChunkManager->SetEnemyManager(m_pEnemyManager);
	m_pChunkManager->SetNPCManager(m_pNPCManager);
	m_pChunkManager->SetBlockParticleManager(m_pBlockParticleManager);
	m_pChunkManager->SetItemManager(m_pItemManager);
	m_pPlayer->SetInventoryManager(m_pInventoryManager);
	m_pPlayer->SetItemManager(m_pItemManager);
	m_pPlayer->SetProjectileManager(m_pProjectileManager);
	m_pPlayer->SetTextEffectsManager(m_pTextEffectsManager);
	m_pPlayer->SetEnemyManager(m_pEnemyManager);
	m_pPlayer->SetInventoryGUI(m_pInventoryGUI);
	m_pPlayer->SetCharacterGUI(m_pCharacterGUI);
	m_pPlayer->SetCraftingGUI(m_pCraftingGUI);
	m_pPlayer->SetLootGUI(m_pLootGUI);
	m_pPlayer->SetActionBar(m_pActionBar);
	m_pNPCManager->SetPlayer(m_pPlayer);
	m_pNPCManager->SetLightingManager(m_pLightingManager);
	m_pNPCManager->SetBlockParticleManager(m_pBlockParticleManager);
	m_pNPCManager->SetTextEffectsManager(m_pTextEffectsManager);
	m_pNPCManager->SetItemManager(m_pItemManager);
	m_pNPCManager->SetQubicleBinaryManager(m_pQubicleBinaryManager);
	m_pNPCManager->SetProjectileManager(m_pProjectileManager);
	m_pNPCManager->SetEnemyManager(m_pEnemyManager);
	m_pEnemyManager->SetLightingManager(m_pLightingManager);
	m_pEnemyManager->SetBlockParticleManager(m_pBlockParticleManager);
	m_pEnemyManager->SetTextEffectsManager(m_pTextEffectsManager);
	m_pEnemyManager->SetItemManager(m_pItemManager);
	m_pEnemyManager->SetProjectileManager(m_pProjectileManager);
	m_pEnemyManager->SetHUD(m_pHUD);
	m_pEnemyManager->SetQubicleBinaryManager(m_pQubicleBinaryManager);
	m_pEnemyManager->SetNPCManager(m_pNPCManager);
	m_pInventoryManager->SetPlayer(m_pPlayer);
	m_pInventoryManager->SetInventoryGUI(m_pInventoryGUI);
	m_pInventoryManager->SetLootGUI(m_pLootGUI);
	m_pInventoryManager->SetActionBar(m_pActionBar);
	m_pItemManager->SetLightingManager(m_pLightingManager);
	m_pItemManager->SetBlockParticleManager(m_pBlockParticleManager);
	m_pItemManager->SetQubicleBinaryManager(m_pQubicleBinaryManager);
	m_pItemManager->SetInventoryManager(m_pInventoryManager);
	m_pItemManager->SetNPCManager(m_pNPCManager);
	m_pProjectileManager->SetLightingManager(m_pLightingManager);
	m_pProjectileManager->SetBlockParticleManager(m_pBlockParticleManager);
	m_pProjectileManager->SetPlayer(m_pPlayer);
	m_pProjectileManager->SetQubicleBinaryManager(m_pQubicleBinaryManager);
	m_pQuestManager->SetNPCManager(m_pNPCManager);
	m_pQuestManager->SetInventoryManager(m_pInventoryManager);
	m_pQuestManager->SetQuestJournal(m_pQuestJournal);
	m_pQuestJournal->SetPlayer(m_pPlayer);
	m_pInventoryGUI->SetCharacterGUI(m_pCharacterGUI);
	m_pInventoryGUI->SetLootGUI(m_pLootGUI);
	m_pInventoryGUI->SetActionBar(m_pActionBar);
	m_pInventoryGUI->SetItemManager(m_pItemManager);
	m_pCharacterGUI->SetInventoryGUI(m_pInventoryGUI);
	m_pCharacterGUI->SetLootGUI(m_pLootGUI);
	m_pCharacterGUI->SetActionBar(m_pActionBar);
	m_pLootGUI->SetInventoryGUI(m_pInventoryGUI);
	m_pLootGUI->SetCharacterGUI(m_pCharacterGUI);
	m_pLootGUI->SetActionBar(m_pActionBar);
	m_pActionBar->SetInventoryGUI(m_pInventoryGUI);
	m_pActionBar->SetCharacterGUI(m_pCharacterGUI);
	m_pActionBar->SetLootGUI(m_pLootGUI);
	m_pQuestGUI->SetQuestJournal(m_pQuestJournal);
	m_pHUD->SetInventoryGUI(m_pInventoryGUI);
	m_pHUD->SetCharacterGUI(m_pCharacterGUI);
	m_pHUD->SetQuestGUI(m_pQuestGUI);
	m_pHUD->SetCraftingGUI(m_pCraftingGUI);

	// Keyboard movement
	m_bKeyboardForward = false;
	m_bKeyboardBackward = false;
	m_bKeyboardStrafeLeft = false;
	m_bKeyboardStrafeRight = false;
	m_bKeyboardLeft = false;
	m_bKeyboardRight = false;
	m_bKeyboardUp = false;
	m_bKeyboardDown = false;
	m_bKeyboardSpace = false;
	m_bKeyboardMenu = false;

	// Joystick flags
	m_bJoystickJump = false;

	// Combat flags
	m_bAttackPressed_Mouse = false;
	m_bAttackReleased_Mouse = false;
	m_bAttackPressed_Joystick = false;
	m_bAttackReleased_Joystick = false;
	m_bCanDoAttack_Mouse = true;
	m_bCanDoAttack_Joystick = true;
	m_bTargetEnemyPressed_Joystick = false;
	m_bTargetEnemyReleased_Joystick = false;

	// Camera movement
	m_bCameraRotate = false;
	m_pressedX = 0;
	m_pressedY = 0;
	m_currentX = 0;
	m_currentY = 0;
	m_cameraDistance = m_pGameCamera->GetZoomAmount();
	m_maxCameraDistance = m_cameraDistance;

	// Auto camera mode
	m_autoCameraMovingModifier = 1.0f;

	// Enemy target camera mode
	m_targetCameraXAxisAmount = 0.0f;
	m_targetCameraXAxisAmount_Target = 0.0f;
	m_targetCameraYRatio = 0.0f;
	m_targetCameraForwardRatio = 0.0f;

	// Player movement
	m_keyboardMovement = false;
	m_gamepadMovement = false;
	m_movementSpeed = 0.0f;
	m_movementDragTime = 0.45f;
	m_movementIncreaseTime = 0.25f;
	m_maxMovementSpeed = 10.0f;
	m_movementStopThreshold = 0.05f;

	// Blur
	m_globalBlurAmount = 0.0f;

	// Cinematic letterbox mode
	m_letterBoxRatio = 0.0f;

	// Water
	m_elapsedWaterTime = 0.0f;

	// Paperdoll rendering
	m_paperdollRenderRotation = 0.0f;

	// Toggle flags
	m_deferredRendering = true;
	m_modelWireframe = false;
	m_modelAnimationIndex = 0;
	m_multiSampling = true;
	m_ssao = true;
	m_blur = false;
	m_dynamicLighting = true;
	m_animationUpdate = true;
	m_fullscreen = m_pVoxSettings->m_fullscreen;
	m_debugRender = false;
	m_instanceRender = true;

	// Camera mode
	m_cameraMode = CameraMode_Debug;
	m_previousCameraMode = CameraMode_Debug;
	m_shouldRestorePreviousCameraMode = false;

	// Game mode
	m_gameMode = GameMode_Loading;
	m_allowToChangeToGame = true;
	m_allowToChangeToFrontend = true;
	SetGameMode(m_gameMode);

	// Turn the cursor initially off if we have custom cursors enabled
	if (m_pVoxSettings->m_customCursors)
	{
		TurnCursorOff(true);
	}

	// Create, setup and skin the GUI components
	CreateGUI();
	SetupGUI();
	SkinGUI();
	UpdateGUI(0.0f);
}

// Destruction
void VoxGame::Destroy()
{
	if (c_instance)
	{
		delete m_pSkybox;
		delete m_pChunkManager;
		delete m_pItemManager;
		delete m_pRandomLootManager;
		delete m_pInventoryManager;
		delete m_pFrontendManager;
		delete m_pPlayer;
		delete m_pNPCManager;
		delete m_pEnemyManager;
		delete m_pLightingManager;
		delete m_pSceneryManager;
		delete m_pBlockParticleManager;
		delete m_pTextEffectsManager;
		delete m_pInstanceManager;
		delete m_pBiomeManager;
		delete m_pQubicleBinaryManager;
		delete m_pModsManager;
		delete m_pGameCamera;
		delete m_pQuestManager;
		delete m_pQuestJournal;
		delete m_pInventoryGUI;
		delete m_pCharacterGUI;
		delete m_pLootGUI;
		delete m_pCraftingGUI;
		delete m_pQuestGUI;
		delete m_pActionBar;
		DestroyGUI();  // Destroy the GUI components before we delete the GUI manager object.
		delete m_pGUI;
		delete m_pRenderer;

		AudioManager::GetInstance()->Shutdown();

		m_pVoxWindow->Destroy();

		delete m_pVoxWindow;

		delete c_instance;
	}
}

// Quitting
void VoxGame::CancelQuitPopup()
{
	m_pFrontendManager->SetFrontendScreen(FrontendScreen_None);

	SetPaused(false);

	SetGlobalBlurAmount(0.0f);

	TurnCursorOff(false);
}

void VoxGame::ShowQuitPopup()
{
	if (m_pFrontendManager->GetFrontendScreen() != FrontendScreen_QuitPopup)
	{
		CloseAllGUIWindows();

		m_pFrontendManager->SetFrontendScreen(FrontendScreen_QuitPopup);

		SetPaused(true);

		SetGlobalBlurAmount(0.0015f);

		TurnCursorOn(false, false);
	}
}

void VoxGame::SetGameQuit(bool quit)
{
	m_bGameQuit = quit;
}

// Pause
bool VoxGame::IsPaused()
{
	return m_bPaused;
}

void VoxGame::SetPaused(bool pause)
{
	m_bPaused = pause;
}

void VoxGame::SetPauseMenu()
{
	m_pFrontendManager->SetFrontendScreen(FrontendScreen_PauseMenu);

	SetPaused(true);

	SetGlobalBlurAmount(0.0015f);

	TurnCursorOn(true, false);
}

void VoxGame::UnsetPauseMenu()
{
	m_pFrontendManager->SetFrontendScreen(FrontendScreen_None);

	SetPaused(false);

	SetGlobalBlurAmount(0.0f);

	TurnCursorOff(false);
}

// Blur
void VoxGame::SetGlobalBlurAmount(float blurAmount)
{
	m_globalBlurAmount = blurAmount;
	m_pBlurCheckBox->SetToggled(m_globalBlurAmount > 0.0f);
}

// Cinematic letterbox
void VoxGame::OpenLetterBox()
{
	Interpolator::GetInstance()->AddFloatInterpolation(&m_letterBoxRatio, m_letterBoxRatio, 1.0f, 0.25f, -100.0f);
}

void VoxGame::CloseLetterBox()
{
	Interpolator::GetInstance()->AddFloatInterpolation(&m_letterBoxRatio, m_letterBoxRatio, 0.0f, 0.25f, 100.0f);
}

// Paperdoll rendering
void VoxGame::SetPaperdollRotation(float rotation)
{
	m_paperdollRenderRotation = rotation;
}

void VoxGame::RotatePaperdollModel(float rot)
{
	m_paperdollRenderRotation += rot;// * m_deltaTime;
}

unsigned int VoxGame::GetDynamicPaperdollTexture()
{
	return m_pRenderer->GetDiffuseTextureFromFrameBuffer(m_paperdollSSAOTextureBuffer);
}

// Portrait
unsigned int VoxGame::GetDynamicPortraitTexture()
{
	return m_pRenderer->GetDiffuseTextureFromFrameBuffer(m_portraitSSAOTextureBuffer);
}

// Events
void VoxGame::PollEvents()
{
	m_pVoxWindow->PollEvents();
}

bool VoxGame::ShouldClose()
{
	return m_bGameQuit;
}

// Window functionality
int VoxGame::GetWindowCursorX()
{
	return m_pVoxWindow->GetCursorX();
}

int VoxGame::GetWindowCursorY()
{
	return m_pVoxWindow->GetCursorY();
}


void VoxGame::TurnCursorOn(bool resetCursorPosition, bool forceOn)
{
	m_pVoxWindow->TurnCursorOn(resetCursorPosition, forceOn);

	m_bCustomCursorOn = true;
}

void VoxGame::TurnCursorOff(bool forceOff)
{
	m_pVoxWindow->TurnCursorOff(forceOff);

	m_bCustomCursorOn = false;

	// Make sure to set the current X and Y when we turn the cursor off, so that camera controls don't glitch.
	m_currentX = m_pVoxWindow->GetCursorX();
	m_currentY = m_pVoxWindow->GetCursorY();
}

bool VoxGame::IsCursorOn()
{
	if (m_pVoxSettings->m_customCursors)
	{
		return m_bCustomCursorOn;
	}
	else
	{
		return m_pVoxWindow->IsCursorOn();
	}
}

void VoxGame::ResizeWindow(int width, int height)
{
	m_windowWidth = width;
	m_windowHeight = height;

	m_pVoxWindow->ResizeWindow(m_windowWidth, m_windowHeight);

	if(m_pRenderer)
	{
		// Let the renderer know we have resized the window
		m_pRenderer->ResizeWindow(m_windowWidth, m_windowHeight);

		// Resize the main viewport
		m_pRenderer->ResizeViewport(m_defaultViewport, 0, 0, m_windowWidth, m_windowHeight, 60.0f);
		m_pRenderer->ResizeViewport(m_firstpersonViewport, 0, 0, m_windowWidth, m_windowHeight, 60.0f);
		m_pRenderer->ResizeViewport(m_paperdollViewport, m_paperdollViewportY, m_paperdollViewportX, m_paperdollViewportWidth, m_paperdollViewportHeight, 60.0f);
		m_pRenderer->ResizeViewport(m_portraitViewport, m_portraitViewportY, m_portraitViewportX, m_portraitViewportWidth, m_portraitViewportHeight, 60.0f);

		// Resize the frame buffers
		bool frameBufferResize = false;
		frameBufferResize = m_pRenderer->CreateFrameBuffer(m_SSAOFrameBuffer, true, true, true, true, m_windowWidth, m_windowHeight, 1.0f, "SSAO", &m_SSAOFrameBuffer);
		frameBufferResize = m_pRenderer->CreateFrameBuffer(m_shadowFrameBuffer, true, true, true, true, m_windowWidth, m_windowHeight, 5.0f, "Shadow", &m_shadowFrameBuffer);
		frameBufferResize = m_pRenderer->CreateFrameBuffer(m_lightingFrameBuffer, true, true, true, true, m_windowWidth, m_windowHeight, 1.0f, "Deferred Lighting", &m_lightingFrameBuffer);
		frameBufferResize = m_pRenderer->CreateFrameBuffer(m_transparencyFrameBuffer, true, true, true, true, m_windowWidth, m_windowHeight, 1.0f, "Transparency", &m_transparencyFrameBuffer);
		frameBufferResize = m_pRenderer->CreateFrameBuffer(m_waterReflectionFrameBuffer, true, true, true, true, m_windowWidth, m_windowHeight, 1.0f, "Water Reflection", &m_waterReflectionFrameBuffer);
		frameBufferResize = m_pRenderer->CreateFrameBuffer(m_FXAAFrameBuffer, true, true, true, true, m_windowWidth, m_windowHeight, 1.0f, "FXAA", &m_FXAAFrameBuffer);
		frameBufferResize = m_pRenderer->CreateFrameBuffer(m_firstPassFullscreenBuffer, true, true, true, true, m_windowWidth, m_windowHeight, 1.0f, "FullScreen 1st Pass", &m_firstPassFullscreenBuffer);
		frameBufferResize = m_pRenderer->CreateFrameBuffer(m_secondPassFullscreenBuffer, true, true, true, true, m_windowWidth, m_windowHeight, 1.0f, "FullScreen 2nd Pass", &m_secondPassFullscreenBuffer);
		frameBufferResize = m_pRenderer->CreateFrameBuffer(m_paperdollBuffer, true, true, true, true, 800, 800, 1.0f, "Paperdoll", &m_paperdollBuffer);
		frameBufferResize = m_pRenderer->CreateFrameBuffer(m_paperdollSSAOTextureBuffer, true, true, true, true, 800, 800, 1.0f, "Paperdoll SSAO Texture", &m_paperdollSSAOTextureBuffer);
		frameBufferResize = m_pRenderer->CreateFrameBuffer(m_portraitBuffer, true, true, true, true, 800, 800, 1.0f, "Portrait", &m_portraitBuffer);
		frameBufferResize = m_pRenderer->CreateFrameBuffer(m_portraitSSAOTextureBuffer, true, true, true, true, 800, 800, 1.0f, "Portrait SSAO Texture", &m_portraitSSAOTextureBuffer);

		// Give the new windows dimensions to the GUI components also
		m_pMainWindow->SetApplicationDimensions(m_windowWidth, m_windowHeight);
		m_pGameWindow->SetApplicationDimensions(m_windowWidth, m_windowHeight);
		m_pConsoleWindow->SetApplicationDimensions(m_windowWidth, m_windowHeight);
	}

	// Frontend
	if (m_pFrontendManager)
	{
		m_pFrontendManager->SetWindowDimensions(m_windowWidth, m_windowHeight);
	}

	// Resize game GUI
	if (m_pInventoryGUI)
	{
		m_pInventoryGUI->SetWindowDimensions(m_windowWidth, m_windowHeight);
	}
	if (m_pCharacterGUI)
	{
		m_pCharacterGUI->SetWindowDimensions(m_windowWidth, m_windowHeight);
	}
	if (m_pLootGUI)
	{
		m_pLootGUI->SetWindowDimensions(m_windowWidth, m_windowHeight);
	}
	if (m_pCraftingGUI)
	{
		m_pCraftingGUI->SetWindowDimensions(m_windowWidth, m_windowHeight);
	}
	if (m_pActionBar)
	{
		m_pActionBar->SetWindowDimensions(m_windowWidth, m_windowHeight);
	}
	if (m_pHUD)
	{
		m_pHUD->SetWindowDimensions(m_windowWidth, m_windowHeight);
	}
}

void VoxGame::CloseWindow()
{
	if (m_gameMode == GameMode_Game)
	{
		ShowQuitPopup();
	}
	else
	{
		m_bGameQuit = true;
	}
}

void VoxGame::UpdateJoySticks()
{
	m_pVoxWindow->UpdateJoySticks();
}

// Music
void VoxGame::StartFrontEndMusic()
{
	string musicModName = VoxGame::GetInstance()->GetModsManager()->GetSoundPack();
	string musicFileName = "media/audio/" + musicModName + "/music/vox_intro.ogg";
	m_pMusicSound = AudioManager::GetInstance()->PlaySound2D(&m_pMusicChannel, musicFileName.c_str(), true, true);

	UpdateMusicVolume(0.0f);
}

void VoxGame::StartGameMusic()
{
	Biome currentBiome = m_pBiomeManager->GetBiome(m_pPlayer->GetCenter());
	m_currentBiomeMusic = currentBiome;

	string biomeFileName = "";
	switch (m_currentBiomeMusic)
	{
		case Biome_None:		{ break; }
		case Biome_GrassLand:	{ biomeFileName = "biome_plains.ogg"; break; }
		case Biome_Desert:		{ biomeFileName = "biome_desert.ogg"; break; }
		//case Biome_Jungle:	{ biomeFileName = "biome_jungle.ogg";break; }
		case Biome_Tundra:		{ biomeFileName = "biome_snow.ogg"; break; }
		//case Biome_Swamp:		{ biomeFileName = "";break; }
		case Biome_AshLand:		{ biomeFileName = "biome_nightmare.ogg"; break; }
		//case Biome_Nightmare:	{ biomeFileName = "";break; }
	}

	string musicModName = VoxGame::GetInstance()->GetModsManager()->GetSoundPack();
	string musicFileName = "media/audio/" + musicModName + "/music/" + biomeFileName;
	m_pMusicSound = AudioManager::GetInstance()->PlaySound2D(&m_pMusicChannel, musicFileName.c_str(), true, true);

	UpdateMusicVolume(0.0f);
}

void VoxGame::StopMusic()
{
	// Stop the music
	AudioManager::GetInstance()->StopSound(m_pMusicChannel);

	m_pMusicChannel = nullptr;
	m_pMusicSound = nullptr;
}

void VoxGame::UpdateGameMusic(float dt)
{
	Biome currentBiome = m_pBiomeManager->GetBiome(m_pPlayer->GetCenter());

	if (currentBiome != m_currentBiomeMusic)
	{
		StopMusic();
		StartGameMusic();
	}
}

void VoxGame::UpdateMusicVolume(float dt)
{
	if (m_pVoxSettings->m_music)
	{
		if (m_pMusicChannel != nullptr)
		{
			m_pMusicChannel->setVolume(0.125f * m_pVoxSettings->m_musicVolume);
		}
	}
	else
	{
		if (m_pMusicChannel != nullptr)
		{
			m_pMusicChannel->setVolume(0.0f);
		}
	}
}

// Sounds
void VoxGame::PlaySoundEffect(eSoundEffect soundEffect, float soundEnhanceMultiplier)
{
	if (m_pVoxSettings->m_audio)
	{
		string soundModName = VoxGame::GetInstance()->GetModsManager()->GetSoundPack();
		string soundeffectFilename = g_soundEffectFilenames[soundEffect];
		string soundFileName = "media/audio/" + soundModName + "/soundeffects/" + soundeffectFilename;

		FMOD::Channel* pSoundChannel;
		FMOD::Sound* pSound;
		pSound = AudioManager::GetInstance()->PlaySound2D(&pSoundChannel, soundFileName.c_str(), false);
		pSoundChannel->setVolume(soundEnhanceMultiplier * m_pVoxSettings->m_audioVolume);
	}
}

void VoxGame::PlaySoundEffect3D(eSoundEffect soundEffect, vec3 soundPosition, float soundEnhanceMultiplier)
{
	if (m_pVoxSettings->m_audio)
	{
		string soundModName = VoxGame::GetInstance()->GetModsManager()->GetSoundPack();
		string soundeffectFilename = g_soundEffectFilenames[soundEffect];
		string soundFileName = "media/audio/" + soundModName + "/soundeffects/" + soundeffectFilename;

		FMOD::Channel* pSoundChannel;
		FMOD::Sound* pSound;
		pSound = AudioManager::GetInstance()->PlaySound3D(&pSoundChannel, soundFileName.c_str(), soundPosition, false);
		pSoundChannel->setVolume(3.0f * soundEnhanceMultiplier * m_pVoxSettings->m_audioVolume);
	}
}

// Game functions
void VoxGame::QuitToFrontEnd()
{
	TurnCursorOn(true, false);
	SetGameMode(GameMode_FrontEnd);

	m_pFrontEndOptionBox->SetToggled(true);
	m_pFrontendCameraOptionBox->SetToggled(true);
	m_pMouseRotateCameraOptionBox->SetDisabled(true);
	m_pAutoCameraOptionBox->SetDisabled(true);
	m_pFrontendCameraOptionBox->SetDisabled(false);
	m_pDebugCameraOptionBox->SetDisabled(false);
	m_pVoxWindow->Update(m_deltaTime);
	GameModeChanged();
	CameraModeChanged();

	// Make sure that movement speed drag is reset, so that we dont carry on moving after quitting and restarting game
	m_movementSpeed = 0.0f;

	// Set front-end page to intro
	m_pFrontendManager->SetFrontendScreen(FrontendScreen_MainMenu);
}

void VoxGame::SetupDataForGame()
{
	// Initial player startup position and rotation
	vec3 starPosition = vec3(10.0f, 8.0f, 23.0f);
	m_pPlayer->SetPosition(starPosition);
	m_pPlayer->SetRespawnPosition(starPosition + vec3(0.0f, 0.1f, 0.0f));
	m_pPlayer->SetRotation(90.0f);

	// Items
	Item* pFurnace = m_pItemManager->CreateItem(vec3(25.0f, 10.0f, 29.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), "media/gamedata/items/Furnace/Furnace.item", eItem_Furnace, "Furnace", true, false, 0.16f);
	pFurnace->SetInteractionPositionOffset(vec3(0.0f, 0.0f, -2.0f));
	Item* pAnvil = m_pItemManager->CreateItem(vec3(32.0f, 9.0f, 26.5f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), "media/gamedata/items/Anvil/Anvil.item", eItem_Anvil, "Anvil", true, false, 0.14f);
	pAnvil->SetInteractionPositionOffset(vec3(0.0f, 0.0f, -1.5f));
	
	// Chest with random loot item
	Item* pChest = m_pItemManager->CreateItem(vec3(17.0f, 12.0f, 28.5f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), "media/gamedata/items/Chest/Chest.item", eItem_Chest, "Chest", true, false, 0.08f);
	eEquipment equipment = eEquipment_None;
	InventoryItem* pRandomLoot = VoxGame::GetInstance()->GetRandomLootManager()->GetRandomLootItem(&equipment);
	if (pRandomLoot != nullptr && equipment != eEquipment_None)
	{
		InventoryItem* pRandomLootItem = pChest->AddLootItem(pRandomLoot, 0, 2);
		pRandomLootItem->m_scale = pRandomLoot->m_scale;
		pRandomLootItem->m_offsetX = pRandomLoot->m_offsetX;
		pRandomLootItem->m_offsetY = pRandomLoot->m_offsetY;
		pRandomLootItem->m_offsetZ = pRandomLoot->m_offsetZ;
		pRandomLootItem->m_left = pRandomLoot->m_left;
		pRandomLootItem->m_right = pRandomLoot->m_right;
	}

	// Item spawners
	ItemSpawner* pItemSpawner1 = m_pItemManager->CreateItemSpawner(vec3(0.0f, 6.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
	pItemSpawner1->SetSpawningParams(0.0f, 0.0f, 15, vec3(0.0f, 0.0f, 0.0f), true, vec3(0.0f, 0.5f, 0.0f), true, true, 25.0f, Biome_GrassLand, 0.15f);
	pItemSpawner1->AddItemTypeToSpawn(eItem_CopperVein);
	pItemSpawner1->AddItemTypeToSpawn(eItem_IronVein);

	ItemSpawner* pItemSpawner2 = m_pItemManager->CreateItemSpawner(vec3(0.0f, 6.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
	pItemSpawner2->SetSpawningParams(0.0f, 0.0f, 15, vec3(0.0f, 0.0f, 0.0f), true, vec3(0.0f, 0.5f, 0.0f), true, true, 25.0f, Biome_AshLand, 0.15f);
	pItemSpawner2->AddItemTypeToSpawn(eItem_SilverVein);
	pItemSpawner2->AddItemTypeToSpawn(eItem_GoldVein);

	ItemSpawner* pItemSpawner3 = m_pItemManager->CreateItemSpawner(vec3(0.0f, 6.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
	pItemSpawner3->SetSpawningParams(0.0f, 0.0f, 3, vec3(0.0f, 0.0f, 0.0f), true, vec3(0.0f, 0.5f, 0.0f), true, true, 25.0f, Biome_GrassLand, 0.08f);
	pItemSpawner3->AddItemTypeToSpawn(eItem_Chest);

	// Npcs
	NPC* pCharacter1 = m_pNPCManager->CreateNPC("Mage", "Human", "Mage", vec3(21.0f, 8.5f, 25.0f), 0.08f, false, true);
	pCharacter1->SetForwards(vec3(0.0f, 0.0f, -1.0f));
	pCharacter1->SetTargetForwards(vec3(0.0f, 0.0f, -1.0f));
	pCharacter1->SetNPCCombatType(eNPCCombatType_Staff, true);

	// Safezones (Where we cannot spawn enemies)
	m_pBiomeManager->AddSafeZone(vec3(21.0f, 8.5f, 20.0f), 25.f, 30.0f, 25.0f);
	m_pBiomeManager->AddTown(vec3(8.0f, 8.0f, 8.0f), 75.f, 15.0f, 75.0f);

	// Enemies
	//Enemy* pEnemy0 = m_pEnemyManager->CreateEnemy(vec3(35.5f, 12.0f, 5.5f), eEnemyType_RedSlime, 0.08f);

	// Enemy spawners
	// Grassland
	EnemySpawner* pEnemySpawner0 = m_pEnemyManager->CreateEnemySpawner(vec3(0.0f, 4.0f, 0.0f), vec3(0.0f, 0.0f, 5.0f));
	pEnemySpawner0->SetSpawningParams(3.0f, 3.0f, 8, vec3(0.0f, 0.0f, 0.0f), true, vec3(0.0f, 1.0f, 0.0f), true, true, 25.0f, Biome_GrassLand);
	pEnemySpawner0->AddEnemyTypeToSpawn(eEnemyType_GreenSlime);
	pEnemySpawner0->AddEnemyTypeToSpawn(eEnemyType_RedSlime);
	//pEnemySpawner0->AddEnemyTypeToSpawn(eEnemyType_BlueSlime);
	pEnemySpawner0->AddEnemyTypeToSpawn(eEnemyType_YellowSlime);

	EnemySpawner* pEnemySpawner1 = m_pEnemyManager->CreateEnemySpawner(vec3(0.0f, 6.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
	pEnemySpawner1->SetSpawningParams(5.0f, 3.5f, 3, vec3(0.0f, 5.0f, 0.0f), true, vec3(0.0f, 5.0f, 0.0f), true, true, 25.0f, Biome_GrassLand);
	pEnemySpawner1->AddEnemyTypeToSpawn(eEnemyType_Bee);

	EnemySpawner* pEnemySpawner2 = m_pEnemyManager->CreateEnemySpawner(vec3(0.0f, 8.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
	pEnemySpawner2->SetSpawningParams(0.0f, 1.0f, 3, vec3(0.0f, 5.0f, 0.0f), true, vec3(0.0f, 0.0f, 0.0f), true, true, 25.0f, Biome_GrassLand);
	pEnemySpawner2->AddEnemyTypeToSpawn(eEnemyType_Mimic);

	// Desert
	EnemySpawner* pEnemySpawner3 = m_pEnemyManager->CreateEnemySpawner(vec3(0.0f, 10.0f, 0.0f), vec3(0.0f, 0.0f, 5.0f));
	pEnemySpawner3->SetSpawningParams(3.0f, 3.0f, 8, vec3(0.0f, 0.0f, 0.0f), true, vec3(0.0f, 1.0f, 0.0f), true, true, 25.0f, Biome_Desert);
	pEnemySpawner3->AddEnemyTypeToSpawn(eEnemyType_NormalSkeleton);
	pEnemySpawner3->AddEnemyTypeToSpawn(eEnemyType_RangedSkeleton);
	pEnemySpawner3->AddEnemyTypeToSpawn(eEnemyType_MeleeSkeleton);
	pEnemySpawner3->AddEnemyTypeToSpawn(eEnemyType_MageSkeleton);

	EnemySpawner* pEnemySpawner4 = m_pEnemyManager->CreateEnemySpawner(vec3(0.0f, 12.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
	pEnemySpawner4->SetSpawningParams(0.0f, 1.0f, 4, vec3(0.0f, 5.0f, 0.0f), true, vec3(0.0f, 0.0f, 0.0f), true, true, 25.0f, Biome_Desert);
	pEnemySpawner4->AddEnemyTypeToSpawn(eEnemyType_Mummy);

	// Ashlands
	EnemySpawner* pEnemySpawner5 = m_pEnemyManager->CreateEnemySpawner(vec3(0.0f, 10.0f, 0.0f), vec3(0.0f, 0.0f, 5.0f));
	pEnemySpawner5->SetSpawningParams(3.0f, 3.0f, 15, vec3(0.0f, 0.0f, 0.0f), true, vec3(0.0f, 1.0f, 0.0f), true, true, 25.0f, Biome_AshLand);
	pEnemySpawner5->AddEnemyTypeToSpawn(eEnemyType_WalkingZombie);
	pEnemySpawner5->AddEnemyTypeToSpawn(eEnemyType_CrawlingZombie);

	EnemySpawner* pEnemySpawner6 = m_pEnemyManager->CreateEnemySpawner(vec3(0.0f, 12.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
	pEnemySpawner6->SetSpawningParams(0.0f, 1.0f, 4, vec3(0.0f, 5.0f, 0.0f), true, vec3(0.0f, 0.0f, 0.0f), true, true, 25.0f, Biome_AshLand);
	pEnemySpawner6->AddEnemyTypeToSpawn(eEnemyType_IronGiant);

	EnemySpawner* pEnemySpawner7 = m_pEnemyManager->CreateEnemySpawner(vec3(0.0f, 12.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
	pEnemySpawner7->SetSpawningParams(0.0f, 1.0f, 8, vec3(0.0f, 5.0f, 0.0f), true, vec3(0.0f, 5.0f, 0.0f), true, true, 25.0f, Biome_AshLand);
	pEnemySpawner7->AddEnemyTypeToSpawn(eEnemyType_Bat);

	// Tundra
	EnemySpawner* pEnemySpawner8 = m_pEnemyManager->CreateEnemySpawner(vec3(0.0f, 4.0f, 0.0f), vec3(0.0f, 0.0f, 5.0f));
	pEnemySpawner8->SetSpawningParams(3.0f, 3.0f, 8, vec3(0.0f, 0.0f, 0.0f), true, vec3(0.0f, 1.0f, 0.0f), true, true, 25.0f, Biome_Tundra);
	pEnemySpawner8->AddEnemyTypeToSpawn(eEnemyType_BlueSlime);

	// Quests
	// Quest 1
	string startText1 = "Hello there brave adventurer, my name is [C=Custom(00A2E8)]Melinda The Witch[C=White] and I have stumbled across a 'valuable treasure'. I can give you this treasure if you will do a quest for me...\nI need you to kill these pesky [C=Red]slimes[C=White] that are overrunning this place. If you do this for me, the treasure is yours. What do you say?";
	string completedText1 = "You have completed the quest, and destroyed the sliminess of these lands, well done!";
	string denyText1 = "You are already on a quest, come back to me once you have finished.";
	Quest* pSlimeQuest = m_pQuestManager->CreateQuest("A Simple Task", startText1, completedText1, denyText1);
	pSlimeQuest->AddQuestObjective("Kill 5 Slimes", QuestType_KillX, 5, eEnemyType_GreenSlime, eItem_None, nullptr, "", nullptr);
	InventoryItem* pQuestReward1 = m_pInventoryManager->CreateEquipmentItemFromType(eEquipment_IronSword);
	pSlimeQuest->SetQuestReward(pQuestReward1);
	pSlimeQuest->ExportQuest();
	// Quest 2
	string startText2 = "You look like the type of adventurer who really needs some copper, Please could you be a good fella and collect some copper nuggets for me, they are required for refining into copper bars.";
	string completedText2 = "Wow... thank you for collecting the copper nuggets, you can keep it as a reward.";
	string denyText2 = "You are already on a quest, come back to me once you have finished.";
	Quest* pCollectQuest = m_pQuestManager->CreateQuest("A Simple Collection", startText2, completedText2, denyText2);
	pCollectQuest->AddQuestObjective("Collect 5 Copper Nuggets", QuestType_CollectX, 5, eEnemyType_None, eItem_CopperOre, nullptr, "", nullptr);
	InventoryItem* pQuestReward2 = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_CopperOre, 5, ItemQuality_Common);
	pCollectQuest->SetQuestReward(pQuestReward2);
	pCollectQuest->ExportQuest();
	// Quest 3
	string startText3 = "My dearest [C=Yellow]Moxie[C=White] is lost and I can't find her. I have looked everywhere, but she is nowhere to be found...\n\nCan you lend me your assistance and help locate my dearest [C=Yellow]Moxie[C=White]? I dont know where she could be, but the last time I saw her she was playing around by [C=Custom(00A2E8)]Joseph Maclure's Barnyard[C=White].\n\nIf you can help find [C=Yellow]Moxie[C=White] for me I will reward you handsomely.";
	string completedText3 = "You have found Moxie? Thank you so much kind stranger!";
	string denyText3 = "You are already on a quest, come back to me once you have finished.";
	Quest* pFindQuest = m_pQuestManager->CreateQuest("Find Moxie", startText3, completedText3, denyText3);
	pFindQuest->ExportQuest();

	m_pQuestJournal->AddQuestJournalEntry(pSlimeQuest);
	m_pQuestJournal->AddQuestJournalEntry(pCollectQuest);
	m_pQuestJournal->AddQuestJournalEntry(pFindQuest);
}

void VoxGame::SetupDataForFrontEnd()
{
	// Safezones (Where we cannot spawn enemies)
	m_pBiomeManager->AddSafeZone(vec3(21.0f, 8.5f, 20.0f), 25.f, 30.0f, 25.0f);
	m_pBiomeManager->AddTown(vec3(8.0f, 8.0f, 8.0f), 75.f, 15.0f, 75.0f);
}

void VoxGame::StartGameFromFrontEnd()
{
	m_pFrontendManager->SetFrontendScreen(FrontendScreen_None);

	m_previousCameraMode = CameraMode_MouseRotate;
	m_pGameOptionBox->SetToggled(true);
	m_pMouseRotateCameraOptionBox->SetToggled(true);
	m_pFrontendCameraOptionBox->SetDisabled(true);
	GameModeChanged();
	CameraModeChanged();

	// Start game from player
	m_pPlayer->StartGame();
}

void VoxGame::PlayerRespawned()
{
	// Stop any movement drag when we respawn
	m_movementSpeed = 0.0f;
}

void VoxGame::SetGameMode(GameMode mode)
{
	GameMode previousgameMode = m_gameMode;
	m_gameMode = mode;

	if (m_gameMode == GameMode_Debug)
	{
	}

	if (m_gameMode == GameMode_FrontEnd)
	{
		if (previousgameMode == GameMode_Game || previousgameMode == GameMode_Loading)
		{
			// Close all open GUI windows
			CloseAllGUIWindows();

			// Clear the items
			m_pItemManager->ClearItems();
			m_pItemManager->ClearItemSpawners();

			// Clear the NPCs
			m_pNPCManager->ClearNPCs();

			// Clear the enemies and enemy spawners
			m_pEnemyManager->ClearEnemies();
			m_pEnemyManager->ClearEnemySpawners();

			// Clear all projectiles
			m_pProjectileManager->ClearProjectiles();

			// Reset the inventory manager
			m_pInventoryManager->Reset();

			// Clear the quests
			m_pQuestManager->ClearQuests();

			// Clear the quest journal
			m_pQuestJournal->ClearJournal();

			// Reset the quest GUI text components
			m_pQuestGUI->SetQuestData("", "");

			// Reset the player
			m_pPlayer->ResetPlayer();

			// Set the water level
			m_pChunkManager->SetWaterHeight(1.3f);

			// Unload actionbar
			if (m_pActionBar->IsLoaded())
			{
				if (m_pVoxSettings->m_renderGUI)
				{
					m_pActionBar->Unload();
				}
			}

			// Unload the HUD
			if (m_pHUD->IsLoaded())
			{
				if (m_pVoxSettings->m_renderGUI)
				{
					m_pHUD->Unload();
				}
			}

			// Setup the gamedata since we have just loaded fresh into the frontend.
			SetupDataForFrontEnd();

			// Music
			StopMusic();
			StartFrontEndMusic();

			// Initial chunk creation
			m_pChunkManager->InitializeChunkCreation();
		}
	}

	if (m_gameMode == GameMode_Game)
	{
		if (previousgameMode == GameMode_FrontEnd || previousgameMode == GameMode_Loading)
		{
			// Close all open GUI windows
			CloseAllGUIWindows();

			// Clear the items
			m_pItemManager->ClearItems();
			m_pItemManager->ClearItemSpawners();

			// Clear the NPCs
			m_pNPCManager->ClearNPCs();

			// Clear the enemies and enemy spawners
			m_pEnemyManager->ClearEnemies();
			m_pEnemyManager->ClearEnemySpawners();

			// Clear all projectiles
			m_pProjectileManager->ClearProjectiles();

			// Clear the quests
			m_pQuestManager->ClearQuests();

			// Clear the quest journal
			m_pQuestJournal->ClearJournal();

			// Reset the quest GUI text components
			m_pQuestGUI->SetQuestData("", "");

			// Reset the player
			m_pPlayer->ResetPlayer();

			// Set the water level
			m_pChunkManager->SetWaterHeight(1.3f);

			// Load action bar
			if (m_pActionBar->IsLoaded() == false)
			{
				if (m_pVoxSettings->m_renderGUI)
				{
					m_pActionBar->Load();
				}
			}

			// Load the HUD
			if (m_pHUD->IsLoaded() == false)
			{
				if (m_pVoxSettings->m_renderGUI)
				{
					m_pHUD->Load();
				}
			}

			// Setup the gamedata since we have just loaded fresh into a game.
			SetupDataForGame();

			// Music
			StopMusic();
			StartGameMusic();

			// Initial chunk creation
			m_pChunkManager->InitializeChunkCreation();
		}
	}
}

GameMode VoxGame::GetGameMode()
{
	return m_gameMode;
}

void VoxGame::SetCameraMode(CameraMode mode)
{
	m_cameraMode = mode;
}

CameraMode VoxGame::GetCameraMode()
{
	return m_cameraMode;
}

// Interactions
bool VoxGame::CheckInteractions()
{
	bool interaction = false;

	if (m_bPaused)
	{
		return false;
	}

	if (m_pPlayer->CanJump() == false)
	{
		// Don't allow interactions if we are jumping
		return false;
	}

	if (m_pPlayer->IsDead())
	{
		// Don't allow interactions if the player is dead.
		return false;
	}

	if (m_pPlayer->GetTargetEnemy() != nullptr)
	{
		// Don't allow interactions while we are in target mode
		return false;
	}

	// Check item interactions
	m_interactItemMutex.lock();
	if (interaction == false && m_pInteractItem != nullptr)
	{
		bool shouldStopMovement = false;

		// Dropped items become collectible by the player and magnet towards him
		if (m_pInteractItem->GetItemType() == eItem_DroppedItem)
		{
			if (m_pInventoryManager->CanAddInventoryItem(m_pInteractItem->GetDroppedInventoryItem()->m_title.c_str(), m_pInteractItem->GetDroppedInventoryItem()->m_item, m_pInteractItem->GetDroppedInventoryItem()->m_quantity))
			{
				m_pInteractItem->SetIsCollectible(true);
				m_pInteractItem->SetCollectionDelay(0.0f);
			}

			interaction = true;
		}

		// Crafting stations
		if (m_pInteractItem->GetItemType() == eItem_Anvil || m_pInteractItem->GetItemType() == eItem_Furnace)
		{
			m_pPlayer->StopMoving();
			shouldStopMovement = true;

			// Load crafting GUI
			if (m_pCraftingGUI->IsLoaded() == false)
			{
				m_pCraftingGUI->SetCraftingRecipesForItem(m_pInteractItem->GetItemType());
				m_pCraftingGUI->Load(true, 0.9f);
				m_pCraftingGUI->SetInteractionitem(m_pInteractItem);

				SavePreviousCameraMode();
				m_shouldRestorePreviousCameraMode = true;
				TurnCursorOn(false, false);
			}

			// Set NPC dialog camera mode
			SetCameraMode(CameraMode_NPCDialog);

			// Figure out which way to position the camera, based on how we are looking at the NPC when interacting
			vec3 toItem = normalize(m_pInteractItem->GetCenter() - m_pInteractItem->GetInteractionPosition());
			vec3 crossResult = cross(vec3(0.0f, 1.0f, 0.0f), toItem);
			float dotAngle = dot(m_pGameCamera->GetFacing(), crossResult);
			if (dotAngle > 0.5f)
			{
				vec3 center = (m_pInteractItem->GetCenter() - m_pInteractItem->GetInteractionPosition());
				vec3 crossRight = cross(normalize(center), vec3(0.0f, 1.0f, 0.0f));
				m_targetCameraPosition_NPCDialog = (m_pInteractItem->GetInteractionPosition() + center*2.0f + crossRight*4.0f + vec3(0.0f, 1.0f, 0.0f)*2.5f);
				m_targetCameraView_NPCDialog = (m_pInteractItem->GetInteractionPosition() + center*0.0f - crossRight*4.0f);
			}
			else
			{
				vec3 center = (m_pInteractItem->GetCenter() - m_pInteractItem->GetInteractionPosition());
				vec3 crossRight = cross(normalize(center), vec3(0.0f, 1.0f, 0.0f));
				m_targetCameraPosition_NPCDialog = (m_pInteractItem->GetInteractionPosition() + center*2.0f - crossRight*4.0f + vec3(0.0f, 1.0f, 0.0f)*2.5f);
				m_targetCameraView_NPCDialog = (m_pInteractItem->GetInteractionPosition() + center*0.0f + crossRight*4.0f);
			}

			// Player move to interaction point and look at interaction item
			m_pPlayer->SetMoveToTargetPosition(m_pInteractItem->GetInteractionPosition());
			m_pPlayer->SetLookAtTargetAfterMoveToPosition(m_pInteractItem->GetCenter());

			// Set player alpha to full opacity
			m_pPlayer->SetPlayerAlpha(1.0f);

			// Open cinematic letterbox
			OpenLetterBox();

			interaction = true;
		}

		// Chest interaction
		if (m_pInteractItem->GetItemType() == eItem_Chest)
		{
			if (m_pInteractItem->IsStillAnimating() == false)
			{
				m_pInteractItem->Interact();

				interaction = true;

				if (m_pInteractItem->IsInteracting() == true) // Only open the GUI screens if we are opening a chest
				{
					m_pPlayer->StopMoving();
					shouldStopMovement = true;

					PlaySoundEffect(eSoundEffect_ChestOpen);

					if (m_pLootGUI->IsLoaded())
					{
						m_pLootGUI->Unload();

						if (IsGUIWindowStillDisplayed() == false)
						{
							TurnCursorOff(false);
						}
					}
					else if (m_pFrontendManager->GetFrontendScreen() == FrontendScreen_None)
					{
						m_pLootGUI->LoadItems(m_pInteractItem);

						m_pLootGUI->Load(true, 1.0f);

						if (m_pInventoryGUI->IsLoaded() == false)
						{
							m_pInventoryGUI->Load(true, 1.0f);
						}

						m_pPlayer->StopMoving();
						shouldStopMovement = true;

						TurnCursorOn(false, false);
					}
				}
			}
		}

		if (shouldStopMovement == true)
		{
			// Stop any movement drag when we interact with item
			m_movementSpeed = 0.0f;
		}
	}
	m_interactItemMutex.unlock();

	return interaction;
}

Item* VoxGame::GetInteractItem()
{
	return m_pInteractItem;
}

// Enemy Targeting
void VoxGame::SetEnemyTarget()
{
	if (m_pPlayer->IsDead() == false && m_pPlayer->GetTargetEnemy() == nullptr)
	{
		// Get the enemy based on the cursor
		int cursorX = (int)(m_windowWidth*0.5f);
		int cursorY = (int)(m_windowHeight*0.5f);

		Enemy* pEnemy = nullptr;
		pEnemy = m_pEnemyManager->GetCursorEnemy(m_pGameCamera, cursorX, cursorY);

		if (pEnemy != nullptr && pEnemy->GetErase() == false)
		{
			// Set the player's enemy pointer
			m_pPlayer->SetTargetEnemy(pEnemy);
			pEnemy->SetOutlineRender(true);

			// Set player alpha
			m_pPlayer->SetPlayerAlpha(0.6f);

			// Set target camera
			SavePreviousCameraMode();
			m_shouldRestorePreviousCameraMode = true;
			SetCameraMode(CameraMode_EnemyTarget);

			// Open cinematic letter box
			OpenLetterBox();
		}
	}
}

void VoxGame::ReleaseEnemyTarget()
{
	if (m_pPlayer->GetTargetEnemy() != nullptr)
	{
		m_pPlayer->GetTargetEnemy()->SetOutlineRender(false);
		m_pPlayer->SetTargetEnemy(nullptr);

		// Reset player alpha
		m_pPlayer->SetPlayerAlpha(1.0f);

		// Return back to the previous camera state
		if (ShouldRestorePreviousCameraMode())
		{
			RestorePreviousCameraMode();
			InitializeCameraRotation();
		}

		// Close cinematic letter box
		CloseLetterBox();
	}
}

// GUI Helper functions
bool VoxGame::IsGUIWindowStillDisplayed()
{
	if (m_pInventoryGUI->IsLoaded())
	{
		return true;
	}

	if (m_pCharacterGUI->IsLoaded())
	{
		return true;
	}

	if (m_pLootGUI->IsLoaded())
	{
		return true;
	}

	if (m_pCraftingGUI->IsLoaded())
	{
		return true;
	}

	if (m_pQuestGUI->IsLoaded())
	{
		return true;
	}

	return false;
}

void VoxGame::CloseAllGUIWindows()
{
	if (m_pInventoryGUI->IsLoaded() && m_pInventoryGUI->IsLoadDelayed() == false)
	{
		m_pInventoryGUI->Unload();
	}

	if (m_pCharacterGUI->IsLoaded() && m_pCharacterGUI->IsLoadDelayed() == false)
	{
		m_pCharacterGUI->Unload();
	}

	if (m_pLootGUI->IsLoaded() && m_pLootGUI->IsLoadDelayed() == false)
	{
		m_pLootGUI->Unload();
	}

	if (m_pCraftingGUI->IsLoaded() && m_pCraftingGUI->IsLoadDelayed() == false)
	{
		m_pCraftingGUI->Unload();
	}

	if (m_pQuestGUI->IsLoaded() && m_pQuestGUI->IsLoadDelayed() == false)
	{
		m_pQuestGUI->Unload();
	}
	
	// Reset focus, also resets any text entry that we might have been doing.
	m_pGUI->ResetFocus();
}

void VoxGame::CloseInteractionGUI()
{
	if (m_pCraftingGUI->IsLoaded())
	{
		m_pCraftingGUI->Unload();

		CloseLetterBox();

		if (IsGUIWindowStillDisplayed() == false)
		{
			TurnCursorOff(false);
		}

		if (ShouldRestorePreviousCameraMode())
		{
			RestorePreviousCameraMode();
			InitializeCameraRotation();
		}
	}
}

// Accessors
unsigned int VoxGame::GetDefaultViewport()
{
	return m_defaultViewport;
}

Camera* VoxGame::GetGameCamera()
{
	return m_pGameCamera;
}

Player* VoxGame::GetPlayer()
{
	return m_pPlayer;
}

ChunkManager* VoxGame::GetChunkManager()
{
	return m_pChunkManager;
}

BiomeManager* VoxGame::GetBiomeManager()
{
	return m_pBiomeManager;
}

FrontendManager* VoxGame::GetFrontendManager()
{
	return m_pFrontendManager;
}

BlockParticleManager* VoxGame::GetBlockParticleManager()
{
	return m_pBlockParticleManager;
}

NPCManager* VoxGame::GetNPCManager()
{
	return m_pNPCManager;
}

ItemManager* VoxGame::GetItemManager()
{
	return m_pItemManager;
}

InventoryManager* VoxGame::GetInventoryManager()
{
	return m_pInventoryManager;
}

RandomLootManager* VoxGame::GetRandomLootManager()
{
	return m_pRandomLootManager;
}

ModsManager* VoxGame::GetModsManager()
{
	return m_pModsManager;
}

CharacterGUI* VoxGame::GetCharacterGUI()
{
	return m_pCharacterGUI;
}

QuestGUI* VoxGame::GetQuestGUI()
{
	return m_pQuestGUI;
}

HUD* VoxGame::GetHUD()
{
	return m_pHUD;
}

ActionBar* VoxGame::GetActionBar()
{
	return m_pActionBar;
}

VoxSettings* VoxGame::GetVoxSettings()
{
	return m_pVoxSettings;
}