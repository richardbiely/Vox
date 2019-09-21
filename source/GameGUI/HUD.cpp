// ******************************************************************************
// Filename:    HUD.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 03/03/15
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "HUD.h"

#include "../frontend/FrontendManager.h"
#include "../Player/Player.h"
#include "../utils/FileUtils.h"
#include "../utils/Random.h"
#include "../utils/Interpolator.h"
#include "../VoxGame.h"


HUD::HUD(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontendManager, ChunkManager* pChunkManager, Player* pPlayer, InventoryManager* pInventoryManager, int windowWidth, int windowHeight)
{
	m_pRenderer = pRenderer;
	m_pGUI = pGUI;
	m_pFrontendManager = pFrontendManager;
	m_pChunkManager = pChunkManager;
	m_pPlayer = pPlayer;
	m_pInventoryManager = pInventoryManager;

	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;

	// Player death
	char deathHeaderText[] = "You Have Died.";
	m_pDeathHeaderLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_80(), deathHeaderText, Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pDeathHeaderLabel->SetOutline(true);
	m_pDeathHeaderLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pDeathHeaderLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_80_Outline());

	m_pRespawnButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_50(), m_pFrontendManager->GetFrontendFont_50_Outline(), "Respawn", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pRespawnButton->SetLabelOffset(0, 3);
	m_pRespawnButton->SetPressedOffset(0, -4);
	m_pRespawnButton->SetCallBackFunction(_RespawnPressed);
	m_pRespawnButton->SetCallBackData(this);

	// Level up
	char levelupHeaderText[] = "Level Up!";
	m_pLevelUpLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_80(), levelupHeaderText, Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pLevelUpLabel->SetOutline(true);
	m_pLevelUpLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pLevelUpLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_80_Outline());

	// Player portrait
	m_pPortraitBackgroundIcon = new Icon(m_pRenderer, "", 72, 72);
	m_pPortraitBackgroundIcon->SetDepth(2.0f);

	// Player health
	char healthText[] = "100/100";
	m_pHealthLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_20(), healthText, Colour(1.0f, 1.0f, 1.0f, 0.75f));
	m_pHealthLabel->SetOutline(true);
	m_pHealthLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 0.95f));
	m_pHealthLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_20_Outline());
	m_pHealthLabel->SetDepth(2.0f);

	m_pHealthContainerIcon = new Icon(m_pRenderer, "", 72, 64);
	m_pHealthContainerIcon->SetDepth(2.0f);

	m_pHealthFillerIcon = new Icon(m_pRenderer, "", 103, 21);
	m_pHealthFillerIcon->SetDepth(1.75f);

	m_pHealthFillerBackgroundIcon = new Icon(m_pRenderer, "", 53, 21);
	m_pHealthFillerBackgroundIcon->SetDepth(1.7f);

	// Player magic
	char magicText[] = "100/100";
	m_pMagicLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_14(), magicText, Colour(1.0f, 1.0f, 1.0f, 0.75f));
	m_pMagicLabel->SetOutline(true);
	m_pMagicLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 0.95f));
	m_pMagicLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_14_Outline());
	m_pMagicLabel->SetDepth(2.0f);

	m_pManaFillerIcon = new Icon(m_pRenderer, "", 84, 11);
	m_pManaFillerIcon->SetDepth(1.75f);

	m_pManaFillerBackgroundIcon = new Icon(m_pRenderer, "", 53, 11);
	m_pManaFillerBackgroundIcon->SetDepth(1.7f);

	// Player experience
	char experienceText[] = "0/100";
	m_pExperienceLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_18(), experienceText, Colour(1.0f, 1.0f, 1.0f, 0.75f));
	m_pExperienceLabel->SetOutline(true);
	m_pExperienceLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 0.95f));
	m_pExperienceLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_18_Outline());
	m_pExperienceLabel->SetDepth(2.0f);

	m_pExperienceContainerIcon = new Icon(m_pRenderer, "", 328, 26);
	m_pExperienceContainerIcon->SetDepth(2.0f);

	m_pExperienceFillerIcon = new Icon(m_pRenderer, "", 320, 14);
	m_pExperienceFillerIcon->SetDepth(1.75f);

	m_pExperienceFillerBackgroundIcon = new Icon(m_pRenderer, "", 53, 14);
	m_pExperienceFillerBackgroundIcon->SetDepth(1.7f);

	// Enemy health
	char enemyHealthText[] = "100/100";
	m_pEnemyHealthLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_20(), enemyHealthText, Colour(1.0f, 1.0f, 1.0f, 0.75f));
	m_pEnemyHealthLabel->SetOutline(true);
	m_pEnemyHealthLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 0.95f));
	m_pEnemyHealthLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_20_Outline());
	m_pEnemyHealthLabel->SetDepth(2.0f);


	m_pEnemyHealthContainerIcon = new Icon(m_pRenderer, "", 300, 33);
	m_pEnemyHealthContainerIcon->SetDepth(2.0f);

	m_pEnemyHealthFillerIcon = new Icon(m_pRenderer, "", 103, 21);
	m_pEnemyHealthFillerIcon->SetDepth(1.75f);

	m_pEnemyHealthFillerBackgroundIcon = new Icon(m_pRenderer, "", 53, 21);
	m_pEnemyHealthFillerBackgroundIcon->SetDepth(1.7f);

	// Enemy name
	char enemyNameText[] = "Enemy";
	m_pEnemyNameLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), enemyNameText, Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pEnemyNameLabel->SetOutline(true);
	m_pEnemyNameLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pEnemyNameLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_30_Outline());

	// Portrait
	m_pDynamicPortraitIcon = new Icon(m_pRenderer);
	m_pDynamicPortraitIcon->SetDepth(2.5f);

	// TODO : Checkboxes for GUI panels on the HUD
	// Checkbox icons
	//m_pInventoryCheckboxIcon = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/inventory_normal.tga", 44, 44);
	//m_pInventoryCheckboxIcon->SetDepth(2.0f);
	//m_pInventoryCheckboxIcon_Hover = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/inventory_hover.tga", 44, 44);
	//m_pInventoryCheckboxIcon_Hover->SetDepth(2.0f);
	//m_pInventoryCheckboxIcon_Pressed = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/inventory_pressed.tga", 44, 44);
	//m_pInventoryCheckboxIcon_Pressed->SetDepth(2.0f);
	//m_pInventoryCheckboxIcon_Toggled = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/inventory_toggled.tga", 44, 44);
	//m_pInventoryCheckboxIcon_Toggled->SetDepth(2.0f);
	//m_pInventoryCheckboxIcon_Toggled_Hover = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/inventory_toggled_hover.tga", 44, 44);
	//m_pInventoryCheckboxIcon_Toggled_Hover->SetDepth(2.0f);
	//m_pInventoryCheckboxIcon_Toggled_Pressed = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/inventory_toggled_pressed.tga", 44, 44);
	//m_pInventoryCheckboxIcon_Toggled_Pressed->SetDepth(2.0f);
	//m_pCharacterCheckboxIcon = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/character_normal.tga", 44, 44);
	//m_pCharacterCheckboxIcon->SetDepth(2.0f);
	//m_pCharacterCheckboxIcon_Hover = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/character_hover.tga", 44, 44);
	//m_pCharacterCheckboxIcon_Hover->SetDepth(2.0f);
	//m_pCharacterCheckboxIcon_Pressed = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/character_pressed.tga", 44, 44);
	//m_pCharacterCheckboxIcon_Pressed->SetDepth(2.0f);
	//m_pCharacterCheckboxIcon_Toggled = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/character_toggled.tga", 44, 44);
	//m_pCharacterCheckboxIcon_Toggled->SetDepth(2.0f);
	//m_pCharacterCheckboxIcon_Toggled_Hover = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/character_toggled_hover.tga", 44, 44);
	//m_pCharacterCheckboxIcon_Toggled_Hover->SetDepth(2.0f);
	//m_pCharacterCheckboxIcon_Toggled_Pressed = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/character_toggled_pressed.tga", 44, 44);
	//m_pCharacterCheckboxIcon_Toggled_Pressed->SetDepth(2.0f);
	//m_pQuestCheckboxIcon = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/quest_normal.tga", 44, 44);
	//m_pQuestCheckboxIcon->SetDepth(2.0f);
	//m_pQuestCheckboxIcon_Hover = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/quest_hover.tga", 44, 44);
	//m_pQuestCheckboxIcon_Hover->SetDepth(2.0f);
	//m_pQuestCheckboxIcon_Pressed = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/quest_pressed.tga", 44, 44);
	//m_pQuestCheckboxIcon_Pressed->SetDepth(2.0f);
	//m_pQuestCheckboxIcon_Toggled = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/quest_toggled.tga", 44, 44);
	//m_pQuestCheckboxIcon_Toggled->SetDepth(2.0f);
	//m_pQuestCheckboxIcon_Toggled_Hover = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/quest_toggled_hover.tga", 44, 44);
	//m_pQuestCheckboxIcon_Toggled_Hover->SetDepth(2.0f);
	//m_pQuestCheckboxIcon_Toggled_Pressed = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/quest_toggled_pressed.tga", 44, 44);
	//m_pQuestCheckboxIcon_Toggled_Pressed->SetDepth(2.0f);
	//m_pCraftingCheckboxIcon = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/crafting_normal.tga", 44, 44);
	//m_pCraftingCheckboxIcon->SetDepth(2.0f);
	//m_pCraftingCheckboxIcon_Hover = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/crafting_hover.tga", 44, 44);
	//m_pCraftingCheckboxIcon_Hover->SetDepth(2.0f);
	//m_pCraftingCheckboxIcon_Pressed = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/crafting_pressed.tga", 44, 44);
	//m_pCraftingCheckboxIcon_Pressed->SetDepth(2.0f);
	//m_pCraftingCheckboxIcon_Toggled = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/crafting_toggled.tga", 44, 44);
	//m_pCraftingCheckboxIcon_Toggled->SetDepth(2.0f);
	//m_pCraftingCheckboxIcon_Toggled_Hover = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/crafting_toggled_hover.tga", 44, 44);
	//m_pCraftingCheckboxIcon_Toggled_Hover->SetDepth(2.0f);
	//m_pCraftingCheckboxIcon_Toggled_Pressed = new Icon(m_pRenderer, "media/textures/gui/Stonewash/HUD/Buttons/crafting_toggled_pressed.tga", 44, 44);
	//m_pCraftingCheckboxIcon_Toggled_Pressed->SetDepth(2.0f);

	//// GUI panel checkboxes
	//m_pInventoryCheckBox = new CheckBox(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), "");
	//m_pInventoryCheckBox->SetCallBackFunction(_InventoryCheckboxPressed);
	//m_pInventoryCheckBox->SetCallBackData(this);
	//m_pInventoryCheckBox->SetDefaultIcon(m_pInventoryCheckboxIcon);
	//m_pInventoryCheckBox->SetHoverIcon(m_pInventoryCheckboxIcon_Hover);
	//m_pInventoryCheckBox->SetSelectedIcon(m_pInventoryCheckboxIcon_Pressed);
	//m_pInventoryCheckBox->SetToggledIcon(m_pInventoryCheckboxIcon_Toggled);
	//m_pInventoryCheckBox->SetToggledHoverIcon(m_pInventoryCheckboxIcon_Toggled_Hover);
	//m_pInventoryCheckBox->SetToggledSelectedIcon(m_pInventoryCheckboxIcon_Toggled_Pressed);

	//m_pCharacterCheckBox = new CheckBox(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), "");
	//m_pCharacterCheckBox->SetCallBackFunction(_CharacterCheckboxPressed);
	//m_pCharacterCheckBox->SetCallBackData(this);
	//m_pCharacterCheckBox->SetDefaultIcon(m_pCharacterCheckboxIcon);
	//m_pCharacterCheckBox->SetHoverIcon(m_pCharacterCheckboxIcon_Hover);
	//m_pCharacterCheckBox->SetSelectedIcon(m_pCharacterCheckboxIcon_Pressed);
	//m_pCharacterCheckBox->SetToggledIcon(m_pCharacterCheckboxIcon_Toggled);
	//m_pCharacterCheckBox->SetToggledHoverIcon(m_pCharacterCheckboxIcon_Toggled_Hover);
	//m_pCharacterCheckBox->SetToggledSelectedIcon(m_pCharacterCheckboxIcon_Toggled_Pressed);

	//m_pQuestCheckBox = new CheckBox(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), "");
	//m_pQuestCheckBox->SetCallBackFunction(_QuestCheckboxPressed);
	//m_pQuestCheckBox->SetCallBackData(this);
	//m_pQuestCheckBox->SetDefaultIcon(m_pQuestCheckboxIcon);
	//m_pQuestCheckBox->SetHoverIcon(m_pQuestCheckboxIcon_Hover);
	//m_pQuestCheckBox->SetSelectedIcon(m_pQuestCheckboxIcon_Pressed);
	//m_pQuestCheckBox->SetToggledIcon(m_pQuestCheckboxIcon_Toggled);
	//m_pQuestCheckBox->SetToggledHoverIcon(m_pQuestCheckboxIcon_Toggled_Hover);
	//m_pQuestCheckBox->SetToggledSelectedIcon(m_pQuestCheckboxIcon_Toggled_Pressed);

	//m_pCraftingCheckBox = new CheckBox(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), "");
	//m_pCraftingCheckBox->SetCallBackFunction(_CraftingCheckboxPressed);
	//m_pCraftingCheckBox->SetCallBackData(this);
	//m_pCraftingCheckBox->SetDefaultIcon(m_pCraftingCheckboxIcon);
	//m_pCraftingCheckBox->SetHoverIcon(m_pCraftingCheckboxIcon_Hover);
	//m_pCraftingCheckBox->SetSelectedIcon(m_pCraftingCheckboxIcon_Pressed);
	//m_pCraftingCheckBox->SetToggledIcon(m_pCraftingCheckboxIcon_Toggled);
	//m_pCraftingCheckBox->SetToggledHoverIcon(m_pCraftingCheckboxIcon_Toggled_Hover);
	//m_pCraftingCheckBox->SetToggledSelectedIcon(m_pCraftingCheckboxIcon_Toggled_Pressed);

	//m_pSkillsCheckBox = new CheckBox(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), "");
	//m_pSkillsCheckBox->SetCallBackFunction(_SkillsCheckboxPressed);
	//m_pSkillsCheckBox->SetCallBackData(this);

	//m_pPetCheckBox = new CheckBox(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), "");
	//m_pPetCheckBox->SetCallBackFunction(_PetCheckboxPressed);
	//m_pPetCheckBox->SetCallBackData(this);

	SetWindowDimensions(m_windowWidth, m_windowHeight);

	m_enemyInfoDisappearTime = 3.0f;
	m_enemyInfoDisappearTimer = 0.0f;
	m_removedEnemyInfo = true;
	m_updatePlayerHealthTextLocation = false;
	m_updatePlayerMagicTextLocation = false;
	m_updatePlayerExperienceTextLocation = false;
	m_updateEnemyTextLocation = false;

	m_playerDied = false;
	m_levelUp = false;

	m_loaded = false;
}

