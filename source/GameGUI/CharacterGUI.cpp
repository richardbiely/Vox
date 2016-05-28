// ******************************************************************************
// Filename:    CharacterGUI.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 11/06/12
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "CharacterGUI.h"
#include "InventoryGUI.h"
#include "LootGUI.h"
#include "ActionBar.h"

#include "../frontend/FrontendManager.h"
#include "../Player/Player.h"
#include "../VoxGame.h"

#include "../utils/FileUtils.h"
#include "../utils/Random.h"
#include "../utils/Interpolator.h"


CharacterGUI::CharacterGUI(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontendManager, ChunkManager* pChunkManager, Player* pPlayer, InventoryManager* pInventoryManager, int windowWidth, int windowHeight)
{
	m_pRenderer = pRenderer;
	m_pGUI = pGUI;
	m_pFrontendManager = pFrontendManager;
	m_pChunkManager = pChunkManager;
	m_pPlayer = pPlayer;
	m_pInventoryManager = pInventoryManager;

	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;

	// Character Window
	m_pCharacterWindow = new GUIWindow(m_pRenderer, m_pFrontendManager->GetFrontendFont_Medium(), "Character");
	m_pCharacterWindow->AllowMoving(true);
	m_pCharacterWindow->AllowClosing(false);
	m_pCharacterWindow->AllowMinimizing(false);
	m_pCharacterWindow->AllowScrolling(false);
	m_pCharacterWindow->SetRenderTitleBar(true);
	m_pCharacterWindow->SetRenderWindowBackground(true);
	m_pCharacterWindow->SetApplicationDimensions(m_windowWidth, m_windowHeight);
	m_pCharacterWindow->Hide();

	m_pDynamicPaperDollIcon = new Icon(m_pRenderer);
	m_pDynamicPaperDollIcon->SetDepth(2.5f);

	m_pTitleBarIcon = new Icon(m_pRenderer, "", 44, 44);
	m_pTitleBarIcon->SetDepth(4.0f);

	m_pCharacterWindowBackgroundIcon = new Icon(m_pRenderer, "", 304, 281);
	m_pCharacterWindowBackgroundIcon->SetDepth(1.0f);

	m_pTitleBarBackgroundIcon = new Icon(m_pRenderer, "", 133, 35);
	m_pTitleBarBackgroundIcon->SetDepth(1.0f);

	m_pCloseExitButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pCloseExitButton->SetLabelOffset(0, 5);
	m_pCloseExitButton->SetCallBackFunction(_CloseExitPressed);
	m_pCloseExitButton->SetCallBackData(this);

	m_pTabOpenCloseButton_Icon_Left = new Icon(m_pRenderer, "", 12, 80);
	m_pTabOpenCloseButton_Icon_Left->SetDepth(0.5f);

	m_pTabOpenCloseButton_Icon_Left_Hover = new Icon(m_pRenderer, "", 12, 80);
	m_pTabOpenCloseButton_Icon_Left_Hover->SetDepth(0.5f);

	m_pTabOpenCloseButton_Icon_Left_Pressed = new Icon(m_pRenderer, "", 12, 80);
	m_pTabOpenCloseButton_Icon_Left_Pressed->SetDepth(0.5f);

	m_pTabOpenCloseButton_Icon_Right = new Icon(m_pRenderer, "", 12, 80);
	m_pTabOpenCloseButton_Icon_Right->SetDepth(0.5f);

	m_pTabOpenCloseButton_Icon_Right_Hover = new Icon(m_pRenderer, "", 12, 80);
	m_pTabOpenCloseButton_Icon_Right_Hover->SetDepth(0.5f);

	m_pTabOpenCloseButton_Icon_Right_Pressed = new Icon(m_pRenderer, "", 12, 80);
	m_pTabOpenCloseButton_Icon_Right_Pressed->SetDepth(0.5f);

	m_pStatsTab_Icon_Left = new Icon(m_pRenderer, "", 164, 198);
	m_pStatsTab_Icon_Left->SetDepth(1.0f);

	m_pStatsTab_Icon_Right = new Icon(m_pRenderer, "", 164, 198);
	m_pStatsTab_Icon_Right->SetDepth(0.5f);

	m_pCharacterTitleLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_Medium(), "[NAME]", Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pCharacterTitleLabel->SetDepth(2.0f);

	m_pStatsHeaderLabel1 = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), "Strength", Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStatsHeaderLabel1->SetOutline(true);
	m_pStatsHeaderLabel1->SetOutlineColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pStatsHeaderLabel1->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_pStatsHeaderLabel1->SetDepth(2.0f);
	m_pStatsHeaderLabel2 = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), "Dexterity", Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStatsHeaderLabel2->SetOutline(true);
	m_pStatsHeaderLabel2->SetOutlineColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pStatsHeaderLabel2->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_pStatsHeaderLabel2->SetDepth(2.0f);
	m_pStatsHeaderLabel3 = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), "Intelligence", Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStatsHeaderLabel3->SetOutline(true);
	m_pStatsHeaderLabel3->SetOutlineColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pStatsHeaderLabel3->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_pStatsHeaderLabel3->SetDepth(2.0f);
	m_pStatsHeaderLabel4 = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), "Vitality", Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStatsHeaderLabel4->SetOutline(true);
	m_pStatsHeaderLabel4->SetOutlineColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pStatsHeaderLabel4->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_pStatsHeaderLabel4->SetDepth(2.0f);
	m_pStatsHeaderLabel5 = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), "Armor", Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStatsHeaderLabel5->SetOutline(true);
	m_pStatsHeaderLabel5->SetOutlineColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pStatsHeaderLabel5->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_pStatsHeaderLabel5->SetDepth(2.0f);
	m_pStatsHeaderLabel6 = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), "Luck", Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStatsHeaderLabel6->SetOutline(true);
	m_pStatsHeaderLabel6->SetOutlineColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pStatsHeaderLabel6->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_pStatsHeaderLabel6->SetDepth(2.0f);

	m_pStatsValueLabel1 = new FormattedLabel(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), "1");
	m_pStatsValueLabel1->SetOutline(true);
	m_pStatsValueLabel1->SetColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pStatsValueLabel1->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStatsValueLabel1->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_pStatsValueLabel1->SetDepth(5.5f);
	m_pStatsValueLabel1->SetWordWrap(true);
	m_pStatsValueLabel1->SetHorizontalAlignment(eHorizontalAlignment_Center);
	m_pStatsValueLabel2 = new FormattedLabel(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), "2");
	m_pStatsValueLabel2->SetOutline(true);
	m_pStatsValueLabel2->SetColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pStatsValueLabel2->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStatsValueLabel2->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_pStatsValueLabel2->SetDepth(5.5f);
	m_pStatsValueLabel2->SetWordWrap(true);
	m_pStatsValueLabel2->SetHorizontalAlignment(eHorizontalAlignment_Center);
	m_pStatsValueLabel3 = new FormattedLabel(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), "3");
	m_pStatsValueLabel3->SetOutline(true);
	m_pStatsValueLabel3->SetColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pStatsValueLabel3->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStatsValueLabel3->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_pStatsValueLabel3->SetDepth(5.5f);
	m_pStatsValueLabel3->SetWordWrap(true);
	m_pStatsValueLabel3->SetHorizontalAlignment(eHorizontalAlignment_Center);
	m_pStatsValueLabel4 = new FormattedLabel(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), "4");
	m_pStatsValueLabel4->SetOutline(true);
	m_pStatsValueLabel4->SetColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pStatsValueLabel4->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStatsValueLabel4->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_pStatsValueLabel4->SetDepth(5.5f);
	m_pStatsValueLabel4->SetWordWrap(true);
	m_pStatsValueLabel4->SetHorizontalAlignment(eHorizontalAlignment_Center);
	m_pStatsValueLabel5 = new FormattedLabel(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), "5");
	m_pStatsValueLabel5->SetOutline(true);
	m_pStatsValueLabel5->SetColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pStatsValueLabel5->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStatsValueLabel5->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_pStatsValueLabel5->SetDepth(5.5f);
	m_pStatsValueLabel5->SetWordWrap(true);
	m_pStatsValueLabel5->SetHorizontalAlignment(eHorizontalAlignment_Center);
	m_pStatsValueLabel6 = new FormattedLabel(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), "6");
	m_pStatsValueLabel6->SetOutline(true);
	m_pStatsValueLabel6->SetColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pStatsValueLabel6->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStatsValueLabel6->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_pStatsValueLabel6->SetDepth(5.5f);
	m_pStatsValueLabel6->SetWordWrap(true);
	m_pStatsValueLabel6->SetHorizontalAlignment(eHorizontalAlignment_Center);

	m_pStat1IncreaseButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStat1IncreaseButton->SetCallBackFunction(_Stat1IncreasePressed);
	m_pStat1IncreaseButton->SetCallBackData(this);
	//m_pStat1IncreaseButton->SetDefaultIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat1IncreaseButton->SetHoverIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat1IncreaseButton->SetSelectedIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat1IncreaseButton->SetDisabledIcon(m_pStatsIncreaseButtonIcon);
	m_pStat1IncreaseButton->SetDepth(1.1f);
	m_pStat2IncreaseButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStat2IncreaseButton->SetCallBackFunction(_Stat2IncreasePressed);
	m_pStat2IncreaseButton->SetCallBackData(this);
	//m_pStat2IncreaseButton->SetDefaultIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat2IncreaseButton->SetHoverIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat2IncreaseButton->SetSelectedIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat2IncreaseButton->SetDisabledIcon(m_pStatsIncreaseButtonIcon);
	m_pStat2IncreaseButton->SetDepth(1.2f);
	m_pStat3IncreaseButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStat3IncreaseButton->SetCallBackFunction(_Stat3IncreasePressed);
	m_pStat3IncreaseButton->SetCallBackData(this);
	//m_pStat3IncreaseButton->SetDefaultIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat3IncreaseButton->SetHoverIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat3IncreaseButton->SetSelectedIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat3IncreaseButton->SetDisabledIcon(m_pStatsIncreaseButtonIcon);
	m_pStat3IncreaseButton->SetDepth(1.3f);
	m_pStat4IncreaseButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStat4IncreaseButton->SetCallBackFunction(_Stat4IncreasePressed);
	m_pStat4IncreaseButton->SetCallBackData(this);
	//m_pStat4IncreaseButton->SetDefaultIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat4IncreaseButton->SetHoverIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat4IncreaseButton->SetSelectedIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat4IncreaseButton->SetDisabledIcon(m_pStatsIncreaseButtonIcon);
	m_pStat4IncreaseButton->SetDepth(1.4f);
	m_pStat5IncreaseButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStat5IncreaseButton->SetCallBackFunction(_Stat5IncreasePressed);
	m_pStat5IncreaseButton->SetCallBackData(this);
	//m_pStat5IncreaseButton->SetDefaultIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat5IncreaseButton->SetHoverIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat5IncreaseButton->SetSelectedIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat5IncreaseButton->SetDisabledIcon(m_pStatsIncreaseButtonIcon);
	m_pStat5IncreaseButton->SetDepth(1.5f);
	m_pStat6IncreaseButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pStat6IncreaseButton->SetCallBackFunction(_Stat6IncreasePressed);
	m_pStat6IncreaseButton->SetCallBackData(this);
	//m_pStat6IncreaseButton->SetDefaultIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat6IncreaseButton->SetHoverIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat6IncreaseButton->SetSelectedIcon(m_pStatsIncreaseButtonIcon);
	//m_pStat6IncreaseButton->SetDisabledIcon(m_pStatsIncreaseButtonIcon);
	m_pStat6IncreaseButton->SetDepth(1.6f);

	m_pTabOpenCloseButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pTabOpenCloseButton->SetDefaultIcon(m_pTabOpenCloseButton_Icon_Left);
	m_pTabOpenCloseButton->SetHoverIcon(m_pTabOpenCloseButton_Icon_Left_Hover);
	m_pTabOpenCloseButton->SetSelectedIcon(m_pTabOpenCloseButton_Icon_Left_Pressed);
	m_pTabOpenCloseButton->SetDisabledIcon(m_pTabOpenCloseButton_Icon_Left);
	m_pTabOpenCloseButton->SetLabelOffset(0, 5);
	m_pTabOpenCloseButton->SetCallBackFunction(_OpenCloseTabPressed);
	m_pTabOpenCloseButton->SetCallBackData(this);

	m_pArrowLeftButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pArrowLeftButton->SetLabelOffset(0, 5);
	m_pArrowLeftButton->SetCallBackFunction_Pressed(_ArrowLeftPressed);
	m_pArrowLeftButton->SetCallBackData_Pressed(this);
	m_pArrowLeftButton->SetCallBackFunction_Released(_ArrowLeftReleased);
	m_pArrowLeftButton->SetCallBackData_Released(this);
	m_pArrowLeftButton->SetDepth(3.0f);

	m_pArrowRightButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pArrowRightButton->SetLabelOffset(0, 5);
	m_pArrowRightButton->SetCallBackFunction_Pressed(_ArrowRightPressed);
	m_pArrowRightButton->SetCallBackData_Pressed(this);
	m_pArrowRightButton->SetCallBackFunction_Released(_ArrowRightReleased);
	m_pArrowRightButton->SetCallBackData_Released(this);
	m_pArrowRightButton->SetDepth(3.0f);

	m_pEquipHoverIcon = new Icon(m_pRenderer, "", 80, 80);
	m_pEquipHoverIcon->SetDepth(2.0f);

	m_pCharacterWindow->SetBackgroundIcon(m_pCharacterWindowBackgroundIcon);
	m_pCharacterWindow->SetTitlebarBackgroundIcon(m_pTitleBarBackgroundIcon);

	m_pCharacterWindow->AddComponent(m_pDynamicPaperDollIcon);
	m_pCharacterWindow->AddComponent(m_pCloseExitButton);
	m_pCharacterWindow->AddComponent(m_pTitleBarIcon);
	m_pCharacterWindow->AddComponent(m_pArrowLeftButton);
	m_pCharacterWindow->AddComponent(m_pArrowRightButton);
	m_pCharacterWindow->AddComponent(m_pStatsTab_Icon_Left);
	m_pCharacterWindow->AddComponent(m_pStatsTab_Icon_Right);
	m_pCharacterWindow->AddComponent(m_pTabOpenCloseButton);
	m_pCharacterWindow->AddComponent(m_pCharacterTitleLabel);
	m_pCharacterWindow->AddComponent(m_pStatsHeaderLabel1);
	m_pCharacterWindow->AddComponent(m_pStatsHeaderLabel2);
	m_pCharacterWindow->AddComponent(m_pStatsHeaderLabel3);
	m_pCharacterWindow->AddComponent(m_pStatsHeaderLabel4);
	m_pCharacterWindow->AddComponent(m_pStatsHeaderLabel5);
	m_pCharacterWindow->AddComponent(m_pStatsHeaderLabel6);
	m_pCharacterWindow->AddComponent(m_pStatsValueLabel1);
	m_pCharacterWindow->AddComponent(m_pStatsValueLabel2);
	m_pCharacterWindow->AddComponent(m_pStatsValueLabel3);
	m_pCharacterWindow->AddComponent(m_pStatsValueLabel4);
	m_pCharacterWindow->AddComponent(m_pStatsValueLabel5);
	m_pCharacterWindow->AddComponent(m_pStatsValueLabel6);

	m_pInventoryBackgroundSlotBorder_Common = new Icon(m_pRenderer, "", 64, 64);
	m_pInventoryBackgroundSlotBorder_Common->SetDepth(2.0f);

	m_pInventoryBackgroundSlotBorder_Uncommon = new Icon(m_pRenderer, "", 64, 64);
	m_pInventoryBackgroundSlotBorder_Uncommon->SetDepth(2.0f);

	m_pInventoryBackgroundSlotBorder_Magical = new Icon(m_pRenderer, "", 64, 64);
	m_pInventoryBackgroundSlotBorder_Magical->SetDepth(2.0f);

	m_pInventoryBackgroundSlotBorder_Rare = new Icon(m_pRenderer, "", 64, 64);
	m_pInventoryBackgroundSlotBorder_Rare->SetDepth(2.0f);

	m_pInventoryBackgroundSlotBorder_Epic = new Icon(m_pRenderer, "", 64, 64);
	m_pInventoryBackgroundSlotBorder_Epic->SetDepth(2.0f);

	// Tooltip
	m_pTooltipBackground_Common = new Icon(m_pRenderer, "", 200, 220);
	m_pTooltipBackground_Common->SetDepth(5.5f);

	m_pTooltipBackground_Uncommon = new Icon(m_pRenderer, "", 200, 220);
	m_pTooltipBackground_Uncommon->SetDepth(5.5f);

	m_pTooltipBackground_Magical = new Icon(m_pRenderer, "", 200, 220);
	m_pTooltipBackground_Magical->SetDepth(5.5f);

	m_pTooltipBackground_Rare = new Icon(m_pRenderer, "", 200, 220);
	m_pTooltipBackground_Rare->SetDepth(5.5f);

	m_pTooltipBackground_Epic = new Icon(m_pRenderer, "", 200, 220);
	m_pTooltipBackground_Epic->SetDepth(5.5f);

	char nameText[] = "[ITEM]";
	m_pTooltipNameLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), nameText, Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pTooltipNameLabel->SetOutline(true);
	m_pTooltipNameLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pTooltipNameLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_30_Outline());
	m_pTooltipNameLabel->SetDepth(5.5f);

	char descText[] = "[REPLACE ME]";
	m_pTooltipDescriptionLabel = new FormattedLabel(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), descText);
	m_pTooltipDescriptionLabel->SetOutline(true);
	m_pTooltipDescriptionLabel->SetColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pTooltipDescriptionLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pTooltipDescriptionLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_pTooltipDescriptionLabel->SetDepth(5.5f);
	m_pTooltipDescriptionLabel->SetWordWrap(true);

	char slotText[] = "[SLOT]";
	m_pTooltipSlotLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_20(), slotText, Colour(0.5f, 0.5f, 0.5f, 1.0f));
	m_pTooltipSlotLabel->SetOutline(true);
	m_pTooltipSlotLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pTooltipSlotLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_20_Outline());
	m_pTooltipSlotLabel->SetDepth(5.5f);

	char qualityText[] = "[QUALITY]";
	m_pTooltipQualityLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_20(), qualityText, Colour(0.5f, 0.5f, 0.5f, 1.0f));
	m_pTooltipQualityLabel->SetOutline(true);
	m_pTooltipQualityLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pTooltipQualityLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_20_Outline());
	m_pTooltipQualityLabel->SetDepth(5.5f);

	SetWindowDimensions(m_windowWidth, m_windowHeight);

	for(int i = 0; i < (int)EquipSlot_NumSlots; i++)
	{
		m_pEquippedItems[i] = NULL;
	}

	m_pressedX = 0;
	m_pressedY = 0;
	m_pPressedCharacterItem = NULL;

	m_leftArrowPressed = false;
	m_rightArrowPressed = false;

	pBlockSlotItem = NULL;

	m_statsTabsOpen = false;
	m_statsTabLeftSide = true;

	m_addedStatIncreaseButtons = false;

	// Load delay
	m_loadDelay = false;
	m_loadDelayTime = 0.0f;

	m_loaded = false;
}

