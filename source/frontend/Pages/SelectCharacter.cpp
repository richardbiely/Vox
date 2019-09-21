// ******************************************************************************
// Filename:    SelectCharacter.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 12/10/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif //_WIN32

#include "SelectCharacter.h"
#include "../FrontendManager.h"
#include "../../gui/openglgui.h"
#include "../../VoxGame.h"
#include "../utils/FileUtils.h"


SelectCharacter::SelectCharacter(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontPageManager, int windowWidth, int windowHeight)
  : FrontendPage(pRenderer, pGUI, pFrontPageManager, FrontendScreen_SelectCharacter, windowWidth, windowHeight)
{
	m_pSelectedNPC = nullptr;

	m_toolTipVisible = false;

	// Tooltip
	m_pTooltipBackground = new Icon(m_pRenderer, "", 200, 220);
	m_pTooltipBackground->SetDepth(5.5f);

	char nameText[] = "[NAME]";
	m_pTooltipNameLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), nameText, Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pTooltipNameLabel->SetOutline(true);
	m_pTooltipNameLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pTooltipNameLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_30_Outline());
	m_pTooltipNameLabel->SetDepth(5.5f);

	char descText[] = "[DESCRIPTION]";
	m_pTooltipDescriptionLabel = new FormattedLabel(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), descText);
	m_pTooltipDescriptionLabel->SetOutline(true);
	m_pTooltipDescriptionLabel->SetColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pTooltipDescriptionLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pTooltipDescriptionLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_pTooltipDescriptionLabel->SetDepth(5.5f);
	m_pTooltipDescriptionLabel->SetWordWrap(true);

	// Popup
	char popupTitleText[] = "[POPUP TITLE]";
	m_popupTitle = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_40(), popupTitleText, Colour(1.0f, 0.0f, 0.0f, 1.0f));
	m_popupTitle->SetOutline(true);
	m_popupTitle->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_popupTitle->SetOutlineFont(m_pFrontendManager->GetFrontendFont_40_Outline());
	m_popupTitle->SetDepth(9.0f);

	char popupText[] = "[POPUP TEXT]";
	m_popupText = new FormattedLabel(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), popupText);
	m_popupText->SetOutline(true);
	m_popupText->SetColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_popupText->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_popupText->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_popupText->SetDepth(9.0f);
	m_popupText->SetWordWrap(true);
	m_popupText->SetHorizontalAlignment(eHorizontalAlignment_Center);

	m_pPopupBackgroundIcon = new Icon(m_pRenderer, "", 270, 200);
	m_pPopupBackgroundIcon->SetDepth(2.0f);

	m_pPopupConfirmButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "Yes", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pPopupConfirmButton->SetLabelOffset(0, 3);
	m_pPopupConfirmButton->SetPressedOffset(0, -4);
	m_pPopupConfirmButton->SetCallBackFunction(_PopupConfirmPressed);
	m_pPopupConfirmButton->SetCallBackData(this);
	m_pPopupConfirmButton->SetDepth(9.0f);

	m_pPopupCancelButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "No", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pPopupCancelButton->SetLabelOffset(0, 3);
	m_pPopupCancelButton->SetPressedOffset(0, -4);
	m_pPopupCancelButton->SetCallBackFunction(_PopupCancelPressed);
	m_pPopupCancelButton->SetCallBackData(this);
	m_pPopupCancelButton->SetDepth(9.1f);

	m_pCreateCharacterButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_40(), m_pFrontendManager->GetFrontendFont_40_Outline(), "Create Character", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pCreateCharacterButton->SetLabelOffset(0, 5);
	m_pCreateCharacterButton->SetPressedOffset(0, -4);
	m_pCreateCharacterButton->SetCallBackFunction(_CreateCharacterPressed);
	m_pCreateCharacterButton->SetCallBackData(this);

	m_pSelectCharacterButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_50(), m_pFrontendManager->GetFrontendFont_50_Outline(), "Select", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pSelectCharacterButton->SetLabelOffset(0, 5);
	m_pSelectCharacterButton->SetPressedOffset(0, -4);
	m_pSelectCharacterButton->SetCallBackFunction(_SelectCharacterPressed);
	m_pSelectCharacterButton->SetCallBackData(this);

	m_pDeleteCharacterButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_35(), m_pFrontendManager->GetFrontendFont_35_Outline(), "Delete", Colour(0.9f, 0.15f, 0.15f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pDeleteCharacterButton->SetLabelOffset(0, 3);
	m_pDeleteCharacterButton->SetPressedOffset(0, -4);
	m_pDeleteCharacterButton->SetCallBackFunction(_DeleteCharacterPressed);
	m_pDeleteCharacterButton->SetCallBackData(this);

	m_pBackButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "Back", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pBackButton->SetLabelOffset(0, 3);
	m_pBackButton->SetPressedOffset(0, -4);
	m_pBackButton->SetCallBackFunction(_BackPressed);
	m_pBackButton->SetCallBackData(this);

	SetWindowDimensions(m_windowWidth, m_windowHeight);
}