HUD::~HUD()
{
	// Player death
	delete m_pDeathHeaderLabel;
	delete m_pRespawnButton;

	// Level up
	delete m_pLevelUpLabel;

	// Player portrait
	delete m_pPortraitBackgroundIcon;

	// Player health
	delete m_pHealthLabel;
	delete m_pHealthContainerIcon;
	delete m_pHealthFillerIcon;
	delete m_pHealthFillerBackgroundIcon;

	// Player magic
	delete m_pMagicLabel;

	// Player experience
	delete m_pExperienceLabel;
	delete m_pExperienceContainerIcon;
	delete m_pExperienceFillerIcon;
	delete m_pExperienceFillerBackgroundIcon;
	delete m_pManaFillerIcon;
	delete m_pManaFillerBackgroundIcon;

	// Enemy health
	delete m_pEnemyHealthLabel;
	delete m_pEnemyNameLabel;
	delete m_pEnemyHealthContainerIcon;
	delete m_pEnemyHealthFillerIcon;
	delete m_pEnemyHealthFillerBackgroundIcon;

	// Portrait
	delete m_pDynamicPortraitIcon;

	// GUI panel checkboxes
	// TODO : Checkboxes for GUI panels on the HUD
	//delete m_pInventoryCheckBox;
	//delete m_pCharacterCheckBox;
	//delete m_pQuestCheckBox;
	//delete m_pCraftingCheckBox;
	//delete m_pSkillsCheckBox;
	//delete m_pPetCheckBox;

	//// Checkbox icons
	//delete m_pInventoryCheckboxIcon;
	//delete m_pInventoryCheckboxIcon_Hover;
	//delete m_pInventoryCheckboxIcon_Pressed;
	//delete m_pInventoryCheckboxIcon_Toggled;
	//delete m_pInventoryCheckboxIcon_Toggled_Hover;
	//delete m_pInventoryCheckboxIcon_Toggled_Pressed;
	//delete m_pCharacterCheckboxIcon;
	//delete m_pCharacterCheckboxIcon_Hover;
	//delete m_pCharacterCheckboxIcon_Pressed;
	//delete m_pCharacterCheckboxIcon_Toggled;
	//delete m_pCharacterCheckboxIcon_Toggled_Hover;
	//delete m_pCharacterCheckboxIcon_Toggled_Pressed;
	//delete m_pQuestCheckboxIcon;
	//delete m_pQuestCheckboxIcon_Hover;
	//delete m_pQuestCheckboxIcon_Pressed;
	//delete m_pQuestCheckboxIcon_Toggled;
	//delete m_pQuestCheckboxIcon_Toggled_Hover;
	//delete m_pQuestCheckboxIcon_Toggled_Pressed;
	//delete m_pCraftingCheckboxIcon;
	//delete m_pCraftingCheckboxIcon_Hover;
	//delete m_pCraftingCheckboxIcon_Pressed;
	//delete m_pCraftingCheckboxIcon_Toggled;
	//delete m_pCraftingCheckboxIcon_Toggled_Hover;
	//delete m_pCraftingCheckboxIcon_Toggled_Pressed;
}

