// ******************************************************************************
// Filename:    FrontendManager.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 01/11/15
//
// Copyright (c) 2005-2015, Steven Ball
// ******************************************************************************

#include "FrontendManager.h"
#include "FrontendPage.h"
#include "Pages/MainMenu.h"
#include "Pages/SelectCharacter.h"
#include "Pages/CreateCharacter.h"
#include "Pages/QuitPopup.h"
#include "Pages/PauseMenu.h"
#include "Pages/ModMenu.h"
#include "Pages/OptionsMenu.h"
#include "Pages/Credits.h"
#include "../VoxGame.h"

#include <iostream>
#include "ini/INIReader.h"

#include <fstream>
#include <ostream>
#include <iostream>
#include <string>
using namespace std;


FrontendManager::FrontendManager(Renderer* pRenderer, OpenGLGUI* pGUI)
{
	m_pRenderer = pRenderer;
	m_pGUI = pGUI;

	m_pCamera = nullptr;

	// Fonts
	m_pRenderer->CreateFreeTypeFont("media/fonts/pf_ronda_seven.ttf", 26, &m_frontendFont_Large, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/pf_ronda_seven.ttf", 18, &m_frontendFont_Medium, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/pf_ronda_seven.ttf", 12, &m_frontendFont_Small, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolbackot.ttf", 14, &m_frontendFont_14, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolot.ttf", 14, &m_frontendFont_14_Outline, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolbackot.ttf", 18, &m_frontendFont_18, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolot.ttf", 18, &m_frontendFont_18_Outline, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolbackot.ttf", 20, &m_frontendFont_20, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolot.ttf", 20, &m_frontendFont_20_Outline, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolbackot.ttf", 22, &m_frontendFont_22, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolot.ttf", 22, &m_frontendFont_22_Outline, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolbackot.ttf", 25, &m_frontendFont_25, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolot.ttf", 25, &m_frontendFont_25_Outline, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolbackot.ttf", 30, &m_frontendFont_30, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolot.ttf", 30, &m_frontendFont_30_Outline, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolbackot.ttf", 35, &m_frontendFont_35, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolot.ttf", 35, &m_frontendFont_35_Outline, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolbackot.ttf", 40, &m_frontendFont_40, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolot.ttf", 40, &m_frontendFont_40_Outline, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolbackot.ttf", 50, &m_frontendFont_50, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolot.ttf", 50, &m_frontendFont_50_Outline, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolbackot.ttf", 80, &m_frontendFont_80, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolot.ttf", 80, &m_frontendFont_80_Outline, true);

	// Text effects fonts
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolbackot.ttf", 34, &m_textEffectFontNormal, true);
	m_pRenderer->CreateFreeTypeFont("media/fonts/screenloggercoolot.ttf", 34, &m_textEffectFontOutline, true);

	// Label colours
	m_normalColour = Colour(1.0f, 1.0f, 1.0f);
	m_hoverColour = Colour(0.74f, 0.49f, 0.26f);
	m_pressedColour = Colour(0.5f, 0.3f, 0.01f);
	m_disabledColour = Colour(0.25f, 0.25f, 0.25f);

	// Checkbox
	m_pCheckboxIcon = new Icon(m_pRenderer, "", 16, 16);
	m_pCheckboxIcon->SetDepth(2.0f);
	m_pCheckboxIconHover = new Icon(m_pRenderer, "", 16, 16);
	m_pCheckboxIconHover->SetDepth(2.0f);
	m_pCheckboxIconPressed = new Icon(m_pRenderer, "", 16, 16);
	m_pCheckboxIconPressed->SetDepth(2.0f);
	m_pCheckboxIconDisabled = new Icon(m_pRenderer, "", 16, 16);
	m_pCheckboxIconDisabled->SetDepth(2.0f);
	m_pCheckboxIconToggled = new Icon(m_pRenderer, "", 16, 16);
	m_pCheckboxIconToggled->SetDepth(2.0f);
	m_pCheckboxIconToggledHover = new Icon(m_pRenderer, "", 16, 16);
	m_pCheckboxIconToggledHover->SetDepth(2.0f);
	m_pCheckboxIconToggledPressed = new Icon(m_pRenderer, "", 16, 16);
	m_pCheckboxIconToggledPressed->SetDepth(2.0f);
	m_pCheckboxIconToggledDisabled = new Icon(m_pRenderer, "", 16, 16);
	m_pCheckboxIconToggledDisabled->SetDepth(2.0f);

	// Optionbox
	m_pOptionboxIcon = new Icon(m_pRenderer, "", 16, 16);
	m_pOptionboxIcon->SetDepth(2.0f);
	m_pOptionboxIconHover = new Icon(m_pRenderer, "", 16, 16);
	m_pOptionboxIconHover->SetDepth(2.0f);
	m_pOptionboxIconPressed = new Icon(m_pRenderer, "", 16, 16);
	m_pOptionboxIconPressed->SetDepth(2.0f);
	m_pOptionboxIconDisabled = new Icon(m_pRenderer, "", 16, 16);
	m_pOptionboxIconDisabled->SetDepth(2.0f);
	m_pOptionboxIconToggled = new Icon(m_pRenderer, "", 16, 16);
	m_pOptionboxIconToggled->SetDepth(2.0f);
	m_pOptionboxIconToggledHover = new Icon(m_pRenderer, "", 16, 16);
	m_pOptionboxIconToggledHover->SetDepth(2.0f);
	m_pOptionboxIconToggledPressed = new Icon(m_pRenderer, "", 16, 16);
	m_pOptionboxIconToggledPressed->SetDepth(2.0f);
	m_pOptionboxIconToggledDisabled = new Icon(m_pRenderer, "", 16, 16);
	m_pOptionboxIconToggledDisabled->SetDepth(2.0f);

	// Scrollbar
	m_pScrollbarArrowDownIcon = new Icon(m_pRenderer, "", 18, 18);
	m_pScrollbarArrowDownIcon->SetDepth(2.0f);
	m_pScrollbarArrowDownIconHover = new Icon(m_pRenderer, "", 18, 18);
	m_pScrollbarArrowDownIconHover->SetDepth(2.0f);
	m_pScrollbarArrowDownIconPressed = new Icon(m_pRenderer, "", 18, 18);
	m_pScrollbarArrowDownIconPressed->SetDepth(2.0f);
	m_pScrollbarArrowDownIconDisabled = new Icon(m_pRenderer, "", 18, 18);
	m_pScrollbarArrowDownIconDisabled->SetDepth(2.0f);
	m_pScrollbarArrowUpIcon = new Icon(m_pRenderer, "", 18, 18);
	m_pScrollbarArrowUpIcon->SetDepth(2.0f);
	m_pScrollbarArrowUpIconHover = new Icon(m_pRenderer, "", 18, 18);
	m_pScrollbarArrowUpIconHover->SetDepth(2.0f);
	m_pScrollbarArrowUpIconPressed = new Icon(m_pRenderer, "", 18, 18);
	m_pScrollbarArrowUpIconPressed->SetDepth(2.0f);
	m_pScrollbarArrowUpIconDisabled = new Icon(m_pRenderer, "", 18, 18);
	m_pScrollbarArrowUpIconDisabled->SetDepth(2.0f);
	m_pScrollbarBackgroundIcon = new Icon(m_pRenderer, "", 18, 18);
	m_pScrollbarBackgroundIcon->SetDepth(2.0f);
	m_pScrollbarBackgroundIconDisabled = new Icon(m_pRenderer, "", 18, 18);
	m_pScrollbarBackgroundIconDisabled->SetDepth(2.0f);
	m_pScrollbarIcon = new Icon(m_pRenderer, "", 18, 18);
	m_pScrollbarIcon->SetDepth(4.0f);
	m_pScrollbarIconHover = new Icon(m_pRenderer, "", 18, 18);
	m_pScrollbarIconHover->SetDepth(4.0f);
	m_pScrollbarIconPressed = new Icon(m_pRenderer, "", 18, 18);
	m_pScrollbarIconPressed->SetDepth(4.0f);
	m_pScrollbarIconDisabled = new Icon(m_pRenderer, "", 18, 18);
	m_pScrollbarIconDisabled->SetDepth(4.0f);

	// Slider
	m_pSliderIconDefault = new Icon(m_pRenderer, "", 12, 24);
	m_pSliderIconDefault->SetDepth(3.0f);
	m_pSliderIconHover = new Icon(m_pRenderer, "", 12, 24);
	m_pSliderIconHover->SetDepth(3.0f);
	m_pSliderIconDrag = new Icon(m_pRenderer, "", 12, 24);
	m_pSliderIconDrag->SetDepth(3.0f);
	m_pSliderIconDefaultBlank = new Icon(m_pRenderer, "", 12, 24);
	m_pSliderIconDefaultBlank->SetDepth(3.0f);
	m_pSliderIconHoverBlank = new Icon(m_pRenderer, "", 12, 24);
	m_pSliderIconHoverBlank->SetDepth(3.0f);
	m_pSliderIconDragBlank = new Icon(m_pRenderer, "", 12, 24);
	m_pSliderIconDragBlank->SetDepth(3.0f);
	m_pSliderIncrementIcon = new Icon(m_pRenderer, "", 12, 12);
	m_pSliderIncrementIcon->SetDepth(3.0f);
	m_pSliderIncrementIconHover = new Icon(m_pRenderer, "", 12, 12);
	m_pSliderIncrementIconHover->SetDepth(3.0f);
	m_pSliderIncrementIconPressed = new Icon(m_pRenderer, "", 12, 12);
	m_pSliderIncrementIconPressed->SetDepth(3.0f);
	m_pSliderIncrementIconDisabled = new Icon(m_pRenderer, "", 12, 12);
	m_pSliderIncrementIconDisabled->SetDepth(3.0f);
	m_pSliderDecrementIcon = new Icon(m_pRenderer, "", 12, 12);
	m_pSliderDecrementIcon->SetDepth(3.0f);
	m_pSliderDecrementIconHover = new Icon(m_pRenderer, "", 12, 12);
	m_pSliderDecrementIconHover->SetDepth(3.0f);
	m_pSliderDecrementIconPressed = new Icon(m_pRenderer, "", 12, 12);
	m_pSliderDecrementIconPressed->SetDepth(3.0f);
	m_pSliderDecrementIconDisabled = new Icon(m_pRenderer, "", 12, 12);
	m_pSliderDecrementIconDisabled->SetDepth(3.0f);

	// Buttons
	m_pButtonIcon = new Icon*[ButtonSize_NUM];
	m_pButtonIconHover = new Icon*[ButtonSize_NUM];
	m_pButtonIconPressed = new Icon*[ButtonSize_NUM];
	m_pButtonIconDisabled = new Icon*[ButtonSize_NUM];
	for (int i = 0; i < ButtonSize_NUM; i++)
	{
		int width = 0;
		int height = 0;
		if (ButtonSize(i) == ButtonSize_85x25) { width = 85; height = 25; }
		if (ButtonSize(i) == ButtonSize_65x25) { width = 65; height = 25; }
		if (ButtonSize(i) == ButtonSize_110x47) { width = 110; height = 47; }
		if (ButtonSize(i) == ButtonSize_225x75) { width = 225; height = 75; }

		m_pButtonIcon[ButtonSize(i)] = new Icon(m_pRenderer, "", width, height);
		m_pButtonIcon[ButtonSize(i)]->SetDepth(2.0f);
		m_pButtonIconHover[ButtonSize(i)] = new Icon(m_pRenderer, "", width, height);
		m_pButtonIconHover[ButtonSize(i)]->SetDepth(2.0f);
		m_pButtonIconPressed[ButtonSize(i)] = new Icon(m_pRenderer, "", width, height);
		m_pButtonIconPressed[ButtonSize(i)]->SetDepth(2.0f);
		m_pButtonIconDisabled[ButtonSize(i)] = new Icon(m_pRenderer, "", width, height);
		m_pButtonIconDisabled[ButtonSize(i)]->SetDepth(2.0f);
	}

	// Close button
	m_pCloseExitButtonIcon = new Icon(m_pRenderer, "", 32, 32);
	m_pCloseExitButtonIcon->SetDepth(1.0f);
	m_pCloseExitButtonIcon_Hover = new Icon(m_pRenderer, "", 32, 32);
	m_pCloseExitButtonIcon_Hover->SetDepth(1.0f);
	m_pCloseExitButtonIcon_Pressed = new Icon(m_pRenderer, "", 32, 32);
	m_pCloseExitButtonIcon_Pressed->SetDepth(1.0f);

	// Rotate buttons
	m_pArrowLeft_Icon = new Icon(m_pRenderer, "", 32, 32);
	m_pArrowLeft_Icon->SetDepth(2.0f);
	m_pArrowLeft_Icon_Hover = new Icon(m_pRenderer, "", 32, 32);
	m_pArrowLeft_Icon_Hover->SetDepth(2.0f);
	m_pArrowLeft_Icon_Pressed = new Icon(m_pRenderer, "", 32, 32);
	m_pArrowLeft_Icon_Pressed->SetDepth(2.0f);
	m_pArrowRight_Icon = new Icon(m_pRenderer, "", 32, 32);
	m_pArrowRight_Icon->SetDepth(2.0f);
	m_pArrowRight_Icon_Hover = new Icon(m_pRenderer, "", 32, 32);
	m_pArrowRight_Icon_Hover->SetDepth(2.0f);
	m_pArrowRight_Icon_Pressed = new Icon(m_pRenderer, "", 32, 32);
	m_pArrowRight_Icon_Pressed->SetDepth(2.0f);

	// Tab options
	m_pTab75OptionIcon = new Icon(m_pRenderer, "", 75, 44);
	m_pTab75OptionIcon->SetDepth(2.0f);
	m_pTab75OptionIcon_Hover = new Icon(m_pRenderer, "", 75, 44);
	m_pTab75OptionIcon_Hover->SetDepth(2.0f);
	m_pTab75OptionIcon_Pressed = new Icon(m_pRenderer, "", 75, 44);
	m_pTab75OptionIcon_Pressed->SetDepth(2.0f);
	m_pTab75OptionIcon_Toggled = new Icon(m_pRenderer, "", 75, 44);
	m_pTab75OptionIcon_Toggled->SetDepth(2.0f);
	m_pTab75OptionIcon_Toggled_Hover = new Icon(m_pRenderer, "", 75, 44);
	m_pTab75OptionIcon_Toggled_Hover->SetDepth(2.0f);
	m_pTab75OptionIcon_Toggled_Pressed = new Icon(m_pRenderer, "", 75, 44);
	m_pTab75OptionIcon_Toggled_Pressed->SetDepth(2.0f);

	// Common, shared frontend page params
	m_cameraOrbitTimer = 15.0f;

	// Constants
	m_tooltipAppearDelay = 0.25f;

	// Load the common graphics that are set as the currently loaded mod
	LoadCommonGraphics(VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme());

	// Pages
	int width = 800;
	int height = 800;
	FrontendPage* pMainMenu = new MainMenu(m_pRenderer, m_pGUI, this, width, height);
	FrontendPage* pSelectCharacter = new SelectCharacter(m_pRenderer, m_pGUI, this, width, height);
	FrontendPage* pCreateCharacter = new CreateCharacter(m_pRenderer, m_pGUI, this, width, height);
	FrontendPage* pQuitPopup = new QuitPopup(m_pRenderer, m_pGUI, this, width, height);
	FrontendPage* pPauseMenu = new PauseMenu(m_pRenderer, m_pGUI, this, width, height);
	FrontendPage* pOptionsMenu = new OptionsMenu(m_pRenderer, m_pGUI, this, width, height);
	FrontendPage* pCredits = new Credits(m_pRenderer, m_pGUI, this, width, height);
	FrontendPage* pModMenu = new ModMenu(m_pRenderer, m_pGUI, this, width, height);

	m_vpFrontendPages.push_back(pMainMenu);
	m_vpFrontendPages.push_back(pSelectCharacter);
	m_vpFrontendPages.push_back(pCreateCharacter);
	m_vpFrontendPages.push_back(pQuitPopup);
	m_vpFrontendPages.push_back(pPauseMenu);
	m_vpFrontendPages.push_back(pOptionsMenu);
	m_vpFrontendPages.push_back(pCredits);
	m_vpFrontendPages.push_back(pModMenu);

	// Initial page
	m_currentScreen = FrontendScreen_None;
	m_currentPage = nullptr;
	SetFrontendScreen(FrontendScreen_None);
}

