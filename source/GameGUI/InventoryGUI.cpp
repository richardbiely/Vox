// ******************************************************************************
// Filename:    InventoryGUI.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 18/11/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "InventoryGUI.h"
#include "CharacterGUI.h"
#include "LootGUI.h"
#include "ActionBar.h"

#include "../frontend/FrontendManager.h"
#include "../Player/Player.h"
#include "../VoxGame.h"
#include "../Items/ItemManager.h"

#include "../utils/FileUtils.h"
#include "../utils/Random.h"


InventoryGUI::InventoryGUI(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontendManager, ChunkManager* pChunkManager, Player* pPlayer, InventoryManager* pInventoryManager, int windowWidth, int windowHeight)
{
	m_pRenderer = pRenderer;
	m_pGUI = pGUI;
	m_pFrontendManager = pFrontendManager;
	m_pChunkManager = pChunkManager;
	m_pPlayer = pPlayer;
	m_pInventoryManager = pInventoryManager;

	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;

	// Inventory Window
	m_pInventoryWindow = new GUIWindow(m_pRenderer, m_pFrontendManager->GetFrontendFont_Medium(), "Inventory");
	m_pInventoryWindow->AllowMoving(true);
	m_pInventoryWindow->AllowClosing(false);
	m_pInventoryWindow->AllowMinimizing(false);
	m_pInventoryWindow->AllowScrolling(false);
	m_pInventoryWindow->SetRenderTitleBar(true);
	m_pInventoryWindow->SetRenderWindowBackground(true);
	m_pInventoryWindow->SetApplicationDimensions(m_windowWidth, m_windowHeight);
	m_pInventoryWindow->Hide();

	m_pTitleBarIcon = new Icon(m_pRenderer, "", 44, 44);
	m_pTitleBarIcon->SetDepth(4.0f);

	m_pInventoryWindowBackgroundIcon = new Icon(m_pRenderer, "", 400, 211);
	m_pInventoryWindowBackgroundIcon->SetDepth(1.0f);

	m_pTitleBarBackgroundIcon = new Icon(m_pRenderer, "", 133, 35);
	m_pTitleBarBackgroundIcon->SetDepth(1.0f);

	m_pCloseExitButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pCloseExitButton->SetLabelOffset(0, 5);
	m_pCloseExitButton->SetCallBackFunction(_CloseExitPressed);
	m_pCloseExitButton->SetCallBackData(this);

	m_pDestroyIcon = new Icon(m_pRenderer, "", 175, 65);
	m_pDestroyIcon->SetDepth(2.1f);

	char destroyText[] = "DESTROY";
	m_pDestroyLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_50(), destroyText, Colour(1.0f, 1.0f, 1.0f, 0.25f));
	m_pDestroyLabel->SetOutline(true);
	m_pDestroyLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pDestroyLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_50_Outline());
	m_pDestroyLabel->SetDepth(3.0f);

	m_pDropIcon = new Icon(m_pRenderer, "", 175, 65);
	m_pDropIcon->SetDepth(2.1f);

	char dropText[] = "DROP";
	m_pDropLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_50(), dropText, Colour(1.0f, 1.0f, 1.0f, 0.25f));
	m_pDropLabel->SetOutline(true);
	m_pDropLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pDropLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_50_Outline());
	m_pDropLabel->SetDepth(3.0f);

	m_pInventoryWindow->SetBackgroundIcon(m_pInventoryWindowBackgroundIcon);
	m_pInventoryWindow->SetTitlebarBackgroundIcon(m_pTitleBarBackgroundIcon);

	m_pInventoryWindow->AddComponent(m_pTitleBarIcon);
	m_pInventoryWindow->AddComponent(m_pCloseExitButton);

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

	SetWindowDimensions(m_windowWidth, m_windowHeight);

	m_pInventoryItemToDelete = nullptr;

	// Load delay
	m_loadDelay = false;
	m_loadDelayTime = 0.0f;

	m_loaded = false;
}

InventoryGUI::~InventoryGUI()
{
	delete m_pInventoryWindow;
	delete m_pTitleBarIcon;
	delete m_pTitleBarBackgroundIcon;
	delete m_pInventoryWindowBackgroundIcon;
	delete m_pCloseExitButton;
	delete m_pDestroyIcon;
	delete m_pDestroyLabel;
	delete m_pDropIcon;
	delete m_pDropLabel;

	delete m_pInventoryBackgroundSlotBorder_Common;
	delete m_pInventoryBackgroundSlotBorder_Uncommon;
	delete m_pInventoryBackgroundSlotBorder_Magical;
	delete m_pInventoryBackgroundSlotBorder_Rare;
	delete m_pInventoryBackgroundSlotBorder_Epic;

	// Tooltip
	delete m_pTooltipBackground_Common;
	delete m_pTooltipBackground_Uncommon;
	delete m_pTooltipBackground_Magical;
	delete m_pTooltipBackground_Rare;
	delete m_pTooltipBackground_Epic;
	delete m_pTooltipNameLabel;
	delete m_pTooltipDescriptionLabel;
	delete m_pTooltipSlotLabel;
	delete m_pTooltipQualityLabel;

	// Popup
	delete m_popupTitle;
	delete m_popupText;
	delete m_pPopupConfirmButton;
	delete m_pPopupCancelButton;
	delete m_pPopupBackgroundIcon;
}

void InventoryGUI::SetCharacterGUI(CharacterGUI* pCharacterGUI)
{
	m_pCharacterGUI = pCharacterGUI;
}

void InventoryGUI::SetLootGUI(LootGUI* pLootGUI)
{
	m_pLootGUI = pLootGUI;
}

void InventoryGUI::SetActionBar(ActionBar* pActionBar)
{
	m_pActionBar = pActionBar;
}

void InventoryGUI::SetItemManager(ItemManager *pItemManager)
{
	m_pItemManager = pItemManager;
}

// Skinning the GUI
void InventoryGUI::SkinGUI()
{
	string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();

	string iconName = "media/textures/gui/" + themeName + "/InventoryGUI/inventory_title_icon.tga";
	m_pTitleBarIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/InventoryGUI/inventory_window_background.tga";
	m_pInventoryWindowBackgroundIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/InventoryGUI/titlebar_background.tga";
	m_pTitleBarBackgroundIcon->SetIcon(iconName);

	m_pInventoryWindow->SetBackgroundIcon(m_pInventoryWindowBackgroundIcon);
	m_pInventoryWindow->SetTitlebarBackgroundIcon(m_pTitleBarBackgroundIcon);

	Point location = m_pInventoryWindow->GetLocation();
	m_pInventoryWindow->SetDimensions(location.m_x, location.m_y, m_inventoryWindowWidth, m_inventoryWindowHeight);
	m_pInventoryWindow->SetTitleBarDimensions(0, 0, m_titlebarWidth, m_titlebarHeight);

	iconName = "media/textures/gui/" + themeName + "/InventoryGUI/delete_background.tga";
	m_pDestroyIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/InventoryGUI/drop_background.tga";
	m_pDropIcon->SetIcon(iconName);

	iconName = "media/textures/gui/" + themeName + "/common/popup_background.tga";
	m_pPopupBackgroundIcon->SetIcon(iconName);

	m_pFrontendManager->SetButtonIcons(m_pPopupConfirmButton, ButtonSize_110x47);
	m_pFrontendManager->SetButtonIcons(m_pPopupCancelButton, ButtonSize_110x47);

	m_pCloseExitButton->SetDefaultIcon(m_pFrontendManager->GetCloseExitButtonIcon());
	m_pCloseExitButton->SetHoverIcon(m_pFrontendManager->GetCloseExitButtonIcon_Hover());
	m_pCloseExitButton->SetSelectedIcon(m_pFrontendManager->GetCloseExitButtonIcon_Pressed());
	m_pCloseExitButton->SetDisabledIcon(m_pFrontendManager->GetCloseExitButtonIcon());

	iconName = "media/textures/gui/" + themeName + "/common/items/border_common.tga";
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

	m_pPopupConfirmButton->SetNormalLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pPopupConfirmButton->SetHoverLabelColour(m_pFrontendManager->GetHoverFontColour());
	m_pPopupConfirmButton->SetPressedLabelColour(m_pFrontendManager->GetPressedFontColour());
	m_pPopupCancelButton->SetNormalLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pPopupCancelButton->SetHoverLabelColour(m_pFrontendManager->GetHoverFontColour());
	m_pPopupCancelButton->SetPressedLabelColour(m_pFrontendManager->GetPressedFontColour());

	m_pInventoryManager->SetInventoryGUINeedsUpdate(true);
}