void HUD::SetInventoryGUI(InventoryGUI* pInventoryGUI)
{
	m_pInventoryGUI = pInventoryGUI;
}

void HUD::SetCharacterGUI(CharacterGUI* pCharacterGUI)
{
	m_pCharacterGUI = pCharacterGUI;
}

void HUD::SetQuestGUI(QuestGUI* pQuestGUI)
{
	m_pQuestGUI = pQuestGUI;
}

void HUD::SetCraftingGUI(CraftingGUI* pCraftingGUI)
{
	m_pCraftingGUI = pCraftingGUI;
}

void HUD::SetWindowDimensions(int windowWidth, int windowHeight)
{
	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;

	// Player death
	int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_80(), "%s", m_pDeathHeaderLabel->GetText().c_str());
	m_pDeathHeaderLabel->SetLocation((int)((m_windowWidth*0.5f) - (int)(textWidth*0.5f)), (int)(m_windowHeight*0.5f + 150));
	m_respawnButtonWidth = 200;
	m_respawnButtonHeight = 75;
	m_pRespawnButton->SetDimensions((int)((m_windowWidth*0.5f)-(int)(m_respawnButtonWidth*0.5f)), (int)((m_windowHeight*0.5f)-(m_respawnButtonHeight*0.5f) + 75), (int)m_respawnButtonWidth, (int)m_respawnButtonHeight);

	// Level up
	textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_80(), "%s", m_pLevelUpLabel->GetText().c_str());
	m_pLevelUpLabel->SetLocation((int)((m_windowWidth*0.5f) - (int)(textWidth*0.5f)), (int)(m_windowHeight*0.5f - 25));

	// Player portrait
	m_pPortraitBackgroundIcon->SetDimensions(20, m_windowHeight-88, 72, 72);

	// Player health
	m_playerHealthX = 92.0f;
	m_playerHealthY = m_windowHeight - 49.0f;
	m_playerHealthWidth = 160.0f;
	m_playerHealthHeight = 17.0f;
	m_playerHealthTitleOffset = 18.0f;
	m_pHealthContainerIcon->SetDimensions(92, m_windowHeight-80, 164, 56);
	m_pHealthFillerIcon->SetDimensions(24, m_windowHeight-84, 103, 21);
	m_pHealthFillerBackgroundIcon->SetDimensions(24, m_windowHeight-84, 53, 21);

	// Player magic
	m_playerMagicX = 92.0f;
	m_playerMagicY = m_windowHeight - 72.0f;
	m_playerMagicWidth = 160.0f;
	m_playerMagicHeight = 11.0f;
	m_playerMagicTitleOffset = 18.0f;
	m_pManaFillerIcon->SetDimensions(24, m_windowHeight-84, 84, 11);
	m_pManaFillerBackgroundIcon->SetDimensions(24, m_windowHeight-84, 53, 11);

	// Player experience
	m_playerExperienceX = m_windowWidth/2 - 328/2 + 4.0f;
	m_playerExperienceY = 92;
	m_playerExperienceWidth = 321.0f;
	m_playerExperienceHeight = 14.0f;
	m_playerExperienceTitleOffset = 18.0f;
	m_pExperienceContainerIcon->SetDimensions(m_windowWidth/2-328/2, 88, 328, 26);
	m_pExperienceFillerIcon->SetDimensions(24, m_windowHeight-84, 84, 11);
	m_pExperienceFillerBackgroundIcon->SetDimensions(24, m_windowHeight-84, 53, 11);

	// Enemy health
	m_enemyHealthWidth = 300.0f;
	m_enemyHealthHeight = 33.0f;
	m_enemyHealthX = (m_windowWidth*0.5f) - (m_enemyHealthWidth*0.5f);
	m_enemyHealthY = (float)(m_windowHeight - 75);
	textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_20(), "%s", m_pEnemyHealthLabel->GetText().c_str());
	m_pEnemyHealthLabel->SetLocation((int)((m_windowWidth*0.5f) - (int)(textWidth*0.5f)), (int)m_windowHeight+7);
	m_pEnemyNameLabel->SetLocation((int)((m_windowWidth*0.5f) - (int)(textWidth*0.5f)), (int)m_windowHeight+35);
	m_pEnemyHealthContainerIcon->SetDimensions((int)m_enemyHealthX, (int)m_enemyHealthY, (int)m_enemyHealthWidth, (int)m_enemyHealthHeight);
	m_pEnemyHealthFillerIcon->SetDimensions((int)m_enemyHealthX, (int)m_enemyHealthY, (int)m_enemyHealthWidth, (int)m_enemyHealthHeight);
	m_pEnemyHealthFillerBackgroundIcon->SetDimensions((int)m_enemyHealthX, (int)m_enemyHealthY, (int)m_enemyHealthWidth, (int)m_enemyHealthHeight);

	// Portrait icon
	m_pDynamicPortraitIcon->SetDimensions(24, m_windowHeight-84, 64, 64);

	// GUI panel checkboxes
	int checkboxSize = 32;
	int windowBorder = 10;
	int spacer= 10;
	// TODO : Checkboxes for GUI panels on the HUD
	//m_pInventoryCheckBox->SetDimensions(m_windowWidth-windowBorder-checkboxSize*1-spacer*0, 10, checkboxSize, checkboxSize);
	//m_pCharacterCheckBox->SetDimensions(m_windowWidth-windowBorder-checkboxSize*2-spacer*1, 10, checkboxSize, checkboxSize);
	//m_pQuestCheckBox->SetDimensions(m_windowWidth-windowBorder-checkboxSize*3-spacer*2, 10, checkboxSize, checkboxSize);
	//m_pCraftingCheckBox->SetDimensions(m_windowWidth-windowBorder-checkboxSize*4-spacer*3, 10, checkboxSize, checkboxSize);
	//m_pSkillsCheckBox->SetDimensions(m_windowWidth-windowBorder-checkboxSize*5-spacer*4, 10, checkboxSize, checkboxSize);
	//m_pPetCheckBox->SetDimensions(m_windowWidth-windowBorder-checkboxSize*6-spacer*5, 10, checkboxSize, checkboxSize);

	// Make sure to update the text locations for various HUD text
	m_updatePlayerHealthTextLocation = true;
	m_updatePlayerMagicTextLocation = true;
	m_updatePlayerExperienceTextLocation = true;
	m_updateEnemyTextLocation = true;
}