FrontendManager::~FrontendManager()
{
	// Frontend pages
	for (unsigned int i = 0; i < m_vpFrontendPages.size(); i++)
	{
		delete m_vpFrontendPages[i];
		m_vpFrontendPages[i] = 0;
	}
	m_vpFrontendPages.clear();

	// Checkbox
	delete m_pCheckboxIcon;
	delete m_pCheckboxIconHover;
	delete m_pCheckboxIconPressed;
	delete m_pCheckboxIconDisabled;
	delete m_pCheckboxIconToggled;
	delete m_pCheckboxIconToggledHover;
	delete m_pCheckboxIconToggledPressed;
	delete m_pCheckboxIconToggledDisabled;

	// Optionbox
	delete m_pOptionboxIcon;
	delete m_pOptionboxIconHover;
	delete m_pOptionboxIconPressed;
	delete m_pOptionboxIconDisabled;
	delete m_pOptionboxIconToggled;
	delete m_pOptionboxIconToggledHover;
	delete m_pOptionboxIconToggledPressed;
	delete m_pOptionboxIconToggledDisabled;

	// Scrollbar
	delete m_pScrollbarArrowDownIcon;
	delete m_pScrollbarArrowDownIconHover;
	delete m_pScrollbarArrowDownIconPressed;
	delete m_pScrollbarArrowDownIconDisabled;
	delete m_pScrollbarArrowUpIcon;
	delete m_pScrollbarArrowUpIconHover;
	delete m_pScrollbarArrowUpIconPressed;
	delete m_pScrollbarArrowUpIconDisabled;
	delete m_pScrollbarBackgroundIcon;
	delete m_pScrollbarBackgroundIconDisabled;
	delete m_pScrollbarIcon;
	delete m_pScrollbarIconHover;
	delete m_pScrollbarIconPressed;
	delete m_pScrollbarIconDisabled;

	// Slider
	delete m_pSliderIconDefault;
	delete m_pSliderIconHover;
	delete m_pSliderIconDrag;
	delete m_pSliderIconDefaultBlank;
	delete m_pSliderIconHoverBlank;
	delete m_pSliderIconDragBlank;
	delete m_pSliderIncrementIcon;
	delete m_pSliderIncrementIconHover;
	delete m_pSliderIncrementIconPressed;
	delete m_pSliderIncrementIconDisabled;
	delete m_pSliderDecrementIcon;
	delete m_pSliderDecrementIconHover;
	delete m_pSliderDecrementIconPressed;
	delete m_pSliderDecrementIconDisabled;

	// Buttons
	for (int i = 0; i < ButtonSize_NUM; i++)
	{
		delete m_pButtonIcon[i];
		delete m_pButtonIconHover[i];
		delete m_pButtonIconPressed[i];
		delete m_pButtonIconDisabled[i];
	}

	// Close button
	delete m_pCloseExitButtonIcon;
	delete m_pCloseExitButtonIcon_Hover;
	delete m_pCloseExitButtonIcon_Pressed;

	// Rotate buttons
	delete m_pArrowLeft_Icon;
	delete m_pArrowLeft_Icon_Hover;
	delete m_pArrowLeft_Icon_Pressed;
	delete m_pArrowRight_Icon;
	delete m_pArrowRight_Icon_Hover;
	delete m_pArrowRight_Icon_Pressed;

	// Tab options
	delete m_pTab75OptionIcon;
	delete m_pTab75OptionIcon_Hover;
	delete m_pTab75OptionIcon_Pressed;
	delete m_pTab75OptionIcon_Toggled;
	delete m_pTab75OptionIcon_Toggled_Hover;
	delete m_pTab75OptionIcon_Toggled_Pressed;
}