void InventoryGUI::UnSkinGUI()
{

}

// Loading
void InventoryGUI::Load(bool loadDelay, float loadDelayTime)
{
	m_loadDelay = loadDelay;
	m_loadDelayTime = loadDelayTime;

	if(m_pInventoryManager->InventoryGUINeedsUpdate())
	{
		DeleteInventoryItems();
		CreateInventoryItems();
		UpdateActionBar();

		for(unsigned int i = 0; i < m_vpInventorySlotItems.size(); i++)
		{
			m_pInventoryWindow->AddComponent(m_vpInventorySlotItems[i]->m_pInventoryIcon);
		}
	}

	if (m_loadDelay == false)
	{
		m_pGUI->AddWindow(m_pInventoryWindow);
		m_pInventoryWindow->Show();
	}

	m_pPopupConfirmButton->SetLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pPopupCancelButton->SetLabelColour(m_pFrontendManager->GetNormalFontColour());

	m_pressedX = 0;
	m_pressedY = 0;
	m_pPressedInventoryItem = nullptr;

	m_toolTipVisible = false;
	m_tooltipAppearDelayTimer = 0.0f;
	m_toolTipComponentsAdded = false;
	m_tooltipQuality = ItemQuality_Common;

	m_pInventoryItemToDelete = nullptr;

	m_loaded = true;
}

void InventoryGUI::Unload()
{
	m_loadDelay = false;
	m_loadDelayTime = 0.0f;

	HideTooltip();

	m_pGUI->RemoveWindow(m_pInventoryWindow);

	if(m_pPressedInventoryItem != nullptr)
	{
		m_pPressedInventoryItem->m_pInventoryIcon->SetDepth(3.0f);
		m_pInventoryWindow->DepthSortComponentChildren();
		m_pPressedInventoryItem->m_pInventoryIcon->SetLocation(m_pressedX, m_pressedY);
	}

	ClosePopup();

	m_pGUI->RemoveComponent(m_pPopupBackgroundIcon);

	m_pInventoryWindow->RemoveComponent(m_pDestroyIcon);
	m_pInventoryWindow->RemoveComponent(m_pDestroyLabel);
	m_pInventoryWindow->RemoveComponent(m_pDropIcon);
	m_pInventoryWindow->RemoveComponent(m_pDropLabel);

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

bool InventoryGUI::IsLoadDelayed()
{
	return (m_loadDelay == true && m_loadDelayTime > 0.0f);
}

void InventoryGUI::SetWindowDimensions(int windowWidth, int windowHeight)
{
	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;

	m_inventoryWindowWidth = 412;
	m_inventoryWindowHeight = 212;
	m_titlebarWidth = 153;
	m_titlebarHeight = 35;

	m_popupWidth = 270;
	m_popupHeight = 200;
	m_popupBorderSpacer = 25;
	m_popupTitleSpacer = 35;
	m_popupIconSize = 50;
	m_popupIconSpacer = 10;

	m_pInventoryWindow->SetDimensions(m_windowWidth - 434, 175, m_inventoryWindowWidth, m_inventoryWindowHeight);
	m_pInventoryWindow->SetTitleBarDimensions(0, 0, m_titlebarWidth, m_titlebarHeight);
	m_pInventoryWindow->SetTitleOffset(50, 5);
	m_pInventoryWindow->SetApplicationDimensions(m_windowWidth, m_windowHeight);
	m_pInventoryWindow->SetApplicationBorder(25, 15, 10, 40);

	m_pTitleBarIcon->SetDimensions(0, m_inventoryWindowHeight, 44, 44);
	m_pCloseExitButton->SetDimensions(m_inventoryWindowWidth-32, m_inventoryWindowHeight, 32, 32);

	int x;
	int y;
	int width;
	int height;
	GetDestroySlotDimensions(&x, &y, &width, &height);
	m_pDestroyIcon->SetDimensions(x, y, width, height);

	int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_50(), "%s", m_pDestroyLabel->GetText().c_str());
	int textHeight = m_pRenderer->GetFreeTypeTextHeight(m_pFrontendManager->GetFrontendFont_50(), "%s", m_pDestroyLabel->GetText().c_str());
	m_pDestroyLabel->SetLocation(x + (int)((width*0.5f) - (textWidth*0.5f)), y + (int)((height*0.5f) - (textHeight*0.5f))+5);

	GetDropSlotDimensions(&x, &y, &width, &height);
	m_pDropIcon->SetDimensions(x, y, width, height);

	textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_50(), "%s", m_pDropLabel->GetText().c_str());
	textHeight = m_pRenderer->GetFreeTypeTextHeight(m_pFrontendManager->GetFrontendFont_50(), "%s", m_pDropLabel->GetText().c_str());
	m_pDropLabel->SetLocation(x + (int)((width*0.5f) - (textWidth*0.5f)), y + (int)((height*0.5f) - (textHeight*0.5f))+5);

	// Popup
	m_pPopupBackgroundIcon->SetDimensions((int)((windowWidth*0.5f)-(m_popupWidth*0.5f)), (int)((windowHeight*0.5f)-(m_popupHeight*0.5f))+100, m_popupWidth, m_popupHeight);
}

void InventoryGUI::GetInventoryDimensions(int indexX, int indexY, int* x, int* y, int* width, int *height)
{
	int slotSize = 64;
	int slotSpacer = 4;
	int borderSize = 4;

	int xSlotPos = borderSize + ((slotSize + slotSpacer) * indexX);
	int ySlotPos = 4 + borderSize + ((slotSize + slotSpacer) * indexY);

	*x = xSlotPos;
	*y = ySlotPos;

	*width = slotSize;
	*height = slotSize;
}

void InventoryGUI::GetDestroySlotDimensions(int* x, int* y, int* width, int* height)
{
	*x = 220;
	*y = -75;

	*width = 175;
	*height = 65;
}

void InventoryGUI::GetDropSlotDimensions(int* x, int* y, int* width, int* height)
{
	*x = 10;
	*y = -75;

	*width = 175;
	*height = 65;
}

GUIWindow* InventoryGUI::GetInventoryWindow()
{
	return m_pInventoryWindow;
}

