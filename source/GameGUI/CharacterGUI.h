// ******************************************************************************
// Filename:    CharacterGUI.h
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

#pragma once

#include "../blocks/ChunkManager.h"
#include "../Player/Player.h"
#include "../Inventory/InventoryManager.h"
#include "../gui/draggablerenderrectangle.h"
#include "../gui/formattedlabel.h"
#include "../gui/guiwindow.h"
#include "../gui/openglgui.h"

class FrontendManager;
class InventoryGUI;
class CharacterGUI;
class LootGUI;
class ActionBar;


class CharacterSlotItem
{
public:
	CharacterGUI* m_pCharacterGUI;
	InventoryItem* m_pInventoryItem;
	DraggableRenderRectangle* m_pInventoryIcon;	

	int m_slotX;
	int m_slotY;

	bool m_erase;
};

class CharacterGUI
{
public:
	/* Public methods */
	CharacterGUI(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontendManager, ChunkManager* pChunkManager, Player* pPlayer, InventoryManager* pInventoryManager, int windowWidth, int windowHeight);
	~CharacterGUI();

	void SetInventoryGUI(InventoryGUI* pInventoryGUI);
	void SetLootGUI(LootGUI* pLootGUI);
	void SetActionBar(ActionBar* pActionBar);

	void SetWindowDimensions(int windowWidth, int windowHeight);

	// Skinning the GUI
	void SkinGUI();
	void UnSkinGUI();

	// Loading
	void Load(bool loadDelay = false, float loadDelayTime = 0.0f);
	void Unload();
	bool IsLoadDelayed();
	bool IsLoaded();

	GUIWindow* GetCharacterWindow();

	void CreateInventoryItems();
	void DeleteInventoryItems();
	
	void ShowEquipHover(EquipSlot equipSlot);
	void UpdateEquipHover();
	void HideEquipHover();

	void StartEquipHoverAnimation();
	void StopEquipHoverAnimation();

	// Tooltips
	void UpdateToolTipAppear(float dt);
	void ShowTooltip(CharacterSlotItem* pCharacterItem);
	void HideTooltip();

	void GetPlayerPortraitDimensions(int* x, int* y, int* width, int* height) const;
	void GetPlayerWeaponLeftDimensions(int* x, int* y, int* width, int* height) const;
	void GetPlayerWeaponRightDimensions(int* x, int* y, int* width, int* height) const;
	void GetPlayerHeadDimensions(int* x, int* y, int* width, int* height) const;
	void GetPlayerShoulderDimensions(int* x, int* y, int* width, int* height) const;
	void GetPlayerBodyDimensions(int* x, int* y, int* width, int* height) const;
	void GetPlayerLegsDimensions(int* x, int* y, int* width, int* height) const;
	void GetPlayerHandDimensions(int* x, int* y, int* width, int* height) const;
	void GetPlayerFeetDimensions(int* x, int* y, int* width, int* height) const;
	void GetPlayerAccessory1Dimensions(int* x, int* y, int* width, int* height) const;
	void GetPlayerAccessory2Dimensions(int* x, int* y, int* width, int* height) const;

	void UpdatePlayerStats();

	void Update(float dt);

protected:
	/* Protected methods */
	static void _CharacterItemEntered(void *apData);
	void CharacterItemEntered(CharacterSlotItem* pCharacterItem);

	static void _CharacterItemExited(void *apData);
	void CharacterItemExited(CharacterSlotItem* pCharacterItem);

	static void _CharacterItemPressed(void *apData);
	void CharacterItemPressed(CharacterSlotItem* pCharacterItem);

	static void _CharacterItemReleased(void *apData);
	void CharacterItemReleased(CharacterSlotItem* pCharacterItem);

	static void _CloseExitPressed(void *pData);
	void CloseExitPressed();

	static void _OpenCloseTabPressed(void *pData);
	void OpenCloseTabPressed();

	static void _ArrowLeftPressed(void *pData);
	void ArrowLeftPressed();

	static void _ArrowLeftReleased(void *pData);
	void ArrowLeftReleased();

	static void _ArrowRightPressed(void *pData);
	void ArrowRightPressed();

	static void _ArrowRightReleased(void *pData);
	void ArrowRightReleased();

	static void _EquipHoverAnimationFinished(void *pData);
	void EquipHoverAnimationFinished();

	static void _Stat1IncreasePressed(void *apData);
	void Stat1IncreasePressed();

	static void _Stat2IncreasePressed(void *apData);
	void Stat2IncreasePressed();

	static void _Stat3IncreasePressed(void *apData);
	void Stat3IncreasePressed();

	static void _Stat4IncreasePressed(void *apData);
	void Stat4IncreasePressed();

	static void _Stat5IncreasePressed(void *apData);
	void Stat5IncreasePressed();