// Skinning the GUI
void HUD::SkinGUI()
{
	string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();

	string iconName = "media/textures/gui/" + themeName + "/HUD/portrait_background.tga";
	m_pPortraitBackgroundIcon->SetIcon(iconName);

	iconName = "media/textures/gui/" + themeName + "/HUD/health_container.tga";
	m_pHealthContainerIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/HUD/health_filler.tga";
	m_pHealthFillerIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/HUD/health_filler_background.tga";
	m_pHealthFillerBackgroundIcon->SetIcon(iconName);

	iconName = "media/textures/gui/" + themeName + "/HUD/mana_filler.tga";
	m_pManaFillerIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/HUD/mana_filler_background.tga";
	m_pManaFillerBackgroundIcon->SetIcon(iconName);

	iconName = "media/textures/gui/" + themeName + "/HUD/experience_container.tga";
	m_pExperienceContainerIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/HUD/experience_filler.tga";
	m_pExperienceFillerIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/HUD/experience_filler_background.tga";
	m_pExperienceFillerBackgroundIcon->SetIcon(iconName);

	iconName = "media/textures/gui/" + themeName + "/HUD/enemy_health_container.tga";
	m_pEnemyHealthContainerIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/HUD/enemy_health_filler.tga";
	m_pEnemyHealthFillerIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/HUD/enemy_health_filler_background.tga";
	m_pEnemyHealthFillerBackgroundIcon->SetIcon(iconName);

	m_pFrontendManager->SetButtonIcons(m_pRespawnButton, ButtonSize_225x75);
	m_pRespawnButton->SetDimensions((int)((m_windowWidth*0.5f) - (int)(m_respawnButtonWidth*0.5f)), (int)((m_windowHeight*0.5f) - (m_respawnButtonHeight*0.5f) + 75), (int)m_respawnButtonWidth, (int)m_respawnButtonHeight);

	m_pRespawnButton->SetNormalLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pRespawnButton->SetHoverLabelColour(m_pFrontendManager->GetHoverFontColour());
	m_pRespawnButton->SetPressedLabelColour(m_pFrontendManager->GetPressedFontColour());
}

void HUD::UnSkinGUI()
{

}

// Loading
void HUD::Load()
{
	m_loaded = true;

	if(m_playerDied)
	{
		// Player death
		m_pGUI->AddComponent(m_pDeathHeaderLabel);
		m_pGUI->AddComponent(m_pRespawnButton);
	}

	// Player portrait
	m_pGUI->AddComponent(m_pPortraitBackgroundIcon);

	// Player health
	m_pGUI->AddComponent(m_pHealthLabel);
	m_pGUI->AddComponent(m_pHealthContainerIcon);
	m_pGUI->AddComponent(m_pHealthFillerIcon);
	m_pGUI->AddComponent(m_pHealthFillerBackgroundIcon);

	// Player magic
	m_pGUI->AddComponent(m_pMagicLabel);
	m_pGUI->AddComponent(m_pManaFillerIcon);
	m_pGUI->AddComponent(m_pManaFillerBackgroundIcon);

	// Player experience
	m_pGUI->AddComponent(m_pExperienceLabel);
	m_pGUI->AddComponent(m_pExperienceContainerIcon);
	m_pGUI->AddComponent(m_pExperienceFillerIcon);
	m_pGUI->AddComponent(m_pExperienceFillerBackgroundIcon);

	// Portrait icon
	m_pGUI->AddComponent(m_pDynamicPortraitIcon);
	
	// GUI panel checkboxes
	// TODO : Checkboxes for GUI panels on the HUD
	//m_pGUI->AddComponent(m_pInventoryCheckBox);
	//m_pGUI->AddComponent(m_pCharacterCheckBox);
	//m_pGUI->AddComponent(m_pQuestCheckBox);
	//m_pGUI->AddComponent(m_pCraftingCheckBox);
	//m_pGUI->AddComponent(m_pSkillsCheckBox);
	//m_pGUI->AddComponent(m_pPetCheckBox);

	UpdatePlayerData();
}

void HUD::Unload()
{
	// Player death
	m_pGUI->RemoveComponent(m_pDeathHeaderLabel);
	m_pGUI->RemoveComponent(m_pRespawnButton);

	// Level up
	m_pGUI->RemoveComponent(m_pLevelUpLabel);

	// Player portrait
	m_pGUI->RemoveComponent(m_pPortraitBackgroundIcon);

	// Player health
	m_pGUI->RemoveComponent(m_pHealthLabel);
	m_pGUI->RemoveComponent(m_pHealthContainerIcon);
	m_pGUI->RemoveComponent(m_pHealthFillerIcon);
	m_pGUI->RemoveComponent(m_pHealthFillerBackgroundIcon);

	// Player magic
	m_pGUI->RemoveComponent(m_pMagicLabel);
	m_pGUI->RemoveComponent(m_pManaFillerIcon);
	m_pGUI->RemoveComponent(m_pManaFillerBackgroundIcon);

	// Player experience
	m_pGUI->RemoveComponent(m_pExperienceLabel);
	m_pGUI->RemoveComponent(m_pExperienceContainerIcon);
	m_pGUI->RemoveComponent(m_pExperienceFillerIcon);
	m_pGUI->RemoveComponent(m_pExperienceFillerBackgroundIcon);

	// Enemy health
	m_pGUI->RemoveComponent(m_pEnemyHealthLabel);
	m_pGUI->RemoveComponent(m_pEnemyNameLabel);
	m_pGUI->RemoveComponent(m_pEnemyHealthContainerIcon);
	m_pGUI->RemoveComponent(m_pEnemyHealthFillerIcon);
	m_pGUI->RemoveComponent(m_pEnemyHealthFillerBackgroundIcon);

	// Portrait icon
	m_pGUI->RemoveComponent(m_pDynamicPortraitIcon);

	// GUI panel checkboxes
	// TODO : Checkboxes for GUI panels on the HUD
	//m_pGUI->RemoveComponent(m_pInventoryCheckBox);
	//m_pGUI->RemoveComponent(m_pCharacterCheckBox);
	//m_pGUI->RemoveComponent(m_pQuestCheckBox);
	//m_pGUI->RemoveComponent(m_pCraftingCheckBox);
	//m_pGUI->RemoveComponent(m_pSkillsCheckBox);
	//m_pGUI->RemoveComponent(m_pPetCheckBox);

	m_loaded = false;
}

bool HUD::IsLoaded()
{
	return m_loaded;
}

// Player death
void HUD::PlayerDied()
{
	m_playerDied = true;

	m_pGUI->AddComponent(m_pDeathHeaderLabel);

	float deathHeaderDelay = 3.5f;
	float deathHeaderTimeIn = 0.5f;
	float deathHeaderWait = 0.5f;
	FloatInterpolation* lDeathAlpha1 = Interpolator::GetInstance()->CreateFloatInterpolation(&m_deathHeaderAlpha, 0.0f, 0.0f, deathHeaderDelay, 100.0f);
	FloatInterpolation* lDeathAlpha2 = Interpolator::GetInstance()->CreateFloatInterpolation(&m_deathHeaderAlpha, 0.0f, 1.0f, deathHeaderTimeIn, 100.0f);
	FloatInterpolation* lDeathAlpha3 = Interpolator::GetInstance()->CreateFloatInterpolation(&m_deathHeaderAlpha, 1.0f, 1.0f, deathHeaderWait, 100.0f, nullptr, _DeathTextFinished, this);
	Interpolator::GetInstance()->LinkFloatInterpolation(lDeathAlpha1, lDeathAlpha2);
	Interpolator::GetInstance()->LinkFloatInterpolation(lDeathAlpha2, lDeathAlpha3);
	Interpolator::GetInstance()->AddFloatInterpolation(lDeathAlpha1);

	m_pDeathHeaderLabel->SetColour(Colour(1.0f, 1.0f, 1.0f, 0.0f));
	m_pDeathHeaderLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 0.0f));
}

void HUD::PlayerRespawn()
{
	m_playerDied = false;

	m_pGUI->RemoveComponent(m_pDeathHeaderLabel);
	m_pGUI->RemoveComponent(m_pRespawnButton);
}