void InventoryGUI::CreateInventoryItems()
{
	// Item draggable Buttons
	for(int i = 0; i < MAX_NUM_SLOTS_VERTICAL; i++)
	{
		for(int j = 0; j < MAX_NUM_SLOTS_HORIZONTAL; j++)
		{
			InventoryItem* pItem = m_pInventoryManager->GetInventoryItemForSlot(j, i);

			if(pItem != nullptr)
			{
				int x;
				int y;
				int width;
				int height;
				GetInventoryDimensions(j, i, &x, &y, &width, &height);

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
				switch(pItem->m_itemQuality)
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
					pNewSlotItem->AddText(m_pRenderer, m_pFrontendManager->GetFrontendFont_18(), m_pFrontendManager->GetFrontendFont_18_Outline(), lQuantity, Colour(1.0f, 1.0f, 1.0f, 1.0f), width-10-textWidth, 8, true, Colour(0.0f, 0.0f, 0.0f, 1.0f));
				}

				InventorySlotItem* pNewItem = new InventorySlotItem();
				pNewItem->m_pInventoryGUI = this;
				pNewItem->m_pInventoryItem = pItem;
				pNewItem->m_pInventoryIcon = pNewSlotItem;
				pNewItem->m_slotX = j;
				pNewItem->m_slotY = i;
				pNewItem->m_dropshadowAdded = false;
				pNewItem->m_erase = false;
				m_vpInventorySlotItems.push_back(pNewItem);

				pNewSlotItem->SetPressedCallBackFunction(_InventoryItemPressed);
				pNewSlotItem->SetPressedCallBackData(pNewItem);

				pNewSlotItem->SetReleasedCallBackFunction(_InventoryItemReleased);
				pNewSlotItem->SetReleasedCallBackData(pNewItem);

				pNewSlotItem->SetEnterCallBackFunction(_InventoryItemEntered);
				pNewSlotItem->SetEnterCallBackData(pNewItem);

				pNewSlotItem->SetExitCallBackFunction(_InventoryItemExited);
				pNewSlotItem->SetExitCallBackData(pNewItem);
			}
		}
	}

	m_pInventoryManager->SetInventoryGUINeedsUpdate(false);
}

void InventoryGUI::DeleteInventoryItems()
{
	// Clear item draggable buttons
	for(unsigned int i = 0; i < m_vpInventorySlotItems.size(); i++)
	{
		m_pInventoryWindow->RemoveComponent(m_vpInventorySlotItems[i]->m_pInventoryIcon);
		delete m_vpInventorySlotItems[i]->m_pInventoryIcon;
		m_vpInventorySlotItems[i]->m_pInventoryIcon = 0;

		delete m_vpInventorySlotItems[i];
		m_vpInventorySlotItems[i] = 0;
	}
	m_vpInventorySlotItems.clear();
}

void InventoryGUI::UpdateActionBar()
{
	// We need to relink the action bar to the inventory GUI, since we have deleted and re-created the inventory buttons
	for(unsigned int i = 0; i < m_vpInventorySlotItems.size(); i++)
	{
		for(int j = 0; j < ActionBar::MAX_NUM_ACTION_SLOTS; j++)
		{
			ActionButtonItem* pActionBarItem = m_pActionBar->GetActionButtonForSlot(j);

			if(pActionBarItem != nullptr)
			{
				if(pActionBarItem->m_inventoryX == m_vpInventorySlotItems[i]->m_slotX && pActionBarItem->m_inventoryY == m_vpInventorySlotItems[i]->m_slotY)
				{
					if(pActionBarItem->m_inventoryX == -1 && pActionBarItem->m_inventoryY == -1)
					{
						if(pActionBarItem->m_equipSlot == m_vpInventorySlotItems[i]->m_pInventoryItem->m_equipSlot)
						{
							// In the situation where the loaded action button has a -1, -1 slot index, we also need to check the item slot type before assigning this pointer
							pActionBarItem->m_itemTitle = m_vpInventorySlotItems[i]->m_pInventoryItem->m_title;
						}
					}
					else
					{
						pActionBarItem->m_itemTitle = m_vpInventorySlotItems[i]->m_pInventoryItem->m_title;
					}
				}
			}
		}
	}
}

// Tooltips
void InventoryGUI::UpdateToolTipAppear(float dt)
{
	if(m_toolTipVisible)
	{
		if(m_tooltipAppearDelayTimer <= 0.0f)
		{
			if(m_toolTipComponentsAdded == false)
			{
				switch(m_tooltipQuality)
				{
					case ItemQuality_Common :  { m_pInventoryWindow->AddComponent(m_pTooltipBackground_Common); break; }
					case ItemQuality_Uncommon :  { m_pInventoryWindow->AddComponent(m_pTooltipBackground_Uncommon); break; }
					case ItemQuality_Magical :  { m_pInventoryWindow->AddComponent(m_pTooltipBackground_Magical); break; }
					case ItemQuality_Rare :  { m_pInventoryWindow->AddComponent(m_pTooltipBackground_Rare); break; }
					case ItemQuality_Epic :  { m_pInventoryWindow->AddComponent(m_pTooltipBackground_Epic); break; }
				}

				m_pInventoryWindow->AddComponent(m_pTooltipNameLabel);
				m_pInventoryWindow->AddComponent(m_pTooltipDescriptionLabel);
				m_pInventoryWindow->AddComponent(m_pTooltipSlotLabel);
				m_pInventoryWindow->AddComponent(m_pTooltipQualityLabel);

				m_toolTipComponentsAdded = true;
			}
		}
		else
		{
			m_tooltipAppearDelayTimer -= dt;
		}
	}
}

void InventoryGUI::ShowTooltip(InventorySlotItem* pInventoryItem)
{
	if(m_toolTipVisible == true)
	{
		return;
	}

	// Set the focused window when we show a tooltip
	m_pInventoryWindow->SetFocusWindow();

	// Replace the tooltip name
	m_pTooltipNameLabel->SetText(pInventoryItem->m_pInventoryItem->m_title);

	// Replace the tooltip description
	string DescriptionText = pInventoryItem->m_pInventoryItem->m_description + pInventoryItem->m_pInventoryItem->GetStatsAttributeString();
	m_pTooltipDescriptionLabel->SetText(DescriptionText);

	// Replace the tooltip equipslot text
	char slotText[32];
	switch(pInventoryItem->m_pInventoryItem->m_equipSlot)
	{
	case EquipSlot_NoSlot: { strcpy(slotText, ""); break; }
	case EquipSlot_LeftHand:
		{
			if(pInventoryItem->m_pInventoryItem->m_right)
			{
				strcpy(slotText, "Two Handed");
			}
			else
			{
				strcpy(slotText, "Left Hand");
			}

			break;
		}
	case EquipSlot_RightHand:
		{
			if(pInventoryItem->m_pInventoryItem->m_left)
			{
				strcpy(slotText, "Two Handed");
			}
			else
			{
				strcpy(slotText, "Right Hand");
			}

			break;
		}
	case EquipSlot_Head: { strcpy(slotText, "Head"); break; }
	case EquipSlot_Shoulders: { strcpy(slotText, "Shoulders"); break; }
	case EquipSlot_Body: { strcpy(slotText, "Body"); break; }
	case EquipSlot_Legs: { strcpy(slotText, "Legs"); break; }
	case EquipSlot_Hand: { strcpy(slotText, "Hand"); break; }
	case EquipSlot_Feet: { strcpy(slotText, "Feet"); break; }
	case EquipSlot_Accessory1: { strcpy(slotText, "Accessory 1"); break; }
	case EquipSlot_Accessory2: { strcpy(slotText, "Accessory 2"); break; }
	}
	m_pTooltipSlotLabel->SetText(slotText);

	// Replace the tooltip quality text
	char qualityText[32];
	Colour qualityColour;
	switch(pInventoryItem->m_pInventoryItem->m_itemQuality)
	{
	case ItemQuality_Common :  { strcpy(qualityText, "Common"); qualityColour = Colour(0.5f, 0.5f, 0.5f, 1.0f); break; }
	case ItemQuality_Uncommon :  { strcpy(qualityText, "Uncommon"); qualityColour = Colour(0.95f, 1.0f, 0.2f, 1.0f); break; }
	case ItemQuality_Magical :  { strcpy(qualityText, "Magical"); qualityColour = Colour(0.0f, 1.0f, 0.0f, 1.0f); break; }
	case ItemQuality_Rare :  { strcpy(qualityText, "Rare"); qualityColour = Colour(0.0f, 0.5f, 1.0f, 1.0f); break; }
	case ItemQuality_Epic :  { strcpy(qualityText, "Epic"); qualityColour = Colour(0.64f, 0.2f, 0.93f, 1.0f); break; }
	}
	m_pTooltipQualityLabel->SetText(qualityText);
	m_pTooltipQualityLabel->SetColour(qualityColour);

	m_pTooltipNameLabel->SetColour(qualityColour);

	// Set tooltip dimensions
	m_tooltipWidth = 200;
	m_tooltipHeight = 220;
	m_tooltipDescBorder = 15;

	int x;
	int y;
	int width;
	int height;
	GetInventoryDimensions(pInventoryItem->m_slotX, pInventoryItem->m_slotY, &x, &y, &width, &height);

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

	m_tooltipQuality = pInventoryItem->m_pInventoryItem->m_itemQuality;

	m_tooltipAppearDelayTimer = m_pFrontendManager->GetToolTipAppearDelay();

	m_toolTipVisible = true;
	m_toolTipComponentsAdded = false;
}

