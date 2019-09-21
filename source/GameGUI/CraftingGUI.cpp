// ******************************************************************************
// Filename:    CraftingGUI.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 15/03/15
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "CraftingGUI.h"

#include "../frontend/FrontendManager.h"
#include "../Player/Player.h"
#include "../VoxGame.h"

#include "../utils/FileUtils.h"
#include "../utils/Random.h"
#include "../utils/Interpolator.h"


CraftingGUI::CraftingGUI(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontendManager, ChunkManager* pChunkManager, Player* pPlayer, InventoryManager* pInventoryManager, int windowWidth, int windowHeight)
{
	m_pRenderer = pRenderer;
	m_pGUI = pGUI;
	m_pFrontendManager = pFrontendManager;
	m_pChunkManager = pChunkManager;
	m_pPlayer = pPlayer;
	m_pInventoryManager = pInventoryManager;

	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;

	// Crafting Window
	m_pCraftingWindow = new GUIWindow(m_pRenderer, m_pFrontendManager->GetFrontendFont_Medium(), "Crafting");
	m_pCraftingWindow->AllowMoving(true);
	m_pCraftingWindow->AllowClosing(false);
	m_pCraftingWindow->AllowMinimizing(false);
	m_pCraftingWindow->AllowScrolling(false);
	m_pCraftingWindow->SetRenderTitleBar(true);
	m_pCraftingWindow->SetRenderWindowBackground(true);
	m_pCraftingWindow->SetApplicationDimensions(m_windowWidth, m_windowHeight);
	m_pCraftingWindow->Hide();

	m_pTitleBarIcon = new Icon(m_pRenderer, "", 44, 44);
	m_pTitleBarIcon->SetDepth(4.0f);

	m_pCraftingWindowBackgroundIcon = new Icon(m_pRenderer, "", 408, 314);
	m_pCraftingWindowBackgroundIcon->SetDepth(1.0f);

	m_pTitleBarBackgroundIcon = new Icon(m_pRenderer, "", 133, 35);
	m_pTitleBarBackgroundIcon->SetDepth(1.0f);

	m_pCloseExitButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pCloseExitButton->SetLabelOffset(0, 5);
	m_pCloseExitButton->SetCallBackFunction(_CloseExitPressed);
	m_pCloseExitButton->SetCallBackData(this);

	m_pResultsScrollbar = new ScrollBar(m_pRenderer);
	m_pResultsScrollbar->SetScrollDirection(EScrollBarDirection_Vertical);
	m_pResultsScrollbar->SetScrollSize(0.25f);
	m_pResultsScrollbar->SetScrollPosition(1.0f);
	m_pResultsScrollbar->SetDepth(2.0f);

	m_pCraftButton_Icon = new Icon(m_pRenderer, "", 115, 32);
	m_pCraftButton_Icon->SetDepth(2.0f);
	m_pCraftButton_Icon_Hover = new Icon(m_pRenderer, "", 115, 32);
	m_pCraftButton_Icon_Hover->SetDepth(2.0f);
	m_pCraftButton_Icon_Pressed = new Icon(m_pRenderer, "", 115, 32);
	m_pCraftButton_Icon_Pressed->SetDepth(2.0f);
	m_pCraftButton_Icon_Disabled = new Icon(m_pRenderer, "", 115, 32);
	m_pCraftButton_Icon_Disabled->SetDepth(2.0f);

	m_pCraftButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25_Outline(), "Craft", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pCraftButton->SetLabelOffset(0, 3);
	m_pCraftButton->SetCallBackFunction(_CraftPressed);
	m_pCraftButton->SetCallBackData(this);
	m_pCraftButton->SetDepth(2.0f);
	m_pCraftButton->SetPressedOffset(0, -4);

	m_pCraftingProgressBarFiller = new Icon(m_pRenderer, "", 96, 14);
	m_pCraftingProgressBarFiller->SetDepth(1.0f);

	m_pCraftingProgressBarBackgroundFiller = new Icon(m_pRenderer, "", 53, 14);
	m_pCraftingProgressBarBackgroundFiller->SetDepth(1.5f);

	m_pCraftingProgressBar = new ProgressBar(m_pRenderer, m_pFrontendManager->GetFrontendFont_20(), 0.0f, 100.0f);
	m_pCraftingProgressBar->SetProgressFiller(m_pCraftingProgressBarFiller);
	m_pCraftingProgressBar->SetProgressBackground(m_pCraftingProgressBarBackgroundFiller);

	m_pSearchTextboxIcon = new Icon(m_pRenderer, "", 156, 24);
	m_pSearchTextboxIcon->SetDepth(1.0f);

	m_pSearchBox = new TextBox(m_pRenderer, m_pFrontendManager->GetFrontendFont_Medium(), "", "");
	m_pSearchBox->SetBackgroundIcon(m_pSearchTextboxIcon);
	m_pSearchBox->SetTextIndent(0);
	m_pSearchBox->SetPipHeight(20);
	m_pSearchBox->SetPipeColour(Colour(0.96f, 0.59f, 0.28f));
	m_pSearchBox->SetTextColour(Colour(1.0f, 1.0f, 1.0f));
	m_pSearchBox->SetDepth(2.0f);

	m_pRecipeButton_Icon = new Icon(m_pRenderer, "", 152, 32);
	m_pRecipeButton_Icon->SetDepth(1.0f);

	m_pRecipeButton_Hover_Icon = new Icon(m_pRenderer, "", 152, 32);
	m_pRecipeButton_Hover_Icon->SetDepth(1.0f);

	m_pRecipeButton_Pressed_Icon = new Icon(m_pRenderer, "", 152, 32);
	m_pRecipeButton_Pressed_Icon->SetDepth(1.0f);

	m_pRecipeButton_Selected_Icon = new Icon(m_pRenderer, "", 152, 32);
	m_pRecipeButton_Selected_Icon->SetDepth(1.0f);

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
	m_pTooltipBackground_Common->SetDepth(7.5f);

	m_pTooltipBackground_Uncommon = new Icon(m_pRenderer, "", 200, 220);
	m_pTooltipBackground_Uncommon->SetDepth(7.5f);

	m_pTooltipBackground_Magical = new Icon(m_pRenderer, "", 200, 220);
	m_pTooltipBackground_Magical->SetDepth(7.5f);

	m_pTooltipBackground_Rare = new Icon(m_pRenderer, "", 200, 220);
	m_pTooltipBackground_Rare->SetDepth(7.5f);

	m_pTooltipBackground_Epic = new Icon(m_pRenderer, "", 200, 220);
	m_pTooltipBackground_Epic->SetDepth(7.5f);

	char nameText[] = "[ITEM]";
	m_pTooltipNameLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), nameText, Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pTooltipNameLabel->SetOutline(true);
	m_pTooltipNameLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pTooltipNameLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_30_Outline());
	m_pTooltipNameLabel->SetDepth(7.5f);

	char descText[] = "[REPLACE ME]";
	m_pTooltipDescriptionLabel = new FormattedLabel(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25(), descText);
	m_pTooltipDescriptionLabel->SetOutline(true);
	m_pTooltipDescriptionLabel->SetColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pTooltipDescriptionLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pTooltipDescriptionLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
	m_pTooltipDescriptionLabel->SetDepth(7.5f);
	m_pTooltipDescriptionLabel->SetWordWrap(true);

	char slotText[] = "[SLOT]";
	m_pTooltipSlotLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_20(), slotText, Colour(0.5f, 0.5f, 0.5f, 1.0f));
	m_pTooltipSlotLabel->SetOutline(true);
	m_pTooltipSlotLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pTooltipSlotLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_20_Outline());
	m_pTooltipSlotLabel->SetDepth(7.5f);

	char qualityText[] = "[QUALITY]";
	m_pTooltipQualityLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_20(), qualityText, Colour(0.5f, 0.5f, 0.5f, 1.0f));
	m_pTooltipQualityLabel->SetOutline(true);
	m_pTooltipQualityLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pTooltipQualityLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_20_Outline());
	m_pTooltipQualityLabel->SetDepth(7.5f);

	m_pCraftingWindow->SetBackgroundIcon(m_pCraftingWindowBackgroundIcon);
	m_pCraftingWindow->SetTitlebarBackgroundIcon(m_pTitleBarBackgroundIcon);

	m_pCraftingWindow->AddComponent(m_pTitleBarIcon);
	m_pCraftingWindow->AddComponent(m_pCloseExitButton);
	m_pCraftingWindow->AddComponent(m_pCraftButton);
	m_pCraftingWindow->AddComponent(m_pCraftingProgressBar);
	m_pCraftingWindow->AddComponent(m_pSearchBox);

	SetWindowDimensions(m_windowWidth, m_windowHeight);

	m_crafting = false;
	m_craftingTime = 2.5f;
	m_craftingTimer = 0.0f;

	m_pRecipeSlotItemSelected = NULL;

	m_pInteractionItem = NULL;

	// Load delay
	m_loadDelay = false;
	m_loadDelayTime = 0.0f;

	m_loaded = false;
}