// Level up
void HUD::LevelUp()
{
	m_levelUp = true;

	m_pGUI->AddComponent(m_pLevelUpLabel);

	float levelUpDelay = 0.0f;
	float levelUpTimeIn = 0.5f;
	float levelUpWait = 1.5f;
	float levelUpTimeOut = 0.25f;
	FloatInterpolation* llevelUpAlpha1 = Interpolator::GetInstance()->CreateFloatInterpolation(&m_levelUpAlpha, 0.0f, 0.0f, levelUpDelay, 100.0f);
	FloatInterpolation* llevelUpAlpha2 = Interpolator::GetInstance()->CreateFloatInterpolation(&m_levelUpAlpha, 0.0f, 1.0f, levelUpTimeIn, 100.0f);
	FloatInterpolation* llevelUpAlpha3 = Interpolator::GetInstance()->CreateFloatInterpolation(&m_levelUpAlpha, 1.0f, 1.0f, levelUpWait, 100.0f);
	FloatInterpolation* llevelUpAlpha4 = Interpolator::GetInstance()->CreateFloatInterpolation(&m_levelUpAlpha, 1.0f, 0.0f, levelUpTimeOut, 100.0f, nullptr, _LevelUpTextFinished, this);
	Interpolator::GetInstance()->LinkFloatInterpolation(llevelUpAlpha1, llevelUpAlpha2);
	Interpolator::GetInstance()->LinkFloatInterpolation(llevelUpAlpha2, llevelUpAlpha3);
	Interpolator::GetInstance()->LinkFloatInterpolation(llevelUpAlpha3, llevelUpAlpha4);
	Interpolator::GetInstance()->AddFloatInterpolation(llevelUpAlpha1);

	// DO a movement up animation
	int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_80(), "%s", m_pLevelUpLabel->GetText().c_str());
	m_pLevelUpLabel->SetLocation((int)((m_windowWidth*0.5f) - (int)(textWidth*0.5f)), (int)(m_windowHeight*0.5f - 50));

	float totalTime = levelUpDelay + levelUpTimeIn + levelUpWait + levelUpTimeOut;
	Interpolator::GetInstance()->AddIntInterpolation(&m_pLevelUpLabel->GetDimensionsPointer()->m_y, m_pLevelUpLabel->GetDimensions().m_y, m_pLevelUpLabel->GetDimensions().m_y+150, totalTime, -100.0f);

	m_pLevelUpLabel->SetColour(Colour(1.0f, 1.0f, 0.0f, 0.0f));
	m_pLevelUpLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 0.0f));
}

// Enemy health
void HUD::SetEnemyHealth(float health, float healthMax, float damagedone)
{
	m_enemyHealth = health;
	m_enemyHealthMax = healthMax;

	// Update the HP label
	char healthText[32];
	sprintf(healthText, "%i/%i", (int)m_enemyHealth, (int)m_enemyHealthMax);

	m_updateEnemyTextLocation = true;

	m_pEnemyHealthLabel->SetText(healthText);

	// Make the enemy health bar appear, or refresh its timer
	m_enemyInfoDisappearTimer = m_enemyInfoDisappearTime;
	if(m_removedEnemyInfo == true && m_loaded)
	{
		m_pGUI->AddComponent(m_pEnemyHealthLabel);
		m_pGUI->AddComponent(m_pEnemyNameLabel);
		m_pGUI->AddComponent(m_pEnemyHealthContainerIcon);
		m_pGUI->AddComponent(m_pEnemyHealthFillerIcon);
		m_pGUI->AddComponent(m_pEnemyHealthFillerBackgroundIcon);
	}
	m_removedEnemyInfo = false;

	// Set the enemy health width decreaser
	m_enemyHealthWidthDecreaser = damagedone;


	// Create an enemy health dropoff
	/*
	EnemyHealthDropOff* lpNewDropOff = new EnemyHealthDropOff();

	Vector3d lTranslation = m_pEnemyHealthContainerIcon->GetAnimationUpdater()->GetAnimatedTranslation();
	float m_enemyHealthBarRatio = m_enemyHealth / m_enemyHealthMax;
	float m_enemyWidthModified = (m_enemyHealthWidth-16.0f) * m_enemyHealthBarRatio;
	float m_enemyHealthXModified = m_enemyHealthX + 8.0f + lTranslation.x;
	float m_enemyHealthYModified = m_enemyHealthY + 8.0f + lTranslation.y;

	lpNewDropOff->m_x = (int)(m_enemyHealthXModified + m_enemyWidthModified);
	lpNewDropOff->m_y = (int)(m_enemyHealthYModified);
	lpNewDropOff->m_width = (int)(damagedone * (m_enemyHealthWidth / m_enemyHealthMax));
	lpNewDropOff->m_height = (int)(m_enemyHealthHeight - 12);

	lpNewDropOff->m_maxLife = 0.75f + GetRandomNumber(-1, 1, 1)*0.1f;
	lpNewDropOff->m_life = lpNewDropOff->m_maxLife;

	float randomNum = GetRandomNumber(-5, 5)*10.0f;
	lpNewDropOff->m_velocity = Vector3d(randomNum, -50.0f + GetRandomNumber(-20, 30, 1), 0.0f);
	lpNewDropOff->m_rotationVelocity = Vector3d(0.0f, 0.0f, -randomNum);

	lpNewDropOff->m_erase = false;

	m_vpEnemyHealthDropOffList.push_back(lpNewDropOff);
	*/
}

void HUD::SetEnemyName(const char* name)
{
	m_pEnemyNameLabel->SetText(name);

	char nameText[MAX_ENEMY_NAME_SIZE];
	sprintf(nameText, "%s", name);
}

// Updating
void HUD::Update(float dt)
{
	if(VoxGame::GetInstance()->IsPaused() == false)
	{
		// Update the player death components
		if(m_playerDied)
		{
			m_pDeathHeaderLabel->SetColour(Colour(1.0f, 1.0f, 1.0f, m_deathHeaderAlpha));
			m_pDeathHeaderLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, m_deathHeaderAlpha));
		}

		// Update the level up text
		if(m_levelUp)
		{
			m_pLevelUpLabel->SetColour(Colour(1.0f, 1.0f, 0.0f, m_levelUpAlpha));
			m_pLevelUpLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, m_levelUpAlpha));
		}

		// Update the enemy health disappear timer
		if(m_enemyInfoDisappearTimer > 0.0f)
		{
			m_enemyInfoDisappearTimer -= dt;
		}
		else
		{
			if(m_removedEnemyInfo == false)
			{
				m_pGUI->RemoveComponent(m_pEnemyHealthLabel);
				m_pGUI->RemoveComponent(m_pEnemyNameLabel);
				m_pGUI->RemoveComponent(m_pEnemyHealthContainerIcon);
				m_pGUI->RemoveComponent(m_pEnemyHealthFillerIcon);
				m_pGUI->RemoveComponent(m_pEnemyHealthFillerBackgroundIcon);

				m_removedEnemyInfo = true;
			}
		}
	}

	// Portrait dynamic icon
	// Set the dynamic icon
	m_pDynamicPortraitIcon->SetDynamicTexture(VoxGame::GetInstance()->GetDynamicPortraitTexture());

	// Update the checkboxes for the GUI panels if open or not
	// TODO : Checkboxes for GUI panels on the HUD
	//m_pInventoryCheckBox->SetToggled(m_pInventoryGUI->IsLoaded());
	//m_pCharacterCheckBox->SetToggled(m_pCharacterGUI->IsLoaded());
	//m_pQuestCheckBox->SetToggled(m_pQuestGUI->IsLoaded());
	//m_pCraftingCheckBox->SetToggled(m_pCraftingGUI->IsLoaded());
	//m_pSkillsCheckBox->SetToggled(m_pSkillsGUI->IsLoaded());
	//m_pPetCheckBox->SetToggled(m_pCompanionPet->IsSummoned());

	// TODO : Checkboxes for GUI panels on the HUD
	//if(m_pGameWindow->IsPaused())
	//{
	//	m_pInventoryCheckBox->SetEnabled(false);
	//	m_pCharacterCheckBox->SetEnabled(false);
	//	m_pQuestCheckBox->SetEnabled(false);
	//	m_pCraftingCheckBox->SetEnabled(false);
	//	m_pSkillsCheckBox->SetEnabled(false);
	//	m_pPetCheckBox->SetEnabled(false);
	//}
	//else
	//{
	//	m_pInventoryCheckBox->SetEnabled(true);
	//	m_pCharacterCheckBox->SetEnabled(true);
	//	m_pQuestCheckBox->SetEnabled(true);
	//	m_pCraftingCheckBox->SetEnabled(true);
	//	m_pSkillsCheckBox->SetEnabled(true);
	//	m_pPetCheckBox->SetEnabled(true);
	//}
}

