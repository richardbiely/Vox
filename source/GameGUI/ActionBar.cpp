// ******************************************************************************
// Filename:    LootGUI.cpp
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

#pragma warning(disable: 4800) // Disable 'casting to bool' warning C4800

#include "ActionBar.h"
#include "InventoryGUI.h"
#include "CharacterGUI.h"
#include "LootGUI.h"

#include "../frontend/FrontendManager.h"
#include "../Inventory/InventoryManager.h"
#include "../VoxGame.h"

#include <fstream>
#include <ostream>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
using namespace std;


ActionBar::ActionBar(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontendManager, ChunkManager* pChunkManager, Player* pPlayer, InventoryManager* pInventoryManager, int windowWidth, int windowHeight)
{
	m_pRenderer = pRenderer;
	m_pGUI = pGUI;
	m_pFrontendManager = pFrontendManager;
	m_pChunkManager = pChunkManager;
	m_pPlayer = pPlayer;
	m_pInventoryManager = pInventoryManager;

	m_loaded = false;

	m_supressExport = false;

	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;

	m_pActionButtonBackground_6_Icon = new Icon(m_pRenderer, "", 392, 76);
	m_pActionButtonBackground_6_Icon->SetDepth(2.0f);

	m_pActionButtonBackgroundSlotBorder_Common = new Icon(m_pRenderer, "", 64, 64);
	m_pActionButtonBackgroundSlotBorder_Common->SetDepth(2.0f);

	m_pActionButtonBackgroundSlotBorder_Uncommon = new Icon(m_pRenderer, "", 64, 64);
	m_pActionButtonBackgroundSlotBorder_Uncommon->SetDepth(2.0f);

	m_pActionButtonBackgroundSlotBorder_Magical = new Icon(m_pRenderer, "", 64, 64);
	m_pActionButtonBackgroundSlotBorder_Magical->SetDepth(2.0f);

	m_pActionButtonBackgroundSlotBorder_Rare = new Icon(m_pRenderer, "", 64, 64);
	m_pActionButtonBackgroundSlotBorder_Rare->SetDepth(2.0f);

	m_pActionButtonBackgroundSlotBorder_Epic = new Icon(m_pRenderer, "", 64, 64);
	m_pActionButtonBackgroundSlotBorder_Epic->SetDepth(2.0f);

	SetWindowDimensions(m_windowWidth, m_windowHeight);
}

ActionBar::~ActionBar()
{
	delete m_pActionButtonBackground_6_Icon;

	delete m_pActionButtonBackgroundSlotBorder_Common;
	delete m_pActionButtonBackgroundSlotBorder_Uncommon;
	delete m_pActionButtonBackgroundSlotBorder_Magical;
	delete m_pActionButtonBackgroundSlotBorder_Rare;
	delete m_pActionButtonBackgroundSlotBorder_Epic;

	DeleteActionSlots();
	ClearActionSlotItems();
}

void ActionBar::SetInventoryGUI(InventoryGUI* pInventoryGUI)
{
	m_pInventoryGUI = pInventoryGUI;
}

void ActionBar::SetCharacterGUI(CharacterGUI* pCharacterGUI)
{
	m_pCharacterGUI = pCharacterGUI;
}

void ActionBar::SetLootGUI(LootGUI* pLootGUI)
{
	m_pLootGUI = pLootGUI;
}

// Skinning the GUI
void ActionBar::SkinGUI()
{
	string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();

	string iconName = "media/textures/gui/" + themeName + "/ActionBar/actionbar_background_6.tga";
	m_pActionButtonBackground_6_Icon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/items/border_common.tga";
	m_pActionButtonBackgroundSlotBorder_Common->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/items/border_uncommon.tga";
	m_pActionButtonBackgroundSlotBorder_Uncommon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/items/border_magical.tga";
	m_pActionButtonBackgroundSlotBorder_Magical->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/items/border_rare.tga";
	m_pActionButtonBackgroundSlotBorder_Rare->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/common/items/border_epic.tga";
	m_pActionButtonBackgroundSlotBorder_Epic->SetIcon(iconName);
}