CharacterGUI::~CharacterGUI()
{
	delete m_pCharacterWindow;
	delete m_pDynamicPaperDollIcon;
	delete m_pTitleBarIcon;
	delete m_pTitleBarBackgroundIcon;
	delete m_pCharacterWindowBackgroundIcon;
	delete m_pCloseExitButton;
	delete m_pArrowLeftButton;
	delete m_pArrowRightButton;
	delete m_pTabOpenCloseButton_Icon_Right;
	delete m_pTabOpenCloseButton_Icon_Right_Hover;
	delete m_pTabOpenCloseButton_Icon_Right_Pressed;
	delete m_pTabOpenCloseButton_Icon_Left;
	delete m_pTabOpenCloseButton_Icon_Left_Hover;
	delete m_pTabOpenCloseButton_Icon_Left_Pressed;
	delete m_pTabOpenCloseButton;
	delete m_pStatsTab_Icon_Left;
	delete m_pStatsTab_Icon_Right;
	delete m_pCharacterTitleLabel;
	delete m_pStatsHeaderLabel1;
	delete m_pStatsHeaderLabel2;
	delete m_pStatsHeaderLabel3;
	delete m_pStatsHeaderLabel4;
	delete m_pStatsHeaderLabel5;
	delete m_pStatsHeaderLabel6;
	delete m_pStatsValueLabel1;
	delete m_pStatsValueLabel2;
	delete m_pStatsValueLabel3;
	delete m_pStatsValueLabel4;
	delete m_pStatsValueLabel5;
	delete m_pStatsValueLabel6;
	delete m_pStat1IncreaseButton;
	delete m_pStat2IncreaseButton;
	delete m_pStat3IncreaseButton;
	delete m_pStat4IncreaseButton;
	delete m_pStat5IncreaseButton;
	delete m_pStat6IncreaseButton;

	delete m_pEquipHoverIcon;

	delete m_pInventoryBackgroundSlotBorder_Common;
	delete m_pInventoryBackgroundSlotBorder_Uncommon;
	delete m_pInventoryBackgroundSlotBorder_Magical;
	delete m_pInventoryBackgroundSlotBorder_Rare;
	delete m_pInventoryBackgroundSlotBorder_Epic;

	delete m_pTooltipBackground_Common;
	delete m_pTooltipBackground_Uncommon;
	delete m_pTooltipBackground_Magical;
	delete m_pTooltipBackground_Rare;
	delete m_pTooltipBackground_Epic;
	delete m_pTooltipNameLabel;
	delete m_pTooltipDescriptionLabel;
	delete m_pTooltipSlotLabel;
	delete m_pTooltipQualityLabel;
}

void CharacterGUI::SetInventoryGUI(InventoryGUI* pInventoryGUI)
{
	m_pInventoryGUI = pInventoryGUI;
}

void CharacterGUI::SetLootGUI(LootGUI* pLootGUI)
{
	m_pLootGUI = pLootGUI;
}

void CharacterGUI::SetActionBar(ActionBar* pActionBar)
{
	m_pActionBar = pActionBar;
}

void CharacterGUI::SetWindowDimensions(int windowWidth, int windowHeight)
{
	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;

	m_characterWindowWidth = 304;
	m_characterWindowHeight = 281;
	m_titlebarWidth = 153;
	m_titlebarHeight = 35;
	m_statsTabOpenButtonWidth = 12;
	m_statsTabOpenButtonHeight = 80;
	m_statsTabWidth = 164;
	m_statsTabHeight = 198;
	m_paperDoll_X = 72;
	m_paperDoll_Y = 86;
	m_paperDollWidth = 160;
	m_paperDollHeight = 175;
	m_statsTitleLabel_Y = 208;
	m_statsHeaderLabel_XOffset = 7;
	m_statsValueLabel_XOffset = 105;

	m_pCharacterWindow->SetDimensions(windowWidth - 370, 450, m_characterWindowWidth, m_characterWindowHeight);
	m_pCharacterWindow->SetTitleBarDimensions(0, 0, m_titlebarWidth, m_titlebarHeight);
	m_pCharacterWindow->SetTitleOffset(50, 5);
	m_pCharacterWindow->SetApplicationDimensions(m_windowWidth, m_windowHeight);
	m_pCharacterWindow->SetApplicationBorder(25, 15, 10, 10);

	m_pDynamicPaperDollIcon->SetDimensions(m_paperDoll_X, m_paperDoll_Y, m_paperDollWidth, m_paperDollHeight);

	m_pTitleBarIcon->SetDimensions(0, m_characterWindowHeight, 44, 44);

	m_pCloseExitButton->SetDimensions(m_characterWindowWidth-32, m_characterWindowHeight, 32, 32);

	m_pArrowLeftButton->SetDimensions(84, 87, 32, 32);
	m_pArrowRightButton->SetDimensions(189, 87, 32, 32);
}