void HUD::UpdatePlayerData()
{
	// Update the player health
	int health = (int)m_pPlayer->GetHealth();
	int maxHealth = (int)m_pPlayer->GetMaxHealth();
	char healthText[32];
	sprintf(healthText, "%i/%i", health, maxHealth);
	m_pHealthLabel->SetText(healthText);

	// Update the player magic
	int magic = (int)m_pPlayer->GetMagic();
	int maxMagic = (int)m_pPlayer->GetMaxMagic();
	char magicText[32];
	sprintf(magicText, "%i/%i", magic, maxMagic);
	m_pMagicLabel->SetText(magicText);

	// Update the player experience
	int experience = (int)m_pPlayer->GetPlayerStats()->GetExperience();
	int maxExperience = (int)m_pPlayer->GetPlayerStats()->GetMaxExperience();
	char experienceText[32];
	sprintf(experienceText, "%i/%i", experience, maxExperience);
	m_pExperienceLabel->SetText(experienceText);

	m_updatePlayerHealthTextLocation = true;
	m_updatePlayerMagicTextLocation = true;
	m_updatePlayerExperienceTextLocation = true;
}

// Rendering
void HUD::Render()
{
	if(m_loaded == false)
	{
		return;
	}

	RenderPlayerHealth();
	RenderPlayerMagic();
	RenderPlayerExperience();
	RenderEnemyHealth();
}

void HUD::RenderPlayerHealth()
{
	if(m_updatePlayerHealthTextLocation)
	{
		int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_20(), "%s", m_pHealthLabel->GetText().c_str());
		m_pHealthLabel->SetLocation((int)(m_playerHealthX + (m_playerHealthWidth*0.5f) - (int)(textWidth*0.5f)), (int)m_playerHealthY);

		m_updatePlayerHealthTextLocation = false;
	}

	// Player health bars
    float m_playerHealthBarRatio = m_pPlayer->GetHealth() / m_pPlayer->GetMaxHealth();
	if(m_playerHealthBarRatio >= 1.0f)
	{
		m_playerHealthBarRatio = 1.0f;
	}

    float m_playerWidthModified = (m_playerHealthWidth-0.0f) * m_playerHealthBarRatio;
	float m_playerHeightModified = m_playerHealthHeight;

	float m_playerHealthXModified = m_playerHealthX + 0.0f;
	float m_playerHealthYModified = m_playerHealthY + 0.0f;

	m_pHealthFillerIcon->SetDimensions((int)m_playerHealthXModified, (int)m_playerHealthYModified, (int)m_playerWidthModified, (int)m_playerHeightModified);
	m_pHealthFillerBackgroundIcon->SetDimensions((int)(m_playerHealthXModified + m_playerWidthModified), (int)m_playerHealthYModified, (int)(m_playerHealthWidth-m_playerWidthModified+((m_pPlayer->GetHealth() <= 0.0f || m_pPlayer->GetHealth() >= m_pPlayer->GetMaxHealth()) ? 0 : 1)), (int)m_playerHeightModified);

	// Render the health bar
	m_pRenderer->PushMatrix();
		m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
		glLoadIdentity();
		m_pRenderer->SetProjectionMode(PM_2D, VoxGame::GetInstance()->GetDefaultViewport());

		m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		m_pRenderer->SetRenderMode(RM_SOLID);

		m_pRenderer->EnableImmediateMode(IM_QUADS);
			/*
			m_pRenderer->ImmediateColourAlpha(0.94f, 0.15f, 0.04f, 1.0f);
			m_pRenderer->ImmediateVertex(m_playerHealthXModified, m_playerHealthYModified, 2.0f);
			m_pRenderer->ImmediateVertex(m_playerHealthXModified + m_playerWidthModified, m_playerHealthYModified, 2.0f);
			m_pRenderer->ImmediateColourAlpha(0.94f, 0.45f, 0.29f, 1.0f);
			m_pRenderer->ImmediateVertex(m_playerHealthXModified + m_playerWidthModified, m_playerHealthYModified + m_playerHeightModified, 2.0f);
			m_pRenderer->ImmediateVertex(m_playerHealthXModified, m_playerHealthYModified + m_playerHeightModified, 2.0f);

			// Background
			m_pRenderer->ImmediateColourAlpha(0.59f, 0.24f, 0.02f, 1.0f);
			m_pRenderer->ImmediateVertex(m_playerHealthXModified, m_playerHealthYModified, 2.0f);
			m_pRenderer->ImmediateVertex(m_playerHealthXModified + m_playerHealthWidth, m_playerHealthYModified, 2.0f);
			m_pRenderer->ImmediateColourAlpha(1.0f, 0.81f, 0.36f, 1.0f);
			m_pRenderer->ImmediateVertex(m_playerHealthXModified + m_playerHealthWidth, m_playerHealthYModified + m_playerHeightModified, 2.0f);
			m_pRenderer->ImmediateVertex(m_playerHealthXModified, m_playerHealthYModified + m_playerHeightModified, 2.0f);
			*/
		m_pRenderer->DisableImmediateMode();
		m_pRenderer->DisableTransparency();
	m_pRenderer->PopMatrix();
}

void HUD::RenderPlayerMagic()
{
	if(m_updatePlayerMagicTextLocation)
	{
		int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_14(), "%s", m_pMagicLabel->GetText().c_str());
		m_pMagicLabel->SetLocation((int)(m_playerMagicX + (m_playerMagicWidth*0.5f) - (int)(textWidth*0.5f)), (int)m_playerMagicY);

		m_updatePlayerMagicTextLocation = false;
	}

	// Player health bars
	float m_playerMagicBarRatio = m_pPlayer->GetMagic() / m_pPlayer->GetMaxMagic();
	if(m_playerMagicBarRatio >= 1.0f)
	{
		m_playerMagicBarRatio = 1.0f;
	}

	float m_playerWidthModified = (m_playerMagicWidth-0.0f) * m_playerMagicBarRatio;
	float m_playerHeightModified = m_playerMagicHeight;

	float m_playerMagicXModified = m_playerMagicX + 0.0f;
	float m_playerMagicYModified = m_playerMagicY + 0.0f;

	m_pManaFillerIcon->SetDimensions((int)m_playerMagicXModified, (int)m_playerMagicYModified, (int)m_playerWidthModified, (int)m_playerHeightModified);
	m_pManaFillerBackgroundIcon->SetDimensions((int)(m_playerMagicXModified + m_playerWidthModified), (int)m_playerMagicYModified, (int)(m_playerMagicWidth-m_playerWidthModified+((m_pPlayer->GetMagic() <= 0.0f || m_pPlayer->GetMagic() >= m_pPlayer->GetMaxMagic()) ? 0 : 1)), (int)m_playerHeightModified);

	// Render the magic bar
	m_pRenderer->PushMatrix();
		m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
		glLoadIdentity();
		m_pRenderer->SetProjectionMode(PM_2D, VoxGame::GetInstance()->GetDefaultViewport());

		m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		m_pRenderer->SetRenderMode(RM_SOLID);

		m_pRenderer->EnableImmediateMode(IM_QUADS);
			/*
			m_pRenderer->ImmediateColourAlpha(0.137f, 0.419f, 0.517f, 1.0f);
			m_pRenderer->ImmediateVertex(m_playerMagicXModified, m_playerMagicYModified, 2.0f);
			m_pRenderer->ImmediateVertex(m_playerMagicXModified + m_playerWidthModified, m_playerMagicYModified, 2.0f);
			m_pRenderer->ImmediateColourAlpha(0.298f, 0.674f, 0.803f, 1.0f);
			m_pRenderer->ImmediateVertex(m_playerMagicXModified + m_playerWidthModified, m_playerMagicYModified + m_playerHeightModified, 2.0f);
			m_pRenderer->ImmediateVertex(m_playerMagicXModified, m_playerMagicYModified + m_playerHeightModified, 2.0f);

			// Background
			m_pRenderer->ImmediateColourAlpha(0.59f, 0.24f, 0.02f, 1.0f);
			m_pRenderer->ImmediateVertex(m_playerMagicXModified, m_playerMagicYModified, 2.0f);
			m_pRenderer->ImmediateVertex(m_playerMagicXModified + m_playerMagicWidth, m_playerMagicYModified, 2.0f);
			m_pRenderer->ImmediateColourAlpha(1.0f, 0.81f, 0.36f, 1.0f);
			m_pRenderer->ImmediateVertex(m_playerMagicXModified + m_playerMagicWidth, m_playerMagicYModified + m_playerHeightModified, 2.0f);
			m_pRenderer->ImmediateVertex(m_playerMagicXModified, m_playerMagicYModified + m_playerHeightModified, 2.0f);
			*/
		m_pRenderer->DisableImmediateMode();
		m_pRenderer->DisableTransparency();
	m_pRenderer->PopMatrix();
}