void ActionBar::UnSkinGUI()
{

}

// Loading
void ActionBar::Load()
{
	m_pGUI->AddComponent(m_pActionButtonBackground_6_Icon);

	CreateActionSlots();
	CreateActionButtons();

	for(unsigned int i = 0; i < m_vpActionSlots.size(); i++)
	{
		m_pGUI->AddComponent(m_vpActionSlots[i]);
	}

	for(unsigned int i = 0; i < m_vpActionLabels.size(); i++)
	{
		m_pGUI->AddComponent(m_vpActionLabels[i]);
	}

	m_pGUI->SetDraggingComponentPriority(NULL);

	m_loaded = true;
}

void ActionBar::Unload()
{
	for(unsigned int i = 0; i < m_vpActionSlots.size(); i++)
	{
		m_pGUI->RemoveComponent(m_vpActionSlots[i]);
	}

	for(unsigned int i = 0; i < m_vpActionLabels.size(); i++)
	{
		m_pGUI->RemoveComponent(m_vpActionLabels[i]);
	}

	m_pGUI->RemoveComponent(m_pActionButtonBackground_6_Icon);

	ClearActionButtons();
	ClearActionSlotItems();
	DeleteActionSlots();

	m_loaded = false;
}

bool ActionBar::IsLoaded()
{
	return m_loaded;
}

void ActionBar::SetWindowDimensions(int windowWidth, int windowHeight)
{
	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;

	m_pActionButtonBackground_6_Icon->SetDimensions(m_windowWidth/2 - 392/2, 12, 392, 76);

	m_actionSlotSize = 64;
	m_actionSlotSpacer = 0;
	m_actionBarX = (int)(m_windowWidth*0.5f - (m_actionSlotSize*MAX_NUM_ACTION_SLOTS)*0.5f - (m_actionSlotSpacer*(MAX_NUM_ACTION_SLOTS-1))*0.5f);
	m_actionBarY = 20;

	RepositionActionSlots();
}

// Supress export
void ActionBar::SetSupressExport(bool supress)
{
	m_supressExport = supress;
}

// Import / Export action bar
void ActionBar::ExportActionBar(string playerName)
{
	if (m_supressExport)
	{
		return;
	}

	ofstream exportFile;
	char lfilename[128];
	sprintf(lfilename, "saves/characters/%s/%s.actionbar", playerName.c_str(), playerName.c_str());
	exportFile.open(lfilename, ios_base::binary);

	if(exportFile.is_open())
	{
		// Export the stored inventory items
		exportFile << m_vpActionSlotItems.size() << "|";

		for(int i = 0; i < MAX_NUM_ACTION_SLOTS; i++)
		{
			ActionButtonItem* lpAction = GetActionButtonForSlot(i);

			if(lpAction != NULL)
			{
				InventoryItem* pInventoryItem = m_pInventoryManager->GetInventoryItemWithTitle(lpAction->m_itemTitle);

				exportFile << lpAction->m_slotNum << "|";
				exportFile << lpAction->m_inventoryX << "|";
				exportFile << lpAction->m_inventoryY << "|";
				exportFile << lpAction->m_equipSlot << "|";
				exportFile << (pInventoryItem == NULL ? false : pInventoryItem->m_equipped) << "|";
			}
		}

		// Export the file signature
		exportFile << "signature: " << "\n";

		exportFile.close();
	}
}

