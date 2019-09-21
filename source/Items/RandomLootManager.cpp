// ******************************************************************************
// Filename:    RandomLootManager.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 01/04/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "RandomLootManager.h"
#include "../utils/Random.h"
#include "../VoxGame.h"

#include <fstream>
#include <ostream>
#include <iostream>
#include <string>
using namespace std;


RandomLootManager::RandomLootManager()
{
	// Create the enemy ingredient spawn data lists
	AddEnemyIngredientSpawnData(eEnemyType_GreenSlime, eItem_SlimeJelly, 1, 3);
	AddEnemyIngredientSpawnData(eEnemyType_RedSlime, eItem_SlimeJelly, 1, 3);
	AddEnemyIngredientSpawnData(eEnemyType_BlueSlime, eItem_SlimeJelly, 1, 3);
	AddEnemyIngredientSpawnData(eEnemyType_YellowSlime, eItem_SlimeJelly, 1, 3);
	AddEnemyIngredientSpawnData(eEnemyType_Bee, eItem_BeeWing, 1, 1);
	AddEnemyIngredientSpawnData(eEnemyType_NormalSkeleton, eItem_Bone, 1, 2);
	AddEnemyIngredientSpawnData(eEnemyType_RangedSkeleton, eItem_Bone, 1, 2);
	AddEnemyIngredientSpawnData(eEnemyType_MeleeSkeleton, eItem_Bone, 1, 2);
	AddEnemyIngredientSpawnData(eEnemyType_MageSkeleton, eItem_Bone, 1, 2);

	// Create the random loot
	bool createRandomLootWeapons = true;
	if (createRandomLootWeapons)
	{
		AddRandomLoot(eEquipment_MageStaff);
		AddRandomLoot(eEquipment_NecroStaff);
		AddRandomLoot(eEquipment_PriestStaff);
		AddRandomLoot(eEquipment_DruidStaff);
		AddRandomLoot(eEquipment_Boomerang);
		AddRandomLoot(eEquipment_DragonBow);
		AddRandomLoot(eEquipment_BoneSword);
		AddRandomLoot(eEquipment_AshSword);
		AddRandomLoot(eEquipment_FireballHandLeft);
		AddRandomLoot(eEquipment_FireballHandRight);
	}

	bool createRandomLootArmor = true;
	if (createRandomLootArmor)
	{
		AddRandomLoot(eEquipment_BlacksmithGloves);
		AddRandomLoot(eEquipment_NormalBoots);
		AddRandomLoot(eEquipment_NormalGloves);
		AddRandomLoot(eEquipment_NormalShoulders);
		AddRandomLoot(eEquipment_RocketBoots);
		AddRandomLoot(eEquipment_RegrowthBoots);
		AddRandomLoot(eEquipment_WoodenHelm);
		AddRandomLoot(eEquipment_WoodenArmor);
		AddRandomLoot(eEquipment_WoodenPants);
		AddRandomLoot(eEquipment_WoodenGloves);
		AddRandomLoot(eEquipment_WoodenBoots);
		AddRandomLoot(eEquipment_WoodenShoulders);
		AddRandomLoot(eEquipment_IronHelm);
		AddRandomLoot(eEquipment_IronArmor);
		AddRandomLoot(eEquipment_IronPants);
		AddRandomLoot(eEquipment_IronGloves);
		AddRandomLoot(eEquipment_IronBoots);
		AddRandomLoot(eEquipment_IronShoulders);
		AddRandomLoot(eEquipment_SteelHelm);
		AddRandomLoot(eEquipment_SteelArmor);
		AddRandomLoot(eEquipment_SteelPants);
		AddRandomLoot(eEquipment_SteelGloves);
		AddRandomLoot(eEquipment_SteelBoots);
		AddRandomLoot(eEquipment_SteelShoulders);
		AddRandomLoot(eEquipment_IceHelm);
		AddRandomLoot(eEquipment_IceArmor);
		AddRandomLoot(eEquipment_IcePants);
		AddRandomLoot(eEquipment_IceGloves);
		AddRandomLoot(eEquipment_IceBoots);
		AddRandomLoot(eEquipment_IceShoulders);
		AddRandomLoot(eEquipment_AshHelm);
		AddRandomLoot(eEquipment_AshArmor);
		AddRandomLoot(eEquipment_AshPants);
		AddRandomLoot(eEquipment_AshGloves);
		AddRandomLoot(eEquipment_AshBoots);
		AddRandomLoot(eEquipment_AshShoulders);
		AddRandomLoot(eEquipment_BoneHelm);
		AddRandomLoot(eEquipment_BoneArmor);
		AddRandomLoot(eEquipment_BonePants);
		AddRandomLoot(eEquipment_BoneGloves);
		AddRandomLoot(eEquipment_BoneBoots);
		AddRandomLoot(eEquipment_BoneShoulders);
		AddRandomLoot(eEquipment_SpikeHelm);
		AddRandomLoot(eEquipment_SpikeArmor);
		AddRandomLoot(eEquipment_SpikePants);
		AddRandomLoot(eEquipment_SpikeGloves);
		AddRandomLoot(eEquipment_SpikeBoots);
		AddRandomLoot(eEquipment_SpikeShoulders);
	}
}