SelectCharacter::~SelectCharacter()
{
	delete m_pTooltipBackground;
	delete m_pTooltipNameLabel;
	delete m_pTooltipDescriptionLabel;

	delete m_popupTitle;
	delete m_popupText;
	delete m_pPopupConfirmButton;
	delete m_pPopupCancelButton;
	delete m_pPopupBackgroundIcon;

	delete m_pSelectCharacterButton;
	delete m_pDeleteCharacterButton;
	delete m_pCreateCharacterButton;
	delete m_pBackButton;
}

void SelectCharacter::Reset()
{
}

void SelectCharacter::SetWindowDimensions(int windowWidth, int windowHeight)
{
	FrontendPage::SetWindowDimensions(windowWidth, windowHeight);

	m_tooltipWidth = 200;
	m_tooltipHeight = 220;
	m_tooltipDescBorder = 15;

	m_popupWidth = 270;
	m_popupHeight = 200;
	m_popupBorderSpacer = 25;
	m_popupTitleSpacer = 35;
	m_popupIconSize = 50;
	m_popupIconSpacer = 10;

	m_pPopupBackgroundIcon->SetDimensions((int)((windowWidth*0.5f)-(m_popupWidth*0.5f)), (int)((windowHeight*0.5f)-(m_popupHeight*0.5f))+225, m_popupWidth, m_popupHeight);

	m_pCreateCharacterButton->SetDimensions(48, 40, 225, 75);
	m_pSelectCharacterButton->SetDimensions((int)((m_pFrontendManager->GetWindowWidth()*0.5f)-(225*0.5f)), 40, 225, 75);
	m_pDeleteCharacterButton->SetDimensions((m_pFrontendManager->GetWindowWidth() - 225 + 92 - 82), 100, 150, 65);
	m_pBackButton->SetDimensions((m_pFrontendManager->GetWindowWidth() - 225 + 112 - 82), 20, 110, 47);
}

void SelectCharacter::SkinGUI()
{
	string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();

	string iconName = "media/textures/gui/" + themeName + "/common/popup_background.tga";
	m_pPopupBackgroundIcon->SetIcon(iconName);

	iconName = "media/textures/gui/" + themeName + "/common/Tooltips/tooltip_background_common.tga";
	m_pTooltipBackground->SetIcon(iconName);

	m_pFrontendManager->SetButtonIcons(m_pPopupConfirmButton, ButtonSize_110x47);
	m_pFrontendManager->SetButtonIcons(m_pPopupCancelButton, ButtonSize_110x47);

	m_pFrontendManager->SetButtonIcons(m_pSelectCharacterButton, ButtonSize_225x75);
	m_pFrontendManager->SetButtonIcons(m_pCreateCharacterButton, ButtonSize_225x75);
	m_pFrontendManager->SetButtonIcons(m_pDeleteCharacterButton, ButtonSize_110x47);
	m_pFrontendManager->SetButtonIcons(m_pBackButton, ButtonSize_110x47);

	m_pDeleteCharacterButton->SetDimensions((m_pFrontendManager->GetWindowWidth() - 225 + 92 - 82), 100, 150, 65);

	m_pSelectCharacterButton->SetNormalLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pSelectCharacterButton->SetHoverLabelColour(m_pFrontendManager->GetHoverFontColour());
	m_pSelectCharacterButton->SetPressedLabelColour(m_pFrontendManager->GetPressedFontColour());
	m_pCreateCharacterButton->SetNormalLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pCreateCharacterButton->SetHoverLabelColour(m_pFrontendManager->GetHoverFontColour());
	m_pCreateCharacterButton->SetPressedLabelColour(m_pFrontendManager->GetPressedFontColour());
	m_pBackButton->SetNormalLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pBackButton->SetHoverLabelColour(m_pFrontendManager->GetHoverFontColour());
	m_pBackButton->SetPressedLabelColour(m_pFrontendManager->GetPressedFontColour());
	m_pPopupConfirmButton->SetNormalLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pPopupConfirmButton->SetHoverLabelColour(m_pFrontendManager->GetHoverFontColour());
	m_pPopupConfirmButton->SetPressedLabelColour(m_pFrontendManager->GetPressedFontColour());
	m_pPopupCancelButton->SetNormalLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pPopupCancelButton->SetHoverLabelColour(m_pFrontendManager->GetHoverFontColour());
	m_pPopupCancelButton->SetPressedLabelColour(m_pFrontendManager->GetPressedFontColour());
}