CraftingGUI::~CraftingGUI()
{
	DeleteRecipeButtons();
	DeleteIngredientsButtons();
	DeleteCraftingRecipes();

	delete m_pCraftingWindow;
	delete m_pTitleBarIcon;
	delete m_pTitleBarBackgroundIcon;
	delete m_pCraftingWindowBackgroundIcon;
	delete m_pCloseExitButton;
	delete m_pResultsScrollbar;
	delete m_pCraftButton_Icon;
	delete m_pCraftButton_Icon_Hover;
	delete m_pCraftButton_Icon_Pressed;
	delete m_pCraftButton_Icon_Disabled;
	delete m_pCraftButton;
	delete m_pCraftingProgressBar;
	delete m_pCraftingProgressBarFiller;
	delete m_pCraftingProgressBarBackgroundFiller;
	delete m_pSearchBox;
	delete m_pSearchTextboxIcon;
	delete m_pRecipeButton_Icon;
	delete m_pRecipeButton_Hover_Icon;
	delete m_pRecipeButton_Pressed_Icon;
	delete m_pRecipeButton_Selected_Icon;

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

void CraftingGUI::SetWindowDimensions(int windowWidth, int windowHeight)
{
	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;

	m_craftingWindowWidth = 408;
	m_craftingWindowHeight = 314;
	m_titlebarWidth = 153;
	m_titlebarHeight = 35;
	m_craftingResultsScrollAreaX = -160;
	m_craftingResultsScrollAreaY = -4;
	m_craftingResultsScrollAreaWidth = 160;
	m_craftingResultsScrollAreaHeight = 238;
	m_craftingProgressX = 4;
	m_craftingProgressY = 4;
	m_craftingProgressWidth = 400;
	m_craftingProgressHeight = 20;

	m_pCraftingWindow->SetDimensions(75, 425, m_craftingWindowWidth, m_craftingWindowHeight);
	m_pCraftingWindow->SetTitleBarDimensions(0, 0, m_titlebarWidth, m_titlebarHeight);
	m_pCraftingWindow->SetTitleOffset(50, 5);
	m_pCraftingWindow->SetApplicationDimensions(m_windowWidth, m_windowHeight);
	m_pCraftingWindow->SetApplicationBorder(25, 15, 10, 10);

	m_pTitleBarIcon->SetDimensions(0, m_craftingWindowHeight, 44, 44);

	m_pCloseExitButton->SetDimensions(m_craftingWindowWidth-32, m_craftingWindowHeight, 32, 32);

	m_pResultsScrollbar->SetDimensions(175, 36, 24, m_craftingResultsScrollAreaHeight-8);
	m_pResultsScrollbar->SetScrollArea(m_craftingResultsScrollAreaX, m_craftingResultsScrollAreaY, m_craftingResultsScrollAreaWidth, m_craftingResultsScrollAreaHeight);

	m_pFrontendManager->SetScrollbarIcons(m_pResultsScrollbar);

	m_pCraftButton->SetDimensions(284, 41, 115, 32);

	m_pCraftingProgressBar->SetDimensions(m_craftingProgressX, m_craftingProgressY, m_craftingProgressWidth, m_craftingProgressHeight);

	m_pSearchTextboxIcon->SetDimensions(0, 0, 156, 24);
	m_pSearchBox->SetDimensions(8, 282, 156, 24);
}

// Skinning the GUI
void CraftingGUI::SkinGUI()
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

	iconName = "media/textures/gui/" + themeName + "/CraftingGUI/craft_button.tga";
	m_pCraftButton_Icon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CraftingGUI/craft_button_hover.tga";
	m_pCraftButton_Icon_Hover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CraftingGUI/craft_button_pressed.tga";
	m_pCraftButton_Icon_Pressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CraftingGUI/craft_button_disabled.tga";
	m_pCraftButton_Icon_Disabled->SetIcon(iconName);
	m_pCraftButton->SetDefaultIcon(m_pCraftButton_Icon);
	m_pCraftButton->SetHoverIcon(m_pCraftButton_Icon_Hover);
	m_pCraftButton->SetSelectedIcon(m_pCraftButton_Icon_Pressed);
	m_pCraftButton->SetDisabledIcon(m_pCraftButton_Icon_Disabled);

	iconName = "media/textures/gui/" + themeName + "/CraftingGUI/craft_progress_filler.tga";
	m_pCraftingProgressBarFiller->SetIcon(iconName);;
	iconName = "media/textures/gui/" + themeName + "/CraftingGUI/craft_progress_filler_background.tga";
	m_pCraftingProgressBarBackgroundFiller->SetIcon(iconName);

	m_pCraftingProgressBar->SetProgressFiller(m_pCraftingProgressBarFiller);
	m_pCraftingProgressBar->SetProgressBackground(m_pCraftingProgressBarBackgroundFiller);

	iconName = "media/textures/gui/" + themeName + "/CraftingGUI/search_textbox.tga";
	m_pSearchTextboxIcon->SetIcon(iconName);

	m_pSearchBox->SetBackgroundIcon(m_pSearchTextboxIcon);

	iconName = "media/textures/gui/" + themeName + "/CraftingGUI/recipe_item_button.tga";
	m_pRecipeButton_Icon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CraftingGUI/recipe_item_hover_button.tga";
	m_pRecipeButton_Hover_Icon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CraftingGUI/recipe_item_pressed_button.tga";
	m_pRecipeButton_Pressed_Icon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CraftingGUI/recipe_item_selected_button.tga";
	m_pRecipeButton_Selected_Icon->SetIcon(iconName);

	iconName = "media/textures/gui/" + themeName + "/CraftingGUI/crafting_title_icon.tga";
	m_pTitleBarIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CraftingGUI/crafting_window_background.tga";
	m_pCraftingWindowBackgroundIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/CraftingGUI/titlebar_background.tga";
	m_pTitleBarBackgroundIcon->SetIcon(iconName);

	m_pCraftingWindow->SetBackgroundIcon(m_pCraftingWindowBackgroundIcon);
	m_pCraftingWindow->SetTitlebarBackgroundIcon(m_pTitleBarBackgroundIcon);

	Point location = m_pCraftingWindow->GetLocation();
	m_pCraftingWindow->SetDimensions(location.m_x, location.m_y, m_craftingWindowWidth, m_craftingWindowHeight);
	m_pCraftingWindow->SetTitleBarDimensions(0, 0, m_titlebarWidth, m_titlebarHeight);

	m_pCloseExitButton->SetDefaultIcon(m_pFrontendManager->GetCloseExitButtonIcon());
	m_pCloseExitButton->SetHoverIcon(m_pFrontendManager->GetCloseExitButtonIcon_Hover());
	m_pCloseExitButton->SetSelectedIcon(m_pFrontendManager->GetCloseExitButtonIcon_Pressed());
	m_pCloseExitButton->SetDisabledIcon(m_pFrontendManager->GetCloseExitButtonIcon());

	m_pCraftButton->SetNormalLabelColour(m_pFrontendManager->GetNormalFontColour());
	m_pCraftButton->SetHoverLabelColour(m_pFrontendManager->GetHoverFontColour());
	m_pCraftButton->SetPressedLabelColour(m_pFrontendManager->GetPressedFontColour());
}