void InventoryGUI::HideTooltip()
{
	m_pInventoryWindow->RemoveComponent(m_pTooltipBackground_Common);
	m_pInventoryWindow->RemoveComponent(m_pTooltipBackground_Uncommon);
	m_pInventoryWindow->RemoveComponent(m_pTooltipBackground_Magical);
	m_pInventoryWindow->RemoveComponent(m_pTooltipBackground_Rare);
	m_pInventoryWindow->RemoveComponent(m_pTooltipBackground_Epic);
	m_pInventoryWindow->RemoveComponent(m_pTooltipNameLabel);
	m_pInventoryWindow->RemoveComponent(m_pTooltipDescriptionLabel);
	m_pInventoryWindow->RemoveComponent(m_pTooltipSlotLabel);
	m_pInventoryWindow->RemoveComponent(m_pTooltipQualityLabel);

	m_toolTipVisible = false;
}

void InventoryGUI::OpenPopup(string popupTitle, string popupText)
{
	m_pPopupConfirmButton->SetLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pPopupCancelButton->SetLabelColour(m_pFrontendManager->GetNormalFontColour());

	int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_40(), "%s", popupTitle.c_str());
	m_popupTitle->SetLocation((int)((m_windowWidth*0.5f)-(textWidth*0.5f)), (int)((m_windowHeight*0.5f)+(m_popupHeight*0.5f))-m_popupTitleSpacer-m_popupBorderSpacer+100);
	m_popupText->SetDimensions((int)((m_windowWidth*0.5f)-(m_popupWidth*0.5f))+m_popupBorderSpacer, (int)((m_windowHeight*0.5f)-(m_popupHeight*0.5f))+100, m_popupWidth-(m_popupBorderSpacer*2), m_popupHeight-m_popupBorderSpacer-m_popupTitleSpacer);
	m_pPopupConfirmButton->SetDimensions((int)((m_windowWidth*0.5f)+(m_popupWidth*0.5f)) - (int)(m_popupBorderSpacer*0.5f) - 110, (int)((m_windowHeight*0.5f)-(m_popupIconSize*0.5f))-50+100, 110, 47);
	m_pPopupCancelButton->SetDimensions((int)((m_windowWidth*0.5f)-(m_popupWidth*0.5f)) + (int)(m_popupBorderSpacer*0.5f), (int)((m_windowHeight*0.5f)-(m_popupIconSize*0.5f))-50+100, 110, 47);

	m_popupTitle->SetText(popupTitle);
	m_popupText->SetText(popupText);

	m_pGUI->AddComponent(m_popupTitle);
	m_pGUI->AddComponent(m_popupText);
	m_pGUI->AddComponent(m_pPopupConfirmButton);
	m_pGUI->AddComponent(m_pPopupCancelButton);
	m_pGUI->AddComponent(m_pPopupBackgroundIcon);
}

void InventoryGUI::ClosePopup()
{
	m_pGUI->RemoveComponent(m_popupTitle);
	m_pGUI->RemoveComponent(m_popupText);
	m_pGUI->RemoveComponent(m_pPopupConfirmButton);
	m_pGUI->RemoveComponent(m_pPopupCancelButton);
	m_pGUI->RemoveComponent(m_pPopupBackgroundIcon);
}

bool InventoryGUI::IsLoaded()
{
	return m_loaded;
}

InventorySlotItem* InventoryGUI::GetInventorySlotItem(int x, int y)
{
	for(unsigned int i = 0; i < m_vpInventorySlotItems.size(); i++)
	{
		if(m_vpInventorySlotItems[i]->m_slotX == x && m_vpInventorySlotItems[i]->m_slotY == y)
		{
			return m_vpInventorySlotItems[i];
		}
	}

	return nullptr;
}

InventorySlotItem* InventoryGUI::GetInventorySlotItemEquipped(EquipSlot equipSlot)
{
	InventoryItem* pInventoryItem = m_pInventoryManager->GetInventoryItemForEquipSlot(equipSlot);

	if(pInventoryItem != nullptr)
	{
		for(unsigned int i = 0; i < m_vpInventorySlotItems.size(); i++)
		{
			if(m_vpInventorySlotItems[i]->m_pInventoryItem == pInventoryItem)
			{
				return m_vpInventorySlotItems[i];
			}
		}
	}

	return nullptr;
}

void InventoryGUI::SetEquippedItem(EquipSlot equipSlot, string title)
{
	m_pEquippedItems[equipSlot] = title;
}

void InventoryGUI::EquipItem(InventoryItem* pInventoryItem, int inventoryX, int inventoryY)
{
	// Set the player to know that we have equipped an item
	m_pPlayer->EquipItem(pInventoryItem);

	// If we already have an item in this equipment slot, switch it out
	if(m_pEquippedItems[pInventoryItem->m_equipSlot].empty())
	{
		m_pActionBar->UpdateActionBarSlots(m_pEquippedItems[pInventoryItem->m_equipSlot], inventoryX, inventoryY);
	}

	m_pActionBar->UpdateActionBarSlots(pInventoryItem, -1, -1);

	// Equip the new item
	m_pEquippedItems[pInventoryItem->m_equipSlot] = pInventoryItem->m_title;
}

void InventoryGUI::EquipItem(LootSlotItem* pLootItem)
{
	// Set the player to know that we have equipped an item
	m_pPlayer->EquipItem(pLootItem->m_pInventoryItem);
}

void InventoryGUI::EquipItem(InventorySlotItem* pInventoryItem)
{
	// Set the player to know that we have equipped an item
	m_pPlayer->EquipItem(pInventoryItem->m_pInventoryItem);

	// If we already have an item in this equipment slot, switch it out
	if(m_pEquippedItems[pInventoryItem->m_pInventoryItem->m_equipSlot].empty())
	{
		m_pActionBar->UpdateActionBarSlots(m_pEquippedItems[pInventoryItem->m_pInventoryItem->m_equipSlot], pInventoryItem->m_slotX, pInventoryItem->m_slotY);
	}

	m_pActionBar->UpdateActionBarSlots(pInventoryItem->m_pInventoryItem, -1, -1);

	// Equip the new item
	m_pEquippedItems[pInventoryItem->m_pInventoryItem->m_equipSlot] = pInventoryItem->m_pInventoryItem->m_title;
	pInventoryItem->m_slotX = -1;
	pInventoryItem->m_slotY = -1;
}