void SelectCharacter::UnSkinGUI()
{
}

void SelectCharacter::Load()
{
	m_pSelectCharacterButton->SetLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pDeleteCharacterButton->SetLabelColour(Colour(0.9f, 0.15f, 0.15f, 1.0f));
	m_pCreateCharacterButton->SetLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pBackButton->SetLabelColour(m_pFrontendManager->GetNormalFontColour());
	
	m_pPopupConfirmButton->SetLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pPopupCancelButton->SetLabelColour(m_pFrontendManager->GetNormalFontColour());

	LoadSelectionCharacters();

	m_pHoverSelectionNPC = nullptr;
	m_pSelectedNPC = nullptr;

	m_nameTextFadeInAlpha = 0.0f;
	m_nameTextFadeInFadeInTime = 2.5f;
	m_nameTextFadeInFadeInTimer = 0.0f;

	m_addedCreateButton = false;
	m_updateCreateButton = false;
	if(GetNumCreatedCharacters() < 6)
	{
		m_pGUI->AddComponent(m_pCreateCharacterButton);
		m_addedCreateButton = true;
	}
	m_pGUI->AddComponent(m_pBackButton);

	SetTitleAndSubtitle("SELECT CHARACTER", "Which adventurer do you want to play as?");
}

void SelectCharacter::Unload()
{
	HideTooltip();

	ClosePopup();

	m_pGUI->RemoveComponent(m_pPopupBackgroundIcon);

	m_pGUI->RemoveComponent(m_pSelectCharacterButton);
	m_pGUI->RemoveComponent(m_pDeleteCharacterButton);
	m_pGUI->RemoveComponent(m_pCreateCharacterButton);
	m_pGUI->RemoveComponent(m_pBackButton);

	for(unsigned int i = 0; i < m_vpCharacterLineUp.size(); i++)
	{
		m_vpCharacterLineUp[i]->UnloadWeapon(true);
		m_vpCharacterLineUp[i]->UnloadWeapon(false);
		VoxGame::GetInstance()->GetNPCManager()->DeleteNPC(m_vpCharacterLineUp[i]->GetName());
	}
	m_vpCharacterLineUp.clear();

	RemoveTitlesAndSubTitle();
}

void SelectCharacter::LoadSelectionCharacters()
{
	float xPosition = 0.0f;
	float zPosition = 0.0f;

	char importDirectory[128];
	sprintf(importDirectory, "saves/characters/*.*");

	vector<string> listFiles;
	listFiles = listFilesInDirectory(importDirectory);
	int characterNumCounter = 0;
	for (unsigned int i = 0; i < listFiles.size(); i++)
	{
		if(strcmp(listFiles[i].c_str(), ".") == 0 || strcmp(listFiles[i].c_str(), "..") == 0)
		{
			continue;
		}

		switch(characterNumCounter)
		{
			case 0: { xPosition = 21.0f; zPosition = 1.5f; } break;
			case 1: { xPosition = 22.5f; zPosition = 0.5f; } break;
			case 2: { xPosition = 24.0f; zPosition = -0.5f; } break;
			case 3: { xPosition = 26.0f; zPosition = -0.5f; } break;
			case 4: { xPosition = 27.5f; zPosition = 0.5f; } break;
			case 5: { xPosition = 29.0f; zPosition = 1.5f; } break;
		}

		vec3 centralLookPoint = vec3(25.0f, 1.0f, 5.0f);

		NPC* pCharacter1 = VoxGame::GetInstance()->GetNPCManager()->CreateNPC(listFiles[i].c_str(), "Human", listFiles[i].c_str(), vec3(xPosition, 0.5f, zPosition), 0.08f, true, false);
		pCharacter1->SetTargetForwardToLookAtPoint(centralLookPoint);
		pCharacter1->SetBodyTurnStopThreshold(0.01f);
		pCharacter1->SetBodyTurnSpeedMultiplier(6.0f);
		pCharacter1->SetPushingCollisionEnabled(false);
		pCharacter1->SetFrontEndNPC(true);

		PlayerStats playerStats(nullptr);
		playerStats.ImportStats(listFiles[i].c_str());
		pCharacter1->SetPlayerClass(playerStats.GetClass());

		char inventoryFile[128];
		sprintf(inventoryFile, "saves/characters/%s/%s.inv", listFiles[i].c_str(), listFiles[i].c_str());
		pCharacter1->ImportEquippedItems(inventoryFile);

		m_vpCharacterLineUp.push_back(pCharacter1);

		characterNumCounter++;
	}
}