void CraftingGUI::UnSkinGUI()
{

}

// Loading
void CraftingGUI::Load(bool loadDelay, float loadDelayTime)
{
	m_loadDelay = loadDelay;
	m_loadDelayTime = loadDelayTime;

	m_pCraftingWindow->AddComponent(m_pResultsScrollbar);

	m_crafting = false;
	m_craftingTime = 2.5f;
	m_craftingTimer = 0.0f;
	m_pCraftingProgressBar->SetCurrentValue(0.0f);

	m_pSearchBox->SetText("");

	m_pRecipeSlotItemSelected = NULL;

	m_pInteractionItem = NULL;

	CreateRecipeButtons();
	UpdateFilteredRecipes();

	UpdateCraftButton();

	m_pCraftingWindow->DepthSortComponentChildren();
	if (m_loadDelay == false)
	{
		m_pGUI->AddWindow(m_pCraftingWindow);
		m_pCraftingWindow->Show();
	}

	m_toolTipVisible = false;
	m_tooltipAppearDelayTimer = 0.0f;
	m_toolTipComponentsAdded = false;
	m_tooltipQuality = ItemQuality_Common;

	m_loaded = true;
}

void CraftingGUI::Unload()
{
	m_loadDelay = false;
	m_loadDelayTime = 0.0f;

	HideTooltip();

	DeleteIngredientsButtons();
	DeleteRecipeButtons();

	m_pCraftingWindow->RemoveComponent(m_pResultsScrollbar);

	m_pGUI->RemoveWindow(m_pCraftingWindow);

	m_pPlayer->SetCrafting(false);
	m_pPlayer->SetCraftingItem(false);
	m_pPlayer->DisableMoveToTargetPosition();

	if(m_pInteractionItem != NULL)
	{
		m_pInteractionItem->SetCollisionEnabled(true);
	}

	VoxGame::GetInstance()->CloseLetterBox();

	m_loaded = false;

	if (VoxGame::GetInstance()->IsGUIWindowStillDisplayed() == false) // Needs to be after setting loaded to false, so that we correctly turn the cursor back off
	{
		VoxGame::GetInstance()->TurnCursorOff(false);
		if (VoxGame::GetInstance()->ShouldRestorePreviousCameraMode())
		{
			VoxGame::GetInstance()->RestorePreviousCameraMode();
			VoxGame::GetInstance()->InitializeCameraRotation();
		}
	}
}

bool CraftingGUI::IsLoadDelayed()
{
	return (m_loadDelay == true && m_loadDelayTime > 0.0f);
}

bool CraftingGUI::IsLoaded()
{
	return m_loaded;
}

void CraftingGUI::CraftingComplete()
{
	m_pPlayer->SetCrafting(false);

	bool canCraft = CanCraftRecipe();

	if(canCraft)
	{
		// Remove the resource items from the inventory, since we have used them in the crafting process
		for(unsigned int i = 0; i < m_pRecipeSlotItemSelected->m_pCraftingReceipe->m_vpCraftingItems.size() && canCraft; i++)
		{
			InventoryItem * item = m_pRecipeSlotItemSelected->m_pCraftingReceipe->m_vpCraftingItems[i];
			m_pInventoryManager->RemoveInventoryItem(item->m_title.c_str(), item->m_item, item->m_quantity);
		}

		InventoryItem* pInventoryItem = m_pRecipeSlotItemSelected->m_pInventoryItem;

		// Add the new item to the inventory
		if(m_pInventoryManager->CanAddInventoryItem(pInventoryItem->m_title.c_str(), pInventoryItem->m_item, m_pRecipeSlotItemSelected->m_pCraftingReceipe->m_pResultItem->m_quantity))
		{
			// Add to inventory
			m_pInventoryManager->AddInventoryItem(pInventoryItem, -1, -1);
		}
		else
		{
			// Drop the item in the world
			vec3 vel = vec3(GetRandomNumber(-1, 1, 2), 0.0f, GetRandomNumber(-1, 1, 2)) * GetRandomNumber(2, 3, 2);

			Item* pItem = VoxGame::GetInstance()->GetItemManager()->CreateItem(m_pInteractionItem->GetCenter(), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), pInventoryItem->m_filename.c_str(), eItem_DroppedItem, pInventoryItem->m_title.c_str(), true, false, 0.08f);
			if (pItem != NULL)
			{
				pItem->SetGravityDirection(vec3(0.0f, -1.0f, 0.0f));
				pItem->SetVelocity(normalize(vel)*4.5f + vec3(0.0f, 9.5f + GetRandomNumber(3, 6, 2), 0.0f));
				pItem->SetRotation(vec3(0.0f, GetRandomNumber(0, 360, 2), 0.0f));
				pItem->SetAngularVelocity(vec3(0.0f, 90.0f, 0.0f));
				pItem->SetDroppedItem(
					pInventoryItem->m_filename.c_str(), pInventoryItem->m_Iconfilename.c_str(),
					pInventoryItem->m_itemType, pInventoryItem->m_item, pInventoryItem->m_status,
					pInventoryItem->m_equipSlot, pInventoryItem->m_itemQuality, pInventoryItem->m_left,
					pInventoryItem->m_right, pInventoryItem->m_title.c_str(), pInventoryItem->m_description.c_str(),
					pInventoryItem->m_placementR, pInventoryItem->m_placementG, pInventoryItem->m_placementB,
					pInventoryItem->m_quantity);
				pItem->SetCollisionEnabled(false);

				for (int i = 0; i < (int)pInventoryItem->m_vpStatAttributes.size(); i++)
				{
					pItem->GetDroppedInventoryItem()->AddStatAttribute(pInventoryItem->m_vpStatAttributes[i]->GetType(), pInventoryItem->m_vpStatAttributes[i]->GetModifyAmount());
				}

				int numY = pItem->GetVoxelItem()->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->GetQubicleMatrix(0)->m_matrixSizeY;
				pItem->GetVoxelItem()->SetRenderOffset(vec3(0.0f, numY*0.5f, 0.0f));
			}
		}
	}
	else
	{
	}

	for(unsigned int i = 0; i < m_vpRecipeSlotItem.size(); i++)
	{
		Button* pIcon = m_vpRecipeSlotItem[i]->m_pResultsIcon;
		if (pIcon == m_pRecipeSlotItemSelected->m_pResultsIcon)
		{
			continue;
		}

		pIcon->SetEnabled(true);
		pIcon->SetSelected(false);
		pIcon->SetHover(false);
	}

	UpdateCraftButton();
}

void CraftingGUI::SetInteractionitem(Item* pInteractionItem)
{
	m_pInteractionItem = pInteractionItem;

	if(m_pInteractionItem != NULL)
	{
		m_pInteractionItem->SetCollisionEnabled(false);
	}
}

