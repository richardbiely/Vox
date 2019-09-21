// ******************************************************************************
// Filename:    QuestManager.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 14/08/12
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "QuestManager.h"

#include "QuestJournal.h"

#include "../NPC/NPCManager.h"
#include "../Enemy/EnemyManager.h"
#include "../Inventory/InventoryManager.h"

#include "../utils/Random.h"


QuestManager::QuestManager()
{
}

QuestManager::~QuestManager()
{
    ClearQuests();
}

void QuestManager::SetNPCManager(NPCManager* pNPCManager)
{
	m_pNPCManager = pNPCManager;
}

void QuestManager::SetInventoryManager(InventoryManager* pInventoryManager)
{
	m_pInventoryManager = pInventoryManager;
}

void QuestManager::SetQuestJournal(QuestJournal* pQuestJournal)
{
	m_pQuestJournal = pQuestJournal;
}

void QuestManager::ClearRandomEnemyList()
{
    m_vRandomEnemyList.clear();
}

void QuestManager::AddRandomEnemyTypeToList(eEnemyType enemy)
{
    m_vRandomEnemyList.push_back(enemy);
}

void QuestManager::ClearQuests()
{
    for(unsigned int i = 0; i < m_vpQuestList.size(); i++)
    {
        delete m_vpQuestList[i];
        m_vpQuestList[i] = 0;
    }
    m_vpQuestList.clear();
}

Quest* QuestManager::CreateQuest(string name, string startText, string completedText, string denyText)
{
    Quest* pQuest = new Quest(name, startText, completedText, denyText);
	pQuest->SetNPCManager(m_pNPCManager);
	pQuest->SetInventoryManager(m_pInventoryManager);
	pQuest->SetQuestJournal(m_pQuestJournal);

    m_vpQuestList.push_back(pQuest);

    return pQuest;
}

Quest* QuestManager::CreateRandomQuest()
{
	return nullptr;
}

Quest* QuestManager::GetQuest(string name)
{
    for(unsigned int i = 0; i < m_vpQuestList.size(); i++)
    {
        if(m_vpQuestList[i]->GetName() == name)
        {
            return m_vpQuestList[i];
        }
    }

    return nullptr;
}

void QuestManager::Update(float dt)
{

}