// Windows dimensions
void FrontendManager::SetWindowDimensions(int width, int height)
{
	m_windowWidth = width;
	m_windowHeight = height;

	for (unsigned int i = 0; i < m_vpFrontendPages.size(); i++)
	{
		m_vpFrontendPages[i]->SetWindowDimensions(m_windowWidth, m_windowHeight);
	}
}

int FrontendManager::GetWindowWidth()
{
	return m_windowWidth;
}

int FrontendManager::GetWindowHeight()
{
	return m_windowHeight;
}

// Camera
void FrontendManager::SetCamera(Camera* pCamera)
{
	m_pCamera = pCamera;
}

// Skinning the GUI
void FrontendManager::SkinGUI()
{
	for (unsigned int i = 0; i < m_vpFrontendPages.size(); i++)
	{
		m_vpFrontendPages[i]->SkinGUI();
	}
}

void FrontendManager::UnSkinGUI()
{
	for (unsigned int i = 0; i < m_vpFrontendPages.size(); i++)
	{
		m_vpFrontendPages[i]->UnSkinGUI();
	}
}

// Frontend screen
eFrontendScreen FrontendManager::GetFrontendScreen()
{
	return m_currentScreen;
}

void FrontendManager::SetFrontendScreen(eFrontendScreen screen)
{
	m_currentScreen = screen;

	// Unload current page
	if (m_currentPage != nullptr)
	{
		m_currentPage->Unload();
		m_currentPage = nullptr;
	}

	// Find new page
	for (unsigned int i = 0; i < m_vpFrontendPages.size(); i++)
	{
		if (m_vpFrontendPages[i]->GetPageType() == screen)
		{
			m_currentPage = m_vpFrontendPages[i];
			break;
		}
	}

	// Load new page
	if (m_currentPage != nullptr)
	{
		m_currentPage->Load();
	}
}