void HUD::RenderPlayerExperience()
{
	if(m_updatePlayerExperienceTextLocation)
	{
		int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_18(), "%s", m_pExperienceLabel->GetText().c_str());
		m_pExperienceLabel->SetLocation((int)(m_playerExperienceX + (m_playerExperienceWidth*0.5f) - (int)(textWidth*0.5f)), (int)m_playerExperienceY);

		m_updatePlayerExperienceTextLocation = false;
	}

	// Player experience bars
	float m_playerExperienceBarRatio = (float)m_pPlayer->GetPlayerStats()->GetExperience() / (float)m_pPlayer->GetPlayerStats()->GetMaxExperience();
	if(m_playerExperienceBarRatio >= 1.0f)
	{
		m_playerExperienceBarRatio = 1.0f;
	}

	float m_playerWidthModified = (m_playerExperienceWidth-0.0f) * m_playerExperienceBarRatio;
	float m_playerHeightModified = m_playerExperienceHeight;

	float m_playerExperienceXModified = m_playerExperienceX + 0.0f;
	float m_playerExperienceYModified = m_playerExperienceY + 0.0f;
	
	m_pExperienceFillerIcon->SetDimensions((int)m_playerExperienceXModified, (int)m_playerExperienceYModified, (int)m_playerWidthModified, (int)m_playerHeightModified);
	m_pExperienceFillerBackgroundIcon->SetDimensions((int)(m_playerExperienceXModified + m_playerWidthModified), (int)m_playerExperienceYModified, (int)(m_playerExperienceWidth-m_playerWidthModified+((m_pPlayer->GetPlayerStats()->GetExperience() <= 0.0f || m_pPlayer->GetPlayerStats()->GetExperience() >= m_pPlayer->GetPlayerStats()->GetMaxExperience()) ? 0 : 1)), (int)m_playerHeightModified);

	// Render the experience bar
	m_pRenderer->PushMatrix();
		m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
		glLoadIdentity();
		m_pRenderer->SetProjectionMode(PM_2D, VoxGame::GetInstance()->GetDefaultViewport());

		m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		m_pRenderer->SetRenderMode(RM_SOLID);

		m_pRenderer->EnableImmediateMode(IM_QUADS);
			/*
			m_pRenderer->ImmediateColourAlpha(0.16f, 0.09f, 0.24f, 1.0f);
			m_pRenderer->ImmediateVertex(m_playerExperienceXModified, m_playerExperienceYModified, 2.0f);
			m_pRenderer->ImmediateVertex(m_playerExperienceXModified + m_playerWidthModified, m_playerExperienceYModified, 2.0f);
			m_pRenderer->ImmediateColourAlpha(0.67f, 0.37f, 0.95f, 1.0f);
			m_pRenderer->ImmediateVertex(m_playerExperienceXModified + m_playerWidthModified, m_playerExperienceYModified + m_playerHeightModified, 2.0f);
			m_pRenderer->ImmediateVertex(m_playerExperienceXModified, m_playerExperienceYModified + m_playerHeightModified, 2.0f);

			// Background
			m_pRenderer->ImmediateColourAlpha(0.59f, 0.24f, 0.02f, 1.0f);
			m_pRenderer->ImmediateVertex(m_playerExperienceXModified, m_playerExperienceYModified, 2.0f);
			m_pRenderer->ImmediateVertex(m_playerExperienceXModified + m_playerExperienceWidth, m_playerExperienceYModified, 2.0f);
			m_pRenderer->ImmediateColourAlpha(1.0f, 0.81f, 0.36f, 1.0f);
			m_pRenderer->ImmediateVertex(m_playerExperienceXModified + m_playerExperienceWidth, m_playerExperienceYModified + m_playerHeightModified, 2.0f);
			m_pRenderer->ImmediateVertex(m_playerExperienceXModified, m_playerExperienceYModified + m_playerHeightModified, 2.0f);
			*/
		m_pRenderer->DisableImmediateMode();
		m_pRenderer->DisableTransparency();
	m_pRenderer->PopMatrix();
}

void HUD::RenderEnemyHealth()
{
	if(m_updateEnemyTextLocation)
	{
		int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_20(), "%s", m_pEnemyHealthLabel->GetText().c_str());
		m_pEnemyHealthLabel->SetLocation((int)((m_windowWidth*0.5f) - (int)(textWidth*0.5f)), (int)m_enemyHealthY+7);

		textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_30(), "%s", m_pEnemyNameLabel->GetText().c_str());
		m_pEnemyNameLabel->SetLocation((int)((m_windowWidth*0.5f) - (int)(textWidth*0.5f)), (int)m_enemyHealthY+35);

		m_updateEnemyTextLocation = false;
	}

	bool componentStillRender = false;
	if(m_pGUI->GetComponent(m_pEnemyHealthContainerIcon) == m_pEnemyHealthContainerIcon)
	{
		componentStillRender = true;
	}

	if(componentStillRender || (m_enemyInfoDisappearTimer > 0.0f && m_removedEnemyInfo == false))
	{
		vec3 lTranslation;// = m_pEnemyHealthContainerIcon->GetAnimationUpdater()->GetAnimatedTranslation();

		// Enemy health bar
		float m_enemyHealthBarRatio = m_enemyHealth / m_enemyHealthMax;
		if(m_enemyHealthBarRatio >= 1.0f)
		{
			m_enemyHealthBarRatio = 1.0f;
		}

		float m_enemyWidthModified = (m_enemyHealthWidth-0.0f) * m_enemyHealthBarRatio;
		float m_enemyHeightModified = m_enemyHealthHeight;

		float m_enemyHealthXModified = m_enemyHealthX + 4.0f + lTranslation.x;
		float m_enemyHealthYModified = m_enemyHealthY + 4.0f + lTranslation.y;

		m_enemyWidthModified -= 8.0f;
		m_enemyHeightModified -= 8.0f;

		float m_enemyYellowX = m_enemyHealthXModified + m_enemyWidthModified;
		float m_enemyYellowY = m_enemyHealthYModified;
		float m_enemyYellowWidth = (m_enemyHealthWidthDecreaser * (m_enemyHealthWidth / m_enemyHealthMax));
		float m_enemyYellowHeight = m_enemyHeightModified;

		m_pEnemyHealthFillerIcon->SetDimensions((int)m_enemyHealthXModified, (int)m_enemyHealthYModified, (int)m_enemyWidthModified, (int)m_enemyHeightModified);
		m_pEnemyHealthFillerBackgroundIcon->SetDimensions((int)(m_enemyHealthXModified + m_enemyWidthModified+(m_enemyHealth <= 0.0f ? 8.0f : 0.0f)), (int)m_enemyHealthYModified, (int)(m_enemyHealthWidth-m_enemyWidthModified-(m_enemyHealth > 0.0f ? 8.0f : 16.0f)), (int)m_enemyHeightModified);

		// Render the health bar
		m_pRenderer->PushMatrix();
			m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
			glLoadIdentity();
			m_pRenderer->SetProjectionMode(PM_2D, VoxGame::GetInstance()->GetDefaultViewport());

			m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

			glActiveTextureARB(GL_TEXTURE0_ARB);
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);

			m_pRenderer->SetRenderMode(RM_SOLID);

			m_pRenderer->EnableImmediateMode(IM_QUADS);
				/*
				m_pRenderer->ImmediateColourAlpha(0.94f, 0.15f, 0.04f, 1.0f);
				m_pRenderer->ImmediateVertex(m_enemyHealthXModified, m_enemyHealthYModified, 2.0f);
				m_pRenderer->ImmediateVertex(m_enemyHealthXModified + m_enemyWidthModified, m_enemyHealthYModified, 2.0f);
				m_pRenderer->ImmediateColourAlpha(0.94f, 0.45f, 0.29f, 1.0f);
				m_pRenderer->ImmediateVertex(m_enemyHealthXModified + m_enemyWidthModified, m_enemyHealthYModified + m_enemyHeightModified, 2.0f);
				m_pRenderer->ImmediateVertex(m_enemyHealthXModified, m_enemyHealthYModified + m_enemyHeightModified, 2.0f);

				// Width decreaser
				m_pRenderer->ImmediateColourAlpha(1.0f, 0.75f, 0.0f, 1.0f);
				m_pRenderer->ImmediateVertex(m_enemyYellowX, m_enemyYellowY, 2.0f);
				m_pRenderer->ImmediateVertex(m_enemyYellowX + m_enemyYellowWidth, m_enemyYellowY, 2.0f);
				m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.5f, 1.0f);
				m_pRenderer->ImmediateVertex(m_enemyYellowX + m_enemyYellowWidth, m_enemyYellowY + m_enemyHeightModified, 2.0f);
				m_pRenderer->ImmediateVertex(m_enemyYellowX, m_enemyYellowY + m_enemyHeightModified, 2.0f);

				// Background
				m_pRenderer->ImmediateColourAlpha(0.59f, 0.24f, 0.02f, 1.0f);
				m_pRenderer->ImmediateVertex(m_enemyHealthXModified, m_enemyHealthYModified, 2.0f);
				m_pRenderer->ImmediateVertex(m_enemyHealthXModified + m_enemyHealthWidth, m_enemyHealthYModified, 2.0f);
				m_pRenderer->ImmediateColourAlpha(1.0f, 0.81f, 0.36f, 1.0f);
				m_pRenderer->ImmediateVertex(m_enemyHealthXModified + m_enemyHealthWidth, m_enemyHealthYModified + m_enemyHeightModified, 2.0f);
				m_pRenderer->ImmediateVertex(m_enemyHealthXModified, m_enemyHealthYModified + m_enemyHeightModified, 2.0f);
				*/
			m_pRenderer->DisableImmediateMode();
			m_pRenderer->DisableTransparency();
		m_pRenderer->PopMatrix();
	}
}