void InventoryGUI::UnequipItem(EquipSlot equipSlot, bool left, bool right)
{
	m_pEquippedItems[equipSlot].clear();

	m_pPlayer->UnequipItem(equipSlot, left, right);
}

void InventoryGUI::Update(float dt)
{
	if (m_loadDelay == true)
	{
		if (m_loadDelayTime <= 0.0f)
		{
			m_loadDelay = false;
			m_pGUI->AddWindow(m_pInventoryWindow);
			m_pInventoryWindow->Show();
		}
		else
		{
			m_loadDelayTime -= dt;
		}
	}

	UpdateToolTipAppear(dt);

	// Check if the inventory GUI needs update (we have moved items in the inventory or got new items)
	if(m_pInventoryManager->InventoryGUINeedsUpdate() && IsLoaded() == true)
	{
		m_pGUI->RemoveWindow(m_pInventoryWindow);

		DeleteInventoryItems();
		CreateInventoryItems();
		UpdateActionBar();

		for(unsigned int i = 0; i < m_vpInventorySlotItems.size(); i++)
		{
			m_pInventoryWindow->AddComponent(m_vpInventorySlotItems[i]->m_pInventoryIcon);
		}

		m_pGUI->AddWindow(m_pInventoryWindow);
		m_pInventoryWindow->Show();
	}
}

void InventoryGUI::_InventoryItemPressed(void *apData)
{
	InventorySlotItem* lpInventoryItem = (InventorySlotItem*)apData;
	lpInventoryItem->m_pInventoryGUI->InventoryItemPressed(lpInventoryItem);
}

void InventoryGUI::InventoryItemPressed(InventorySlotItem* pInventoryItem)
{
	m_pPressedInventoryItem = pInventoryItem;

	Dimensions lDimensions = m_pPressedInventoryItem->m_pInventoryIcon->GetDimensions();
	m_pressedX = lDimensions.m_x;
	m_pressedY = lDimensions.m_y;

	m_pInventoryWindow->AddComponent(m_pDestroyIcon);
	m_pInventoryWindow->AddComponent(m_pDestroyLabel);
	m_pInventoryWindow->AddComponent(m_pDropIcon);
	m_pInventoryWindow->AddComponent(m_pDropLabel);

	// Temporarily increase the depth of the dragged icon
	if (m_pPressedInventoryItem->m_dropshadowAdded == false)
	{
		m_pPressedInventoryItem->m_dropshadowAdded = true;
		m_pPressedInventoryItem->m_pInventoryIcon->SetDepth(5.0f);
		//m_pPressedInventoryItem->m_pInventoryIcon->SetLocation(m_pressedX - 4, m_pressedY + 4);
		string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();
		string dropShadowIcon = "media/textures/gui/" + themeName + "/common/items/drop_shadow.tga";
		m_pPressedInventoryItem->m_pInventoryIcon->AddIcon(m_pRenderer, dropShadowIcon.c_str(), 64, 64, 64, 64, 4, -4, 0.5f);
	}

	m_pInventoryWindow->DepthSortComponentChildren();

	HideTooltip();
}

bool needs_erasing(InventorySlotItem* aI)
{
	bool needsErase = aI->m_erase;

	if(needsErase == true)
	{
		delete aI->m_pInventoryIcon;
		delete aI;
	}

	return needsErase;
}

void InventoryGUI::_InventoryItemReleased(void *apData)
{
	InventorySlotItem* lpInventoryItem = (InventorySlotItem*)apData;
	lpInventoryItem->m_pInventoryGUI->InventoryItemReleased(lpInventoryItem);
}

