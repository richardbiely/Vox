// ******************************************************************************
// Filename:    Quest.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 14/08/12
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma warning(disable: 4800) // Disable 'casting to bool' warning C4800

#include "Quest.h"
#include "QuestJournal.h"

#include "../Enemy/EnemyManager.h"
#include "../Items/ItemManager.h"
#include "../Inventory/InventoryManager.h"
#include "../NPC/NPCManager.h"
#include "../VoxGame.h"

#include <algorithm>
#include <fstream>
#include <ostream>
#include <iostream>
#include <string>
using namespace std;


void QuestObjective::Reset()
{
	m_completed = false;

	m_progressX = 0;
}

void QuestObjective::UpdateObjective()
{
	if(m_progressX == m_numberOfX)
	{
		m_completed = true;
	}

	m_pParent->Update();
}

Quest::Quest(string name, string startText, string completedText, string denyText)
{
    m_erase = false;

    m_completed = false;

    m_questName = name;

    m_questStartText = startText;
	m_questCompletedText = completedText;
	m_questDenyText = denyText;

	m_pQuestReward = nullptr;
}

Quest::~Quest()
{
	ClearObjectives();

	if(m_pQuestReward != nullptr)
	{
		delete m_pQuestReward;
		m_pQuestReward = nullptr;
	}
}

void Quest::SetNPCManager(NPCManager* pNPCManager)
{
	m_pNPCManager = pNPCManager;
}

void Quest::SetInventoryManager(InventoryManager* pInventoryManager)
{
	m_pInventoryManager = pInventoryManager;
}

void Quest::SetQuestJournal(QuestJournal* pQuestJournal)
{
	m_pQuestJournal = pQuestJournal;
}

void Quest::SetNPCGiver(NPC* npcGiver)
{
	m_pNPC = npcGiver;
}

void Quest::SetQuestReward(InventoryItem* pReward)
{
	m_pQuestReward = new InventoryItem();
	m_pQuestReward->m_filename = pReward->m_filename;
	m_pQuestReward->m_Iconfilename = pReward->m_Iconfilename;
	m_pQuestReward->m_title = pReward->m_title;
	m_pQuestReward->m_description = pReward->m_description;
	m_pQuestReward->m_itemType = pReward->m_itemType;
	m_pQuestReward->m_item = pReward->m_item;
	m_pQuestReward->m_status = pReward->m_status;
	m_pQuestReward->m_equipSlot = pReward->m_equipSlot;
	m_pQuestReward->m_itemQuality = pReward->m_itemQuality;
	m_pQuestReward->m_left = pReward->m_left;
	m_pQuestReward->m_right = pReward->m_right;
	m_pQuestReward->m_placementR = pReward->m_placementR;
	m_pQuestReward->m_placementG = pReward->m_placementG;
	m_pQuestReward->m_placementB = pReward->m_placementB;
	m_pQuestReward->m_lootSlotX = pReward->m_lootSlotX;
	m_pQuestReward->m_lootSlotY = pReward->m_lootSlotY;
	m_pQuestReward->m_equipped = pReward->m_equipped;
	m_pQuestReward->m_scale = pReward->m_scale;
	m_pQuestReward->m_offsetX = pReward->m_offsetX;
	m_pQuestReward->m_offsetY = pReward->m_offsetY;
	m_pQuestReward->m_offsetZ = pReward->m_offsetZ;
	m_pQuestReward->m_quantity = pReward->m_quantity;
}

void Quest::ClearObjectives()
{
	for(unsigned int i = 0; i < m_vpObjectives.size(); i++)
	{
		//if(m_vpObjectives[i]->m_placementItem != nullptr)
		//{
		//	delete m_vpObjectives[i]->m_placementItem;
		//}

		delete m_vpObjectives[i];
		m_vpObjectives[i] = 0;
	}
	m_vpObjectives.clear();	
}

void Quest::AddQuestObjective(string objectiveText, QuestType type, int numX, eEnemyType enemie, eItem item, NPC* talkNPC, string talkingNPCDialog, InventoryItem* placementItem)
{
	QuestObjective* pObjective = new QuestObjective();

	pObjective->m_objectiveText = objectiveText;
	pObjective->m_questType = type;
	pObjective->m_numberOfX = numX;
	pObjective->m_progressX = 0;
	pObjective->m_enemie = enemie;
    pObjective->m_item = item;
	pObjective->m_talkNPC = talkNPC;
	pObjective->m_talkingNPCDialog = talkingNPCDialog;
	pObjective->m_placementItem = placementItem;

	pObjective->m_completed = false;

	pObjective->m_pParent = this;

	m_vpObjectives.push_back(pObjective);
}