// Recipes
void CraftingGUI::SetCraftingRecipesForItem(eItem craftingItem)
{
	DeleteCraftingRecipes();

	if(craftingItem == eItem_Anvil)
	{
		// Pickaxe
		{
			CraftingRecipe* pPickaxeRecipe = new CraftingRecipe();
			InventoryItem* pStone = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_Block_Stone, 3, ItemQuality_Common);
			InventoryItem* pWood = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_Block_Wood, 4, ItemQuality_Common);
			InventoryItem* pIronBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_IronBar, 3, ItemQuality_Common);
			pPickaxeRecipe->m_vpCraftingItems.push_back(pStone);
			pPickaxeRecipe->m_vpCraftingItems.push_back(pWood);
			pPickaxeRecipe->m_vpCraftingItems.push_back(pIronBar);

			InventoryItem* pPickaxe = m_pInventoryManager->CreateEquipmentItemFromType(eEquipment_NormalPickaxe);
			pPickaxe->m_scale = 0.0f; pPickaxe->m_offsetX = 0.5f; pPickaxe->m_offsetY = 0.95f; pPickaxe->m_offsetZ = 0.5f;
			pPickaxeRecipe->m_pResultItem = pPickaxe;

			AddCraftingRecipe(pPickaxeRecipe);
		}

		// Hammer
		{
			CraftingRecipe* pHammerRecipe = new CraftingRecipe();
			InventoryItem* pStone = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_Block_Stone, 4, ItemQuality_Common);
			InventoryItem* pIronBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_IronBar, 4, ItemQuality_Common);
			pHammerRecipe->m_vpCraftingItems.push_back(pStone);
			pHammerRecipe->m_vpCraftingItems.push_back(pIronBar);

			InventoryItem* pHammer = m_pInventoryManager->CreateEquipmentItemFromType(eEquipment_Hammer);
			pHammer->m_scale = 0.5f; pHammer->m_offsetX = 0.5f; pHammer->m_offsetY = 0.83f; pHammer->m_offsetZ = 0.5f;
			pHammerRecipe->m_pResultItem = pHammer;

			AddCraftingRecipe(pHammerRecipe);
		}

		// Iron Sword
		{
			CraftingRecipe* pIronSwordRecipe = new CraftingRecipe();
			InventoryItem* pIronBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_IronBar, 3, ItemQuality_Common);
			pIronSwordRecipe->m_vpCraftingItems.push_back(pIronBar);

			InventoryItem* pIronSword = m_pInventoryManager->CreateEquipmentItemFromType(eEquipment_IronSword);
			pIronSword->m_scale = 0.5f; pIronSword->m_offsetX = 0.5f; pIronSword->m_offsetY = 0.83f; pIronSword->m_offsetZ = 0.5f;
			pIronSwordRecipe->m_pResultItem = pIronSword;

			AddCraftingRecipe(pIronSwordRecipe);
		}

		// Iron Shield
		{
			CraftingRecipe* pIronShieldRecipe = new CraftingRecipe();
			InventoryItem* pIronBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_IronBar, 4, ItemQuality_Common);
			pIronShieldRecipe->m_vpCraftingItems.push_back(pIronBar);

			InventoryItem* pIronShield = m_pInventoryManager->CreateEquipmentItemFromType(eEquipment_IronShield);
			pIronShieldRecipe->m_pResultItem = pIronShield;

			AddCraftingRecipe(pIronShieldRecipe);
		}

		// Iron Helm
		{
			CraftingRecipe* pIronHelmRecipe = new CraftingRecipe();
			InventoryItem* pIronBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_IronBar, 10, ItemQuality_Common);
			pIronHelmRecipe->m_vpCraftingItems.push_back(pIronBar);

			InventoryItem* pIronHelm = m_pInventoryManager->CreateEquipmentItemFromType(eEquipment_IronHelm);
			pIronHelmRecipe->m_pResultItem = pIronHelm;

			AddCraftingRecipe(pIronHelmRecipe);
		}

		// Iron Armor
		{
			CraftingRecipe* pIronArmorRecipe = new CraftingRecipe();
			InventoryItem* pIronBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_IronBar, 10, ItemQuality_Common);
			pIronArmorRecipe->m_vpCraftingItems.push_back(pIronBar);

			InventoryItem* pIronArmor = m_pInventoryManager->CreateEquipmentItemFromType(eEquipment_IronArmor);
			pIronArmorRecipe->m_pResultItem = pIronArmor;

			AddCraftingRecipe(pIronArmorRecipe);
		}

		// Iron Pants
		{
			CraftingRecipe* pIronPantsRecipe = new CraftingRecipe();
			InventoryItem* pIronBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_IronBar, 10, ItemQuality_Common);
			pIronPantsRecipe->m_vpCraftingItems.push_back(pIronBar);

			InventoryItem* pIronPants = m_pInventoryManager->CreateEquipmentItemFromType(eEquipment_IronPants);
			pIronPantsRecipe->m_pResultItem = pIronPants;

			AddCraftingRecipe(pIronPantsRecipe);
		}

		// Iron Gloves
		{
			CraftingRecipe* pIronGlovesRecipe = new CraftingRecipe();
			InventoryItem* pIronBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_IronBar, 10, ItemQuality_Common);
			pIronGlovesRecipe->m_vpCraftingItems.push_back(pIronBar);

			InventoryItem* pIronGloves = m_pInventoryManager->CreateEquipmentItemFromType(eEquipment_IronGloves);
			pIronGlovesRecipe->m_pResultItem = pIronGloves;

			AddCraftingRecipe(pIronGlovesRecipe);
		}

		// Iron Boots
		{
			CraftingRecipe* pIronBootsRecipe = new CraftingRecipe();
			InventoryItem* pIronBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_IronBar, 10, ItemQuality_Common);
			pIronBootsRecipe->m_vpCraftingItems.push_back(pIronBar);

			InventoryItem* pIronBoots = m_pInventoryManager->CreateEquipmentItemFromType(eEquipment_IronBoots);
			pIronBootsRecipe->m_pResultItem = pIronBoots;

			AddCraftingRecipe(pIronBootsRecipe);
		}

		// Iron Shoulders
		{
			CraftingRecipe* pIronShouldersRecipe = new CraftingRecipe();
			InventoryItem* pIronBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_IronBar, 10, ItemQuality_Common);
			pIronShouldersRecipe->m_vpCraftingItems.push_back(pIronBar);

			InventoryItem* pIronShoulders = m_pInventoryManager->CreateEquipmentItemFromType(eEquipment_IronShoulders);
			pIronShouldersRecipe->m_pResultItem = pIronShoulders;

			AddCraftingRecipe(pIronShouldersRecipe);
		}

		// 2 Handed Sword
		{
			CraftingRecipe* p2HandedSwordRecipe = new CraftingRecipe();
			InventoryItem* pIronBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_IronBar, 10, ItemQuality_Common);
			InventoryItem* pCopperBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_CopperBar, 5, ItemQuality_Common);
			InventoryItem* pWood = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_Block_Wood, 3, ItemQuality_Common);
			p2HandedSwordRecipe->m_vpCraftingItems.push_back(pIronBar);
			p2HandedSwordRecipe->m_vpCraftingItems.push_back(pCopperBar);
			p2HandedSwordRecipe->m_vpCraftingItems.push_back(pWood);

			InventoryItem* p2HandedSword = m_pInventoryManager->CreateEquipmentItemFromType(eEquipment_2HandedSword);
			p2HandedSword->m_scale = 0.5f; p2HandedSword->m_offsetX = 0.5f; p2HandedSword->m_offsetY = 0.83f; p2HandedSword->m_offsetZ = 0.5f;
			p2HandedSwordRecipe->m_pResultItem = p2HandedSword;

			AddCraftingRecipe(p2HandedSwordRecipe);
		}
	}
	else if(craftingItem == eItem_Furnace)
	{
		// Copper
		{
			CraftingRecipe* pCopperRecipe = new CraftingRecipe();
			InventoryItem* pCopperNugget = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_CopperOre, 1, ItemQuality_Common);
			pCopperRecipe->m_vpCraftingItems.push_back(pCopperNugget);

			InventoryItem* pCopperBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_CopperBar, 1, ItemQuality_Common);
			pCopperRecipe->m_pResultItem = pCopperBar;

			AddCraftingRecipe(pCopperRecipe);
		}

		// Iron
		{
			CraftingRecipe* pIronRecipe = new CraftingRecipe();
			InventoryItem* pIronNugget = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_IronOre, 2, ItemQuality_Common); 
			pIronRecipe->m_vpCraftingItems.push_back(pIronNugget);

			InventoryItem* pIronBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_IronBar, 1, ItemQuality_Common);
			pIronRecipe->m_pResultItem = pIronBar;

			AddCraftingRecipe(pIronRecipe);
		}

		// Silver
		{
			CraftingRecipe* pSilverRecipe = new CraftingRecipe();
			InventoryItem* pSilverNugget = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_SilverOre, 3, ItemQuality_Common);
			pSilverRecipe->m_vpCraftingItems.push_back(pSilverNugget);

			InventoryItem* pSilverBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_SilverBar, 1, ItemQuality_Common);
			pSilverRecipe->m_pResultItem = pSilverBar;

			AddCraftingRecipe(pSilverRecipe);
		}

		// Gold
		{
			CraftingRecipe* pGoldRecipe = new CraftingRecipe();
			InventoryItem* pGoldNugget = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_GoldOre, 3, ItemQuality_Common);
			pGoldRecipe->m_vpCraftingItems.push_back(pGoldNugget);

			InventoryItem* pGoldBar = m_pInventoryManager->CreateInventoryItemForCrafting(eItem_GoldBar, 1, ItemQuality_Common);
			pGoldRecipe->m_pResultItem = pGoldBar;

			AddCraftingRecipe(pGoldRecipe);
		}
	}
	else
	{
	}
}