int SelectCharacter::GetNumCreatedCharacters()
{
	char importDirectory[128];
	sprintf(importDirectory, "saves/characters/*.*");

	vector<string> listFiles;
	listFiles = listFilesInDirectory(importDirectory);
	int characterNumCounter = 0;
	for (unsigned int i = 0; i < listFiles.size(); i++)
	{
		if(strcmp(listFiles[i].c_str(), ".") == 0 || strcmp(listFiles[i].c_str(), "..") == 0)
		{
			continue;
		}

		characterNumCounter++;
	}

	return characterNumCounter;
}

void SelectCharacter::SetHoverNPC(NPC* pHoverNPC)
{
	if(pHoverNPC != m_pHoverSelectionNPC)
	{
		m_nameTextFadeInFadeInTimer = 0.0f;
	}

	m_pHoverSelectionNPC = pHoverNPC;
}

void SelectCharacter::SetSelectedNPC(NPC* pSelectedNPC)
{
	if(m_pSelectedNPC != nullptr && pSelectedNPC == nullptr)
	{
		// If we are resetting the selected character, make sure to set the current selected character back to normal
		m_pSelectedNPC->GetVoxelCharacter()->PlayFacialExpression("Normal");
		m_pSelectedNPC->GetVoxelCharacter()->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "BindPose", 0.25f);
	}

	ClosePopup();

	m_pSelectedNPC = pSelectedNPC;

	if(m_pSelectedNPC != nullptr)
	{
		vec2 screenPos = m_pSelectedNPC->GetScreenPosition();
		screenPos.y -= 250;
		screenPos.x -= m_tooltipWidth*0.5f;

		//ShowTooltip((int)screenPos.x, (int)screenPos.y);

		if(m_pGUI->GetComponent(m_pSelectCharacterButton) == nullptr)
		{
			m_pGUI->AddComponent(m_pSelectCharacterButton);
		}

		if(m_pGUI->GetComponent(m_pDeleteCharacterButton) == nullptr)
		{
			m_pGUI->AddComponent(m_pDeleteCharacterButton);
		}
	}
	else
	{
		HideTooltip();

		m_pGUI->RemoveComponent(m_pSelectCharacterButton);
		m_pGUI->RemoveComponent(m_pDeleteCharacterButton);
	}
}

void SelectCharacter::ShowTooltip(int x, int y)
{
	if(m_pSelectedNPC != nullptr)
	{
		m_pTooltipNameLabel->SetText(m_pSelectedNPC->GetName());

		string characterDescription = "[C=Custom(888888)]Level: [C=White]" + string("8") + "\n[C=Custom(888888)]Race: [C=Green]" + m_pSelectedNPC->GetType() + "\n[C=Custom(888888)]Class: [C=Red]" + m_pSelectedNPC->GetModelName();
		m_pTooltipDescriptionLabel->SetText(characterDescription);
	}

	m_pTooltipBackground->SetDimensions(x, y, m_tooltipWidth, m_tooltipHeight);

	int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_30(), "%s", m_pTooltipNameLabel->GetText().c_str());
	m_pTooltipNameLabel->SetLocation(x + (int)(m_tooltipWidth*0.5f)-(int)(textWidth*0.5f), y + m_tooltipHeight - 35);

	textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_25(), "%s", m_pTooltipDescriptionLabel->GetText().c_str());
	m_pTooltipDescriptionLabel->SetDimensions(x + m_tooltipDescBorder, y + m_tooltipDescBorder, m_tooltipWidth-(m_tooltipDescBorder*2), m_tooltipHeight-(m_tooltipDescBorder*2)-35);

	if(m_toolTipVisible == false)
	{
		m_pGUI->AddComponent(m_pTooltipBackground);
		m_pGUI->AddComponent(m_pTooltipNameLabel);
		m_pGUI->AddComponent(m_pTooltipDescriptionLabel);
	}

	m_toolTipVisible = true;
}