// Load the icon graphics based on a theme
void FrontendManager::LoadCommonGraphics(string themeName)
{
	string iconName;

	// Checkbox
	iconName = "media/textures/gui/" + themeName + "/common/checkbox/checkDefault.tga";
	m_pCheckboxIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/checkbox/checkHover.tga";
	m_pCheckboxIconHover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/checkbox/checkPressed.tga";
	m_pCheckboxIconPressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/checkbox/checkDisabled.tga";
	m_pCheckboxIconDisabled->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/checkbox/checkToggled.tga";
	m_pCheckboxIconToggled->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/checkbox/checkToggledHover.tga";
	m_pCheckboxIconToggledHover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/checkbox/checkToggledPressed.tga";
	m_pCheckboxIconToggledPressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/checkbox/checkToggledDisabled.tga";
	m_pCheckboxIconToggledDisabled->SetIcon(iconName);

	// Optionbox
	iconName = "media/textures/gui/" + themeName + "/common/optionbox/optionDefault.tga";
	m_pOptionboxIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/optionbox/optionHover.tga";
	m_pOptionboxIconHover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/optionbox/optionPressed.tga";
	m_pOptionboxIconPressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/optionbox/optionDisabled.tga";
	m_pOptionboxIconDisabled->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/optionbox/optionToggled.tga";
	m_pOptionboxIconToggled->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/optionbox/optionToggledHover.tga";
	m_pOptionboxIconToggledHover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/optionbox/optionToggledPressed.tga";
	m_pOptionboxIconToggledPressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/optionbox/optionToggledDisabled.tga";
	m_pOptionboxIconToggledDisabled->SetIcon(iconName);

	// Scrollbar
	iconName = "media/textures/gui/" + themeName + "/common/scrollbar/arrowDownDefault.tga";
	m_pScrollbarArrowDownIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/scrollbar/arrowDownHover.tga";
	m_pScrollbarArrowDownIconHover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/scrollbar/arrowDownPressed.tga";
	m_pScrollbarArrowDownIconPressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/scrollbar/arrowDownDisabled.tga";
	m_pScrollbarArrowDownIconDisabled->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/scrollbar/arrowUpDefault.tga";
	m_pScrollbarArrowUpIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/scrollbar/arrowUpHover.tga";
	m_pScrollbarArrowUpIconHover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/scrollbar/arrowUpPressed.tga";
	m_pScrollbarArrowUpIconPressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/scrollbar/arrowUpDisabled.tga";
	m_pScrollbarArrowUpIconDisabled->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/scrollbar/backgroundDefault.tga";
	m_pScrollbarBackgroundIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/scrollbar/backgroundDisabled.tga";
	m_pScrollbarBackgroundIconDisabled->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/scrollbar/scrollbarDefault.tga";
	m_pScrollbarIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/scrollbar/scrollbarHover.tga";
	m_pScrollbarIconHover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/scrollbar/scrollbarPressed.tga";
	m_pScrollbarIconPressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/scrollbar/scrollbarDisabled.tga";
	m_pScrollbarIconDisabled->SetIcon(iconName);

	// Slider
	iconName = "media/textures/gui/" + themeName + "/common/slider/slider_slide.tga";
	m_pSliderIconDefault->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/slider/slider_slide_hover.tga";
	m_pSliderIconHover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/slider/slider_slide_pressed.tga";
	m_pSliderIconDrag->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/slider/slider_slide_blank.tga";
	m_pSliderIconDefaultBlank->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/slider/slider_slide_hover_blank.tga";
	m_pSliderIconHoverBlank->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/slider/slider_slide_pressed_blank.tga";
	m_pSliderIconDragBlank->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/increment_buttons/plus_default.tga";
	m_pSliderIncrementIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/increment_buttons/plus_hover.tga";
	m_pSliderIncrementIconHover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/increment_buttons/plus_pressed.tga";
	m_pSliderIncrementIconPressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/increment_buttons/plus_disabled.tga";
	m_pSliderIncrementIconDisabled->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/increment_buttons/minus_default.tga";
	m_pSliderDecrementIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/increment_buttons/minus_hover.tga";
	m_pSliderDecrementIconHover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/increment_buttons/minus_pressed.tga";
	m_pSliderDecrementIconPressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/increment_buttons/minus_disabled.tga";
	m_pSliderDecrementIconDisabled->SetIcon(iconName);

	// Buttons
	for (int i = 0; i < ButtonSize_NUM; i++)
	{
		string sizeFolder = "";
		if (ButtonSize(i) == ButtonSize_85x25) { sizeFolder = "85x25"; }
		if (ButtonSize(i) == ButtonSize_65x25) { sizeFolder = "65x25"; }
		if (ButtonSize(i) == ButtonSize_110x47) { sizeFolder = "110x47"; }
		if (ButtonSize(i) == ButtonSize_225x75) { sizeFolder = "225x75"; }

		iconName = "media/textures/gui/" + themeName + "/common/buttons/" + sizeFolder + "/buttonDefault.tga";
		m_pButtonIcon[ButtonSize(i)]->SetIcon(iconName);
		iconName = "media/textures/gui/" + themeName + "/common/buttons/" + sizeFolder + "/buttonHover.tga";
		m_pButtonIconHover[ButtonSize(i)]->SetIcon(iconName);
		iconName = "media/textures/gui/" + themeName + "/common/buttons/" + sizeFolder + "/buttonPressed.tga";
		m_pButtonIconPressed[ButtonSize(i)]->SetIcon(iconName);
		iconName = "media/textures/gui/" + themeName + "/common/buttons/" + sizeFolder + "/buttonDisabled.tga";
		m_pButtonIconDisabled[ButtonSize(i)]->SetIcon(iconName);
	}

	// Close button
	iconName = "media/textures/gui/" + themeName + "/common/close_button/close_exit.tga";
	m_pCloseExitButtonIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/close_button/close_exit_hover.tga";
	m_pCloseExitButtonIcon_Hover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/close_button/close_exit_pressed.tga";
	m_pCloseExitButtonIcon_Pressed->SetIcon(iconName);

	// Rotate buttons
	iconName = "media/textures/gui/" + themeName + "/common/rotate/arrow_left.tga";
	m_pArrowLeft_Icon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/rotate/arrow_left_hover.tga";
	m_pArrowLeft_Icon_Hover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/rotate/arrow_left_pressed.tga";
	m_pArrowLeft_Icon_Pressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/rotate/arrow_right.tga";
	m_pArrowRight_Icon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/rotate/arrow_right_hover.tga";
	m_pArrowRight_Icon_Hover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/rotate/arrow_right_pressed.tga";
	m_pArrowRight_Icon_Pressed->SetIcon(iconName);

	// Tab options
	iconName = "media/textures/gui/" + themeName + "/common/tab_option/tab.tga";
	m_pTab75OptionIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/tab_option/tab_hover.tga";
	m_pTab75OptionIcon_Hover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/tab_option/tab_pressed.tga";
	m_pTab75OptionIcon_Pressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/tab_option/tab_toggled.tga";
	m_pTab75OptionIcon_Toggled->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/tab_option/tab_toggled_hover.tga";
	m_pTab75OptionIcon_Toggled_Hover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/tab_option/tab_toggled_pressed.tga";
	m_pTab75OptionIcon_Toggled_Pressed->SetIcon(iconName);

	// Label colours
	string settingsIniFile = "media/textures/gui/" + themeName + "/theme.ini";
	INIReader reader(settingsIniFile);

	if (reader.ParseError() >= 0)
	{
		float r = (float)reader.GetReal("NormalLabel", "Red", 1.0f);
		float g = (float)reader.GetReal("NormalLabel", "Green", 1.0f);
		float b = (float)reader.GetReal("NormalLabel", "Blue", 1.0f);
		m_normalColour = Colour(r, g, b);

		r = (float)reader.GetReal("HoverLabel", "Red", 1.0f);
		g = (float)reader.GetReal("HoverLabel", "Green", 1.0f);
		b = (float)reader.GetReal("HoverLabel", "Blue", 1.0f);
		m_hoverColour = Colour(r, g, b);

		r = (float)reader.GetReal("PressedLabel", "Red", 1.0f);
		g = (float)reader.GetReal("PressedLabel", "Green", 1.0f);
		b = (float)reader.GetReal("PressedLabel", "Blue", 1.0f);
		m_pressedColour = Colour(r, g, b);

		r = (float)reader.GetReal("DisabledLabel", "Red", 1.0f);
		g = (float)reader.GetReal("DisabledLabel", "Green", 1.0f);
		b = (float)reader.GetReal("DisabledLabel", "Blue", 1.0f);
		m_disabledColour = Colour(r, g, b);
	}
}