// Skinning the GUI
void CharacterGUI::SkinGUI()
{
	string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();

	string iconName = "media/textures/gui/" + themeName + "/common/items/border_common.tga";
	m_pInventoryBackgroundSlotBorder_Common->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/items/border_uncommon.tga";
	m_pInventoryBackgroundSlotBorder_Uncommon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/items/border_magical.tga";
	m_pInventoryBackgroundSlotBorder_Magical->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/items/border_rare.tga";
	m_pInventoryBackgroundSlotBorder_Rare->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/items/border_epic.tga";
	m_pInventoryBackgroundSlotBorder_Epic->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/Tooltips/tooltip_background_common.tga";
	m_pTooltipBackground_Common->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/Tooltips/tooltip_background_uncommon.tga";
	m_pTooltipBackground_Uncommon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/Tooltips/tooltip_background_magical.tga";
	m_pTooltipBackground_Magical->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/Tooltips/tooltip_background_rare.tga";
	m_pTooltipBackground_Rare->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/Tooltips/tooltip_background_epic.tga";
	m_pTooltipBackground_Epic->SetIcon(iconName);

	iconName = "media/textures/gui/" + themeName + "/CharacterGUI/tab_openclose_left.tga";
	m_pTabOpenCloseButton_Icon_Left->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CharacterGUI/tab_openclose_left_hover.tga";
	m_pTabOpenCloseButton_Icon_Left_Hover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CharacterGUI/tab_openclose_left_pressed.tga";
	m_pTabOpenCloseButton_Icon_Left_Pressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CharacterGUI/tab_openclose_right.tga";
	m_pTabOpenCloseButton_Icon_Right->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CharacterGUI/tab_openclose_right_hover.tga";
	m_pTabOpenCloseButton_Icon_Right_Hover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CharacterGUI/tab_openclose_right_pressed.tga";
	m_pTabOpenCloseButton_Icon_Right_Pressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CharacterGUI/stats_tab_left.tga";
	m_pStatsTab_Icon_Left->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CharacterGUI/stats_tab_right.tga";
	m_pStatsTab_Icon_Right->SetIcon(iconName);

	iconName = "media/textures/gui/" + themeName + "/CharacterGUI/equip_hover_highlight.tga";
	m_pEquipHoverIcon->SetIcon(iconName);

	iconName = "media/textures/gui/" + themeName + "/CharacterGUI/character_title_icon.tga";
	m_pTitleBarIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CharacterGUI/character_window_background.tga";
	m_pCharacterWindowBackgroundIcon->SetIcon(iconName);
	m_pCharacterWindowBackgroundIcon->SetDepth(2.0f);
	iconName = "media/textures/gui/" + themeName + "/CharacterGUI/titlebar_background.tga";
	m_pTitleBarBackgroundIcon->SetIcon(iconName);

	m_pCharacterWindow->SetBackgroundIcon(m_pCharacterWindowBackgroundIcon);
	m_pCharacterWindow->SetTitlebarBackgroundIcon(m_pTitleBarBackgroundIcon);

	Point location = m_pCharacterWindow->GetLocation();
	m_pCharacterWindow->SetDimensions(location.m_x, location.m_y, m_characterWindowWidth, m_characterWindowHeight);
	m_pCharacterWindow->SetTitleBarDimensions(0, 0, m_titlebarWidth, m_titlebarHeight);

	m_pCloseExitButton->SetDefaultIcon(m_pFrontendManager->GetCloseExitButtonIcon());
	m_pCloseExitButton->SetHoverIcon(m_pFrontendManager->GetCloseExitButtonIcon_Hover());
	m_pCloseExitButton->SetSelectedIcon(m_pFrontendManager->GetCloseExitButtonIcon_Pressed());
	m_pCloseExitButton->SetDisabledIcon(m_pFrontendManager->GetCloseExitButtonIcon());

	m_pArrowLeftButton->SetDefaultIcon(m_pFrontendManager->GetLeftRotateArrow());
	m_pArrowLeftButton->SetHoverIcon(m_pFrontendManager->GetLeftRotateArrow_Hover());
	m_pArrowLeftButton->SetSelectedIcon(m_pFrontendManager->GetLeftRotateArrow_Pressed());
	m_pArrowLeftButton->SetDisabledIcon(m_pFrontendManager->GetLeftRotateArrow());
	m_pArrowRightButton->SetDefaultIcon(m_pFrontendManager->GetRightRotateArrow());
	m_pArrowRightButton->SetHoverIcon(m_pFrontendManager->GetRightRotateArrow_Hover());
	m_pArrowRightButton->SetSelectedIcon(m_pFrontendManager->GetRightRotateArrow_Pressed());
	m_pArrowRightButton->SetDisabledIcon(m_pFrontendManager->GetRightRotateArrow());
}

void CharacterGUI::UnSkinGUI()
{

}

// Loading
void CharacterGUI::Load(bool loadDelay, float loadDelayTime)
{
	m_loadDelay = loadDelay;
	m_loadDelayTime = loadDelayTime;

	CreateInventoryItems();

	for(unsigned int i = 0; i < m_vpInventorySlotItems.size(); i++)
	{
		m_pCharacterWindow->AddComponent(m_vpInventorySlotItems[i]->m_pInventoryIcon);
	}

	if(m_pPlayer->GetPlayerStats()->GetNumPointsAvailable() > 0)
	{
		m_pCharacterWindow->AddComponent(m_pStat1IncreaseButton);
		m_pCharacterWindow->AddComponent(m_pStat2IncreaseButton);
		m_pCharacterWindow->AddComponent(m_pStat3IncreaseButton);
		m_pCharacterWindow->AddComponent(m_pStat4IncreaseButton);
		m_pCharacterWindow->AddComponent(m_pStat5IncreaseButton);
		m_pCharacterWindow->AddComponent(m_pStat6IncreaseButton);

		m_addedStatIncreaseButtons = true;
	}

	m_pCharacterWindow->DepthSortComponentChildren();
	if (m_loadDelay == false)
	{
		m_pGUI->AddWindow(m_pCharacterWindow);
		m_pCharacterWindow->Show();
	}

	UpdatePlayerStats();

	StartEquipHoverAnimation();

	m_statsTabsOpen = false;
	Update(0.01f);

	m_pressedX = 0;
	m_pressedY = 0;
	m_pPressedCharacterItem = NULL;

	m_toolTipVisible = false;
	m_tooltipAppearDelayTimer = 0.0f;
	m_toolTipComponentsAdded = false;
	m_tooltipQuality = ItemQuality_Common;

	m_leftArrowPressed = false;
	m_rightArrowPressed = false;

	VoxGame::GetInstance()->SetPaperdollRotation(0.0f);

	m_loaded = true;
}

void CharacterGUI::Unload()
{
	m_loadDelay = false;
	m_loadDelayTime = 0.0f;

	HideTooltip();

	HideEquipHover();

	StopEquipHoverAnimation();

	m_pCharacterWindow->RemoveComponent(m_pStat1IncreaseButton);
	m_pCharacterWindow->RemoveComponent(m_pStat2IncreaseButton);
	m_pCharacterWindow->RemoveComponent(m_pStat3IncreaseButton);
	m_pCharacterWindow->RemoveComponent(m_pStat4IncreaseButton);
	m_pCharacterWindow->RemoveComponent(m_pStat5IncreaseButton);
	m_pCharacterWindow->RemoveComponent(m_pStat6IncreaseButton);

	m_pGUI->RemoveWindow(m_pCharacterWindow);

	DeleteInventoryItems();

	m_loaded = false;

	if (VoxGame::GetInstance()->IsGUIWindowStillDisplayed() == false)
	{
		VoxGame::GetInstance()->TurnCursorOff(false);
		if (VoxGame::GetInstance()->ShouldRestorePreviousCameraMode())
		{
			VoxGame::GetInstance()->RestorePreviousCameraMode();
			VoxGame::GetInstance()->InitializeCameraRotation();
		}
	}
}

bool CharacterGUI::IsLoadDelayed()
{
	return (m_loadDelay == true && m_loadDelayTime > 0.0f);
}

bool CharacterGUI::IsLoaded()
{
	return m_loaded;
}

GUIWindow* CharacterGUI::GetCharacterWindow()
{
	return m_pCharacterWindow;
}