void InventoryGUI::InventoryItemReleased(InventorySlotItem* pInventoryItem)
{
	if(m_pPressedInventoryItem == nullptr)
	{
		return;
	}

	m_pPressedInventoryItem = nullptr;

	if (m_pPlayer->IsCrafting())
	{
		// Don't allow to do any inventory changing if we are crafting.

		// Reset back to the original position
		pInventoryItem->m_pInventoryIcon->SetLocation(m_pressedX, m_pressedY);
		if (pInventoryItem->m_dropshadowAdded == true)
		{
			pInventoryItem->m_dropshadowAdded = false;
			string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();
			string dropShadowIcon = "media/textures/gui/" + themeName + "/common/items/drop_shadow.tga";
			pInventoryItem->m_pInventoryIcon->RemoveIcon(dropShadowIcon.c_str());
		}

		m_pInventoryWindow->RemoveComponent(m_pDestroyIcon);
		m_pInventoryWindow->RemoveComponent(m_pDestroyLabel);
		m_pInventoryWindow->RemoveComponent(m_pDropIcon);
		m_pInventoryWindow->RemoveComponent(m_pDropLabel);

		return;
	}

	// Figure out if we need to change to a different inventory slot
	int x;
	int y;
	int width;
	int height;
	POINT lMouse = { VoxGame::GetInstance()->GetWindowCursorX(), (m_windowHeight-VoxGame::GetInstance()->GetWindowCursorY()) };
	bool switched = false;
	for(int i = 0; i < MAX_NUM_SLOTS_VERTICAL; i++)
	{
		for(int j = 0; j < MAX_NUM_SLOTS_HORIZONTAL; j++)
		{
			GetInventoryDimensions(j, i, &x, &y, &width, &height);

			// Check if we released (mouse cursor) in the boundary of another slot
			if(lMouse.x > m_pInventoryWindow->GetDimensions().m_x+x && lMouse.x < m_pInventoryWindow->GetDimensions().m_x+x+width && lMouse.y > m_pInventoryWindow->GetDimensions().m_y+y && lMouse.y < m_pInventoryWindow->GetDimensions().m_y+y+height)
			{
				if(pInventoryItem->m_pInventoryItem->m_equipped == true)
				{
					// Check if another inventory item already exists in this slot
					InventorySlotItem* pInventorySlotItem = GetInventorySlotItem(j, i);
					if(pInventorySlotItem == nullptr)
					{
						// We are unquipping an item that is in one of the equipment slots
						m_pInventoryManager->UnequipItem(j, i, pInventoryItem->m_pInventoryItem->m_equipSlot);

						UnequipItem(pInventoryItem->m_pInventoryItem->m_equipSlot, pInventoryItem->m_pInventoryItem->m_left, pInventoryItem->m_pInventoryItem->m_right);

						// Set the new location for the released inventory icon
						pInventoryItem->m_slotX = j;
						pInventoryItem->m_slotY = i;
						pInventoryItem->m_pInventoryIcon->SetLocation(x, y);
						m_pActionBar->UpdateActionBarSlots(pInventoryItem->m_pInventoryItem, j, i);

						switched = true;
					}
					else
					{
						if(pInventorySlotItem->m_pInventoryItem->m_equipSlot == pInventoryItem->m_pInventoryItem->m_equipSlot)
						{
							// We are swapping an equipped item for one in the inventory
							UnequipItem(pInventoryItem->m_pInventoryItem->m_equipSlot, pInventoryItem->m_pInventoryItem->m_left, pInventoryItem->m_pInventoryItem->m_right);
							m_pInventoryManager->UnequipItem(j, i, pInventoryItem->m_pInventoryItem->m_equipSlot);

							// Equip the new item
							//m_pInventoryManager->EquipInventoryItem(pInventorySlotItem->m_slotX, pInventorySlotItem->m_slotY, pInventorySlotItem->m_pInventoryItem->m_equipSlot);
							EquipItem(pInventorySlotItem);

							// Set the new location for the released inventory icon
							pInventoryItem->m_slotX = j;
							pInventoryItem->m_slotY = i;
							pInventoryItem->m_pInventoryIcon->SetLocation(x, y);

							switched = true;
						}
					}
				}
				else
				{
					// Check if another inventory item already exists in this slot
					InventorySlotItem* pInventorySlotItem = GetInventorySlotItem(j, i);

					// Switch the inventory item in this slot to the pressed (previous) position
					if(pInventorySlotItem != nullptr)
					{
						pInventorySlotItem->m_slotX = pInventoryItem->m_slotX;
						pInventorySlotItem->m_slotY = pInventoryItem->m_slotY;
						pInventorySlotItem->m_pInventoryIcon->SetLocation(m_pressedX, m_pressedY);
						m_pActionBar->UpdateActionBarSlots(pInventorySlotItem->m_pInventoryItem, pInventoryItem->m_slotX, pInventoryItem->m_slotY);
					}

					// Switch the items in the inventory manager
					m_pInventoryManager->SwitchInventoryItems(pInventoryItem->m_slotX, pInventoryItem->m_slotY, j, i);

					// Set the new location for the released inventory icon
					pInventoryItem->m_slotX = j;
					pInventoryItem->m_slotY = i;
					pInventoryItem->m_pInventoryIcon->SetLocation(x, y);
					m_pActionBar->UpdateActionBarSlots(pInventoryItem->m_pInventoryItem, j, i);

					switched = true;
				}
			}
		}
	}

	if(switched)
	{
		ShowTooltip(pInventoryItem);

		VoxGame::GetInstance()->PlaySoundEffect(eSoundEffect_EquipMove);
	}


	bool equipped = false;
	bool deleted = false;
	if(switched == false)
	{
		if(pInventoryItem->m_pInventoryItem->m_equipSlot != EquipSlot_NoSlot)
		{
			// Check if we need to equip the item after dropping on the player portrait
			if(m_pCharacterGUI->IsLoaded())
			{
				m_pCharacterGUI->GetPlayerPortraitDimensions(&x, &y, &width, &height);
				GUIWindow* pCharacterWindow = m_pCharacterGUI->GetCharacterWindow();
				if(lMouse.x > pCharacterWindow->GetDimensions().m_x+x && lMouse.x < pCharacterWindow->GetDimensions().m_x+x+width && lMouse.y > pCharacterWindow->GetDimensions().m_y+y && lMouse.y < pCharacterWindow->GetDimensions().m_y+y+height)
				{
					if(pInventoryItem->m_pInventoryItem->m_equipped == false)
					{
						// Dual handed weapon checks
						if(pInventoryItem->m_pInventoryItem->m_equipSlot == EquipSlot_RightHand)
						{
							InventoryItem* pLeftHanded = m_pInventoryManager->GetInventoryItemForEquipSlot(EquipSlot_LeftHand);
							if(pInventoryItem->m_pInventoryItem->m_left || (pLeftHanded != nullptr && pLeftHanded->m_right))
							{
								int slotX;
								int slotY;
								// Unequip the left hand slot since we are dual handed, OR the already equipped left hand item needs both hands
								UnequipItem(EquipSlot_LeftHand, false, false);
								if(m_pInventoryManager->UnequipItemToFreeInventorySlot(EquipSlot_LeftHand, &slotX, &slotY) == false)
								{
									// We can't fit the other item in the inventory
								}
								else if(pLeftHanded != nullptr)
								{
									m_pActionBar->UpdateActionBarSlots(pLeftHanded, slotX, slotY);
								}
							}
						}
						if(pInventoryItem->m_pInventoryItem->m_equipSlot == EquipSlot_LeftHand)
						{
							InventoryItem* pRightHanded = m_pInventoryManager->GetInventoryItemForEquipSlot(EquipSlot_RightHand);
							if(pInventoryItem->m_pInventoryItem->m_right || (pRightHanded != nullptr && pRightHanded->m_left))
							{
								int slotX;
								int slotY;
								// Unequip the right hand slot since we are dual handed, OR the already equipped right hand item needs both hands
								UnequipItem(EquipSlot_RightHand, false, false);
								if(m_pInventoryManager->UnequipItemToFreeInventorySlot(EquipSlot_RightHand, &slotX, &slotY) == false)
								{
									// We can't fit the other item in the inventory
								}
								else if(pRightHanded != nullptr)
								{
									m_pActionBar->UpdateActionBarSlots(pRightHanded, slotX, slotY);
								}
							}
						}

						m_pPlayer->UnequipItem(pInventoryItem->m_pInventoryItem->m_equipSlot, pInventoryItem->m_pInventoryItem->m_left, pInventoryItem->m_pInventoryItem->m_right);
						m_pInventoryManager->EquipInventoryItem(pInventoryItem->m_slotX, pInventoryItem->m_slotY, pInventoryItem->m_pInventoryItem->m_equipSlot);
						EquipItem(pInventoryItem);

						m_pInventoryWindow->RemoveComponent(pInventoryItem->m_pInventoryIcon);

						pInventoryItem->m_erase = true;
						m_vpInventorySlotItems.erase( remove_if(m_vpInventorySlotItems.begin(), m_vpInventorySlotItems.end(), needs_erasing), m_vpInventorySlotItems.end() );

						m_pInventoryManager->SetInventoryGUINeedsUpdate(true);
						m_pInventoryManager->SetCharacterGUINeedsUpdate(true);

						m_pCharacterGUI->HideEquipHover();

						equipped = true;
					}
				}

				if(equipped == false)
				{
					// Check if we need to equip the item after dropping on a equipment slot
					switch(pInventoryItem->m_pInventoryItem->m_equipSlot)
					{
					case EquipSlot_NoSlot : { x = 0; y = 0; width = 0; height = 0; } break;
					case EquipSlot_RightHand : { m_pCharacterGUI->GetPlayerWeaponRightDimensions(&x, &y, &width, &height); } break;
					case EquipSlot_LeftHand : { m_pCharacterGUI->GetPlayerWeaponLeftDimensions(&x, &y, &width, &height); } break;
					case EquipSlot_Head : { m_pCharacterGUI->GetPlayerHeadDimensions(&x, &y, &width, &height); } break;
					case EquipSlot_Shoulders : { m_pCharacterGUI->GetPlayerShoulderDimensions(&x, &y, &width, &height); } break;
					case EquipSlot_Body : { m_pCharacterGUI->GetPlayerBodyDimensions(&x, &y, &width, &height); } break;
					case EquipSlot_Legs : { m_pCharacterGUI->GetPlayerLegsDimensions(&x, &y, &width, &height); } break;
					case EquipSlot_Hand : { m_pCharacterGUI->GetPlayerHandDimensions(&x, &y, &width, &height); } break;
					case EquipSlot_Feet : { m_pCharacterGUI->GetPlayerFeetDimensions(&x, &y, &width, &height); } break;
					case EquipSlot_Accessory1 : { m_pCharacterGUI->GetPlayerAccessory1Dimensions(&x, &y, &width, &height); } break;
					case EquipSlot_Accessory2 : { m_pCharacterGUI->GetPlayerAccessory2Dimensions(&x, &y, &width, &height); } break;
					}
					GUIWindow* pCharacterWindow = m_pCharacterGUI->GetCharacterWindow();
					if(lMouse.x > pCharacterWindow->GetDimensions().m_x+x && lMouse.x < pCharacterWindow->GetDimensions().m_x+x+width && lMouse.y > pCharacterWindow->GetDimensions().m_y+y && lMouse.y < pCharacterWindow->GetDimensions().m_y+y+height)
					{
						if(pInventoryItem->m_pInventoryItem->m_equipped == false)
						{
							// Dual handed weapon checks
							if(pInventoryItem->m_pInventoryItem->m_equipSlot == EquipSlot_RightHand)
							{
								InventoryItem* pLeftHanded = m_pInventoryManager->GetInventoryItemForEquipSlot(EquipSlot_LeftHand);
								if(pInventoryItem->m_pInventoryItem->m_left || (pLeftHanded != nullptr && pLeftHanded->m_right))
								{
									int slotX;
									int slotY;
									// Unequip the left hand slot since we are dual handed, OR the already equipped left hand item needs both hands
									UnequipItem(EquipSlot_LeftHand, false, false);
									if(m_pInventoryManager->UnequipItemToFreeInventorySlot(EquipSlot_LeftHand, &slotX, &slotY) == false)
									{
										// We can't fit the other item in the inventory
									}
									else if(pLeftHanded != nullptr)
									{
										m_pActionBar->UpdateActionBarSlots(pLeftHanded, slotX, slotY);
									}
								}
							}
							if(pInventoryItem->m_pInventoryItem->m_equipSlot == EquipSlot_LeftHand)
							{
								InventoryItem* pRightHanded = m_pInventoryManager->GetInventoryItemForEquipSlot(EquipSlot_RightHand);
								if(pInventoryItem->m_pInventoryItem->m_right || (pRightHanded != nullptr && pRightHanded->m_left))
								{
									int slotX;
									int slotY;
									// Unequip the right hand slot since we are dual handed, OR the already equipped right hand item needs both hands
									UnequipItem(EquipSlot_RightHand, false, false);
									if(m_pInventoryManager->UnequipItemToFreeInventorySlot(EquipSlot_RightHand, &slotX, &slotY) == false)
									{
										// We can't fit the other item in the inventory
									}
									else if(pRightHanded != nullptr)
									{
										m_pActionBar->UpdateActionBarSlots(pRightHanded, slotX, slotY);
									}
								}
							}

							m_pPlayer->UnequipItem(pInventoryItem->m_pInventoryItem->m_equipSlot, pInventoryItem->m_pInventoryItem->m_left, pInventoryItem->m_pInventoryItem->m_right);
							m_pInventoryManager->EquipInventoryItem(pInventoryItem->m_slotX, pInventoryItem->m_slotY, pInventoryItem->m_pInventoryItem->m_equipSlot);
							EquipItem(pInventoryItem);

							m_pInventoryWindow->RemoveComponent(pInventoryItem->m_pInventoryIcon);

							pInventoryItem->m_erase = true;
							m_vpInventorySlotItems.erase( remove_if(m_vpInventorySlotItems.begin(), m_vpInventorySlotItems.end(), needs_erasing), m_vpInventorySlotItems.end() );

							m_pInventoryManager->SetInventoryGUINeedsUpdate(true);
							m_pInventoryManager->SetCharacterGUINeedsUpdate(true);

							m_pCharacterGUI->HideEquipHover();

							equipped = true;
						}
					}
				}
			}
		}


		if(equipped == false)
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
							if(m_pLootGUI->GetLootSlotItem(j, i) == nullptr) // ONLY if an item doesn't already exist in the loot slot position
							{
								m_pLootGUI->AddLootItemFromInventory(pInventoryItem->m_pInventoryItem, j, i);

								m_pInventoryManager->RemoveInventoryItem(pInventoryItem->m_slotX, pInventoryItem->m_slotY);

								m_pActionBar->RemoveInventoryItemFromActionBar(pInventoryItem->m_pInventoryItem->m_title);

								m_pInventoryWindow->RemoveComponent(pInventoryItem->m_pInventoryIcon);

								pInventoryItem->m_erase = true;
								m_vpInventorySlotItems.erase( remove_if(m_vpInventorySlotItems.begin(), m_vpInventorySlotItems.end(), needs_erasing), m_vpInventorySlotItems.end() );

								m_pCharacterGUI->HideEquipHover();

								switched = true;
								deleted = true;

								VoxGame::GetInstance()->PlaySoundEffect(eSoundEffect_EquipMove);
							}
						}
					}
				}
			}

			// Check if we released on a actionbar slot
			if (VoxGame::GetInstance()->GetVoxSettings()->m_renderGUI)
			{
				if (m_pActionBar->IsLoaded())
				{
					for (int i = 0; i < ActionBar::MAX_NUM_ACTION_SLOTS; i++)
					{
						m_pActionBar->GetActionSlotDimensions(i, &x, &y, &width, &height);

						// Check if we released (mouse cursor) in the boundary of another slot
						if (lMouse.x > x && lMouse.x < x + width && lMouse.y > y && lMouse.y < y + height)
						{
							m_pActionBar->AddItemToActionBar(pInventoryItem->m_pInventoryItem, i, pInventoryItem->m_slotX, pInventoryItem->m_slotY);
							m_pActionBar->ExportActionBar(m_pPlayer->GetName());

							VoxGame::GetInstance()->PlaySoundEffect(eSoundEffect_EquipMove);
						}
					}
				}
			}

			if(switched == false)
			{
				// Check if we need to drop the item into the world
				GetDropSlotDimensions(&x, &y, &width, &height);
				if(lMouse.x > m_pInventoryWindow->GetDimensions().m_x+x && lMouse.x < m_pInventoryWindow->GetDimensions().m_x+x+width && lMouse.y > m_pInventoryWindow->GetDimensions().m_y+y && lMouse.y < m_pInventoryWindow->GetDimensions().m_y+y+height)
				{
					if(pInventoryItem->m_slotX == -1 && pInventoryItem->m_slotY == -1)
					{
					}
					else
					{
						// Drop the item in the world
						vec3 vel = vec3(GetRandomNumber(-1, 1, 2), 0.0f, GetRandomNumber(-1, 1, 2)) * GetRandomNumber(2, 3, 2);

						Item* pItem = m_pItemManager->CreateItem(m_pPlayer->GetCenter(), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), pInventoryItem->m_pInventoryItem->m_filename.c_str(), eItem_DroppedItem, pInventoryItem->m_pInventoryItem->m_title.c_str(), true, false, 0.08f);
						if(pItem != nullptr)
						{
							pItem->SetGravityDirection(vec3(0.0f, -1.0f, 0.0f));
							pItem->SetVelocity(normalize(vel)*4.5f + vec3(0.0f, 9.5f+GetRandomNumber(3, 6, 2), 0.0f));
							pItem->SetRotation(vec3(0.0f, GetRandomNumber(0, 360, 2), 0.0f));
							pItem->SetAngularVelocity(vec3(0.0f, 90.0f, 0.0f));
							pItem->SetDroppedItem(pInventoryItem->m_pInventoryItem->m_filename.c_str(), pInventoryItem->m_pInventoryItem->m_Iconfilename.c_str(), pInventoryItem->m_pInventoryItem->m_itemType, pInventoryItem->m_pInventoryItem->m_item, pInventoryItem->m_pInventoryItem->m_status, pInventoryItem->m_pInventoryItem->m_equipSlot, pInventoryItem->m_pInventoryItem->m_itemQuality, pInventoryItem->m_pInventoryItem->m_left, pInventoryItem->m_pInventoryItem->m_right, pInventoryItem->m_pInventoryItem->m_title.c_str(), pInventoryItem->m_pInventoryItem->m_description.c_str(), pInventoryItem->m_pInventoryItem->m_placementR, pInventoryItem->m_pInventoryItem->m_placementG, pInventoryItem->m_pInventoryItem->m_placementB, pInventoryItem->m_pInventoryItem->m_quantity);
							pItem->SetCollisionEnabled(false);

							for(int i = 0; i < (int)pInventoryItem->m_pInventoryItem->m_vpStatAttributes.size(); i++)
							{
								pItem->GetDroppedInventoryItem()->AddStatAttribute(pInventoryItem->m_pInventoryItem->m_vpStatAttributes[i]->GetType(), pInventoryItem->m_pInventoryItem->m_vpStatAttributes[i]->GetModifyAmount());
							}

							int numY = pItem->GetVoxelItem()->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->GetQubicleMatrix(0)->m_matrixSizeY;
							pItem->GetVoxelItem()->SetRenderOffset(vec3(0.0f, numY*0.5f, 0.0f));
						}

						m_pInventoryManager->RemoveInventoryItem(pInventoryItem->m_slotX, pInventoryItem->m_slotY);

						m_pActionBar->RemoveInventoryItemFromActionBar(pInventoryItem->m_pInventoryItem->m_title);

						m_pInventoryWindow->RemoveComponent(pInventoryItem->m_pInventoryIcon);

						pInventoryItem->m_erase = true;
						m_vpInventorySlotItems.erase( remove_if(m_vpInventorySlotItems.begin(), m_vpInventorySlotItems.end(), needs_erasing), m_vpInventorySlotItems.end() );

						m_pCharacterGUI->HideEquipHover();

						switched = true;
						deleted = true;
					}
				}

				// Check if we need to delete the item
				GetDestroySlotDimensions(&x, &y, &width, &height);
				if(lMouse.x > m_pInventoryWindow->GetDimensions().m_x+x && lMouse.x < m_pInventoryWindow->GetDimensions().m_x+x+width && lMouse.y > m_pInventoryWindow->GetDimensions().m_y+y && lMouse.y < m_pInventoryWindow->GetDimensions().m_y+y+height)
				{
					if(pInventoryItem->m_slotX == -1 && pInventoryItem->m_slotY == -1)
					{
					}
					else
					{
						if (VoxGame::GetInstance()->GetVoxSettings()->m_confirmItemDelete)
						{
							char popupTitle[256];
							strcpy(popupTitle, "Delete");

							char popupText[256];
							sprintf(popupText, "Are you sure you want to delete [C=Custom(00A2E8)]%s[C=White]?", pInventoryItem->m_pInventoryItem->m_title.c_str());

							OpenPopup(popupTitle, popupText);

							m_pInventoryItemToDelete = pInventoryItem;

							m_pCharacterGUI->HideEquipHover();

							switched = false;
							deleted = false;
						}
						else
						{
							if (pInventoryItem != nullptr)
							{
								m_pInventoryManager->RemoveInventoryItem(pInventoryItem->m_slotX, pInventoryItem->m_slotY);

								m_pActionBar->RemoveInventoryItemFromActionBar(pInventoryItem->m_pInventoryItem->m_title);

								m_pInventoryWindow->RemoveComponent(pInventoryItem->m_pInventoryIcon);

								pInventoryItem->m_erase = true;
								m_vpInventorySlotItems.erase(remove_if(m_vpInventorySlotItems.begin(), m_vpInventorySlotItems.end(), needs_erasing), m_vpInventorySlotItems.end());

								switched = true;
								deleted = true;
							}
						}
					}
				}
			}
		}
	}

	// Revert depth back to normal for inventory icons
	if(deleted == false && equipped == false)
	{
		pInventoryItem->m_pInventoryIcon->SetDepth(3.0f);
		if (pInventoryItem->m_dropshadowAdded == true)
		{
			pInventoryItem->m_dropshadowAdded = false;
			string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();
			string dropShadowIcon = "media/textures/gui/" + themeName + "/common/items/drop_shadow.tga";
			pInventoryItem->m_pInventoryIcon->RemoveIcon(dropShadowIcon.c_str());
		}

		m_pInventoryWindow->DepthSortComponentChildren();
	}

	// Reset back to the original position
	if(switched == false && equipped == false)
	{
		pInventoryItem->m_pInventoryIcon->SetLocation(m_pressedX, m_pressedY);
		if (pInventoryItem->m_dropshadowAdded == true)
		{
			pInventoryItem->m_dropshadowAdded = false;
			string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();
			string dropShadowIcon = "media/textures/gui/" + themeName + "/common/items/drop_shadow.tga";
			pInventoryItem->m_pInventoryIcon->RemoveIcon(dropShadowIcon.c_str());
		}
	}

	m_pInventoryWindow->RemoveComponent(m_pDestroyIcon);
	m_pInventoryWindow->RemoveComponent(m_pDestroyLabel);
	m_pInventoryWindow->RemoveComponent(m_pDropIcon);
	m_pInventoryWindow->RemoveComponent(m_pDropLabel);
}