void Quest::ExportQuest()
{
	ofstream exportFile;

	char lfilename[128];

	// Replace all spaces in the quest title, with underscores for the quest filename
	string questName(m_questName);
	replace(questName.begin(), questName.end(), ' ', '_');
	sprintf(lfilename, "media/gamedata/quests/%s.quest", questName.c_str());

	exportFile.open(lfilename, ios_base::binary);

	if(exportFile.is_open())
	{
		// Quest name
		exportFile << m_questName << "|";

		// Quest started text
		exportFile << m_questStartText << "|";

		// Quest completed text
		exportFile << m_questCompletedText << "|";

		// Quest deny text
		exportFile << m_questDenyText << "|";

		// Num objectives
		exportFile << m_vpObjectives.size() << "|";

		// Objectives
		for(unsigned int i = 0; i < m_vpObjectives.size(); i++)
		{
			exportFile << m_vpObjectives[i]->m_objectiveText << "|";
			exportFile << m_vpObjectives[i]->m_questType << "|";
			exportFile << m_vpObjectives[i]->m_numberOfX << "|";
			exportFile << m_vpObjectives[i]->m_enemie << "|";
            exportFile << m_vpObjectives[i]->m_item << "|";
			if(m_vpObjectives[i]->m_talkNPC)
			{
				exportFile << m_vpObjectives[i]->m_talkNPC->GetName() << "|";
				exportFile <<m_vpObjectives[i]->m_talkingNPCDialog << "|";
			}
			else
			{
				exportFile << 0 << "|0|";
			}

			if(m_vpObjectives[i]->m_placementItem)
			{
				exportFile << m_vpObjectives[i]->m_placementItem->m_filename << "|";
				exportFile << m_vpObjectives[i]->m_placementItem->m_Iconfilename << "|";
				exportFile << (int)m_vpObjectives[i]->m_placementItem->m_itemType << "|";
				exportFile << (int)m_vpObjectives[i]->m_placementItem->m_item << "|";
				exportFile << (int)m_vpObjectives[i]->m_placementItem->m_equipSlot << "|";
				exportFile << (int)m_vpObjectives[i]->m_placementItem->m_itemQuality << "|";
				exportFile << m_vpObjectives[i]->m_placementItem->m_left << "|";
				exportFile << m_vpObjectives[i]->m_placementItem->m_right << "|";
				exportFile << m_vpObjectives[i]->m_placementItem->m_title << "|";
				exportFile << m_vpObjectives[i]->m_placementItem->m_description << "|";
				exportFile << m_vpObjectives[i]->m_placementItem->m_placementR << "|";
				exportFile << m_vpObjectives[i]->m_placementItem->m_placementG << "|";
				exportFile << m_vpObjectives[i]->m_placementItem->m_placementB << "|";
				exportFile << m_vpObjectives[i]->m_placementItem->m_scale << "|";
				exportFile << m_vpObjectives[i]->m_placementItem->m_offsetX << "|";
				exportFile << m_vpObjectives[i]->m_placementItem->m_offsetY << "|";
				exportFile << m_vpObjectives[i]->m_placementItem->m_offsetZ << "|";
				exportFile << m_vpObjectives[i]->m_placementItem->m_quantity << "|";
			}
			else
			{
				exportFile << "nullptr" << "|";
				exportFile << "nullptr" << "|";
				exportFile << 0 << "|";
				exportFile << 0 << "|";
				exportFile << 0 << "|";
				exportFile << 0 << "|";
				exportFile << 0 << "|";
				exportFile << 0 << "|";
				exportFile << "nullptr" << "|";
				exportFile << "nullptr" << "|";
				exportFile << 0 << "|";
				exportFile << 0 << "|";
				exportFile << 0 << "|";
				exportFile << 0 << "|";
				exportFile << 0 << "|";
				exportFile << 0 << "|";
				exportFile << 0 << "|";
				exportFile << 0 << "|";
			}
			
		}

		exportFile.close();
	}
}