void SelectCharacter::HideTooltip()
{
	m_pGUI->RemoveComponent(m_pTooltipBackground);
	m_pGUI->RemoveComponent(m_pTooltipNameLabel);
	m_pGUI->RemoveComponent(m_pTooltipDescriptionLabel);

	m_toolTipVisible = false;
}

void SelectCharacter::OpenPopup(string popupTitle, string popupText)
{
	m_pPopupConfirmButton->SetLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pPopupCancelButton->SetLabelColour(m_pFrontendManager->GetNormalFontColour());

	int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_40(), "%s", popupTitle.c_str());
	m_popupTitle->SetLocation((int)((m_windowWidth*0.5f)-(textWidth*0.5f)), (int)((m_windowHeight*0.5f)+(m_popupHeight*0.5f))-m_popupTitleSpacer-m_popupBorderSpacer+225);
	m_popupText->SetDimensions((int)((m_windowWidth*0.5f)-(m_popupWidth*0.5f))+m_popupBorderSpacer, (int)((m_windowHeight*0.5f)-(m_popupHeight*0.5f))+225, m_popupWidth-(m_popupBorderSpacer*2), m_popupHeight-m_popupBorderSpacer-m_popupTitleSpacer);
	m_pPopupConfirmButton->SetDimensions((int)((m_windowWidth*0.5f)+(m_popupWidth*0.5f)) - (int)(m_popupBorderSpacer*0.5f) - 110, (int)((m_windowHeight*0.5f)-(m_popupIconSize*0.5f))-50+225, 110, 47);
	m_pPopupCancelButton->SetDimensions((int)((m_windowWidth*0.5f)-(m_popupWidth*0.5f)) + (int)(m_popupBorderSpacer*0.5f), (int)((m_windowHeight*0.5f)-(m_popupIconSize*0.5f))-50+225, 110, 47);

	m_popupTitle->SetText(popupTitle);
	m_popupText->SetText(popupText);

	m_pGUI->AddComponent(m_popupTitle);
	m_pGUI->AddComponent(m_popupText);
	m_pGUI->AddComponent(m_pPopupConfirmButton);
	m_pGUI->AddComponent(m_pPopupCancelButton);
	m_pGUI->AddComponent(m_pPopupBackgroundIcon);
}

void SelectCharacter::ClosePopup()
{
	m_pGUI->RemoveComponent(m_popupTitle);
	m_pGUI->RemoveComponent(m_popupText);
	m_pGUI->RemoveComponent(m_pPopupConfirmButton);
	m_pGUI->RemoveComponent(m_pPopupCancelButton);
	m_pGUI->RemoveComponent(m_pPopupBackgroundIcon);
}

void SelectCharacter::Update(float dt)
{
	FrontendPage::Update(dt);

	m_cameraPosition = vec3(25.0f, 2.75f, 8.0f);
	m_cameraView = vec3(25.0f, 2.0f, 0.0f);

	// Character fade in text alpha
	if(m_nameTextFadeInFadeInTimer < m_nameTextFadeInFadeInTime)
	{
		m_nameTextFadeInFadeInTimer += dt;
	}
	else
	{
		m_nameTextFadeInFadeInTimer = m_nameTextFadeInFadeInTime;
	}

	if(m_updateCreateButton)
	{
		if(GetNumCreatedCharacters() < 6)
		{
			if(m_addedCreateButton == false)
			{
				m_pGUI->AddComponent(m_pCreateCharacterButton);
				m_addedCreateButton = true;
			}
		}
		else
		{
			if(m_addedCreateButton == true)
			{
				m_pGUI->RemoveComponent(m_pCreateCharacterButton);
				m_addedCreateButton = false;
			}
		}

		m_updateCreateButton = false;
	}
}