void InventoryGUI::_InventoryItemEntered(void *apData)
{
	InventorySlotItem* lpInventoryItem = (InventorySlotItem*)apData;
	lpInventoryItem->m_pInventoryGUI->InventoryItemEntered(lpInventoryItem);
}

void InventoryGUI::InventoryItemEntered(InventorySlotItem* pInventoryItem)
{
	ShowTooltip(pInventoryItem);

	if(m_pCharacterGUI->IsLoaded())
	{
		m_pCharacterGUI->ShowEquipHover(pInventoryItem->m_pInventoryItem->m_equipSlot);
	}
}

void InventoryGUI::_InventoryItemExited(void *apData)
{
	InventorySlotItem* lpInventoryItem = (InventorySlotItem*)apData;
	lpInventoryItem->m_pInventoryGUI->InventoryItemExited(lpInventoryItem);
}

void InventoryGUI::InventoryItemExited(InventorySlotItem* pInventoryItem)
{
	HideTooltip();
	m_pCharacterGUI->HideEquipHover();
}

void InventoryGUI::_CloseExitPressed(void *pData)
{
	InventoryGUI* lpInventoryGUI = (InventoryGUI*)pData;
	lpInventoryGUI->CloseExitPressed();
}

void InventoryGUI::CloseExitPressed()
{
	Unload();
}