	static void _Stat6IncreasePressed(void *apData);
	void Stat6IncreasePressed();

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	Renderer* m_pRenderer;
	OpenGLGUI* m_pGUI;
	FrontendManager* m_pFrontendManager;
	ChunkManager* m_pChunkManager;
	Player* m_pPlayer;
	InventoryManager* m_pInventoryManager;
	InventoryGUI* m_pInventoryGUI;
	LootGUI* m_pLootGUI;
	ActionBar* m_pActionBar;

	int m_windowWidth;
	int m_windowHeight;

	bool m_loaded;

	// Load delay
	bool m_loadDelay;
	float m_loadDelayTime;

	int m_characterWindowWidth;
	int m_characterWindowHeight;
	int m_titlebarWidth;
	int m_titlebarHeight;
	int m_statsTabOpenButtonWidth;
	int m_statsTabOpenButtonHeight;
	int m_statsTabWidth;
	int m_statsTabHeight;
	int m_paperDoll_X;
	int m_paperDoll_Y;
	int m_paperDollWidth;
	int m_paperDollHeight;
	int m_statsTitleLabel_Y;
	int m_statsHeaderLabel_XOffset;
	int m_statsValueLabel_XOffset;

	bool m_leftArrowPressed;
	bool m_rightArrowPressed;

	bool m_statsTabsOpen;
	bool m_statsTabLeftSide;

	// GUI components
	GUIWindow* m_pCharacterWindow;
	Icon* m_pDynamicPaperDollIcon;
	Icon* m_pTitleBarIcon;
	Icon* m_pTitleBarBackgroundIcon;
	Icon* m_pCharacterWindowBackgroundIcon;
	Button* m_pCloseExitButton;
	Button* m_pArrowLeftButton;
	Button* m_pArrowRightButton;
	Icon* m_pTabOpenCloseButton_Icon_Right;
	Icon* m_pTabOpenCloseButton_Icon_Right_Hover;
	Icon* m_pTabOpenCloseButton_Icon_Right_Pressed;
	Icon* m_pTabOpenCloseButton_Icon_Left;
	Icon* m_pTabOpenCloseButton_Icon_Left_Hover;
	Icon* m_pTabOpenCloseButton_Icon_Left_Pressed;
	Button* m_pTabOpenCloseButton;
	Icon* m_pStatsTab_Icon_Left;
	Icon* m_pStatsTab_Icon_Right;
	Label* m_pCharacterTitleLabel;
	Label* m_pStatsHeaderLabel1;
	Label* m_pStatsHeaderLabel2;
	Label* m_pStatsHeaderLabel3;
	Label* m_pStatsHeaderLabel4;
	Label* m_pStatsHeaderLabel5;
	Label* m_pStatsHeaderLabel6;
	FormattedLabel* m_pStatsValueLabel1;
	FormattedLabel* m_pStatsValueLabel2;
	FormattedLabel* m_pStatsValueLabel3;
	FormattedLabel* m_pStatsValueLabel4;
	FormattedLabel* m_pStatsValueLabel5;
	FormattedLabel* m_pStatsValueLabel6;
	Button* m_pStat1IncreaseButton;
	Button* m_pStat2IncreaseButton;
	Button* m_pStat3IncreaseButton;
	Button* m_pStat4IncreaseButton;
	Button* m_pStat5IncreaseButton;
	Button* m_pStat6IncreaseButton;

	Icon* m_pInventoryBackgroundSlotBorder_Common;
	Icon* m_pInventoryBackgroundSlotBorder_Uncommon;
	Icon* m_pInventoryBackgroundSlotBorder_Magical;
	Icon* m_pInventoryBackgroundSlotBorder_Rare;
	Icon* m_pInventoryBackgroundSlotBorder_Epic;

	Icon* pBlockSlotItem;

	Icon* m_pEquipHoverIcon;
	int m_equipHoverXOrigin;
	float m_equipHoverXOffset;

	// Pressed icons
	std::vector<CharacterSlotItem*> m_vpInventorySlotItems;
	CharacterSlotItem* m_pEquippedItems[EquipSlot_NumSlots];

	int m_pressedX;
	int m_pressedY;
	CharacterSlotItem* m_pPressedCharacterItem;

	bool m_addedStatIncreaseButtons;

	// Tooltip
	Icon* m_pTooltipBackground_Common;
	Icon* m_pTooltipBackground_Uncommon;
	Icon* m_pTooltipBackground_Magical;
	Icon* m_pTooltipBackground_Rare;
	Icon* m_pTooltipBackground_Epic;
	Label* m_pTooltipNameLabel;
	FormattedLabel* m_pTooltipDescriptionLabel;
	Label* m_pTooltipSlotLabel;
	Label* m_pTooltipQualityLabel;
	bool m_toolTipVisible;
	int m_tooltipWidth;
	int m_tooltipHeight;
	int m_tooltipDescBorder;
	ItemQuality m_tooltipQuality;
	float m_tooltipAppearDelayTimer;
	bool m_toolTipComponentsAdded;
};