void SelectCharacter::Render()
{
	FrontendPage::Render();
}

void SelectCharacter::Render2D()
{
	FrontendPage::Render2D();

	if(m_pHoverSelectionNPC != nullptr && m_pHoverSelectionNPC != m_pSelectedNPC)
	{
		vec2 screenPos = m_pHoverSelectionNPC->GetScreenPosition();
		char characterName[32];
		sprintf(characterName, "%s", m_pHoverSelectionNPC->GetName().c_str());

		int lTextWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_80(), "%s", characterName);
		float xText = screenPos.x - (int)(lTextWidth*0.5f);
		float yText = screenPos.y + 250.0f;

		m_nameTextFadeInAlpha = m_nameTextFadeInFadeInTimer / m_nameTextFadeInFadeInTime;

		m_pRenderer->PushMatrix();
			m_pRenderer->SetRenderMode(RM_SOLID);
			m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
			m_pRenderer->SetProjectionMode(PM_2D, VoxGame::GetInstance()->GetDefaultViewport());
			m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
			m_pRenderer->RenderFreeTypeText(m_pFrontendManager->GetFrontendFont_80(), xText, yText, 1.0f, Colour(1.0f, 1.0f, 1.0f, m_nameTextFadeInAlpha), 1.0f, characterName);
			m_pRenderer->RenderFreeTypeText(m_pFrontendManager->GetFrontendFont_80_Outline(), xText, yText, 1.0f, Colour(0.0f, 0.0f, 0.0f, m_nameTextFadeInAlpha), 1.0f, characterName);
			m_pRenderer->DisableTransparency();
		m_pRenderer->PopMatrix();
	}

	if(m_pSelectedNPC != nullptr)
	{
		vec2 screenPos = m_pSelectedNPC->GetScreenPosition();
		char characterName[32];
		sprintf(characterName, "%s", m_pSelectedNPC->GetModelName().c_str());

		int lTextWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_80(), "%s", characterName);
		float xText = screenPos.x - (int)(lTextWidth*0.5f);
		float yText = screenPos.y + 250.0f;

		m_pRenderer->PushMatrix();
			m_pRenderer->SetRenderMode(RM_SOLID);
			m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
			m_pRenderer->SetProjectionMode(PM_2D, VoxGame::GetInstance()->GetDefaultViewport());
			m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
			m_pRenderer->RenderFreeTypeText(m_pFrontendManager->GetFrontendFont_80(), xText, yText, 1.0f, Colour(1.0f, 1.0f, 0.0f, 1.0f), 1.0f, characterName);
			m_pRenderer->RenderFreeTypeText(m_pFrontendManager->GetFrontendFont_80_Outline(), xText, yText, 1.0f, Colour(0.0f, 0.0f, 0.0f, 1.0f), 1.0f, characterName);
			m_pRenderer->DisableTransparency();
		m_pRenderer->PopMatrix();
	}
}

void SelectCharacter::_SelectCharacterPressed(void *apData)
{
	SelectCharacter* lpSelectCharacter = (SelectCharacter*)apData;
	lpSelectCharacter->SelectCharacterPressed();
}

void SelectCharacter::SelectCharacterPressed()
{
	string characterName = m_pSelectedNPC->GetName();

	VoxGame::GetInstance()->GetPlayer()->ResetPlayer();
	VoxGame::GetInstance()->GetPlayer()->SetName(characterName.c_str());
	VoxGame::GetInstance()->GetPlayer()->SetClass(m_pSelectedNPC->GetPlayerClass());
	VoxGame::GetInstance()->GetPlayer()->LoadCharacter(characterName.c_str(), true);

	m_pSelectedNPC = nullptr;

	//m_pFrontendManager->SetFrontendScreen(FrontendScreen_SelectWorld);

	VoxGame::GetInstance()->StartGameFromFrontEnd();
}

void SelectCharacter::_DeleteCharacterPressed(void *apData)
{
	SelectCharacter* lpSelectCharacter = (SelectCharacter*)apData;
	lpSelectCharacter->DeleteCharacterPressed();
}