void CraftingGUI::AddCraftingRecipe(CraftingRecipe* pRecipe)
{
	m_vpCraftingRecipes.push_back(pRecipe);
}

void CraftingGUI::DeleteCraftingRecipes()
{
	for(unsigned int i = 0; i < m_vpCraftingRecipes.size(); i++)
	{
		delete m_vpCraftingRecipes[i];
		m_vpCraftingRecipes[i] = 0;
	}
	m_vpCraftingRecipes.clear();
}

void CraftingGUI::CreateRecipeButtons()
{
	int numToCreate = 50;

	for(int i = 0; i < numToCreate; i++)
	{
		if(m_vpCraftingRecipes.size() == 0 || i > m_vpCraftingRecipes.size()-1)
		{
			continue;
		}

		InventoryItem* pResultsItem = m_vpCraftingRecipes[i]->m_pResultItem;

		RecipeSlotItem* lpSlotItem = new RecipeSlotItem();

		// Create the item icon
		Button* pNewResultsItem = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), m_pFrontendManager->GetFrontendFont_25_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
		pNewResultsItem->SetDefaultIcon(m_pRecipeButton_Icon);
		pNewResultsItem->SetHoverIcon(m_pRecipeButton_Hover_Icon);
		pNewResultsItem->SetSelectedIcon(m_pRecipeButton_Pressed_Icon);
		pNewResultsItem->SetDisabledIcon(m_pRecipeButton_Icon);
		pNewResultsItem->SetPressedOffset(0, -4);
		pNewResultsItem->SetLabelColour(m_pFrontendManager->GetNormalFontColour());
		pNewResultsItem->SetNormalLabelColour(m_pFrontendManager->GetNormalFontColour());
		pNewResultsItem->SetHoverLabelColour(m_pFrontendManager->GetHoverFontColour());
		pNewResultsItem->SetPressedLabelColour(m_pFrontendManager->GetPressedFontColour());

		int xPos = -m_craftingResultsScrollAreaWidth;
		int yPos = m_craftingResultsScrollAreaHeight - ((i+1)*36);

		pNewResultsItem->SetDimensions(xPos, yPos, 152, 32);
		pNewResultsItem->SetDepth(2.0f);

		char lItemTexture[128];
		sprintf(lItemTexture, "%s", pResultsItem->m_Iconfilename.c_str());
		pNewResultsItem->AddIcon(m_pRenderer, lItemTexture, 64, 64, 24, 24, 4, 4, 2.5f);

		// Create the item label
		pNewResultsItem->AddText(m_pRenderer, m_pFrontendManager->GetFrontendFont_20(), m_pFrontendManager->GetFrontendFont_20_Outline(), pResultsItem->m_title, Colour(1.0f, 1.0f, 1.0f, 1.0f), 34, 7, true, Colour(0.0f, 0.0f, 0.0f, 1.0f));

		pNewResultsItem->SetCallBackFunction(_ResultsItemPressed);
		pNewResultsItem->SetCallBackData(lpSlotItem);

		pNewResultsItem->SetEnterCallBackFunction(_RecipeItemEntered);
		pNewResultsItem->SetEnterCallBackData(lpSlotItem);

		pNewResultsItem->SetExitCallBackFunction(_RecipeItemExited);
		pNewResultsItem->SetExitCallBackData(lpSlotItem);

		lpSlotItem->m_pInventoryItem = pResultsItem;
		lpSlotItem->m_pCraftingGUI = this;
		lpSlotItem->m_pResultsIcon = pNewResultsItem;
		lpSlotItem->m_recipeName = pResultsItem->m_title;
		lpSlotItem->m_pCraftingReceipe = m_vpCraftingRecipes[i];
		lpSlotItem->m_slotIndex = i;
		lpSlotItem->m_erase = false;

		m_vpRecipeSlotItem.push_back(lpSlotItem);
	}
}

void CraftingGUI::DeleteRecipeButtons()
{
	m_pResultsScrollbar->ClearScrollAreaItems();

	for(unsigned int i = 0; i < m_vpRecipeSlotItem.size(); i++)
	{
		m_pCraftingWindow->RemoveComponent(m_vpRecipeSlotItem[i]->m_pResultsIcon);
		delete m_vpRecipeSlotItem[i]->m_pResultsIcon;

		delete m_vpRecipeSlotItem[i];
		m_vpRecipeSlotItem[i] = 0;
	}
	m_vpRecipeSlotItem.clear();
}