void CharacterGUI::CreateInventoryItems()
{
	// Also go through all the equipped items and reset their positions
	for(int i = 0; i < EquipSlot_NumSlots; i++)
	{
		if(i == EquipSlot_NoSlot)
		{
			continue;
		}

		InventoryItem* pItem = m_pInventoryManager->GetInventoryItemForEquipSlot((EquipSlot)i);
		if(pItem != NULL)
		{
			int x;
			int y;
			int width;
			int height;
			switch(pItem->m_equipSlot)
			{
			case EquipSlot_NoSlot : { x = 0; y = 0; width = 0; height = 0; } break;
			case EquipSlot_RightHand : { GetPlayerWeaponRightDimensions(&x, &y, &width, &height); } break;
			case EquipSlot_LeftHand : { GetPlayerWeaponLeftDimensions(&x, &y, &width, &height); } break;
			case EquipSlot_Head : { GetPlayerHeadDimensions(&x, &y, &width, &height); } break;
			case EquipSlot_Shoulders : { GetPlayerShoulderDimensions(&x, &y, &width, &height); } break;
			case EquipSlot_Body : { GetPlayerBodyDimensions(&x, &y, &width, &height); } break;
			case EquipSlot_Legs : { GetPlayerLegsDimensions(&x, &y, &width, &height); } break;
			case EquipSlot_Hand : { GetPlayerHandDimensions(&x, &y, &width, &height); } break;
			case EquipSlot_Feet : { GetPlayerFeetDimensions(&x, &y, &width, &height); } break;
			case EquipSlot_Accessory1 : { GetPlayerAccessory1Dimensions(&x, &y, &width, &height); } break;
			case EquipSlot_Accessory2 : { GetPlayerAccessory2Dimensions(&x, &y, &width, &height); } break;
			}

			DraggableRenderRectangle* pNewSlotItem = new DraggableRenderRectangle(m_pRenderer);
			switch(pItem->m_itemQuality)
			{
			case ItemQuality_Common: { pNewSlotItem->SetIcon(m_pInventoryBackgroundSlotBorder_Common); break; }
			case ItemQuality_Uncommon: { pNewSlotItem->SetIcon(m_pInventoryBackgroundSlotBorder_Uncommon); break; }
			case ItemQuality_Magical: { pNewSlotItem->SetIcon(m_pInventoryBackgroundSlotBorder_Magical); break; }
			case ItemQuality_Rare: { pNewSlotItem->SetIcon(m_pInventoryBackgroundSlotBorder_Rare); break; }
			case ItemQuality_Epic: { pNewSlotItem->SetIcon(m_pInventoryBackgroundSlotBorder_Epic); break; }
			}
			pNewSlotItem->SetDimensions(x, y, width, height);
			pNewSlotItem->SetDepth(3.0f);

			char lItemTexture[128];
			sprintf(lItemTexture, "%s", pItem->m_Iconfilename.c_str());
			pNewSlotItem->AddIcon(m_pRenderer, lItemTexture, 64, 64, 56, 56, 4, 4, 1.5f);

			string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();
			switch (pItem->m_itemQuality)
			{
				case ItemQuality_Common:
				{
					string itemBackgroundIcon = "media/textures/gui/" + themeName + "/common/items/item_background_common.tga";
					pNewSlotItem->AddIcon(m_pRenderer, itemBackgroundIcon.c_str(), 64, 64, 64, 64, 0, 0, 1.0f);
					break;
				}
				case ItemQuality_Uncommon:
				{
					string itemBackgroundIcon = "media/textures/gui/" + themeName + "/common/items/item_background_uncommon.tga";
					pNewSlotItem->AddIcon(m_pRenderer, itemBackgroundIcon.c_str(), 64, 64, 64, 64, 0, 0, 1.0f);
					break;
				}
				case ItemQuality_Magical:
				{
					string itemBackgroundIcon = "media/textures/gui/" + themeName + "/common/items/item_background_magical.tga";
					pNewSlotItem->AddIcon(m_pRenderer, itemBackgroundIcon.c_str(), 64, 64, 64, 64, 0, 0, 1.0f);
					break;
				}
				case ItemQuality_Rare:
				{
					string itemBackgroundIcon = "media/textures/gui/" + themeName + "/common/items/item_background_rare.tga";
					pNewSlotItem->AddIcon(m_pRenderer, itemBackgroundIcon.c_str(), 64, 64, 64, 64, 0, 0, 1.0f);
					break;
				}
				case ItemQuality_Epic:
				{
					string itemBackgroundIcon = "media/textures/gui/" + themeName + "/common/items/item_background_epic.tga";
					pNewSlotItem->AddIcon(m_pRenderer, itemBackgroundIcon.c_str(), 64, 64, 64, 64, 0, 0, 1.0f);
					break;
				}
			}

			if(pItem->m_quantity != -1)
			{
				char lQuantity[128];
				sprintf(lQuantity, "%i", pItem->m_quantity);
				int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_18(), "%s", lQuantity);
				pNewSlotItem->AddText(m_pRenderer, m_pFrontendManager->GetFrontendFont_18(), m_pFrontendManager->GetFrontendFont_18_Outline(), lQuantity, Colour(1.0f, 1.0f, 1.0f, 1.0f), width-9-textWidth, 7, true, Colour(0.0f, 0.0f, 0.0f, 1.0f));
			}

			CharacterSlotItem* pNewItem = new CharacterSlotItem();
			pNewItem->m_pCharacterGUI = this;
			pNewItem->m_pInventoryItem = pItem;
			pNewItem->m_pInventoryIcon = pNewSlotItem;
			pNewItem->m_slotX = -1;
			pNewItem->m_slotY = -1;
			m_vpInventorySlotItems.push_back(pNewItem);

			pNewSlotItem->SetPressedCallBackFunction(_CharacterItemPressed);
			pNewSlotItem->SetPressedCallBackData(pNewItem);

			pNewSlotItem->SetReleasedCallBackFunction(_CharacterItemReleased);
			pNewSlotItem->SetReleasedCallBackData(pNewItem);

			pNewSlotItem->SetEnterCallBackFunction(_CharacterItemEntered);
			pNewSlotItem->SetEnterCallBackData(pNewItem);

			pNewSlotItem->SetExitCallBackFunction(_CharacterItemExited);
			pNewSlotItem->SetExitCallBackData(pNewItem);

			pNewItem->m_pInventoryIcon->SetLocation(x, y);

			m_pEquippedItems[pItem->m_equipSlot] = pNewItem;

			if(pItem->m_equipSlot == EquipSlot_LeftHand && pItem->m_right)
			{
				int x;
				int y;
				int width;
				int height;
				GetPlayerWeaponRightDimensions(&x, &y, &width, &height);

				pBlockSlotItem = new Icon(m_pRenderer);
				pBlockSlotItem->SetIcon(m_pInventoryBackgroundSlotBorder_Common);
				string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();
				string itemForegroundIcon = "media/textures/gui/" + themeName + "/common/items/item_foreground_alpha.tga";
				pBlockSlotItem->AddIcon(m_pRenderer, itemForegroundIcon.c_str(), 64, 64, 64, 64, 0, 0, 3.0f);

				char lItemTexture[128];
				sprintf(lItemTexture, "%s", pItem->m_Iconfilename.c_str());
				pBlockSlotItem->AddIcon(m_pRenderer, lItemTexture, 64, 64, 56, 56, 4, 4, 1.5f);

				pBlockSlotItem->SetDimensions(x, y, width, height);
				pBlockSlotItem->SetDepth(2.0f);

				m_pCharacterWindow->AddComponent(pBlockSlotItem);
			}
			if(pItem->m_equipSlot == EquipSlot_RightHand && pItem->m_left)
			{
				int x;
				int y;
				int width;
				int height;
				GetPlayerWeaponLeftDimensions(&x, &y, &width, &height);

				pBlockSlotItem = new Icon(m_pRenderer);
				pBlockSlotItem->SetIcon(m_pInventoryBackgroundSlotBorder_Common);
				string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();
				string itemForegroundIcon = "media/textures/gui/" + themeName + "/common/items/item_foreground_alpha.tga";
				pBlockSlotItem->AddIcon(m_pRenderer, itemForegroundIcon.c_str(), 64, 64, 64, 64, 0, 0, 3.0f);

				char lItemTexture[128];
				sprintf(lItemTexture, "%s", pItem->m_Iconfilename.c_str());
				pBlockSlotItem->AddIcon(m_pRenderer, lItemTexture, 64, 64, 56, 56, 4, 4, 1.5f);

				pBlockSlotItem->SetDimensions(x, y, width, height);
				pBlockSlotItem->SetDepth(2.0f);

				m_pCharacterWindow->AddComponent(pBlockSlotItem);
			}
		}
	}
}

void CharacterGUI::DeleteInventoryItems()
{
	// Clear equipped items
	for(unsigned int i = 0; i < EquipSlot_NumSlots; i++)
	{
		m_pEquippedItems[i] = NULL;
	}

	// Clear item draggable buttons
	for(unsigned int i = 0; i < m_vpInventorySlotItems.size(); i++)
	{
		m_pCharacterWindow->RemoveComponent(m_vpInventorySlotItems[i]->m_pInventoryIcon);
		delete m_vpInventorySlotItems[i]->m_pInventoryIcon;
		m_vpInventorySlotItems[i]->m_pInventoryIcon = 0;

		delete m_vpInventorySlotItems[i];
		m_vpInventorySlotItems[i] = 0;
	}
	m_vpInventorySlotItems.clear();

	if(pBlockSlotItem != NULL)
	{
		m_pCharacterWindow->RemoveComponent(pBlockSlotItem);
		delete pBlockSlotItem;
		pBlockSlotItem = NULL;
	}
}