// Setup icons for components
void FrontendManager::SetCheckboxIcons(CheckBox* pCheckBox)
{
	pCheckBox->SetDefaultIcon(GetCheckboxIcon());
	pCheckBox->SetHoverIcon(GetCheckboxIconHover());
	pCheckBox->SetSelectedIcon(GetCheckboxIconPressed());
	pCheckBox->SetDisabledIcon(GetCheckboxIconDisabled());
	pCheckBox->SetToggledIcon(GetCheckboxIconToggled());
	pCheckBox->SetToggledHoverIcon(GetCheckboxIconToggledHover());
	pCheckBox->SetToggledSelectedIcon(GetCheckboxIconToggledPressed());
	pCheckBox->SetToggledDisabledIcon(GetCheckboxIconToggledDisabled());
}

void FrontendManager::SetOptionboxIcons(OptionBox* pOptionBox)
{
	pOptionBox->SetDefaultIcon(GetOptionboxIcon());
	pOptionBox->SetHoverIcon(GetOptionboxIconHover());
	pOptionBox->SetSelectedIcon(GetOptionboxIconPressed());
	pOptionBox->SetDisabledIcon(GetOptionboxIconDisabled());
	pOptionBox->SetToggledIcon(GetOptionboxIconToggled());
	pOptionBox->SetToggledHoverIcon(GetOptionboxIconToggledHover());
	pOptionBox->SetToggledSelectedIcon(GetOptionboxIconToggledPressed());
	pOptionBox->SetToggledDisabledIcon(GetOptionboxIconToggledDisabled());
}