void CraftingGUI::CreateIngredientsButtons()
{
	DeleteIngredientsButtons();

	if(m_pRecipeSlotItemSelected == NULL)
	{
		return;
	}

	int ingredientsHorizontalIndex = 0;
	int ingredientsVerticalIndex = 0;

	m_pGUI->RemoveWindow(m_pCraftingWindow);

	for(unsigned int i = 0; i < m_pRecipeSlotItemSelected->m_pCraftingReceipe->m_vpCraftingItems.size(); i++)
	{
		InventoryItem* pInventoryItem = m_pRecipeSlotItemSelected->m_pCraftingReceipe->m_vpCraftingItems[i];

		IngredientsSlotItem* lpSlotItem = new IngredientsSlotItem();

		// Create the requirement button
		Button* pNewCraftingItem = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_20(), m_pFrontendManager->GetFrontendFont_20_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
		switch(pInventoryItem->m_itemQuality)
		{
		case ItemQuality_Common: { pNewCraftingItem->SetDefaultIcon(m_pInventoryBackgroundSlotBorder_Common); pNewCraftingItem->SetHoverIcon(m_pInventoryBackgroundSlotBorder_Common); pNewCraftingItem->SetSelectedIcon(m_pInventoryBackgroundSlotBorder_Common); break; }
		case ItemQuality_Uncommon: { pNewCraftingItem->SetDefaultIcon(m_pInventoryBackgroundSlotBorder_Uncommon); pNewCraftingItem->SetHoverIcon(m_pInventoryBackgroundSlotBorder_Uncommon); pNewCraftingItem->SetSelectedIcon(m_pInventoryBackgroundSlotBorder_Uncommon); break; }
		case ItemQuality_Magical: { pNewCraftingItem->SetDefaultIcon(m_pInventoryBackgroundSlotBorder_Magical); pNewCraftingItem->SetHoverIcon(m_pInventoryBackgroundSlotBorder_Magical); pNewCraftingItem->SetSelectedIcon(m_pInventoryBackgroundSlotBorder_Magical); break; }
		case ItemQuality_Rare: { pNewCraftingItem->SetDefaultIcon(m_pInventoryBackgroundSlotBorder_Rare); pNewCraftingItem->SetHoverIcon(m_pInventoryBackgroundSlotBorder_Rare); pNewCraftingItem->SetSelectedIcon(m_pInventoryBackgroundSlotBorder_Rare); break; }
		case ItemQuality_Epic: { pNewCraftingItem->SetDefaultIcon(m_pInventoryBackgroundSlotBorder_Epic); pNewCraftingItem->SetHoverIcon(m_pInventoryBackgroundSlotBorder_Epic); pNewCraftingItem->SetSelectedIcon(m_pInventoryBackgroundSlotBorder_Epic); break; }
		}
		//pNewCraftingItem->SetDisabled(true);

		int xPos = 216 + (ingredientsHorizontalIndex*62);
		int yPos = 246 - (ingredientsVerticalIndex*62);

		pNewCraftingItem->SetDimensions(xPos, yPos, 54, 54);
		pNewCraftingItem->SetDepth(2.0f);

		char lItemTexture[128];
		sprintf(lItemTexture, "%s", pInventoryItem->m_Iconfilename.c_str());
		pNewCraftingItem->AddIcon(m_pRenderer, lItemTexture, 64, 64, 50, 50, 2, 2, 1.5f);

		string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();
		switch (pInventoryItem->m_itemQuality)
		{
			case ItemQuality_Common:
			{
				string itemBackgroundIcon = "media/textures/gui/" + themeName + "/common/items/item_background_common.tga";
				pNewCraftingItem->AddIcon(m_pRenderer, itemBackgroundIcon.c_str(), 64, 64, 54, 54, 0, 0, 1.0f);
				break;
			}
			case ItemQuality_Uncommon:
			{
				string itemBackgroundIcon = "media/textures/gui/" + themeName + "/common/items/item_background_uncommon.tga";
				pNewCraftingItem->AddIcon(m_pRenderer, itemBackgroundIcon.c_str(), 64, 64, 54, 54, 0, 0, 1.0f);
				break;
			}
			case ItemQuality_Magical:
			{
				string itemBackgroundIcon = "media/textures/gui/" + themeName + "/common/items/item_background_magical.tga";
				pNewCraftingItem->AddIcon(m_pRenderer, itemBackgroundIcon.c_str(), 64, 64, 54, 54, 0, 0, 1.0f);
				break;
			}
			case ItemQuality_Rare:
			{
				string itemBackgroundIcon = "media/textures/gui/" + themeName + "/common/items/item_background_rare.tga";
				pNewCraftingItem->AddIcon(m_pRenderer, itemBackgroundIcon.c_str(), 64, 64, 54, 54, 0, 0, 1.0f);
				break;
			}
			case ItemQuality_Epic:
			{
				string itemBackgroundIcon = "media/textures/gui/" + themeName + "/common/items/item_background_epic.tga";
				pNewCraftingItem->AddIcon(m_pRenderer, itemBackgroundIcon.c_str(), 64, 64, 54, 54, 0, 0, 1.0f);
				break;
			}
		}

		if(pInventoryItem->m_quantity != -1)
		{
			char lQuantity[128];
			sprintf(lQuantity, "%i", pInventoryItem->m_quantity);
			int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_18(), "%s", lQuantity);
			pNewCraftingItem->AddText(m_pRenderer, m_pFrontendManager->GetFrontendFont_18(), m_pFrontendManager->GetFrontendFont_18_Outline(), lQuantity, Colour(1.0f, 1.0f, 1.0f, 1.0f), 48-textWidth, 4, true, Colour(0.0f, 0.0f, 0.0f, 1.0f));
		}

		pNewCraftingItem->SetEnterCallBackFunction(_IngredientItemEntered);
		pNewCraftingItem->SetEnterCallBackData(lpSlotItem);

		pNewCraftingItem->SetExitCallBackFunction(_IngredientItemExited);
		pNewCraftingItem->SetExitCallBackData(lpSlotItem);

		lpSlotItem->m_pInventoryItem = pInventoryItem;
		lpSlotItem->m_pCraftingGUI = this;
		lpSlotItem->m_pIngredientIcon = pNewCraftingItem;
		lpSlotItem->m_slotXIndex = ingredientsHorizontalIndex;
		lpSlotItem->m_slotYIndex = ingredientsVerticalIndex;
		lpSlotItem->m_erase = false;

		ingredientsHorizontalIndex++;
		if(ingredientsHorizontalIndex == 3)
		{
			ingredientsVerticalIndex++;
			ingredientsHorizontalIndex = 0;
		}

		m_pCraftingWindow->AddComponent(pNewCraftingItem);

		m_vpIngredientsSlotItem.push_back(lpSlotItem);
	}

	m_pGUI->AddWindow(m_pCraftingWindow);
	m_pCraftingWindow->Show();
}

void CraftingGUI::DeleteIngredientsButtons()
{
	for(unsigned int i = 0; i < m_vpIngredientsSlotItem.size(); i++)
	{
		m_pCraftingWindow->RemoveComponent(m_vpIngredientsSlotItem[i]->m_pIngredientIcon);
		delete m_vpIngredientsSlotItem[i]->m_pIngredientIcon;

		delete m_vpIngredientsSlotItem[i];
		m_vpIngredientsSlotItem[i] = 0;
	}
	m_vpIngredientsSlotItem.clear();
}

bool CraftingGUI::CanCraftRecipe()
{
	bool canCraft = true;
	if(m_pRecipeSlotItemSelected != NULL)
	{
		CraftingRecipe* pRecipe = m_pRecipeSlotItemSelected->m_pCraftingReceipe;
		for (unsigned int i = 0; i < pRecipe->m_vpCraftingItems.size() && canCraft == true; i++)
		{
			InventoryItem* pIntentoryItem = m_pInventoryManager->GetInventoryItemWithTitle(pRecipe->m_vpCraftingItems[i]->m_title);

			if(pIntentoryItem == NULL)
			{
				canCraft = false;
			}
			else
			{
				if (pRecipe->m_vpCraftingItems[i]->m_quantity != -1)
				{
					if (pIntentoryItem->m_quantity < pRecipe->m_vpCraftingItems[i]->m_quantity)
					{
						canCraft = false;
					}
				}
			}
		}
	}
	else
	{
		canCraft = false;
	}

	return canCraft;
}

// Tooltips
void CraftingGUI::UpdateToolTipAppear(float dt)
{
	if(m_toolTipVisible)
	{
		if(m_tooltipAppearDelayTimer <= 0.0f)
		{
			if(m_toolTipComponentsAdded == false)
			{
				switch(m_tooltipQuality)
				{
					case ItemQuality_Common :  { m_pCraftingWindow->AddComponent(m_pTooltipBackground_Common); break; }
					case ItemQuality_Uncommon :  { m_pCraftingWindow->AddComponent(m_pTooltipBackground_Uncommon); break; }
					case ItemQuality_Magical :  { m_pCraftingWindow->AddComponent(m_pTooltipBackground_Magical); break; }
					case ItemQuality_Rare :  { m_pCraftingWindow->AddComponent(m_pTooltipBackground_Rare); break; }
					case ItemQuality_Epic :  { m_pCraftingWindow->AddComponent(m_pTooltipBackground_Epic); break; }
				}

				m_pCraftingWindow->AddComponent(m_pTooltipNameLabel);
				m_pCraftingWindow->AddComponent(m_pTooltipDescriptionLabel);
				m_pCraftingWindow->AddComponent(m_pTooltipSlotLabel);
				m_pCraftingWindow->AddComponent(m_pTooltipQualityLabel);

				m_toolTipComponentsAdded = true;
			}
		}
		else
		{
			m_tooltipAppearDelayTimer -= dt;
		}
	}
}