void SelectCharacter::DeleteCharacterPressed()
{
	char popupTitle[256];
	sprintf(popupTitle, "Delete");

	char popupText[256];
	sprintf(popupText, "Are you sure you want to delete [C=Custom(00A2E8)]%s[C=White]?", m_pSelectedNPC->GetName().c_str());

	OpenPopup(popupTitle, popupText);

	if(m_pSelectedNPC != nullptr)
	{
		m_pSelectedNPC->GetVoxelCharacter()->PlayFacialExpression("Cry");
		m_pSelectedNPC->GetVoxelCharacter()->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "Cry", 0.5f);
	}
}

void SelectCharacter::_CreateCharacterPressed(void *apData)
{
	SelectCharacter* lpSelectCharacter = (SelectCharacter*)apData;
	lpSelectCharacter->CreateCharacterPressed();
}

void SelectCharacter::CreateCharacterPressed()
{
	m_pFrontendManager->SetFrontendScreen(FrontendScreen_CreateCharacter);
}

void SelectCharacter::_BackPressed(void *apData)
{
	SelectCharacter* lpSelectCharacter = (SelectCharacter*)apData;
	lpSelectCharacter->BackPressed();
}

void SelectCharacter::BackPressed()
{
	m_pFrontendManager->SetFrontendScreen(FrontendScreen_MainMenu);
}

void SelectCharacter::_PopupConfirmPressed(void *apData)
{
	SelectCharacter* lpSelectCharacter = (SelectCharacter*)apData;
	lpSelectCharacter->PopupConfirmPressed();
}

void SelectCharacter::PopupConfirmPressed()
{
	string characterName = m_pSelectedNPC->GetName();

	char characterBaseFolder[128];
	char qbFilename[128];
	char facesFilename[128];
	char characterFilename[128];
	char inventoryFilename[128];
	char statsFilename[128];
	char actionbarFilename[128];

	sprintf(characterBaseFolder, "saves/characters/%s", characterName.c_str());
	sprintf(qbFilename, "saves/characters/%s/%s.qb", characterName.c_str(), characterName.c_str());
	sprintf(facesFilename, "saves/characters/%s/%s.faces", characterName.c_str(), characterName.c_str());
	sprintf(characterFilename, "saves/characters/%s/%s.character", characterName.c_str(), characterName.c_str());
	sprintf(inventoryFilename, "saves/characters/%s/%s.inv", characterName.c_str(), characterName.c_str());
	sprintf(statsFilename, "saves/characters/%s/%s.stats", characterName.c_str(), characterName.c_str());
	sprintf(actionbarFilename, "saves/characters/%s/%s.actionbar", characterName.c_str(), characterName.c_str());

	//if(dirExists(characterBaseFolder)) // TODO : dirExists
	{
		remove(qbFilename);
		remove(facesFilename);
		remove(characterFilename);
		remove(inventoryFilename);
		remove(statsFilename);
		remove(actionbarFilename);

		// Remove the directory
#ifdef _WIN32
		int error = _rmdir(characterBaseFolder);
#else
		int error = rmdir(characterBaseFolder);
#endif //_WIN32		
	}

	NPCList::iterator iter = std::find(m_vpCharacterLineUp.begin(), m_vpCharacterLineUp.end(), m_pSelectedNPC);
	if(iter != m_vpCharacterLineUp.end())
	{
		m_vpCharacterLineUp.erase(iter);
	}

	m_updateCreateButton = true;

	m_pSelectedNPC->Explode();
	VoxGame::GetInstance()->GetNPCManager()->DeleteNPC(m_pSelectedNPC->GetName());
	m_pSelectedNPC = nullptr;

	HideTooltip();
	m_pGUI->RemoveComponent(m_pSelectCharacterButton);
	m_pGUI->RemoveComponent(m_pDeleteCharacterButton);

	ClosePopup();
}

void SelectCharacter::_PopupCancelPressed(void *apData)
{
	SelectCharacter* lpSelectCharacter = (SelectCharacter*)apData;
	lpSelectCharacter->PopupCancelPressed();
}

void SelectCharacter::PopupCancelPressed()
{
	ClosePopup();

	if(m_pSelectedNPC != nullptr)
	{
		m_pSelectedNPC->GetVoxelCharacter()->PlayFacialExpression("Normal");
		m_pSelectedNPC->GetVoxelCharacter()->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "BindPose", 0.25f);
	}
}