void ActionBar::ImportActionBar(string playerName)
{
	ifstream importFile;
	char lfilename[128];
	sprintf(lfilename, "saves/characters/%s/%s.actionbar", playerName.c_str(), playerName.c_str());
	importFile.open(lfilename, ios_base::binary);

	if(importFile.is_open())
	{
		char input[128];
		char * pEnd;

		// Import the stored inventory items
		importFile.getline(input, 128, '|');
		int numitems = strtol(input, &pEnd, 10);

		for(int i = 0; i < numitems; i++)
		{
			importFile.getline(input, 128, '|');
			int slotIndex = strtol(input, &pEnd, 10);

			importFile.getline(input, 128, '|');
			int inventoryX = strtol(input, &pEnd, 10);

			importFile.getline(input, 128, '|');
			int inventoryY = strtol(input, &pEnd, 10);

			importFile.getline(input, 128, '|');
			int equipSlot = strtol(input, &pEnd, 10);

			importFile.getline(input, 128, '|');
			int equipped = strtol(input, &pEnd, 10);

			InventoryItem* pItem = NULL;

			if((bool)equipped == true)
			{
				pItem = m_pInventoryManager->GetInventoryItemForEquipSlot((EquipSlot)equipSlot);
			}
			else
			{
				pItem = m_pInventoryGUI->GetInventorySlotItem(inventoryX, inventoryY) != NULL ? m_pInventoryGUI->GetInventorySlotItem(inventoryX, inventoryY)->m_pInventoryItem : NULL;
			}

			if(pItem != NULL)
			{
				AddItemToActionBar(pItem, slotIndex, inventoryX, inventoryY);
			}
		}

		// Import the file signature
		string tempString;
		string signature;
		importFile >> tempString >> signature;
		
		importFile.close();
	}
}

void ActionBar::GetActionSlotDimensions(int indexX, int* x, int* y, int* width, int *height)
{
	int slotSize = m_actionSlotSize;
	int slotSpacer = m_actionSlotSpacer;

	int xSlotPos = m_actionBarX + (indexX * (m_actionSlotSpacer + m_actionSlotSize));
	int ySlotPos = m_actionBarY;

	*x = xSlotPos;
	*y = ySlotPos;

	*width = slotSize;
	*height = slotSize;
}

void ActionBar::AddItemToActionBar(InventoryItem* pInventoryItem, int slotIndex, int inventoryX, int inventoryY)
{
	// First check if any other action bar item already exists and remove it
	ActionButtonItem* pBarItem = GetActionButtonForSlot(slotIndex);
	if(pBarItem != NULL)
	{
		RemoveItemFromActionBar(pBarItem);
	}

	ActionButtonItem* pActionButton = new ActionButtonItem();

	pActionButton->m_pActionBar = this;
	//pActionButton->m_pInventorySlotItem = pItem;
	//pActionButton->m_pItemIcon = pNewSlotItem;
	pActionButton->m_pItemIcon = NULL;
	pActionButton->m_type = ActionBarItemType_Item;
	pActionButton->m_itemTitle = pInventoryItem->m_title;
	pActionButton->m_slotNum = slotIndex;
	pActionButton->m_inventoryX = inventoryX;
	pActionButton->m_inventoryY = inventoryY;
	pActionButton->m_equipSlot = pInventoryItem->m_equipSlot;
	pActionButton->m_erase = false;

	CreateActionButtonIcon(pActionButton);

	m_vpActionSlotItems.push_back(pActionButton);
}

bool needs_erasing(ActionButtonItem* aI)
{
	bool needsErase = aI->m_erase;

	if(needsErase == true)
	{
		delete aI;
	}

	return needsErase;
}

void ActionBar::RemoveItemFromActionBar(ActionButtonItem* pButton)
{
	pButton->m_erase = true;

	m_pGUI->RemoveComponent(pButton->m_pItemIcon);

	m_vpActionSlotItems.erase( remove_if(m_vpActionSlotItems.begin(), m_vpActionSlotItems.end(), needs_erasing), m_vpActionSlotItems.end() );
}

void ActionBar::RemoveInventoryItemFromActionBar(string itemTitle)
{
	for(unsigned int i = 0; i < m_vpActionSlotItems.size(); i++)
	{
		if(strcmp(m_vpActionSlotItems[i]->m_itemTitle.c_str(), itemTitle.c_str()) == 0)
		{
			RemoveItemFromActionBar(m_vpActionSlotItems[i]);
		}
	}

	ExportActionBar(m_pPlayer->GetName());
}