void FrontendManager::SetScrollbarIcons(ScrollBar* pScrollbar)
{
	pScrollbar->SetRightArrowDefaultIcon(GetScrollbarArrowDownIcon());
	pScrollbar->SetRightArrowHoverIcon(GetScrollbarArrowDownHoverIcon());
	pScrollbar->SetRightArrowSelectedIcon(GetScrollbarArrowDownPressedIcon());
	pScrollbar->SetRightArrowDisabledIcon(GetScrollbarArrowDownDisabledIcon());
	pScrollbar->SetLeftArrowDefaultIcon(GetScrollbarArrowUpIcon());
	pScrollbar->SetLeftArrowHoverIcon(GetScrollbarArrowUpHoverIcon());
	pScrollbar->SetLeftArrowSelectedIcon(GetScrollbarArrowUpPressedIcon());
	pScrollbar->SetLeftArrowDisabledIcon(GetScrollbarArrowUpDisabledIcon());
	pScrollbar->SetBackbarIcon(GetScrollbarBackgroundIcon());
	pScrollbar->SetScrollbarDefaultIcon(GetScrollbarIcon());
	pScrollbar->SetScrollbarHoverIcon(GetScrollbarHoverIcon());
	pScrollbar->SetScrollbarSelectedIcon(GetScrollbarPressedIcon());
	pScrollbar->SetScrollbarDisabledIcon(GetScrollbarDisabledIcon());
}