void CraftingGUI::ShowTooltip(RecipeSlotItem* pRecipeItem)
{
	if(m_toolTipVisible == true)
	{
		return;
	}

	// Set the focused window when we show a tooltip
	m_pCraftingWindow->SetFocusWindow();

	// Replace the tooltip name
	m_pTooltipNameLabel->SetText(pRecipeItem->m_pInventoryItem->m_title);

	// Replace the tooltip description
	string DescriptionText = pRecipeItem->m_pInventoryItem->m_description + pRecipeItem->m_pInventoryItem->GetStatsAttributeString();
	m_pTooltipDescriptionLabel->SetText(DescriptionText);

	// Replace the tooltip equipslot text
	char slotText[32];
	switch(pRecipeItem->m_pInventoryItem->m_equipSlot)
	{
	case EquipSlot_NoSlot: { sprintf(slotText, ""); break; }
	case EquipSlot_LeftHand:
		{
			if(pRecipeItem->m_pInventoryItem->m_right)
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
			if(pRecipeItem->m_pInventoryItem->m_left)
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
	switch(pRecipeItem->m_pInventoryItem->m_itemQuality)
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

	// Set tooltip dimensions
	m_tooltipWidth = 200;
	m_tooltipHeight = 220;
	m_tooltipDescBorder = 15;

	int x = pRecipeItem->m_pResultsIcon->GetLocation().m_x - m_craftingResultsScrollAreaX + 20;
	int y = pRecipeItem->m_pResultsIcon->GetLocation().m_y + 20;

	if(VoxGame::GetInstance()->GetWindowCursorX() > m_windowWidth - m_tooltipWidth - 50)
	{
		x = x + 20 - m_tooltipWidth;
	}
	else
	{
		x = x + 15;
	}
	if((m_windowHeight-VoxGame::GetInstance()->GetWindowCursorY()) > m_windowHeight - m_tooltipHeight - 50)
	{
		y = y + 35 - m_tooltipHeight;
	}
	else
	{
		y = y + 20;
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

	m_tooltipQuality = pRecipeItem->m_pInventoryItem->m_itemQuality;

	m_tooltipAppearDelayTimer = m_pFrontendManager->GetToolTipAppearDelay();

	m_toolTipVisible = true;
	m_toolTipComponentsAdded = false;
}

void CraftingGUI::ShowTooltip(IngredientsSlotItem* pIngredientItem)
{
	if(m_toolTipVisible == true)
	{
		return;
	}

	// Set the focused window when we show a tooltip
	m_pCraftingWindow->SetFocusWindow();

	// Replace the tooltip name
	m_pTooltipNameLabel->SetText(pIngredientItem->m_pInventoryItem->m_title);

	// Replace the tooltip description
	string DescriptionText = pIngredientItem->m_pInventoryItem->m_description + pIngredientItem->m_pInventoryItem->GetStatsAttributeString();
	m_pTooltipDescriptionLabel->SetText(DescriptionText);

	// Replace the tooltip equipslot text
	char slotText[32];
	switch(pIngredientItem->m_pInventoryItem->m_equipSlot)
	{
	case EquipSlot_NoSlot: { sprintf(slotText, ""); break; }
	case EquipSlot_LeftHand:
		{
			if(pIngredientItem->m_pInventoryItem->m_right)
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
			if(pIngredientItem->m_pInventoryItem->m_left)
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
	switch(pIngredientItem->m_pInventoryItem->m_itemQuality)
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

	// Set tooltip dimensions
	m_tooltipWidth = 200;
	m_tooltipHeight = 220;
	m_tooltipDescBorder = 15;

	int x = pIngredientItem->m_pIngredientIcon->GetLocation().m_x;
	int y = pIngredientItem->m_pIngredientIcon->GetLocation().m_y;

	if(VoxGame::GetInstance()->GetWindowCursorX() > m_windowWidth - m_tooltipWidth - 50)
	{
		x = x + 20 - m_tooltipWidth;
	}
	else
	{
		x = x + 15;
	}
	if((m_windowHeight-VoxGame::GetInstance()->GetWindowCursorY()) > m_windowHeight - m_tooltipHeight - 50)
	{
		y = y + 20 - m_tooltipHeight;
	}
	else
	{
		y = y + 20;
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

	m_tooltipQuality = pIngredientItem->m_pInventoryItem->m_itemQuality;

	m_tooltipAppearDelayTimer = m_pFrontendManager->GetToolTipAppearDelay();

	m_toolTipVisible = true;
	m_toolTipComponentsAdded = false;
}

void CraftingGUI::HideTooltip()
{
	m_pCraftingWindow->RemoveComponent(m_pTooltipBackground_Common);
	m_pCraftingWindow->RemoveComponent(m_pTooltipBackground_Uncommon);
	m_pCraftingWindow->RemoveComponent(m_pTooltipBackground_Magical);
	m_pCraftingWindow->RemoveComponent(m_pTooltipBackground_Rare);
	m_pCraftingWindow->RemoveComponent(m_pTooltipBackground_Epic);
	m_pCraftingWindow->RemoveComponent(m_pTooltipNameLabel);
	m_pCraftingWindow->RemoveComponent(m_pTooltipDescriptionLabel);
	m_pCraftingWindow->RemoveComponent(m_pTooltipSlotLabel);
	m_pCraftingWindow->RemoveComponent(m_pTooltipQualityLabel);

	m_toolTipVisible = false;
}

// Updating
void CraftingGUI::Update(float dt)
{
	if (m_loadDelay == true)
	{
		if (m_loadDelayTime <= 0.0f)
		{
			m_loadDelay = false;
			m_pGUI->AddWindow(m_pCraftingWindow);
			m_pCraftingWindow->Show();
		}
		else
		{
			m_loadDelayTime -= dt;
		}
	}

	UpdateToolTipAppear(dt);

	if(VoxGame::GetInstance()->IsPaused() == false)
	{
		if(m_crafting)
		{
			m_craftingTimer += dt;

			if(m_craftingTimer > m_craftingTime)
			{
				m_craftingTimer = m_craftingTime;

				m_crafting = false;
				CraftingComplete();
			}

			m_pCraftingProgressBar->SetCurrentValue((m_craftingTimer / m_craftingTime) * 100.0f);
			int stopperX = (int)(m_pCraftingProgressBar->GetLocation().m_x + m_pCraftingProgressBar->GetDimensions().m_width * (m_craftingTimer / m_craftingTime));
			int stopperY = m_pCraftingProgressBar->GetLocation().m_y - 4;
		}
	}

	// Update the filtered
	if(strcmp(m_cachedSearchText.c_str(), m_pSearchBox->GetText().c_str()) != 0)
	{
		m_cachedSearchText = m_pSearchBox->GetText();
		UpdateFilteredRecipes();
	}

	// Update the results items based on the scroll position and scroll ratio
	int visibleSize = m_pResultsScrollbar->GetScrollArea().m_height;
	int neededHeight = ((int)m_vpRecipeSlotItem_Filtered.size() * 36) + 4;
	int heightDiff = neededHeight - visibleSize;

	if(heightDiff > 0)
	{
		m_pResultsScrollbar->SetScrollSize((float)visibleSize / (float)neededHeight);
		m_pResultsScrollbar->SetDisabled(false);

		float scrollRatio = m_pResultsScrollbar->GetScrollRatio();
		int offsetButtonY = (int)(heightDiff * scrollRatio);

		for(unsigned int i = 0; i < m_vpRecipeSlotItem_Filtered.size(); i++)
		{
			int xPos = -m_craftingResultsScrollAreaWidth;
			int yPos = 4 + m_pResultsScrollbar->GetScrollArea().m_y+m_pResultsScrollbar->GetScrollArea().m_height - 4 - ((i+1)*36);

			m_vpRecipeSlotItem_Filtered[i]->m_pResultsIcon->SetLocation(xPos, yPos + offsetButtonY);
		}
	}
	else
	{
		m_pResultsScrollbar->SetScrollSize(0.0f);
		m_pResultsScrollbar->SetDisabled(true);
	}
}

void CraftingGUI::UpdateCraftButton()
{
	bool canCraft = CanCraftRecipe();

	if(m_crafting == false && canCraft)
	{
		m_pCraftButton->SetDisabled(false);
		m_pCraftButton->SetLabelColour(m_pFrontendManager->GetNormalFontColour());
		m_pCraftButton->SetLabelOutlineColour(Colour(0.0f, 0.0f, 0.0f));
	}
	else
	{
		m_pCraftButton->SetDisabled(true);
		m_pCraftButton->SetLabelColour(m_pFrontendManager->GetDisabledFontColour());
		m_pCraftButton->SetLabelOutlineColour(Colour(0.25f, 0.25f, 0.25f));
	}
}

void CraftingGUI::UpdateFilteredRecipes()
{
	m_pResultsScrollbar->ClearScrollAreaItems();

	m_vpRecipeSlotItem_Filtered.clear();
	for(unsigned int i = 0; i < m_vpRecipeSlotItem.size(); i++)
	{
		string lowerSearchBox = m_pSearchBox->GetText();
		std::transform(lowerSearchBox.begin(), lowerSearchBox.end(), lowerSearchBox.begin(), ::tolower);
		string lowerRecipeName = m_vpRecipeSlotItem[i]->m_recipeName;
		std::transform(lowerRecipeName.begin(), lowerRecipeName.end(), lowerRecipeName.begin(), ::tolower);

		bool foundString = false;
		if(lowerRecipeName.find(lowerSearchBox) != std::string::npos)
		{
			foundString = true;
		}

		for(unsigned int j = 0; j < m_vpRecipeSlotItem[i]->m_pCraftingReceipe->m_vpCraftingItems.size() && foundString == false; j++)
		{
			string lowerIngredientName = m_vpRecipeSlotItem[i]->m_pCraftingReceipe->m_vpCraftingItems[j]->m_title;
			std::transform(lowerIngredientName.begin(), lowerIngredientName.end(), lowerIngredientName.begin(), ::tolower);

			if(lowerIngredientName.find(lowerSearchBox) != std::string::npos)
			{
				foundString = true;
			}
		}

		if(lowerSearchBox == "" || foundString)
		{
			m_vpRecipeSlotItem_Filtered.push_back(m_vpRecipeSlotItem[i]);
		}
	}

	bool stillContainsSelected = false;

	for(unsigned int i = 0; i < m_vpRecipeSlotItem_Filtered.size(); i++)
	{
		int xPos = -m_craftingResultsScrollAreaWidth;
		int yPos = m_craftingResultsScrollAreaHeight - ((i+1)*36);

		m_vpRecipeSlotItem_Filtered[i]->m_pResultsIcon->SetDimensions(xPos, yPos, 152, 32);
		m_vpRecipeSlotItem_Filtered[i]->m_slotIndex = i;

		m_pResultsScrollbar->AddScrollAreaItem(m_vpRecipeSlotItem_Filtered[i]->m_pResultsIcon);

		if(m_pRecipeSlotItemSelected != NULL)
		{
			if(strcmp(m_pRecipeSlotItemSelected->m_pCraftingReceipe->m_pResultItem->m_title.c_str(), m_vpRecipeSlotItem_Filtered[i]->m_recipeName.c_str()) == 0)
			{
				stillContainsSelected = true;
			}
		}
	}

	if(stillContainsSelected == false)
	{
		// Reset pressed button
		ResultsItemPressed(NULL);
	}
}

// Rendering
void CraftingGUI::Render()
{

}

void CraftingGUI::_CloseExitPressed(void *pData)
{
	CraftingGUI* lpCraftingGUI = (CraftingGUI*)pData;
	lpCraftingGUI->CloseExitPressed();
}

void CraftingGUI::CloseExitPressed()
{
	Unload();
}

void CraftingGUI::_RecipeItemEntered(void *apData)
{
	RecipeSlotItem* lpRecipeSlotItem = (RecipeSlotItem*)apData;
	lpRecipeSlotItem->m_pCraftingGUI->RecipeItemEntered(lpRecipeSlotItem);
}

void CraftingGUI::RecipeItemEntered(RecipeSlotItem* pRecipeButtonData)
{
	ShowTooltip(pRecipeButtonData);
}

void CraftingGUI::_RecipeItemExited(void *apData)
{
	RecipeSlotItem* lpRecipeSlotItem = (RecipeSlotItem*)apData;
	lpRecipeSlotItem->m_pCraftingGUI->RecipeItemExited(lpRecipeSlotItem);
}

void CraftingGUI::RecipeItemExited(RecipeSlotItem* pRecipeButtonData)
{
	HideTooltip();
}

void CraftingGUI::_IngredientItemEntered(void *apData)
{
	IngredientsSlotItem* lpIngredientsSlotItem = (IngredientsSlotItem*)apData;
	lpIngredientsSlotItem->m_pCraftingGUI->IngredientItemEntered(lpIngredientsSlotItem);
}

void CraftingGUI::IngredientItemEntered(IngredientsSlotItem* pIngredientButtonData)
{
	ShowTooltip(pIngredientButtonData);
}

void CraftingGUI::_IngredientItemExited(void *apData)
{
	IngredientsSlotItem* lpIngredientsSlotItem = (IngredientsSlotItem*)apData;
	lpIngredientsSlotItem->m_pCraftingGUI->IngredientItemExited(lpIngredientsSlotItem);
}

void CraftingGUI::IngredientItemExited(IngredientsSlotItem* pIngredientButtonData)
{
	HideTooltip();
}

void CraftingGUI::_CraftPressed(void *apData)
{
	CraftingGUI* lpCraftingGUI = (CraftingGUI*)apData;
	lpCraftingGUI->CraftPressed(false);
}

void CraftingGUI::CraftPressed(bool fakeCraft)
{
	if(m_crafting == false)
	{
		m_crafting = true;
		m_craftingTime = 3.5f;
		m_craftingTimer = 0.0f;

		m_pPlayer->SetCrafting(true);
		m_pPlayer->SetCraftingItem(true);

		for(unsigned int i = 0; i < m_vpRecipeSlotItem_Filtered.size(); i++)
		{
			m_vpRecipeSlotItem_Filtered[i]->m_pResultsIcon->SetEnabled(false);
		}

		UpdateCraftButton();
	}
}

void CraftingGUI::_ResultsItemPressed(void *apData)
{
	RecipeSlotItem* lpRecipeSlotItem = (RecipeSlotItem*)apData;
	lpRecipeSlotItem->m_pCraftingGUI->ResultsItemPressed(lpRecipeSlotItem);
}

void CraftingGUI::ResultsItemPressed(RecipeSlotItem* pRecipeButtonData)
{
	// First reset previous selected
	if(m_pRecipeSlotItemSelected != NULL)
	{
		m_pRecipeSlotItemSelected->m_pResultsIcon->SetDefaultIcon(m_pRecipeButton_Icon);
		m_pRecipeSlotItemSelected->m_pResultsIcon->SetHoverIcon(m_pRecipeButton_Hover_Icon);
		m_pRecipeSlotItemSelected->m_pResultsIcon->SetSelectedIcon(m_pRecipeButton_Pressed_Icon);
		m_pRecipeSlotItemSelected->m_pResultsIcon->SetLabelColour(m_pFrontendManager->GetNormalFontColour());
		m_pRecipeSlotItemSelected->m_pResultsIcon->SetEnabled(true);
		m_pRecipeSlotItemSelected->m_pResultsIcon->SetSelected(false);
		m_pRecipeSlotItemSelected->m_pResultsIcon->SetHover(false);
	}

	// Assign new
	m_pRecipeSlotItemSelected = pRecipeButtonData;

	if(m_pRecipeSlotItemSelected != NULL)
	{
		m_pRecipeSlotItemSelected->m_pResultsIcon->SetDefaultIcon(m_pRecipeButton_Selected_Icon);
		m_pRecipeSlotItemSelected->m_pResultsIcon->SetHoverIcon(m_pRecipeButton_Selected_Icon);
		m_pRecipeSlotItemSelected->m_pResultsIcon->SetSelectedIcon(m_pRecipeButton_Selected_Icon);
		m_pRecipeSlotItemSelected->m_pResultsIcon->SetLabelColour(Colour(0.73f, 0.26f, 0.47f));
		m_pRecipeSlotItemSelected->m_pResultsIcon->SetEnabled(false);
	}

	CreateIngredientsButtons();

	UpdateCraftButton();

	HideTooltip();
}