void ActionBar::SwitchItems(int slot1, int slot2)
{
	ActionButtonItem* pSlot1 = GetActionButtonForSlot(slot1);
	ActionButtonItem* pSlot2 = GetActionButtonForSlot(slot2);

	if(pSlot1 != NULL)
	{
		pSlot1->m_slotNum = slot2;

		int x;
		int y;
		int width;
		int height;
		GetActionSlotDimensions(slot2, &x, &y, &width, &height);
		pSlot1->m_pItemIcon->SetLocation(x, y);
	}

	if(pSlot2 != NULL)
	{
		pSlot2->m_slotNum = slot1;

		int x;
		int y;
		int width;
		int height;
		GetActionSlotDimensions(slot1, &x, &y, &width, &height);
		pSlot2->m_pItemIcon->SetLocation(x, y);
	}
}

void ActionBar::UpdateActionBarSlots(InventoryItem* pInventoryItem, int slotX, int slotY)
{
	for(unsigned int i = 0; i < m_vpActionSlotItems.size(); i++)
	{
		if(strcmp(m_vpActionSlotItems[i]->m_itemTitle.c_str(), pInventoryItem->m_title.c_str()) == 0)
		{
			m_vpActionSlotItems[i]->m_inventoryX = slotX;
			m_vpActionSlotItems[i]->m_inventoryY = slotY;
		}
	}

	ExportActionBar(m_pPlayer->GetName());
}

void ActionBar::UpdateActionBarSlots(string itemTitle, int slotX, int slotY)
{
	for(unsigned int i = 0; i < m_vpActionSlotItems.size(); i++)
	{
		if(strcmp(m_vpActionSlotItems[i]->m_itemTitle.c_str(), itemTitle.c_str()) == 0)
		{
			m_vpActionSlotItems[i]->m_inventoryX = slotX;
			m_vpActionSlotItems[i]->m_inventoryY = slotY;
		}
	}

	ExportActionBar(m_pPlayer->GetName());
}

void ActionBar::UseActionBarslot(int slotIndex)
{
	if(m_pInventoryGUI->IsLoaded())
	{
		// Dont allow to use the action bar when the inventory GUI is open.
		return;
	}

	if (m_pPlayer->IsCrafting())
	{
		// Don't allow to use the action bar when we are crafting.
		return;
	}

	ActionButtonItem* pItem = GetActionButtonForSlot(slotIndex);
	if(pItem != NULL && pItem->m_inventoryX != -1 && pItem->m_inventoryY != -1)
	{
		if(pItem->m_type == ActionBarItemType_Item)
		{
			InventoryItem* pInventoryItem = m_pInventoryManager->GetInventoryItemWithTitle(pItem->m_itemTitle);

			if(pInventoryItem != NULL)
			{
				if(pInventoryItem->m_itemType == InventoryType_Consumable)
				{
					// Consume the item
				}
				else
				{
					if(pInventoryItem->m_equipSlot != EquipSlot_NoSlot)
					{
						// Equip the item
						if(pInventoryItem->m_equipped == false)
						{
							// Dual handed weapon checks
							if(pInventoryItem->m_equipSlot == EquipSlot_RightHand)
							{
								InventoryItem* pLeftHanded = m_pInventoryManager->GetInventoryItemForEquipSlot(EquipSlot_LeftHand);
								if(pInventoryItem->m_left || (pLeftHanded != NULL && pLeftHanded->m_right))
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
										UpdateActionBarSlots(pLeftHanded, slotX, slotY);
									}
								}
							}
							if(pInventoryItem->m_equipSlot == EquipSlot_LeftHand)
							{
								InventoryItem* pRightHanded = m_pInventoryManager->GetInventoryItemForEquipSlot(EquipSlot_RightHand);
								if(pInventoryItem->m_right || (pRightHanded != NULL && pRightHanded->m_left))
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
										UpdateActionBarSlots(pRightHanded, slotX, slotY);
									}
								}
							}

							m_pPlayer->UnequipItem(pInventoryItem->m_equipSlot, pInventoryItem->m_left, pInventoryItem->m_right);

							m_pInventoryManager->EquipInventoryItem(pItem->m_inventoryX, pItem->m_inventoryY, pInventoryItem->m_equipSlot);
							m_pInventoryGUI->EquipItem(pInventoryItem, pItem->m_inventoryX, pItem->m_inventoryY);

							m_pInventoryManager->SetInventoryGUINeedsUpdate(true);
							m_pInventoryManager->SetCharacterGUINeedsUpdate(true);
						}
					}
				}
			}
			else
			{
				// Try to see if we have an equipped consumable item that we need to consume,
				InventoryItem* pEquippedItem = m_pInventoryManager->GetInventoryItemForEquipSlot(EquipSlot_LeftHand);

				if(pEquippedItem != NULL)
				{
					if(strcmp(pItem->m_itemTitle.c_str(), pEquippedItem->m_title.c_str()) == 0)
					{
						if(pEquippedItem->m_itemType == InventoryType_Consumable)
						{
							// Consume the item
						}
					}
				}
			}
		}
	}

	ExportActionBar(m_pPlayer->GetName());
}