RandomLootManager::~RandomLootManager()
{
	ClearEnemyIngredientsSpawnData();
	ClearRandomLootList();
}

// Deletion
void RandomLootManager::ClearEnemyIngredientsSpawnData()
{
	for (unsigned int i = 0; i < m_vpEnemyIngredientsSpawnDataList.size(); i++)
	{
		delete m_vpEnemyIngredientsSpawnDataList[i];
		m_vpEnemyIngredientsSpawnDataList[i] = 0;
	}
	m_vpEnemyIngredientsSpawnDataList.clear();
}

void RandomLootManager::ClearRandomLootList()
{
	for (unsigned int i = 0; i < m_vpRandomLootItemList.size(); i++)
	{
		delete m_vpRandomLootItemList[i];
		m_vpRandomLootItemList[i] = 0;
	}
	m_vpRandomLootItemList.clear();
}

// Enemy dropping ingredients
void RandomLootManager::AddEnemyIngredientSpawnData(eEnemyType sourceEnemy, eItem spawnedItem, int minSpawn, int maxSpawn)
{
	EnemyIngredientsSpawnData* pEnemyIngredientsSpawnData = new EnemyIngredientsSpawnData();

	pEnemyIngredientsSpawnData->m_sourceEnemy = sourceEnemy;
	pEnemyIngredientsSpawnData->m_spawnedItem = spawnedItem;
	pEnemyIngredientsSpawnData->m_minSpawn = minSpawn;
	pEnemyIngredientsSpawnData->m_maxSpawn = maxSpawn;

	m_vpEnemyIngredientsSpawnDataList.push_back(pEnemyIngredientsSpawnData);
}

void RandomLootManager::GetSpawnedIngredientItemForEnemy(eEnemyType sourceEnemy, eItem *item, int *quantity)
{
	for (unsigned int i = 0; i < m_vpEnemyIngredientsSpawnDataList.size(); i++)
	{
		if (m_vpEnemyIngredientsSpawnDataList[i]->m_sourceEnemy == sourceEnemy)
		{
			*item = m_vpEnemyIngredientsSpawnDataList[i]->m_spawnedItem;

			if (m_vpEnemyIngredientsSpawnDataList[i]->m_minSpawn == m_vpEnemyIngredientsSpawnDataList[i]->m_maxSpawn)
			{
				*quantity = m_vpEnemyIngredientsSpawnDataList[i]->m_minSpawn;
			}
			else
			{
				*quantity = GetRandomNumber(m_vpEnemyIngredientsSpawnDataList[i]->m_minSpawn, m_vpEnemyIngredientsSpawnDataList[i]->m_maxSpawn);
			}

			return;
		}
	}

	*item = eItem_None;
	*quantity = 0;
}

// Random loot
void RandomLootManager::AddRandomLoot(eEquipment equipment)
{
	RandomLootItem* pNewRandomLoot = new RandomLootItem();

	pNewRandomLoot->m_repickValue = 500;
	pNewRandomLoot->m_equipmentType = equipment;
	pNewRandomLoot->m_pLootItem = VoxGame::GetInstance()->GetInventoryManager()->CreateEquipmentItemFromType(equipment);

	m_vpRandomLootItemList.push_back(pNewRandomLoot);
}

InventoryItem* RandomLootManager::GetRandomLootItem(eEquipment *equipment)
{
	if (m_vpRandomLootItemList.size() > 0)
	{
		int numRetries = 0;
		while (numRetries < 10)
		{
			int randomSelection = GetRandomNumber(0, (int)m_vpRandomLootItemList.size() - 1);
			int repickValue = GetRandomNumber(0, 1000);

			if (numRetries < 9)
			{
				if (m_vpRandomLootItemList[randomSelection]->m_repickValue > repickValue)
				{
					m_vpRandomLootItemList[randomSelection]->m_repickValue = (int)(m_vpRandomLootItemList[randomSelection]->m_repickValue * 0.5f);

					*equipment = m_vpRandomLootItemList[randomSelection]->m_equipmentType;
					return m_vpRandomLootItemList[randomSelection]->m_pLootItem;
				}
				else
				{
					m_vpRandomLootItemList[randomSelection]->m_repickValue += (int)((1000 - m_vpRandomLootItemList[randomSelection]->m_repickValue)*0.5f);

					numRetries++;
				}
			}
			else
			{
				*equipment = m_vpRandomLootItemList[randomSelection]->m_equipmentType;
				return m_vpRandomLootItemList[randomSelection]->m_pLootItem;
			}
		}
	}

	*equipment = eEquipment_None;
	return nullptr;
}