void FrontendManager::SetSliderIcons(Slider* pSlider)
{
	pSlider->SetScrollerDefaultIcon(GetSliderIconDefault());
	pSlider->SetScrollerHoverIcon(GetSliderIconHover());
	pSlider->SetScrollerDraggingIcon(GetSliderIconDrag());
	pSlider->SetScrollBackbarIncrementIconDefault(GetSliderIconIncrementButtonDefault());
	pSlider->SetScrollBackbarIncrementIconHover(GetSliderIconIncrementButtonHover());
	pSlider->SetScrollBackbarIncrementIconPressed(GetSliderIconIncrementButtonPressed());
	pSlider->SetScrollBackbarIncrementIconDisabled(GetSliderIconIncrementButtonDisabled());
	pSlider->SetScrollBackbarDecrementIconDefault(GetSliderIconDecrementButtonDefault());
	pSlider->SetScrollBackbarDecrementIconHover(GetSliderIconDecrementButtonHover());
	pSlider->SetScrollBackbarDecrementIconPressed(GetSliderIconDecrementButtonPressed());
	pSlider->SetScrollBackbarDecrementIconDisabled(GetSliderIconDecrementButtonDisabled());
}

void FrontendManager::SetPulldownMenuIcons(PulldownMenu* pPulldownMenu)
{
	SetScrollbarIcons(pPulldownMenu->GetPulldownScrollBar());
	pPulldownMenu->SetPulldownIconDefault(GetScrollbarArrowDownIcon());
	pPulldownMenu->SetPulldownIconHover(GetScrollbarArrowDownHoverIcon());
	pPulldownMenu->SetPulldownIconPressed(GetScrollbarArrowDownPressedIcon());
	pPulldownMenu->SetPulldownIconDisabled(GetScrollbarArrowDownDisabledIcon());
}

void FrontendManager::SetButtonIcons(Button* pButton, ButtonSize size)
{
	pButton->SetDefaultIcon(GetButtonIcon(size));
	pButton->SetHoverIcon(GetButtonIconHover(size));
	pButton->SetSelectedIcon(GetButtonIconPressed(size));
	pButton->SetDisabledIcon(GetButtonIconDisabled(size));
}

void FrontendManager::SetTabIcons(OptionBox* pTab)
{
	pTab->SetDefaultIcon(GetTab75OptionIcon());
	pTab->SetHoverIcon(GetTab75OptionIcon_Hover());
	pTab->SetSelectedIcon(GetTab75OptionIcon_Pressed());
	pTab->SetDisabledIcon(GetTab75OptionIcon());  // HACK : Missing disabled graphic
	pTab->SetToggledIcon(GetTab75OptionIcon_Toggled());
	pTab->SetToggledHoverIcon(GetTab75OptionIcon_Toggled_Hover());
	pTab->SetToggledSelectedIcon(GetTab75OptionIcon_Toggled_Pressed());
	pTab->SetToggledDisabledIcon(GetTab75OptionIcon_Toggled());  // HACK : Missing disabled graphic
}

// Common, shared frontend page params
float FrontendManager::GetCameraOrbitTimer()
{
	return m_cameraOrbitTimer;
}

void FrontendManager::SetCameraOrbitTimer(float orbit)
{
	m_cameraOrbitTimer = orbit;
}