void ActionBar::CreateActionButtonIcon(ActionButtonItem* pActionButton)
{
	int x;
	int y;
	int width;
	int height;
	GetActionSlotDimensions(pActionButton->m_slotNum, &x, &y, &width, &height);

	DraggableRenderRectangle* pNewSlotItem = new DraggableRenderRectangle(m_pRenderer);
	InventoryItem* pInventoryItem = NULL;
	if(pActionButton->m_inventoryX == -1 && pActionButton->m_inventoryY == -1)
	{
		pInventoryItem = m_pInventoryManager->GetInventoryItemForEquipSlot(pActionButton->m_equipSlot);
	}
	else
	{
		pInventoryItem = m_pInventoryManager->GetInventoryItemWithTitle(pActionButton->m_itemTitle);
	}

	switch(pInventoryItem->m_itemQuality)
	{
	case ItemQuality_Common: { pNewSlotItem->SetIcon(m_pActionButtonBackgroundSlotBorder_Common); break; }
	case ItemQuality_Uncommon: { pNewSlotItem->SetIcon(m_pActionButtonBackgroundSlotBorder_Uncommon); break; }
	case ItemQuality_Magical: { pNewSlotItem->SetIcon(m_pActionButtonBackgroundSlotBorder_Magical); break; }
	case ItemQuality_Rare: { pNewSlotItem->SetIcon(m_pActionButtonBackgroundSlotBorder_Rare); break; }
	case ItemQuality_Epic: { pNewSlotItem->SetIcon(m_pActionButtonBackgroundSlotBorder_Epic); break; }
	}
	pNewSlotItem->SetDimensions(x, y, width, height);
	pNewSlotItem->SetDepth(8.0f);

	char lItemTexture[128];
	sprintf(lItemTexture, "%s", pInventoryItem->m_Iconfilename.c_str());
	pNewSlotItem->AddIcon(m_pRenderer, lItemTexture, 64, 64, 56, 56, 4, 4, 1.5f);

	string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();
	switch (pInventoryItem->m_itemQuality)
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

	pActionButton->m_pItemIcon = pNewSlotItem;

	pNewSlotItem->SetPressedCallBackFunction(_ActionButtonPressed);
	pNewSlotItem->SetPressedCallBackData(pActionButton);

	pNewSlotItem->SetReleasedCallBackFunction(_ActionButtonReleased);
	pNewSlotItem->SetReleasedCallBackData(pActionButton);

	pNewSlotItem->SetEnterCallBackFunction(_ActionButtonEntered);
	pNewSlotItem->SetEnterCallBackData(pActionButton);

	pNewSlotItem->SetExitCallBackFunction(_ActionButtonExited);
	pNewSlotItem->SetExitCallBackData(pActionButton);

	m_pGUI->AddComponent(pNewSlotItem);
}