void HUD::_DeathTextFinished(void *apData)
{
	HUD* lpHUD = (HUD*)apData;
	lpHUD->DeathTextFinished();
}

void HUD::DeathTextFinished()
{
	m_pRespawnButton->SetLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pGUI->AddComponent(m_pRespawnButton);

	VoxGame::GetInstance()->TurnCursorOn(true, false);
}

void HUD::_RespawnPressed(void *apData)
{
	HUD* lpHUD = (HUD*)apData;
	lpHUD->RespawnPressed();
}

void HUD::RespawnPressed()
{
	m_pPlayer->Respawn();

	PlayerRespawn();

	VoxGame::GetInstance()->PlayerRespawned();
	VoxGame::GetInstance()->InitializeCameraRotation();

	VoxGame::GetInstance()->TurnCursorOff(false);
}

void HUD::_LevelUpTextFinished(void *apData)
{
	HUD* lpHUD = (HUD*)apData;
	lpHUD->LevelUpTextFinished();
}

void HUD::LevelUpTextFinished()
{
	m_levelUp = false;

	m_pGUI->RemoveComponent(m_pLevelUpLabel);
}

// TODO : Checkboxes for GUI panels on the HUD
//void HUD::_InventoryCheckboxPressed(void *apData)
//{
//	HUD* lpHUD = (HUD*)apData;
//	lpHUD->InventoryCheckboxPressed();
//}
//
//void HUD::InventoryCheckboxPressed()
//{
//	if(m_pGameWindow->IsPaused())
//	{
//		return;
//	}
//
//	if(m_pInventoryGUI->IsLoaded())
//	{
//		m_pInventoryGUI->Unload();
//
//		if(m_pGameWindow->IsGUIWindowStillDisplayed() == false)
//		{
//			m_pGameWindow->TurnCursorOff();
//		}
//	}
//	else
//	{
//		m_pInventoryGUI->Load();
//	}
//}
//
//void HUD::_CharacterCheckboxPressed(void *apData)
//{
//	HUD* lpHUD = (HUD*)apData;
//	lpHUD->CharacterCheckboxPressed();
//}
//
//void HUD::CharacterCheckboxPressed()
//{
//	if(m_pGameWindow->IsPaused())
//	{
//		return;
//	}
//
//	if(m_pCharacterGUI->IsLoaded())
//	{
//		m_pCharacterGUI->Unload();
//
//		if(m_pGameWindow->IsGUIWindowStillDisplayed() == false)
//		{
//			m_pGameWindow->TurnCursorOff();
//		}
//	}
//	else
//	{
//		m_pCharacterGUI->Load();
//	}
//}
//
//void HUD::_QuestCheckboxPressed(void *apData)
//{
//	HUD* lpHUD = (HUD*)apData;
//	lpHUD->QuestCheckboxPressed();
//}
//
//void HUD::QuestCheckboxPressed()
//{
//	if(m_pGameWindow->IsPaused())
//	{
//		return;
//	}
//
//	if(m_pQuestGUI->IsLoaded())
//	{
//		m_pQuestGUI->Unload();
//
//		if(m_pGameWindow->IsGUIWindowStillDisplayed() == false)
//		{
//			m_pGameWindow->TurnCursorOff();
//		}
//	}
//	else
//	{
//		m_pQuestGUI->Load();
//	}
//}
//
//void HUD::_CraftingCheckboxPressed(void *apData)
//{
//	HUD* lpHUD = (HUD*)apData;
//	lpHUD->CraftingCheckboxPressed();
//}
//
//void HUD::CraftingCheckboxPressed()
//{
//	if(m_pGameWindow->IsPaused())
//	{
//		return;
//	}
//
//	if(m_pCraftingGUI->IsLoaded())
//	{
//		m_pCraftingGUI->Unload();
//
//		if(m_pGameWindow->IsGUIWindowStillDisplayed() == false)
//		{
//			m_pGameWindow->TurnCursorOff();
//		}
//	}
//	else
//	{
//		m_pCraftingGUI->SetCraftingRecipesForItem(eItem_None);
//		m_pCraftingGUI->Load();
//	}
//}
//
//void HUD::_SkillsCheckboxPressed(void *apData)
//{
//	HUD* lpHUD = (HUD*)apData;
//	lpHUD->SkillsCheckboxPressed();
//}
//
//void HUD::SkillsCheckboxPressed()
//{
//	if(m_pGameWindow->IsPaused())
//	{
//		return;
//	}
//
//	if(m_pSkillsGUI->IsLoaded())
//	{
//		m_pSkillsGUI->Unload();
//
//		if(m_pGameWindow->IsGUIWindowStillDisplayed() == false)
//		{
//			m_pGameWindow->TurnCursorOff();
//		}
//	}
//	else
//	{
//		m_pSkillsGUI->Load();
//	}
//}
//
//void HUD::_PetCheckboxPressed(void *apData)
//{
//	HUD* lpHUD = (HUD*)apData;
//	lpHUD->PetCheckboxPressed();
//}
//
//void HUD::PetCheckboxPressed()
//{
//	if(m_pGameWindow->IsPaused())
//	{
//		return;
//	}
//
//	if(m_pCompanionPet->IsSummoned())
//	{
//		m_pCompanionPet->UnsummonPet();
//	}
//	else
//	{
//		m_pCompanionPet->SummonPet();
//	}
//}