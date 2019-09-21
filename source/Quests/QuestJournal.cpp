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

#include "QuestJournal.h"
#include "QuestManager.h"
#include "../Player/Player.h"

#include <algorithm>
#include <fstream>
#include <ostream>
#include <iostream>
#include <string>
using namespace std;


QuestJournal::QuestJournal(QuestManager* pQuestManager)
{
	m_pQuestManager = pQuestManager;
}

QuestJournal::~QuestJournal()
{
	ClearJournal();
}

void QuestJournal::ClearJournal()
{
	for(unsigned int i = 0; i < m_vpQuestJournalList.size(); i++)
	{
		delete m_vpQuestJournalList[i];
		m_vpQuestJournalList[i] = 0;
	}
	m_vpQuestJournalList.clear();
}

void QuestJournal::SetPlayer(Player* pPlayer)
{
	m_pPlayer = pPlayer;
}

void QuestJournal::AddQuestJournalEntry(Quest* pQuest)
{
	QuestJournalEntry* pNewEntry = new QuestJournalEntry();
	pNewEntry->m_pQuest = pQuest;
	pNewEntry->m_status = QuestEntryStatus_Uncompleted;

	m_vpQuestJournalList.push_back(pNewEntry);
}

void QuestJournal::UpdateQuestJournalEntry(Quest* pQuest)
{
	for(unsigned int i = 0; i < m_vpQuestJournalList.size(); i++)
	{
		if(m_vpQuestJournalList[i]->m_pQuest == pQuest)
		{
			m_vpQuestJournalList[i]->m_status = QuestEntryStatus_Completed;
		}
	}
}

void QuestJournal::CompleteAllCurrentQuests()
{
	for(unsigned int i = 0; i < m_vpQuestJournalList.size(); i++)
	{
		if(m_vpQuestJournalList[i]->m_status == QuestEntryStatus_Uncompleted)
		{
			m_vpQuestJournalList[i]->m_status = QuestEntryStatus_Completed;
			m_vpQuestJournalList[i]->m_pQuest->CompleteQuest();
		}
	}
}

int QuestJournal::GetNumCurrentQuests()
{
	int current = 0;
	for(unsigned int i = 0; i < m_vpQuestJournalList.size(); i++)
	{
		if(m_vpQuestJournalList[i]->m_status == QuestEntryStatus_Uncompleted)
		{
			current++;
		}
	}

	return current;
}

int QuestJournal::GetNumCompletedQuests()
{
	int completed = 0;
	for(unsigned int i = 0; i < m_vpQuestJournalList.size(); i++)
	{
		if(m_vpQuestJournalList[i]->m_status == QuestEntryStatus_Completed)
		{
			completed++;
		}
	}

	return completed;
}

Quest* QuestJournal::GetCurrentQuest(int index)
{
	int current = 0;
	for(unsigned int i = 0; i < m_vpQuestJournalList.size(); i++)
	{
		if(m_vpQuestJournalList[i]->m_status == QuestEntryStatus_Uncompleted)
		{
			if(current == index)
			{
				return m_vpQuestJournalList[i]->m_pQuest;
			}

			current++;
		}
	}

	return nullptr;
}

Quest* QuestJournal::GetCompletedQuest(int index)
{
	int completed = 0;
	for(unsigned int i = 0; i < m_vpQuestJournalList.size(); i++)
	{
		if(m_vpQuestJournalList[i]->m_status == QuestEntryStatus_Completed)
		{
			if(completed == index)
			{
				return m_vpQuestJournalList[i]->m_pQuest;
			}

			completed++;
		}
	}

	return nullptr;
}

void QuestJournal::ExportQuestJournal(int playerNum)
{
	ofstream exportFile;
	char lfilename[128];
	sprintf(lfilename, "media/characters/character%i/journal.quests", playerNum);
	exportFile.open(lfilename); 

	if(exportFile.is_open())
	{
		exportFile << (int)m_vpQuestJournalList.size() << "|";

		for(int i = 0; i < (int)m_vpQuestJournalList.size(); i++)
		{
			exportFile << m_vpQuestJournalList[i]->m_status << "|" << m_vpQuestJournalList[i]->m_pQuest->GetName() << "|";

			for(int j = 0; j < m_vpQuestJournalList[i]->m_pQuest->GetNumObjectives(); j++)
			{
				exportFile << m_vpQuestJournalList[i]->m_pQuest->GetObjective(j)->m_progressX << "|";
			}

			exportFile << "\n";
		}

		exportFile.close();
	}
}

void QuestJournal::ImportQuestJournal(int playerNum)
{
	ClearJournal();

	ifstream importFile;
	char lfilename[128];
	sprintf(lfilename, "media/characters/character%i/journal.quests", playerNum);
	importFile.open(lfilename);

	if(importFile.is_open())
	{
		char input[128];
		char * pEnd;

		int numQuests;
		importFile.getline(input, 128, '|');
		numQuests = strtol(input, &pEnd, 10);

		for(int i = 0; i < numQuests; i++)
		{
			int questStatus;
			importFile.getline(input, 128, '|');
			questStatus = strtol(input, &pEnd, 10);

			char questName[128];
			importFile.getline(questName, 128, '|');

			QuestJournalEntry* pNewEntry = new QuestJournalEntry();
			pNewEntry->m_pQuest = m_pQuestManager->GetQuest(questName);
			if(pNewEntry->m_pQuest != nullptr)
			{
				pNewEntry->m_status = (QuestEntryStatus)questStatus;

				for(int j = 0; j < pNewEntry->m_pQuest->GetNumObjectives(); j++)
				{
					int numProgress;
					importFile.getline(input, 128, '|');
					numProgress = strtol(input, &pEnd, 10);

					if((QuestEntryStatus)questStatus == QuestEntryStatus_Uncompleted)
					{
						pNewEntry->m_pQuest->GetObjective(j)->m_progressX = numProgress;
						pNewEntry->m_pQuest->GetObjective(j)->UpdateObjective();
					}
				}

				m_vpQuestJournalList.push_back(pNewEntry);

				if((QuestEntryStatus)questStatus == QuestEntryStatus_Uncompleted)
				{
					// SB REDO m_pPlayer->SetQuest(pNewEntry->m_pQuest);
					// SB REDO m_pGameWindow->GetHUD()->SetCurrentQuest(pNewEntry->m_pQuest);

					// SB REDO pNewEntry->m_pQuest->GetNPC()->SetOverHeadMarkerStatus(OverHeadMarkerStatus_None);

					pNewEntry->m_pQuest->Update();
				}
			}			
		}

		importFile.close();
	}
}