// Frontend functionality
void FrontendManager::SetOptionsReturnToMainMenu(bool mainMenu)
{
	for (unsigned int i = 0; i < m_vpFrontendPages.size(); i++)
	{
		if (m_vpFrontendPages[i]->GetPageType() == FrontendScreen_OptionsMenu)
		{
			((OptionsMenu*)m_vpFrontendPages[i])->SetReturnToMainMenu(mainMenu);
		}
	}
}

void FrontendManager::SetModsMenuReturnToMainMenu(bool mainMenu)
{
	for (unsigned int i = 0; i < m_vpFrontendPages.size(); i++)
	{
		if (m_vpFrontendPages[i]->GetPageType() == FrontendScreen_ModMenu)
		{
			((ModMenu*)m_vpFrontendPages[i])->SetReturnToMainMenu(mainMenu);
		}
	}
}

void FrontendManager::SetHoverNPC(NPC* pHoverNPC)
{
	if (m_currentScreen == FrontendScreen_SelectCharacter)
	{
		((SelectCharacter*)m_currentPage)->SetHoverNPC(pHoverNPC);
	}
	else if (m_currentScreen == FrontendScreen_CreateCharacter)
	{
		((CreateCharacter*)m_currentPage)->SetHoverNPC(pHoverNPC);
	}
}

void FrontendManager::SetSelectedNPC(NPC* pSelectedNPC)
{
	if (m_currentScreen == FrontendScreen_SelectCharacter)
	{
		((SelectCharacter*)m_currentPage)->SetSelectedNPC(pSelectedNPC);
	}
	else if (m_currentScreen == FrontendScreen_CreateCharacter)
	{
		((CreateCharacter*)m_currentPage)->SetSelectedNPC(pSelectedNPC);
	}
}

void FrontendManager::SetCharacterSubSelection(string subSelection)
{
	if (m_currentScreen == FrontendScreen_CreateCharacter)
	{
		((CreateCharacter*)m_currentPage)->DeletePresetButtons();

		PresetSection section = PresetSection_None;

		if (subSelection == "Head") { section = PresetSection_Head; }
		else if (subSelection == "Body") { section = PresetSection_Body; }
		else if (subSelection == "Legs") { section = PresetSection_Legs; }
		else if (subSelection == "Right_Shoulder") { section = PresetSection_RightShoulder; }
		else if (subSelection == "Left_Shoulder") { section = PresetSection_LeftShoulder; }
		else if (subSelection == "Right_Hand") { section = PresetSection_RightHand; }
		else if (subSelection == "Left_Hand") { section = PresetSection_LeftHand; }
		else if (subSelection == "Right_Foot") { section = PresetSection_RightFoot; }
		else if (subSelection == "Left_Foot") { section = PresetSection_LeftFoot; }

		if (section != PresetSection_None)
		{
			((CreateCharacter*)m_currentPage)->CreatePresetButtons(section, true);
		}
	}
}

void FrontendManager::GotoNextCreditScreen()
{
	if (m_currentScreen == FrontendScreen_Credits)
	{
		((Credits*)m_currentPage)->GotoNextCreditPage();
	}
}

void FrontendManager::DisableShadowOption()
{
	for (unsigned int i = 0; i < m_vpFrontendPages.size(); i++)
	{
		if (m_vpFrontendPages[i]->GetPageType() == FrontendScreen_OptionsMenu)
		{
			((OptionsMenu*)m_vpFrontendPages[i])->DisableShadowOption();
		}
	}
}

// Constants
float FrontendManager::GetToolTipAppearDelay()
{
	if (VoxGame::GetInstance()->GetVoxSettings()->m_tooltipDelay)
	{
		return m_tooltipAppearDelay;
	}
	else
	{
		return 0.0f;
	}
}

// Updating
void FrontendManager::Update(float dt)
{
	if (m_currentPage != nullptr)
	{
		m_currentPage->Update(dt);
	}

	if (m_currentScreen != FrontendScreen_None && m_currentScreen != FrontendScreen_PauseMenu && m_currentScreen != FrontendScreen_QuitPopup)
	{
		if (VoxGame::GetInstance()->GetGameMode() == GameMode_FrontEnd && VoxGame::GetInstance()->GetCameraMode() == CameraMode_Frontend)
		{
			UpdateFrontEndCamera(dt);
		}
	}
}

// Updating
void FrontendManager::UpdateFrontEndCamera(float dt)
{
	if (m_currentPage != nullptr)
	{
		if (m_pCamera != nullptr)
		{
			// Position
			m_pCamera->SetFakePosition(m_currentPage->GetCameraPosition());

			// Forward
			vec3 cameraLookAt = m_currentPage->GetCameraView();
			vec3 cameraForward = normalize(cameraLookAt - m_pCamera->GetFakePosition());
			m_pCamera->SetFacing(cameraForward);

			// Right
			vec3 cameraRight = normalize(cross(cameraForward, vec3(0.0f, 1.0f, 0.0f)));
			m_pCamera->SetRight(cameraRight);

			// Up
			vec3 cameraUp = normalize(cross(cameraRight, cameraForward));
			m_pCamera->SetUp(cameraUp);
		}
	}
}

// Rendering
void FrontendManager::Render()
{
	if (m_currentPage != nullptr)
	{
		m_currentPage->Render();
	}
}

void FrontendManager::Render2D()
{
	if (m_currentPage != nullptr)
	{
		m_currentPage->Render2D();
	}
}

void FrontendManager::RenderDebug()
{

}