void Quest::ImportQuest(const char* filename)
{
	ifstream importFile;

	char lfilename[128];
	sprintf(lfilename, "media/gamedata/quests/%s.quest", filename);

	importFile.open(lfilename, ios_base::binary);

	if(importFile.is_open())
	{
		char input[128];
		char * pEnd;

		// Name
		char questName[128];
		importFile.getline(questName, 128, '|');
		m_questName = questName;
		
		// Quest started text
		char questStartText[1024];
		importFile.getline(questStartText, 1024, '|');
		m_questStartText = questStartText;

		// Quest completed text
		char questCompletedText[1024];
		importFile.getline(questCompletedText, 1024, '|');
		m_questCompletedText = questCompletedText;

		// Quest deny text
		char questDenyText[1024];
		importFile.getline(questDenyText, 1024, '|');
		m_questDenyText = questDenyText;

		// Num Objectives
		int numObjectives;
		importFile.getline(input, 128, '|');
		numObjectives = strtol(input, &pEnd, 10);

		// OBjectives
		for(int i = 0; i < numObjectives; i++)
		{
			QuestObjective* pNewObjective = new QuestObjective();

			// Objective text
			char objText[128];
			importFile.getline(objText, 128, '|');
			pNewObjective->m_objectiveText = objText;

			// Type
			int questType;
			importFile.getline(input, 128, '|');
			questType = strtol(input, &pEnd, 10);
			pNewObjective->m_questType = (QuestType)questType;

			// Num X
			importFile.getline(input, 128, '|');
			pNewObjective->m_numberOfX = strtol(input, &pEnd, 10);

			// Enemie
			int enemie;
			importFile.getline(input, 128, '|');
			enemie = strtol(input, &pEnd, 10);
			pNewObjective->m_enemie = (eEnemyType)enemie;

      // Item
      int item;
      importFile.getline(input, 128, '|');
      item = strtol(input, &pEnd, 10);
      pNewObjective->m_item = (eItem)item;

			// Talk NPC
			char npcName[128];
			importFile.getline(npcName, 128, '|');
			NPC* talkNPC = m_pNPCManager->GetNPCByName(npcName);
			pNewObjective->m_talkNPC = talkNPC;

			// Talk NPC dialog
			char talkNPCDialog[256];
			importFile.getline(talkNPCDialog, 128, '|');
			pNewObjective->m_talkingNPCDialog = talkNPCDialog;

			// Scenery placement item
			InventoryItem* pNewItem = nullptr;
			char filename[128];
			importFile.getline(filename, 128, '|');

			char iconFilename[128];
			importFile.getline(iconFilename, 128, '|');

			importFile.getline(input, 128, '|');
			int itemType = strtol(input, &pEnd, 10);

			importFile.getline(input, 128, '|');
			int objectiveItem = strtol(input, &pEnd, 10);

			importFile.getline(input, 128, '|');
			int equipSlot = strtol(input, &pEnd, 10);

			importFile.getline(input, 128, '|');
			int itemQuality = strtol(input, &pEnd, 10);

			importFile.getline(input, 128, '|');
			int left = strtol(input, &pEnd, 10);

			importFile.getline(input, 128, '|');
			int right = strtol(input, &pEnd, 10);

			char title[128];
			importFile.getline(title, 128, '|');

			char description[128];
			importFile.getline(description, 128, '|');

			float r;
			float g;
			float b;
			importFile.getline(input, 128, '|');
			r = (float)strtod(input, &pEnd);
			importFile.getline(input, 128, '|');
			g = (float)strtod(input, &pEnd);
			importFile.getline(input, 128, '|');
			b = (float)strtod(input, &pEnd);

			float scale;
			float offsetX;
			float offsetY;
			float offsetZ;

			importFile.getline(input, 128, '|');
			scale = (float)strtod(input, &pEnd);
			importFile.getline(input, 128, '|');
			offsetX = (float)strtod(input, &pEnd);
			importFile.getline(input, 128, '|');
			offsetY = (float)strtod(input, &pEnd);
			importFile.getline(input, 128, '|');
			offsetZ = (float)strtod(input, &pEnd);

			importFile.getline(input, 128, '|');
			int quantity = strtol(input, &pEnd, 10);

			// Create the new inventory item		
			if(strcmp(filename, "nullptr") != 0)
			{
				pNewItem = new InventoryItem();

				pNewItem->m_filename = filename;

				pNewItem->m_Iconfilename = iconFilename;

				pNewItem->m_title = title;
				pNewItem->m_description = description;

				pNewItem->m_itemType = (InventoryType)itemType;
				pNewItem->m_item = (eItem)objectiveItem;
				pNewItem->m_equipSlot = (EquipSlot)equipSlot;
				pNewItem->m_itemQuality = (ItemQuality)itemQuality;

				pNewItem->m_left = (bool)left;
				pNewItem->m_right = (bool)right;

				pNewItem->m_placementR = r;
				pNewItem->m_placementG = g;
				pNewItem->m_placementB = b;

				pNewItem->m_scale = scale;
				pNewItem->m_offsetX = offsetX;
				pNewItem->m_offsetY = offsetY;
				pNewItem->m_offsetZ = offsetZ;

				pNewItem->m_quantity = quantity;

				pNewItem->m_equipped = false;

				pNewItem->m_remove = false;
			}
			
			pNewObjective->m_placementItem = pNewItem;

			m_vpObjectives.push_back(pNewObjective);
		}

		importFile.close();
	}
}