void InventoryGUI::_PopupConfirmPressed(void *apData)
{
	InventoryGUI* lpInventoryGUI = (InventoryGUI*)apData;
	lpInventoryGUI->PopupConfirmPressed();
}

void InventoryGUI::PopupConfirmPressed()
{
	if(m_pInventoryItemToDelete != nullptr)
	{
		m_pInventoryManager->RemoveInventoryItem(m_pInventoryItemToDelete->m_slotX, m_pInventoryItemToDelete->m_slotY);

		m_pActionBar->RemoveInventoryItemFromActionBar(m_pInventoryItemToDelete->m_pInventoryItem->m_title);

		m_pInventoryWindow->RemoveComponent(m_pInventoryItemToDelete->m_pInventoryIcon);

		m_pInventoryItemToDelete->m_erase = true;
		m_vpInventorySlotItems.erase( remove_if(m_vpInventorySlotItems.begin(), m_vpInventorySlotItems.end(), needs_erasing), m_vpInventorySlotItems.end() );
	}

	ClosePopup();
}

void InventoryGUI::_PopupCancelPressed(void *apData)
{
	InventoryGUI* lpInventoryGUI = (InventoryGUI*)apData;
	lpInventoryGUI->PopupCancelPressed();
}

void InventoryGUI::PopupCancelPressed()
{
	m_pInventoryItemToDelete = nullptr;

	ClosePopup();
}