void CharacterGUI::ShowEquipHover(EquipSlot equipSlot)
{
	if (VoxGame::GetInstance()->GetVoxSettings()->m_equipHelper == false)
	{
		// If we have turned off the equip helper in the options menu, do nothing
		return;
	}

	int x;
	int y;
	int width;
	int height;
	switch(equipSlot)
	{
	case EquipSlot_NoSlot : { x = 0; y = 0; width = 0; height = 0; } break;
	case EquipSlot_RightHand : { GetPlayerWeaponRightDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_LeftHand : {  GetPlayerWeaponLeftDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Head : {  GetPlayerHeadDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Shoulders : {  GetPlayerShoulderDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Body : {  GetPlayerBodyDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Legs : {  GetPlayerLegsDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Hand : {  GetPlayerHandDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Feet : {  GetPlayerFeetDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Accessory1 : {  GetPlayerAccessory1Dimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Accessory2 : {  GetPlayerAccessory2Dimensions(&x, &y, &width, &height); } break;
	default: assert(false);
	}
	m_equipHoverXOrigin = x;
	m_pEquipHoverIcon->SetDimensions(m_equipHoverXOrigin+(int)m_equipHoverXOffset, y, width+16, height+16);
	m_pEquipHoverIcon->SetDepth(5.5f);

	m_pCharacterWindow->AddComponent(m_pEquipHoverIcon);
}

void CharacterGUI::HideEquipHover()
{
	if (VoxGame::GetInstance()->GetVoxSettings()->m_equipHelper == false)
	{
		// If we have turned off the equip helper in the options menu, do nothing
		return;
	}

	m_pCharacterWindow->RemoveComponent(m_pEquipHoverIcon);
}

void CharacterGUI::UpdateEquipHover()
{
	Dimensions equipHoverDimensions = m_pEquipHoverIcon->GetDimensions();
	m_pEquipHoverIcon->SetDimensions(m_equipHoverXOrigin+(int)m_equipHoverXOffset, equipHoverDimensions.m_y, equipHoverDimensions.m_width, equipHoverDimensions.m_height);
}

void CharacterGUI::StartEquipHoverAnimation()
{
	m_equipHoverXOffset = 0.0f;

	FloatInterpolation* lpXPosInterp1;
	FloatInterpolation* lpXPosInterp2;
	lpXPosInterp1 = Interpolator::GetInstance()->CreateFloatInterpolation(&m_equipHoverXOffset, m_equipHoverXOffset, m_equipHoverXOffset+10.0f, 0.5f, 100.0f);
	lpXPosInterp2 = Interpolator::GetInstance()->CreateFloatInterpolation(&m_equipHoverXOffset, m_equipHoverXOffset+10.0f, m_equipHoverXOffset, 0.5f, -100.0f, NULL, _EquipHoverAnimationFinished, this);
	Interpolator::GetInstance()->LinkFloatInterpolation(lpXPosInterp1, lpXPosInterp2);
	Interpolator::GetInstance()->AddFloatInterpolation(lpXPosInterp1);
}

void CharacterGUI::StopEquipHoverAnimation()
{
	Interpolator::GetInstance()->RemoveFloatInterpolationByVariable(&m_equipHoverXOffset);
}

// Tooltips
void CharacterGUI::UpdateToolTipAppear(float dt)
{
	if (m_loadDelay == true)
	{
		if (m_loadDelayTime <= 0.0f)
		{
			m_loadDelay = false;
			m_pGUI->AddWindow(m_pCharacterWindow);
			m_pCharacterWindow->Show();
		}
		else
		{
			m_loadDelayTime -= dt;
		}
	}

	if(m_toolTipVisible)
	{
		if(m_tooltipAppearDelayTimer <= 0.0f)
		{
			if(m_toolTipComponentsAdded == false)
			{
				switch(m_tooltipQuality)
				{
				case ItemQuality_Common :  { m_pCharacterWindow->AddComponent(m_pTooltipBackground_Common); break; }
				case ItemQuality_Uncommon :  { m_pCharacterWindow->AddComponent(m_pTooltipBackground_Uncommon); break; }
				case ItemQuality_Magical :  { m_pCharacterWindow->AddComponent(m_pTooltipBackground_Magical); break; }
				case ItemQuality_Rare :  { m_pCharacterWindow->AddComponent(m_pTooltipBackground_Rare); break; }
				case ItemQuality_Epic :  { m_pCharacterWindow->AddComponent(m_pTooltipBackground_Epic); break; }
				}

				m_pCharacterWindow->AddComponent(m_pTooltipNameLabel);
				m_pCharacterWindow->AddComponent(m_pTooltipDescriptionLabel);
				m_pCharacterWindow->AddComponent(m_pTooltipSlotLabel);
				m_pCharacterWindow->AddComponent(m_pTooltipQualityLabel);

				m_toolTipComponentsAdded = true;
			}
		}
		else
		{
			m_tooltipAppearDelayTimer -= dt;
		}
	}
}

void CharacterGUI::ShowTooltip(CharacterSlotItem* pCharacterItem)
{
	if(m_toolTipVisible == true)
	{
		return;
	}

	// Set the focused window when we show a tooltip
	m_pCharacterWindow->SetFocusWindow();

	// Replace the tooltip name
	m_pTooltipNameLabel->SetText(pCharacterItem->m_pInventoryItem->m_title);

	// Replace the tooltip description
	string DescriptionText = pCharacterItem->m_pInventoryItem->m_description + pCharacterItem->m_pInventoryItem->GetStatsAttributeString();
	m_pTooltipDescriptionLabel->SetText(DescriptionText);

	// Replace the tooltip equipslot text
	char slotText[32];
	switch(pCharacterItem->m_pInventoryItem->m_equipSlot)
	{
	case EquipSlot_NoSlot: { sprintf(slotText, ""); break; }
	case EquipSlot_LeftHand:
		{
			if(pCharacterItem->m_pInventoryItem->m_right)
			{
				sprintf(slotText, "Two Handed");
			}
			else
			{
				sprintf(slotText, "Left Hand");
			}

			break;
		}
	case EquipSlot_RightHand:
		{
			if(pCharacterItem->m_pInventoryItem->m_left)
			{
				sprintf(slotText, "Two Handed");
			}
			else
			{
				sprintf(slotText, "Right Hand");
			}

			break;
		}
	case EquipSlot_Head: { sprintf(slotText, "Head"); break; }
	case EquipSlot_Shoulders: { sprintf(slotText, "Shoulders"); break; }
	case EquipSlot_Body: { sprintf(slotText, "Body"); break; }
	case EquipSlot_Legs: { sprintf(slotText, "Legs"); break; }
	case EquipSlot_Hand: { sprintf(slotText, "Hand"); break; }
	case EquipSlot_Feet: { sprintf(slotText, "Feet"); break; }
	case EquipSlot_Accessory1: { sprintf(slotText, "Accessory 1"); break; }
	case EquipSlot_Accessory2: { sprintf(slotText, "Accessory 2"); break; }
	}
	m_pTooltipSlotLabel->SetText(slotText);

	// Replace the tooltip quality text
	char qualityText[32];
	Colour qualityColour;
	switch(pCharacterItem->m_pInventoryItem->m_itemQuality)
	{
	case ItemQuality_Common :  { sprintf(qualityText, "Common"); qualityColour = Colour(0.5f, 0.5f, 0.5f, 1.0f); break; }
	case ItemQuality_Uncommon :  { sprintf(qualityText, "Uncommon"); qualityColour = Colour(0.95f, 1.0f, 0.2f, 1.0f); break; }
	case ItemQuality_Magical :  { sprintf(qualityText, "Magical"); qualityColour = Colour(0.0f, 1.0f, 0.0f, 1.0f); break; }
	case ItemQuality_Rare :  { sprintf(qualityText, "Rare"); qualityColour = Colour(0.0f, 0.5f, 1.0f, 1.0f); break; }
	case ItemQuality_Epic :  { sprintf(qualityText, "Epic"); qualityColour = Colour(0.64f, 0.2f, 0.93f, 1.0f); break; }
	}
	m_pTooltipQualityLabel->SetText(qualityText);
	m_pTooltipQualityLabel->SetColour(qualityColour);

	m_pTooltipNameLabel->SetColour(qualityColour);

	// Set Component dimensions
	m_tooltipWidth = 200;
	m_tooltipHeight = 220;
	m_tooltipDescBorder = 15;

	int x;
	int y;
	int width;
	int height;
	switch(pCharacterItem->m_pInventoryItem->m_equipSlot)
	{
	case EquipSlot_NoSlot : { x = 0; y = 0; width = 0; height = 0; } break;
	case EquipSlot_RightHand : { GetPlayerWeaponRightDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_LeftHand : { GetPlayerWeaponLeftDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Head : { GetPlayerHeadDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Shoulders : { GetPlayerShoulderDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Body : { GetPlayerBodyDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Legs : { GetPlayerLegsDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Hand : { GetPlayerHandDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Feet : { GetPlayerFeetDimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Accessory1 : { GetPlayerAccessory1Dimensions(&x, &y, &width, &height); } break;
	case EquipSlot_Accessory2 : { GetPlayerAccessory2Dimensions(&x, &y, &width, &height); } break;
	}

	if(VoxGame::GetInstance()->GetWindowCursorX() > m_tooltipWidth + 50)
	{
		x = x + 20 - m_tooltipWidth;
	}
	else
	{
		x = x + 30;
	}
	if((m_windowHeight-VoxGame::GetInstance()->GetWindowCursorY()) > m_windowHeight - m_tooltipHeight - 50)
	{
		y = y + 20 - m_tooltipHeight;
	}
	else
	{
		y = y + 30;
	}

	m_pTooltipBackground_Common->SetDimensions(x, y, m_tooltipWidth, m_tooltipHeight);
	m_pTooltipBackground_Uncommon->SetDimensions(x, y, m_tooltipWidth, m_tooltipHeight);
	m_pTooltipBackground_Magical->SetDimensions(x, y, m_tooltipWidth, m_tooltipHeight);
	m_pTooltipBackground_Rare->SetDimensions(x, y, m_tooltipWidth, m_tooltipHeight);
	m_pTooltipBackground_Epic->SetDimensions(x, y, m_tooltipWidth, m_tooltipHeight);

	int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_30(), "%s", m_pTooltipNameLabel->GetText().c_str());
	m_pTooltipNameLabel->SetLocation(x + (int)(m_tooltipWidth*0.5f)-(int)(textWidth*0.5f), y + m_tooltipHeight - 35);

	textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_25(), "%s", m_pTooltipDescriptionLabel->GetText().c_str());
	m_pTooltipDescriptionLabel->SetDimensions(x + m_tooltipDescBorder, y + m_tooltipDescBorder, m_tooltipWidth-(m_tooltipDescBorder*2), m_tooltipHeight-(m_tooltipDescBorder*2)-35);

	m_pTooltipSlotLabel->SetLocation(x + m_tooltipDescBorder, y+m_tooltipDescBorder);

	textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_20(), "%s", m_pTooltipQualityLabel->GetText().c_str());
	m_pTooltipQualityLabel->SetLocation(x + m_tooltipWidth - m_tooltipDescBorder - textWidth, y+m_tooltipDescBorder);

	m_tooltipQuality = pCharacterItem->m_pInventoryItem->m_itemQuality;

	m_tooltipAppearDelayTimer = m_pFrontendManager->GetToolTipAppearDelay();

	m_toolTipVisible = true;
	m_toolTipComponentsAdded = false;
}

void CharacterGUI::HideTooltip()
{
	m_pCharacterWindow->RemoveComponent(m_pTooltipBackground_Common);
	m_pCharacterWindow->RemoveComponent(m_pTooltipBackground_Uncommon);
	m_pCharacterWindow->RemoveComponent(m_pTooltipBackground_Magical);
	m_pCharacterWindow->RemoveComponent(m_pTooltipBackground_Rare);
	m_pCharacterWindow->RemoveComponent(m_pTooltipBackground_Epic);
	m_pCharacterWindow->RemoveComponent(m_pTooltipNameLabel);
	m_pCharacterWindow->RemoveComponent(m_pTooltipDescriptionLabel);
	m_pCharacterWindow->RemoveComponent(m_pTooltipSlotLabel);
	m_pCharacterWindow->RemoveComponent(m_pTooltipQualityLabel);

	m_toolTipVisible = false;
}

void CharacterGUI::GetPlayerPortraitDimensions(int* x, int* y, int* width, int* height)
{
	*x = m_paperDoll_X;
	*y = m_paperDoll_Y;

	*width = m_paperDollWidth;
	*height = m_paperDollHeight;
}

void CharacterGUI::GetPlayerWeaponLeftDimensions(int* x, int* y, int* width, int* height)
{
	*x = 157;
	*y = 18;

	*width = 64;
	*height = 64;
}

void CharacterGUI::GetPlayerWeaponRightDimensions(int* x, int* y, int* width, int* height)
{
	*x = 84;
	*y = 18;

	*width = 64;
	*height = 64;
}

void CharacterGUI::GetPlayerHeadDimensions(int* x, int* y, int* width, int* height)
{
	*x = 4;
	*y = 213;

	*width = 64;
	*height = 64;
}

void CharacterGUI::GetPlayerShoulderDimensions(int* x, int* y, int* width, int* height)
{
	*x = 236;
	*y = 213;

	*width = 64;
	*height = 64;
}

void CharacterGUI::GetPlayerBodyDimensions(int* x, int* y, int* width, int* height)
{
	*x = 4;
	*y = 145;

	*width = 64;
	*height = 64;
}

void CharacterGUI::GetPlayerLegsDimensions(int* x, int* y, int* width, int* height)
{
	*x = 236;
	*y = 145;

	*width = 64;
	*height = 64;
}

void CharacterGUI::GetPlayerHandDimensions(int* x, int* y, int* width, int* height)
{
	*x = 4;
	*y = 77;

	*width = 64;
	*height = 64;
}

void CharacterGUI::GetPlayerFeetDimensions(int* x, int* y, int* width, int* height)
{
	*x = 236;
	*y = 77;

	*width = 64;
	*height = 64;
}

void CharacterGUI::GetPlayerAccessory1Dimensions(int* x, int* y, int* width, int* height)
{
	*x = 4;
	*y = 8;

	*width = 64;
	*height = 64;
}

void CharacterGUI::GetPlayerAccessory2Dimensions(int* x, int* y, int* width, int* height)
{
	*x = 236;
	*y = 8;

	*width = 64;
	*height = 64;
}

void CharacterGUI::UpdatePlayerStats()
{
	// Player name
	m_pCharacterTitleLabel->SetText(m_pPlayer->GetName());

	// Stats
	// Strength
	char stat1Value[32];
	sprintf(stat1Value, "%i %s", m_pPlayer->GetPlayerStats()->GetStrengthStat(), m_pPlayer->GetStrengthModifierString().c_str());
	m_pStatsValueLabel1->SetText(stat1Value);

	// Dexterity
	char stat2Value[32];
	sprintf(stat2Value, "%i %s", m_pPlayer->GetPlayerStats()->GetDexterityStat(), m_pPlayer->GetDexterityModifierString().c_str());
	m_pStatsValueLabel2->SetText(stat2Value);

	// Magic
	char stat3Value[32];
	sprintf(stat3Value, "%i %s", m_pPlayer->GetPlayerStats()->GetIntelligenceStat(), m_pPlayer->GetIntelligenceModifierString().c_str());
	m_pStatsValueLabel3->SetText(stat3Value);

	// Vitality
	char stat4Value[32];
	sprintf(stat4Value, "%i %s", m_pPlayer->GetPlayerStats()->GetVitalityStat(), m_pPlayer->GetVitalityModifierString().c_str());
	m_pStatsValueLabel4->SetText(stat4Value);

	// Armor
	char stat5Value[32];
	sprintf(stat5Value, "%i %s", m_pPlayer->GetPlayerStats()->GetArmorStat(), m_pPlayer->GetArmorModifierString().c_str());
	m_pStatsValueLabel5->SetText(stat5Value);

	// Luck
	char stat6Value[32];
	sprintf(stat6Value, "%i %s", m_pPlayer->GetPlayerStats()->GetLuckStat(), m_pPlayer->GetLuckModifierString().c_str());
	m_pStatsValueLabel6->SetText(stat6Value);

	if(m_pPlayer->GetPlayerStats()->GetNumPointsAvailable() <= 0)
	{
		m_pCharacterWindow->RemoveComponent(m_pStat1IncreaseButton);
		m_pCharacterWindow->RemoveComponent(m_pStat2IncreaseButton);
		m_pCharacterWindow->RemoveComponent(m_pStat3IncreaseButton);
		m_pCharacterWindow->RemoveComponent(m_pStat4IncreaseButton);
		m_pCharacterWindow->RemoveComponent(m_pStat5IncreaseButton);
		m_pCharacterWindow->RemoveComponent(m_pStat6IncreaseButton);

		m_addedStatIncreaseButtons = false;
	}
	else
	{
		if(m_loaded)
		{
			if(m_addedStatIncreaseButtons == false)
			{
				m_pGUI->RemoveWindow(m_pCharacterWindow);

				m_pCharacterWindow->AddComponent(m_pStat1IncreaseButton);
				m_pCharacterWindow->AddComponent(m_pStat2IncreaseButton);
				m_pCharacterWindow->AddComponent(m_pStat3IncreaseButton);
				m_pCharacterWindow->AddComponent(m_pStat4IncreaseButton);
				m_pCharacterWindow->AddComponent(m_pStat5IncreaseButton);
				m_pCharacterWindow->AddComponent(m_pStat6IncreaseButton);

				m_pGUI->AddWindow(m_pCharacterWindow);
				m_pCharacterWindow->Show();

				m_addedStatIncreaseButtons = true;
			}
		}
	}
}

void CharacterGUI::Update(float dt)
{
	UpdateToolTipAppear(dt);

	m_pDynamicPaperDollIcon->SetDynamicTexture(VoxGame::GetInstance()->GetDynamicPaperdollTexture());

	// Check if the character GUI needs update (we have equipped items, or quantity changed, etc)
	if(m_pInventoryManager->CharacterGUINeedsUpdate() && IsLoaded() == true)
	{
		m_pGUI->RemoveWindow(m_pCharacterWindow);

		DeleteInventoryItems();
		CreateInventoryItems();

		for(unsigned int i = 0; i < m_vpInventorySlotItems.size(); i++)
		{
			m_pCharacterWindow->AddComponent(m_vpInventorySlotItems[i]->m_pInventoryIcon);
		}

		m_pGUI->AddWindow(m_pCharacterWindow);
		m_pCharacterWindow->Show();

		m_pInventoryManager->SetCharacterGUINeedsUpdate(false);
	}

	// Update equip hover icon
	UpdateEquipHover();

	if(m_rightArrowPressed)
	{
		VoxGame::GetInstance()->RotatePaperdollModel(150.0f * dt);
	}
	if(m_leftArrowPressed)
	{
		VoxGame::GetInstance()->RotatePaperdollModel(-150.0f * dt);
	}

	// Update the stats tab left and right sides
	if(m_statsTabLeftSide)
	{
		m_pTabOpenCloseButton->SetDefaultIcon(m_pTabOpenCloseButton_Icon_Left);
		m_pTabOpenCloseButton->SetHoverIcon(m_pTabOpenCloseButton_Icon_Left_Hover);
		m_pTabOpenCloseButton->SetSelectedIcon(m_pTabOpenCloseButton_Icon_Left_Pressed);
		m_pTabOpenCloseButton->SetDisabledIcon(m_pTabOpenCloseButton_Icon_Left);

		if(m_statsTabsOpen)
		{
			m_pTabOpenCloseButton->SetDimensions(-m_statsTabWidth-m_statsTabOpenButtonWidth, (m_characterWindowHeight/2)-(m_statsTabOpenButtonHeight/2), m_statsTabOpenButtonWidth, m_statsTabOpenButtonHeight);
			m_pStatsTab_Icon_Left->SetDimensions(-m_statsTabWidth, (m_characterWindowHeight/2)-(m_statsTabHeight/2), m_statsTabWidth, m_statsTabHeight);
			m_pStatsTab_Icon_Right->SetDimensions(0, (m_characterWindowHeight/2)-(m_statsTabHeight/2), m_statsTabWidth, m_statsTabHeight);

			int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_Medium(), "%s", m_pCharacterTitleLabel->GetText().c_str());
			m_pCharacterTitleLabel->SetLocation(-m_statsTabWidth/2-textWidth/2, m_statsTitleLabel_Y);

			m_pStatsHeaderLabel1->SetLocation(-m_statsTabWidth+m_statsHeaderLabel_XOffset, m_statsTitleLabel_Y-26);
			m_pStatsHeaderLabel2->SetLocation(-m_statsTabWidth+m_statsHeaderLabel_XOffset, m_statsTitleLabel_Y-52);
			m_pStatsHeaderLabel3->SetLocation(-m_statsTabWidth+m_statsHeaderLabel_XOffset, m_statsTitleLabel_Y-78);
			m_pStatsHeaderLabel4->SetLocation(-m_statsTabWidth+m_statsHeaderLabel_XOffset, m_statsTitleLabel_Y-104);
			m_pStatsHeaderLabel5->SetLocation(-m_statsTabWidth+m_statsHeaderLabel_XOffset, m_statsTitleLabel_Y-130);
			m_pStatsHeaderLabel6->SetLocation(-m_statsTabWidth+m_statsHeaderLabel_XOffset, m_statsTitleLabel_Y-156);

			m_pStatsValueLabel1->SetDimensions(-m_statsTabWidth+m_statsValueLabel_XOffset, m_statsTitleLabel_Y-26, 59, 26);
			m_pStatsValueLabel2->SetDimensions(-m_statsTabWidth+m_statsValueLabel_XOffset, m_statsTitleLabel_Y-52, 59, 26);
			m_pStatsValueLabel3->SetDimensions(-m_statsTabWidth+m_statsValueLabel_XOffset, m_statsTitleLabel_Y-78, 59, 26);
			m_pStatsValueLabel4->SetDimensions(-m_statsTabWidth+m_statsValueLabel_XOffset, m_statsTitleLabel_Y-104, 59, 26);
			m_pStatsValueLabel5->SetDimensions(-m_statsTabWidth+m_statsValueLabel_XOffset, m_statsTitleLabel_Y-130, 59, 26);
			m_pStatsValueLabel6->SetDimensions(-m_statsTabWidth+m_statsValueLabel_XOffset, m_statsTitleLabel_Y-156, 59, 26);

			if(m_pPlayer->GetPlayerStats()->GetNumPointsAvailable() > 0)
			{
				m_pStat1IncreaseButton->SetDimensions(-73, m_statsTitleLabel_Y-25, 16, 16);
				m_pStat2IncreaseButton->SetDimensions(-73, m_statsTitleLabel_Y-51, 16, 16);
				m_pStat3IncreaseButton->SetDimensions(-73, m_statsTitleLabel_Y-77, 16, 16);
				m_pStat4IncreaseButton->SetDimensions(-73, m_statsTitleLabel_Y-103, 16, 16);
				m_pStat5IncreaseButton->SetDimensions(-73, m_statsTitleLabel_Y-129, 16, 16);
				m_pStat6IncreaseButton->SetDimensions(-73, m_statsTitleLabel_Y-155, 16, 16);
			}
		}
		else
		{
			m_pTabOpenCloseButton->SetDimensions(-m_statsTabOpenButtonWidth, (m_characterWindowHeight/2)-(m_statsTabOpenButtonHeight/2), m_statsTabOpenButtonWidth, m_statsTabOpenButtonHeight);
			m_pStatsTab_Icon_Left->SetDimensions(0, (m_characterWindowHeight/2)-(m_statsTabHeight/2), m_statsTabWidth, m_statsTabHeight);
			m_pStatsTab_Icon_Right->SetDimensions(0, (m_characterWindowHeight/2)-(m_statsTabHeight/2), m_statsTabWidth, m_statsTabHeight);

			m_pCharacterTitleLabel->SetLocation(-5000, -5000);

			m_pStatsHeaderLabel1->SetLocation(-5000, -5000);
			m_pStatsHeaderLabel2->SetLocation(-5000, -5000);
			m_pStatsHeaderLabel3->SetLocation(-5000, -5000);
			m_pStatsHeaderLabel4->SetLocation(-5000, -5000);
			m_pStatsHeaderLabel5->SetLocation(-5000, -5000);
			m_pStatsHeaderLabel6->SetLocation(-5000, -5000);
			m_pStatsValueLabel1->SetLocation(-5000, -5000);
			m_pStatsValueLabel2->SetLocation(-5000, -5000);
			m_pStatsValueLabel3->SetLocation(-5000, -5000);
			m_pStatsValueLabel4->SetLocation(-5000, -5000);
			m_pStatsValueLabel5->SetLocation(-5000, -5000);
			m_pStatsValueLabel6->SetLocation(-5000, -5000);

			m_pStat1IncreaseButton->SetDimensions(-5000, -5000, 20, 20);
			m_pStat2IncreaseButton->SetDimensions(-5000, -5000, 20, 20);
			m_pStat3IncreaseButton->SetDimensions(-5000, -5000, 20, 20);
			m_pStat4IncreaseButton->SetDimensions(-5000, -5000, 20, 20);
			m_pStat5IncreaseButton->SetDimensions(-5000, -5000, 20, 20);
			m_pStat6IncreaseButton->SetDimensions(-5000, -5000, 20, 20);
		}
	}
	else
	{
		m_pTabOpenCloseButton->SetDefaultIcon(m_pTabOpenCloseButton_Icon_Right);
		m_pTabOpenCloseButton->SetHoverIcon(m_pTabOpenCloseButton_Icon_Right_Hover);
		m_pTabOpenCloseButton->SetSelectedIcon(m_pTabOpenCloseButton_Icon_Right_Pressed);
		m_pTabOpenCloseButton->SetDisabledIcon(m_pTabOpenCloseButton_Icon_Right_Pressed);

		if(m_statsTabsOpen)
		{
			m_pTabOpenCloseButton->SetDimensions(m_characterWindowWidth+m_statsTabWidth, (m_characterWindowHeight/2)-(m_statsTabOpenButtonHeight/2), m_statsTabOpenButtonWidth, m_statsTabOpenButtonHeight);
			m_pStatsTab_Icon_Left->SetDimensions(0, (m_characterWindowHeight/2)-(m_statsTabHeight/2), m_statsTabWidth, m_statsTabHeight);
			m_pStatsTab_Icon_Right->SetDimensions(m_characterWindowWidth, (m_characterWindowHeight/2)-(m_statsTabHeight/2), m_statsTabWidth, m_statsTabHeight);

			int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_Medium(), "%s", m_pCharacterTitleLabel->GetText().c_str());
			m_pCharacterTitleLabel->SetLocation(m_characterWindowWidth+m_statsTabWidth/2-textWidth/2, m_statsTitleLabel_Y);

			m_pStatsHeaderLabel1->SetLocation(m_characterWindowWidth+m_statsHeaderLabel_XOffset-4, m_statsTitleLabel_Y-26);
			m_pStatsHeaderLabel2->SetLocation(m_characterWindowWidth+m_statsHeaderLabel_XOffset-4, m_statsTitleLabel_Y-52);
			m_pStatsHeaderLabel3->SetLocation(m_characterWindowWidth+m_statsHeaderLabel_XOffset-4, m_statsTitleLabel_Y-78);
			m_pStatsHeaderLabel4->SetLocation(m_characterWindowWidth+m_statsHeaderLabel_XOffset-4, m_statsTitleLabel_Y-104);
			m_pStatsHeaderLabel5->SetLocation(m_characterWindowWidth+m_statsHeaderLabel_XOffset-4, m_statsTitleLabel_Y-130);
			m_pStatsHeaderLabel6->SetLocation(m_characterWindowWidth+m_statsHeaderLabel_XOffset-4, m_statsTitleLabel_Y-156);

			m_pStatsValueLabel1->SetDimensions(m_characterWindowWidth+m_statsValueLabel_XOffset-4, m_statsTitleLabel_Y-26, 59, 26);
			m_pStatsValueLabel2->SetDimensions(m_characterWindowWidth+m_statsValueLabel_XOffset-4, m_statsTitleLabel_Y-52, 59, 26);
			m_pStatsValueLabel3->SetDimensions(m_characterWindowWidth+m_statsValueLabel_XOffset-4, m_statsTitleLabel_Y-78, 59, 26);
			m_pStatsValueLabel4->SetDimensions(m_characterWindowWidth+m_statsValueLabel_XOffset-4, m_statsTitleLabel_Y-104, 59, 26);
			m_pStatsValueLabel5->SetDimensions(m_characterWindowWidth+m_statsValueLabel_XOffset-4, m_statsTitleLabel_Y-130, 59, 26);
			m_pStatsValueLabel6->SetDimensions(m_characterWindowWidth+m_statsValueLabel_XOffset-4, m_statsTitleLabel_Y-156, 59, 26);

			if(m_pPlayer->GetPlayerStats()->GetNumPointsAvailable() > 0)
			{
				m_pStat1IncreaseButton->SetDimensions(m_characterWindowWidth+87, m_statsTitleLabel_Y-25, 16, 16);
				m_pStat2IncreaseButton->SetDimensions(m_characterWindowWidth+87, m_statsTitleLabel_Y-51, 16, 16);
				m_pStat3IncreaseButton->SetDimensions(m_characterWindowWidth+87, m_statsTitleLabel_Y-77, 16, 16);
				m_pStat4IncreaseButton->SetDimensions(m_characterWindowWidth+87, m_statsTitleLabel_Y-103, 16, 16);
				m_pStat5IncreaseButton->SetDimensions(m_characterWindowWidth+87, m_statsTitleLabel_Y-129, 16, 16);
				m_pStat6IncreaseButton->SetDimensions(m_characterWindowWidth+87, m_statsTitleLabel_Y-155, 16, 16);
			}
		}
		else
		{
			m_pTabOpenCloseButton->SetDimensions(m_characterWindowWidth, (m_characterWindowHeight/2)-(m_statsTabOpenButtonHeight/2), m_statsTabOpenButtonWidth, m_statsTabOpenButtonHeight);
			m_pStatsTab_Icon_Left->SetDimensions(0, (m_characterWindowHeight/2)-(m_statsTabHeight/2), m_statsTabWidth, m_statsTabHeight);
			m_pStatsTab_Icon_Right->SetDimensions(0, (m_characterWindowHeight/2)-(m_statsTabHeight/2), m_statsTabWidth, m_statsTabHeight);

			m_pCharacterTitleLabel->SetLocation(-5000, -5000);

			m_pStatsHeaderLabel1->SetLocation(-5000, -5000);
			m_pStatsHeaderLabel2->SetLocation(-5000, -5000);
			m_pStatsHeaderLabel3->SetLocation(-5000, -5000);
			m_pStatsHeaderLabel4->SetLocation(-5000, -5000);
			m_pStatsHeaderLabel5->SetLocation(-5000, -5000);
			m_pStatsHeaderLabel6->SetLocation(-5000, -5000);
			m_pStatsValueLabel1->SetLocation(-5000, -5000);
			m_pStatsValueLabel2->SetLocation(-5000, -5000);
			m_pStatsValueLabel3->SetLocation(-5000, -5000);
			m_pStatsValueLabel4->SetLocation(-5000, -5000);
			m_pStatsValueLabel5->SetLocation(-5000, -5000);
			m_pStatsValueLabel6->SetLocation(-5000, -5000);

			m_pStat1IncreaseButton->SetDimensions(-5000, -5000, 20, 20);
			m_pStat2IncreaseButton->SetDimensions(-5000, -5000, 20, 20);
			m_pStat3IncreaseButton->SetDimensions(-5000, -5000, 20, 20);
			m_pStat4IncreaseButton->SetDimensions(-5000, -5000, 20, 20);
			m_pStat5IncreaseButton->SetDimensions(-5000, -5000, 20, 20);
			m_pStat6IncreaseButton->SetDimensions(-5000, -5000, 20, 20);
		}
	}

	if(m_pCharacterWindow->GetDimensions().m_x < 200)
	{
		m_statsTabLeftSide = false;
	}
	else if(m_pCharacterWindow->GetDimensions().m_x+m_pCharacterWindow->GetDimensions().m_width > m_windowWidth - 200)
	{
		m_statsTabLeftSide = true;
	}
}

void CharacterGUI::_CharacterItemEntered(void *apData)
{
	CharacterSlotItem* lpCharacterSlotItem = (CharacterSlotItem*)apData;
	lpCharacterSlotItem->m_pCharacterGUI->CharacterItemEntered(lpCharacterSlotItem);
}

void CharacterGUI::CharacterItemEntered(CharacterSlotItem* pCharacterItem)
{
	ShowTooltip(pCharacterItem);
}

void CharacterGUI::_CharacterItemExited(void *apData)
{
	CharacterSlotItem* lpCharacterSlotItem = (CharacterSlotItem*)apData;
	lpCharacterSlotItem->m_pCharacterGUI->CharacterItemExited(lpCharacterSlotItem);
}

void CharacterGUI::CharacterItemExited(CharacterSlotItem* pCharacterItem)
{
	HideTooltip();
}

void CharacterGUI::_CharacterItemPressed(void *apData)
{
	CharacterSlotItem* lpCharacterSlotItem = (CharacterSlotItem*)apData;
	lpCharacterSlotItem->m_pCharacterGUI->CharacterItemPressed(lpCharacterSlotItem);
}

void CharacterGUI::CharacterItemPressed(CharacterSlotItem* pCharacterItem)
{
	m_pPressedCharacterItem = pCharacterItem;

	Dimensions lDimensions = m_pPressedCharacterItem->m_pInventoryIcon->GetDimensions();
	m_pressedX = lDimensions.m_x;
	m_pressedY = lDimensions.m_y;

	// Temporarily increase the depth of the dragged icon
	m_pPressedCharacterItem->m_pInventoryIcon->SetDepth(5.0f);
	//m_pPressedCharacterItem->m_pInventoryIcon->SetLocation(m_pressedX - 4, m_pressedY + 4);
	string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();
	string dropShadowIcon = "media/textures/gui/" + themeName + "/common/items/drop_shadow.tga";
	m_pPressedCharacterItem->m_pInventoryIcon->AddIcon(m_pRenderer, dropShadowIcon.c_str(), 64, 64, 64, 64, 4, -4, 0.5f);
	m_pCharacterWindow->DepthSortComponentChildren();

	HideTooltip();
}

void CharacterGUI::_CharacterItemReleased(void *apData)
{
	CharacterSlotItem* lpCharacterSlotItem = (CharacterSlotItem*)apData;
	lpCharacterSlotItem->m_pCharacterGUI->CharacterItemReleased(lpCharacterSlotItem);
}

bool needs_erasing(CharacterSlotItem* aI)
{
	bool needsErase = aI->m_erase;

	if(needsErase == true)
	{
		delete aI->m_pInventoryIcon;
		delete aI;
	}

	return needsErase;
}

void CharacterGUI::CharacterItemReleased(CharacterSlotItem* pCharacterItem)
{
	if(m_pPressedCharacterItem == NULL)
	{
		return;
	}

	m_pPressedCharacterItem = NULL;

	int x;
	int y;
	int width;
	int height;
	POINT lMouse = { VoxGame::GetInstance()->GetWindowCursorX(), (m_windowHeight-VoxGame::GetInstance()->GetWindowCursorY()) };
	bool unequip = false;

	// Check if we have released on an inventory slot
	for(int i = 0; i < InventoryGUI::MAX_NUM_SLOTS_VERTICAL; i++)
	{
		for(int j = 0; j < InventoryGUI::MAX_NUM_SLOTS_HORIZONTAL; j++)
		{
			m_pInventoryGUI->GetInventoryDimensions(j, i, &x, &y, &width, &height);
			GUIWindow* pInventoryWindow = m_pInventoryGUI->GetInventoryWindow();

			if(lMouse.x > pInventoryWindow->GetDimensions().m_x+x && lMouse.x < pInventoryWindow->GetDimensions().m_x+x+width && lMouse.y > pInventoryWindow->GetDimensions().m_y+y && lMouse.y < pInventoryWindow->GetDimensions().m_y+y+height)
			{
				if(pCharacterItem->m_pInventoryItem->m_equipped == true)
				{
					// Check if another inventory item already exists in this slot
					InventorySlotItem* pInventorySlotItem = m_pInventoryGUI->GetInventorySlotItem(j, i);
					if(pInventorySlotItem == NULL)
					{
						// We are unquipping an item that is in one of the equipment slots
						m_pInventoryManager->UnequipItem(j, i, pCharacterItem->m_pInventoryItem->m_equipSlot);

						m_pInventoryGUI->UnequipItem(pCharacterItem->m_pInventoryItem->m_equipSlot, pCharacterItem->m_pInventoryItem->m_left, pCharacterItem->m_pInventoryItem->m_right);

						// Set the new location for the released inventory icon
						pCharacterItem->m_slotX = j;
						pCharacterItem->m_slotY = i;
						pCharacterItem->m_pInventoryIcon->SetLocation(x, y);
						m_pActionBar->UpdateActionBarSlots(pCharacterItem->m_pInventoryItem, j, i);

						m_pCharacterWindow->RemoveComponent(pCharacterItem->m_pInventoryIcon);

						pCharacterItem->m_erase = true;
						m_vpInventorySlotItems.erase( remove_if(m_vpInventorySlotItems.begin(), m_vpInventorySlotItems.end(), needs_erasing), m_vpInventorySlotItems.end() );

						m_pInventoryManager->SetInventoryGUINeedsUpdate(true);
						m_pInventoryManager->SetCharacterGUINeedsUpdate(true);

						m_pPlayer->RefreshStatModifierCacheValues();

						unequip = true;

						VoxGame::GetInstance()->PlaySoundEffect(eSoundEffect_EquipMove);
					}
					else
					{
						if(pInventorySlotItem->m_pInventoryItem->m_equipSlot == pCharacterItem->m_pInventoryItem->m_equipSlot)
						{
							// Dual handed weapon checks
							if(pInventorySlotItem->m_pInventoryItem->m_equipSlot == EquipSlot_RightHand)
							{
								InventoryItem* pLeftHanded = m_pInventoryManager->GetInventoryItemForEquipSlot(EquipSlot_LeftHand);
								if(pInventorySlotItem->m_pInventoryItem->m_left || (pLeftHanded != NULL && pLeftHanded->m_right))
								{
									int slotX;
									int slotY;
									// Unequip the left hand slot since we are dual handed, OR the already equipped left hand item needs both hands
									m_pInventoryGUI->UnequipItem(EquipSlot_LeftHand, false, false);
									if(m_pInventoryManager->UnequipItemToFreeInventorySlot(EquipSlot_LeftHand, &slotX, &slotY) == false)
									{
										// We can't fit the other item in the inventory
									}
									else if(pLeftHanded != NULL)
									{
										m_pActionBar->UpdateActionBarSlots(pLeftHanded, slotX, slotY);
									}
								}
							}
							if(pInventorySlotItem->m_pInventoryItem->m_equipSlot == EquipSlot_LeftHand)
							{
								InventoryItem* pRightHanded = m_pInventoryManager->GetInventoryItemForEquipSlot(EquipSlot_RightHand);
								if(pInventorySlotItem->m_pInventoryItem->m_right || (pRightHanded != NULL && pRightHanded->m_left))
								{
									int slotX;
									int slotY;
									// Unequip the right hand slot since we are dual handed, OR the already equipped right hand item needs both hands
									m_pInventoryGUI->UnequipItem(EquipSlot_RightHand, false, false);
									if(m_pInventoryManager->UnequipItemToFreeInventorySlot(EquipSlot_RightHand, &slotX, &slotY) == false)
									{
										// We can't fit the other item in the inventory
									}
									else if(pRightHanded != NULL)
									{
										m_pActionBar->UpdateActionBarSlots(pRightHanded, slotX, slotY);
									}
								}
							}

							// We are swapping an equipped item for one in the inventory
							m_pInventoryGUI->UnequipItem(pCharacterItem->m_pInventoryItem->m_equipSlot, pCharacterItem->m_pInventoryItem->m_left, pCharacterItem->m_pInventoryItem->m_right);
							m_pInventoryManager->UnequipItem(j, i, pCharacterItem->m_pInventoryItem->m_equipSlot);

							// Equip the new item
							//m_pInventoryManager->EquipInventoryItem(pInventorySlotItem->m_slotX, pInventorySlotItem->m_slotY, pInventorySlotItem->m_pInventoryItem->m_equipSlot);
							m_pInventoryGUI->EquipItem(pInventorySlotItem);

							// Set the new location for the released inventory icon
							pCharacterItem->m_slotX = j;
							pCharacterItem->m_slotY = i;
							pCharacterItem->m_pInventoryIcon->SetLocation(x, y);
							m_pActionBar->UpdateActionBarSlots(pCharacterItem->m_pInventoryItem, j, i);

							m_pCharacterWindow->RemoveComponent(pCharacterItem->m_pInventoryIcon);

							pCharacterItem->m_erase = true;
							m_vpInventorySlotItems.erase( remove_if(m_vpInventorySlotItems.begin(), m_vpInventorySlotItems.end(), needs_erasing), m_vpInventorySlotItems.end() );

							m_pInventoryManager->SetInventoryGUINeedsUpdate(true);
							m_pInventoryManager->SetCharacterGUINeedsUpdate(true);

							m_pPlayer->RefreshStatModifierCacheValues();

							unequip = true;

							VoxGame::GetInstance()->PlaySoundEffect(eSoundEffect_EquipMove);
						}
					}
				}
			}
		}
	}

	if(unequip == false)
	{
		// Check if we have released on a loot slot
		if(m_pLootGUI->IsLoaded())
		{
			for(int i = 0; i < LootGUI::MAX_NUM_SLOTS_VERTICAL; i++)
			{
				for(int j = 0; j < LootGUI::MAX_NUM_SLOTS_HORIZONTAL; j++)
				{
					m_pLootGUI->GetLootDimensions(j, i, &x, &y, &width, &height);
					GUIWindow* pLootWindow = m_pLootGUI->GetLootWindow();
					// Check if we released (mouse cursor) in the boundary of another slot
					if(lMouse.x > pLootWindow->GetDimensions().m_x+x && lMouse.x < pLootWindow->GetDimensions().m_x+x+width && lMouse.y > pLootWindow->GetDimensions().m_y+y && lMouse.y < pLootWindow->GetDimensions().m_y+y+height)
					{
						if(m_pLootGUI->GetLootSlotItem(j, i) == NULL) // ONLY if an item doesn't already exist in the loot slot position
						{
							m_pInventoryGUI->UnequipItem(pCharacterItem->m_pInventoryItem->m_equipSlot, pCharacterItem->m_pInventoryItem->m_left, pCharacterItem->m_pInventoryItem->m_right);
							m_pInventoryManager->UnequipItemToLootGUI(pCharacterItem->m_pInventoryItem->m_equipSlot);

							m_pActionBar->RemoveInventoryItemFromActionBar(pCharacterItem->m_pInventoryItem->m_title);

							m_pLootGUI->AddLootItemFromInventory(pCharacterItem->m_pInventoryItem, j, i);

							m_pCharacterWindow->RemoveComponent(pCharacterItem->m_pInventoryIcon);

							pCharacterItem->m_erase = true;
							m_vpInventorySlotItems.erase( remove_if(m_vpInventorySlotItems.begin(), m_vpInventorySlotItems.end(), needs_erasing), m_vpInventorySlotItems.end() );

							m_pInventoryManager->SetInventoryGUINeedsUpdate(true);
							m_pInventoryManager->SetCharacterGUINeedsUpdate(true);

							m_pPlayer->RefreshStatModifierCacheValues();

							unequip = true;

							VoxGame::GetInstance()->PlaySoundEffect(eSoundEffect_EquipMove);
						}
					}
				}
			}
		}

		// Check if we released on a actionbar slot
		if(m_pActionBar->IsLoaded())
		{
			for(int i = 0; i < ActionBar::MAX_NUM_ACTION_SLOTS; i++)
			{
				m_pActionBar->GetActionSlotDimensions(i, &x, &y, &width, &height);

				// Check if we released (mouse cursor) in the boundary of another slot
				if(lMouse.x > x && lMouse.x < x+width && lMouse.y > y && lMouse.y < y+height)
				{
					m_pActionBar->AddItemToActionBar(pCharacterItem->m_pInventoryItem, i, pCharacterItem->m_slotX, pCharacterItem->m_slotY);
					m_pActionBar->ExportActionBar(m_pPlayer->GetName());

					VoxGame::GetInstance()->PlaySoundEffect(eSoundEffect_EquipMove);
				}
			}
		}
	}


	if(unequip == false)
	{
		// Revert depth back to normal for character icons
		pCharacterItem->m_pInventoryIcon->SetDepth(3.0f);
		m_pCharacterWindow->DepthSortComponentChildren();

		// Reset back to the original position
		pCharacterItem->m_pInventoryIcon->SetLocation(m_pressedX, m_pressedY);
		string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();
		string dropShadowIcon = "media/textures/gui/" + themeName + "/common/items/drop_shadow.tga";
		pCharacterItem->m_pInventoryIcon->RemoveIcon(dropShadowIcon.c_str());

		// Figure out if we are still in the same slot, to re-show the tooltip
		int x;
		int y;
		int width;
		int height;
		switch(pCharacterItem->m_pInventoryItem->m_equipSlot)
		{
		case EquipSlot_NoSlot : { x = 0; y = 0; width = 0; height = 0; } break;
		case EquipSlot_RightHand : { GetPlayerWeaponRightDimensions(&x, &y, &width, &height); } break;
		case EquipSlot_LeftHand : { GetPlayerWeaponLeftDimensions(&x, &y, &width, &height); } break;
		case EquipSlot_Head : { GetPlayerHeadDimensions(&x, &y, &width, &height); } break;
		case EquipSlot_Shoulders : { GetPlayerShoulderDimensions(&x, &y, &width, &height); } break;
		case EquipSlot_Body : { GetPlayerBodyDimensions(&x, &y, &width, &height); } break;
		case EquipSlot_Legs : { GetPlayerLegsDimensions(&x, &y, &width, &height); } break;
		case EquipSlot_Hand : { GetPlayerHandDimensions(&x, &y, &width, &height); } break;
		case EquipSlot_Feet : { GetPlayerFeetDimensions(&x, &y, &width, &height); } break;
		case EquipSlot_Accessory1 : { GetPlayerAccessory1Dimensions(&x, &y, &width, &height); } break;
		case EquipSlot_Accessory2 : { GetPlayerAccessory2Dimensions(&x, &y, &width, &height); } break;
		}
		if(lMouse.x > m_pCharacterWindow->GetDimensions().m_x+x && lMouse.x < m_pCharacterWindow->GetDimensions().m_x+x+width && lMouse.y > m_pCharacterWindow->GetDimensions().m_y+y && lMouse.y < m_pCharacterWindow->GetDimensions().m_y+y+height)
		{
			ShowTooltip(pCharacterItem);
		}
	}
}

void CharacterGUI::_CloseExitPressed(void *pData)
{
	CharacterGUI* lpCharacterGUI = (CharacterGUI*)pData;
	lpCharacterGUI->CloseExitPressed();
}

void CharacterGUI::CloseExitPressed()
{
	Unload();
}

void CharacterGUI::_OpenCloseTabPressed(void *pData)
{
	CharacterGUI* lpCharacterGUI = (CharacterGUI*)pData;
	lpCharacterGUI->OpenCloseTabPressed();
}

void CharacterGUI::OpenCloseTabPressed()
{
	m_statsTabsOpen = !m_statsTabsOpen;
}

void CharacterGUI::_ArrowLeftPressed(void *pData)
{
	CharacterGUI* lpCharacterGUI = (CharacterGUI*)pData;
	lpCharacterGUI->ArrowLeftPressed();
}

void CharacterGUI::ArrowLeftPressed()
{
	m_leftArrowPressed = true;
}

void CharacterGUI::_ArrowLeftReleased(void *pData)
{
	CharacterGUI* lpCharacterGUI = (CharacterGUI*)pData;
	lpCharacterGUI->ArrowLeftReleased();
}

void CharacterGUI::ArrowLeftReleased()
{
	m_leftArrowPressed = false;
}

void CharacterGUI::_ArrowRightPressed(void *pData)
{
	CharacterGUI* lpCharacterGUI = (CharacterGUI*)pData;
	lpCharacterGUI->ArrowRightPressed();
}

void CharacterGUI::ArrowRightPressed()
{
	m_rightArrowPressed = true;
}

void CharacterGUI::_ArrowRightReleased(void *pData)
{
	CharacterGUI* lpCharacterGUI = (CharacterGUI*)pData;
	lpCharacterGUI->ArrowRightReleased();
}

void CharacterGUI::ArrowRightReleased()
{
	m_rightArrowPressed = false;
}

void CharacterGUI::_EquipHoverAnimationFinished(void *pData)
{
	CharacterGUI* lpCharacterGUI = (CharacterGUI*)pData;
	lpCharacterGUI->EquipHoverAnimationFinished();
}

void CharacterGUI::EquipHoverAnimationFinished()
{
	StartEquipHoverAnimation();
}

void CharacterGUI::_Stat1IncreasePressed(void *apData)
{
	CharacterGUI* lpCharacterGUI = (CharacterGUI*)apData;
	lpCharacterGUI->Stat1IncreasePressed();
}

void CharacterGUI::Stat1IncreasePressed()
{
	m_pPlayer->GetPlayerStats()->SetStrengthStat(m_pPlayer->GetPlayerStats()->GetStrengthStat()+1);
	m_pPlayer->GetPlayerStats()->SetNumPointsAvailable(m_pPlayer->GetPlayerStats()->GetNumPointsAvailable()-1);
	UpdatePlayerStats();
}

void CharacterGUI::_Stat2IncreasePressed(void *apData)
{
	CharacterGUI* lpCharacterGUI = (CharacterGUI*)apData;
	lpCharacterGUI->Stat2IncreasePressed();
}

void CharacterGUI::Stat2IncreasePressed()
{
	m_pPlayer->GetPlayerStats()->SetDexterityStat(m_pPlayer->GetPlayerStats()->GetDexterityStat()+1);
	m_pPlayer->GetPlayerStats()->SetNumPointsAvailable(m_pPlayer->GetPlayerStats()->GetNumPointsAvailable()-1);
	UpdatePlayerStats();
}

void CharacterGUI::_Stat3IncreasePressed(void *apData)
{
	CharacterGUI* lpCharacterGUI = (CharacterGUI*)apData;
	lpCharacterGUI->Stat3IncreasePressed();
}

void CharacterGUI::Stat3IncreasePressed()
{
	m_pPlayer->GetPlayerStats()->SetIntelligenceStat(m_pPlayer->GetPlayerStats()->GetIntelligenceStat()+1);
	m_pPlayer->GetPlayerStats()->SetNumPointsAvailable(m_pPlayer->GetPlayerStats()->GetNumPointsAvailable()-1);
	UpdatePlayerStats();
}

void CharacterGUI::_Stat4IncreasePressed(void *apData)
{
	CharacterGUI* lpCharacterGUI = (CharacterGUI*)apData;
	lpCharacterGUI->Stat4IncreasePressed();
}

void CharacterGUI::Stat4IncreasePressed()
{
	m_pPlayer->GetPlayerStats()->SetVitalityStat(m_pPlayer->GetPlayerStats()->GetVitalityStat()+1);
	m_pPlayer->GetPlayerStats()->SetNumPointsAvailable(m_pPlayer->GetPlayerStats()->GetNumPointsAvailable()-1);
	UpdatePlayerStats();
}

void CharacterGUI::_Stat5IncreasePressed(void *apData)
{
	CharacterGUI* lpCharacterGUI = (CharacterGUI*)apData;
	lpCharacterGUI->Stat5IncreasePressed();
}

void CharacterGUI::Stat5IncreasePressed()
{
	m_pPlayer->GetPlayerStats()->SetArmorStat(m_pPlayer->GetPlayerStats()->GetArmorStat()+1);
	m_pPlayer->GetPlayerStats()->SetNumPointsAvailable(m_pPlayer->GetPlayerStats()->GetNumPointsAvailable()-1);
	UpdatePlayerStats();
}

void CharacterGUI::_Stat6IncreasePressed(void *apData)
{
	CharacterGUI* lpCharacterGUI = (CharacterGUI*)apData;
	lpCharacterGUI->Stat6IncreasePressed();
}

void CharacterGUI::Stat6IncreasePressed()
{
	m_pPlayer->GetPlayerStats()->SetLuckStat(m_pPlayer->GetPlayerStats()->GetLuckStat()+1);
	m_pPlayer->GetPlayerStats()->SetNumPointsAvailable(m_pPlayer->GetPlayerStats()->GetNumPointsAvailable()-1);
	UpdatePlayerStats();
}