void Quest::CompleteQuest()
{
	for(unsigned int i = 0; i < m_vpObjectives.size(); i++)
	{
		m_vpObjectives[i]->m_progressX = m_vpObjectives[i]->m_numberOfX;

		m_vpObjectives[i]->m_completed = true;
	}

	Update();
}

bool Quest::CanAcceptQuest(string *acceptError)
{
	for(unsigned int i = 0; i < m_vpObjectives.size(); i++)
	{
		if(m_vpObjectives[i]->m_questType == QuestType_PlaceXScenery)
		{
			if(m_vpObjectives[i]->m_placementItem != nullptr)
			{
				if(m_pInventoryManager->CanAddInventoryItem(m_vpObjectives[i]->m_placementItem->m_title.c_str(), m_vpObjectives[i]->m_placementItem->m_item, m_vpObjectives[i]->m_placementItem->m_quantity) == false)
				{
					*acceptError = "Inventory full";

					return false;
				}
			}
		}
	}

	*acceptError = "";
	return true;
}

void Quest::AcceptQuest()
{
	// Go through each quest objective and add a speech marker over any NPCs that need to be 'talked to'
	for(unsigned int i = 0; i < m_vpObjectives.size(); i++)
	{
		if(m_vpObjectives[i]->m_questType == QuestType_TalkToPerson)
		{
			// SB REDO m_vpObjectives[i]->m_talkNPC->SetOverHeadMarkerStatus(OverHeadMarkerStatus_Speech);
			// SB REDO m_vpObjectives[i]->m_talkNPC->SetTalkQuestDialog(m_vpObjectives[i]->m_talkingNPCDialog);
		}

		if(m_vpObjectives[i]->m_questType == QuestType_PlaceXScenery)
		{
			if(m_vpObjectives[i]->m_placementItem != nullptr)
			{
				m_pInventoryManager->AddInventoryItem(m_vpObjectives[i]->m_placementItem, -1, -1);
			}
		}
	}
}

void Quest::Reset()
{
	m_completed = false;

	for(unsigned int i = 0; i < m_vpObjectives.size(); i++)
	{
		m_vpObjectives[i]->Reset();
	}
}

void Quest::Update()
{
	// If we have a collection quest, check objectives to see if we already have the items we are needing to collect
	for(unsigned int i = 0; i < m_vpObjectives.size(); i++)
	{
		if(m_vpObjectives[i]->m_questType == QuestType_CollectX)
		{
			for(int itemIndex = 0; itemIndex < InventoryManager::MAX_NUM_INVENTORY_SLOTS; itemIndex++)
			{
				InventoryItem* pItem = m_pInventoryManager->GetInventoryItemForSlot(itemIndex);

				if(pItem != nullptr)
				{
					if(m_vpObjectives[i]->m_item == pItem->m_item)
					{
						m_vpObjectives[i]->m_progressX = pItem->m_quantity;

						if(m_vpObjectives[i]->m_progressX > m_vpObjectives[i]->m_numberOfX)
						{
							m_vpObjectives[i]->m_progressX = m_vpObjectives[i]->m_numberOfX;
							m_vpObjectives[i]->m_completed = true;
						}
					}
				}
			}
		}
	}

	bool allCompleted = true;

	for(unsigned int i = 0; i < m_vpObjectives.size(); i++)
	{
		if(m_vpObjectives[i]->m_completed == false)
		{
			allCompleted = false;
		}
	}

	if(allCompleted)
	{
		m_completed = true;

		// SB REDO m_pNPC->SetOverHeadMarkerStatus(OverHeadMarkerStatus_Question);
	}

	VoxGame::GetInstance()->GetQuestGUI()->RefreshQuestButtons();
}

bool Quest::GetCompleted()
{
    return m_completed;
}

string Quest::GetName()
{
    return m_questName;
}

string Quest::GetStartText()
{
    return m_questStartText;
}

string Quest::GetCompletedText()
{
	return m_questCompletedText;
}

string Quest::GetDenyText()
{
	return m_questDenyText;
}

int Quest::GetNumObjectives()
{
	return (int)m_vpObjectives.size();
}

QuestObjective* Quest::GetObjective(int index)
{
	return m_vpObjectives[index];
}

NPC* Quest::GetNPC()
{
	return m_pNPC;
}

InventoryItem* Quest::GetReward()
{
	return m_pQuestReward;
}