void ActionBar::CreateActionButtons()
{
	for(unsigned int i = 0; i < m_vpActionSlotItems.size(); i++)
	{
		if(m_vpActionSlotItems[i]->m_pItemIcon == NULL)
		{
			CreateActionButtonIcon(m_vpActionSlotItems[i]);
		}
	}
}

void ActionBar::ClearActionButtons()
{
	for(unsigned int i = 0; i < m_vpActionSlotItems.size(); i++)
	{
		if(m_vpActionSlotItems[i]->m_pItemIcon != NULL)
		{
			m_pGUI->RemoveComponent(m_vpActionSlotItems[i]->m_pItemIcon);
			delete m_vpActionSlotItems[i]->m_pItemIcon;
			m_vpActionSlotItems[i]->m_pItemIcon = NULL;
		}
	}
}

void ActionBar::ClearActionSlotItems()
{
	for(unsigned int i = 0; i < m_vpActionSlotItems.size(); i++)
	{
		delete m_vpActionSlotItems[i];
		m_vpActionSlotItems[i] = 0;
	}
	m_vpActionSlotItems.clear();
}

void ActionBar::CreateActionSlots()
{
	for(int i = 0; i < MAX_NUM_ACTION_SLOTS; i++)
	{
		int x;
		int y;
		int width;
		int height;
		GetActionSlotDimensions(i, &x, &y, &width, &height);

		// Create the action backgrounds
		string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();
		string blankSlotIcon = "media/textures/gui/" + themeName + "/ActionBar/blank_slot.tga";
		Icon* pNewIcon = new Icon(m_pRenderer, blankSlotIcon.c_str(), 64, 64);
		pNewIcon->SetDimensions(x, y, width, height);
		pNewIcon->SetDepth(3.0f);

		m_vpActionSlots.push_back(pNewIcon);

		// Create the action labels
		char labelText[32];
		sprintf(labelText, "%i", (i < 9 ? i+1 : 0));

		int xPos = x + 9;
		int yPos = y + 9;
		Label* pNewLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_25(), labelText, Colour(1.0f, 1.0f, 1.0f, 1.0f));
		pNewLabel->SetOutline(true);
		pNewLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
		pNewLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_25_Outline());
		pNewLabel->SetLocation(xPos, yPos);
		pNewLabel->SetDepth(9.0f);

		m_vpActionLabels.push_back(pNewLabel);
	}
}

void ActionBar::DeleteActionSlots()
{
	// Clear action slots
	for(unsigned int i = 0; i < m_vpActionSlots.size(); i++)
	{
		delete m_vpActionSlots[i];
		m_vpActionSlots[i] = 0;
	}
	m_vpActionSlots.clear();

	// Clear action labels
	for(unsigned int i = 0; i < m_vpActionLabels.size(); i++)
	{
		delete m_vpActionLabels[i];
		m_vpActionLabels[i] = 0;
	}
	m_vpActionLabels.clear();

	ClearActionButtons();
}

void ActionBar::RepositionActionSlots()
{
	int x;
	int y;
	int width;
	int height;
	for(unsigned int i = 0; i < MAX_NUM_ACTION_SLOTS && i < m_vpActionSlots.size(); i++)
	{
		GetActionSlotDimensions(i, &x, &y, &width, &height);

		m_vpActionSlots[i]->SetDimensions(x, y, width, height);
	}

	for(unsigned int i = 0; i < m_vpActionLabels.size(); i++)
	{
		GetActionSlotDimensions(i, &x, &y, &width, &height);

		int xPos = x + 7;
		int yPos = y + 7;
		m_vpActionLabels[i]->SetLocation(xPos, yPos);
	}

	for(unsigned int i = 0; i < m_vpActionSlotItems.size(); i++)
	{
		GetActionSlotDimensions(m_vpActionSlotItems[i]->m_slotNum, &x, &y, &width, &height);

		if(m_vpActionSlotItems[i]->m_pItemIcon != NULL)
		{
			m_vpActionSlotItems[i]->m_pItemIcon->SetDimensions(x, y, width, height);
		}
	}
}

