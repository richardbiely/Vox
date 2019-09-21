// ******************************************************************************
// Filename:    ActionBar.h
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
#include "../gui/draggablerenderrectangle.h"
#include "../gui/formattedlabel.h"
#include "../gui/guiwindow.h"

class FrontendManager;
class ActionBar;
class InventoryGUI;
class LootGUI;
class CharacterGUI;
class Icon;

enum ActionBarItemType
{
    ActionBarItemType_Item = 0,
    ActionBarItemType_Skill,
};

class ActionButtonItem
{
public:
	ActionBar* m_pActionBar;
	DraggableRenderRectangle* m_pItemIcon;	

    ActionBarItemType m_type;

	int m_slotNum;

	string m_itemTitle;

	int m_inventoryX;
	int m_inventoryY;
	EquipSlot m_equipSlot;

	bool m_erase;
};

class ActionBar
{
public:
	/* Public methods */
	ActionBar(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontendManager, ChunkManager* pChunkManager, Player* pPlayer, InventoryManager* pInventoryManager, int windowWidth, int windowHeight);
	~ActionBar();

	void SetInventoryGUI(InventoryGUI* pInventoryGUI);
	void SetCharacterGUI(CharacterGUI* pCharacterGUI);
	void SetLootGUI(LootGUI* pLootGUI);

	// Skinning the GUI
	void SkinGUI();
	void UnSkinGUI();

	// Loading
	void Load();
	void Unload();
	bool IsLoaded();

	void SetWindowDimensions(int windowWidth, int windowHeight);

	// Supress export
	void SetSupressExport(bool supress);

	// Import / Export action bar
	void ExportActionBar(string playerName);
	void ImportActionBar(string playerName);

	void GetActionSlotDimensions(int indexX, int* x, int* y, int* width, int *height);

	void AddItemToActionBar(InventoryItem* pInventoryItem, int slotIndex, int inventoryX, int inventoryY);
	void RemoveItemFromActionBar(ActionButtonItem* pButton);
	void RemoveInventoryItemFromActionBar(string itemTitle);
	void SwitchItems(int slot1, int slot2);
	void UpdateActionBarSlots(InventoryItem* pInventoryItem, int slotX, int slotY);
	void UpdateActionBarSlots(string itemTitle, int slotX, int slotY);
	ActionButtonItem* GetActionButtonForSlot(int slotIndex);

	void UseActionBarslot(int slotIndex);

	void CreateActionButtonIcon(ActionButtonItem* pActionButton);
	void CreateActionButtons();
	void ClearActionButtons();
	void ClearActionSlotItems();

	void CreateActionSlots();
	void DeleteActionSlots();

	void RepositionActionSlots();

protected:
	/* Protected methods */
	static void _ActionButtonPressed(void *apData);
	void ActionButtonPressed(ActionButtonItem* pActionButton);

	static void _ActionButtonReleased(void *apData);
	void ActionButtonReleased(ActionButtonItem* pActionButton);

	static void _ActionButtonEntered(void *apData);
	void ActionButtonEntered(ActionButtonItem* pActionButton);

	static void _ActionButtonExited(void *apData);
	void ActionButtonExited(ActionButtonItem* pActionButton);

private:
	/* Private methods */

public:
	/* Public members */
	static const int MAX_NUM_ACTION_SLOTS = 6;

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
	CharacterGUI* m_pCharacterGUI;
	LootGUI* m_pLootGUI;
	InventoryGUI* m_pInventoryGUI;

	bool m_loaded;

	int m_windowWidth;
	int m_windowHeight;

	// Force to stop export action bar file
	bool m_supressExport;

	int m_actionSlotSize;
	int m_actionSlotSpacer;
	int m_actionBarX;
	int m_actionBarY;

	Icon* m_pActionButtonBackground_6_Icon;

	Icon* m_pActionButtonBackgroundSlotBorder_Common;
	Icon* m_pActionButtonBackgroundSlotBorder_Uncommon;
	Icon* m_pActionButtonBackgroundSlotBorder_Magical;
	Icon* m_pActionButtonBackgroundSlotBorder_Rare;
	Icon* m_pActionButtonBackgroundSlotBorder_Epic;

	int m_pressedX;
	int m_pressedY;
	ActionButtonItem* m_pPressedActionButtonItem;

	std::vector<RenderRectangle*> m_vpActionSlots;
	std::vector<Label*> m_vpActionLabels;
	std::vector<ActionButtonItem*> m_vpActionSlotItems;
};