ActionButtonItem* ActionBar::GetActionButtonForSlot(int slotIndex)
{
	for(unsigned int i = 0; i < m_vpActionSlotItems.size(); i++)
	{
		if(m_vpActionSlotItems[i]->m_slotNum == slotIndex)
		{
			return m_vpActionSlotItems[i];
		}
	}

	return NULL;
}

void ActionBar::_ActionButtonPressed(void *apData)
{
	ActionButtonItem* lpActionButton = (ActionButtonItem*)apData;
	lpActionButton->m_pActionBar->ActionButtonPressed(lpActionButton);
}

void ActionBar::ActionButtonPressed(ActionButtonItem* pActionButton)
{
	m_pPressedActionButtonItem = pActionButton;

	Dimensions lDimensions = pActionButton->m_pItemIcon->GetDimensions();
	m_pressedX = lDimensions.m_x;
	m_pressedY = lDimensions.m_y;

	m_pGUI->SetDraggingComponentPriority(pActionButton->m_pItemIcon);

	// Temporarily increase the depth of the dragged icon
	pActionButton->m_pItemIcon->SetDepth(100.0f);
}

void ActionBar::_ActionButtonReleased(void *apData)
{
	ActionButtonItem* lpActionButton = (ActionButtonItem*)apData;
	lpActionButton->m_pActionBar->ActionButtonReleased(lpActionButton);
}

void ActionBar::ActionButtonReleased(ActionButtonItem* pActionButton)
{
	if(m_pPressedActionButtonItem == NULL)
	{
		return;
	}

	m_pPressedActionButtonItem = NULL;

	m_pGUI->SetDraggingComponentPriority(NULL);

	int x;
	int y;
	int width;
	int height;
	POINT lMouse = { VoxGame::GetInstance()->GetWindowCursorX(), (m_windowHeight-VoxGame::GetInstance()->GetWindowCursorY()) };
	bool switched = false;
	bool sameSlot = false;
	int switchSlot = -1;
	for(int i = 0; i < MAX_NUM_ACTION_SLOTS && switched == false; i++)
	{
		GetActionSlotDimensions(i, &x, &y, &width, &height);

		// Check if we released (mouse cursor) in the boundary of another slot
		if(lMouse.x > x && lMouse.x < x+width && lMouse.y > y && lMouse.y < y+height)
		{
			switched = true;
			switchSlot = i;

			if(pActionButton->m_slotNum == i)
			{
				sameSlot = true;
			}
		}
	}

	if(switched == true && sameSlot == false)
	{
		// Switch two action buttons
		SwitchItems(pActionButton->m_slotNum, switchSlot);
		ExportActionBar(m_pPlayer->GetName());
	}
	else if(switched == true && sameSlot == true)
	{
		// Return the action button to the original place
		pActionButton->m_pItemIcon->SetLocation(m_pressedX, m_pressedY);
	}
	else if(switched == false)
	{
		// Remove the action button
		RemoveItemFromActionBar(pActionButton);
		ExportActionBar(m_pPlayer->GetName());
	}

	if(switched == true)
	{
		// Revert depth back to normal for action button
		pActionButton->m_pItemIcon->SetDepth(8.0f);
	}
}

void ActionBar::_ActionButtonEntered(void *apData)
{
	ActionButtonItem* lpActionButton = (ActionButtonItem*)apData;
	lpActionButton->m_pActionBar->ActionButtonEntered(lpActionButton);
}

void ActionBar::ActionButtonEntered(ActionButtonItem* pActionButton)
{

}

void ActionBar::_ActionButtonExited(void *apData)
{
	ActionButtonItem* lpActionButton = (ActionButtonItem*)apData;
	lpActionButton->m_pActionBar->ActionButtonExited(lpActionButton);
}

void ActionBar::ActionButtonExited(ActionButtonItem* pActionButton)
{

}
