// ******************************************************************************
// Filename:    ItemSpawner.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 18/11/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "ItemSpawner.h"
#include "ItemManager.h"
#include "Item.h"

#include "../utils/Interpolator.h"
#include "../utils/Random.h"
#include "../utils/FileUtils.h"

#include "../Lighting/LightingManager.h"
#include "../Particles/BlockParticleManager.h"
#include "../TextEffects/TextEffectsManager.h"
#include "../Items/ItemManager.h"
#include "../Projectile/ProjectileManager.h"
#include "../Projectile/Projectile.h"
#include "../NPC/NPCManager.h"
#include "../NPC/NPC.h"
#include "../VoxGame.h"


ItemSpawner::ItemSpawner(Renderer* pRenderer, ChunkManager* pChunkManager, Player* pPlayer, ItemManager* pItemManager, NPCManager* pNPCManager)
{
	m_pRenderer = pRenderer;
	m_pChunkManager = pChunkManager;
	m_pPlayer = pPlayer;
	m_pItemManager = pItemManager;
	m_pNPCManager = pNPCManager;

	m_spawning = false;
	m_canSpawn = false;
	m_spawnTime = 1.0f;
	m_followPlayerIntheWorld = false;
	m_spawnFullLoaderRange = false;
	m_minDistanceFromPlayer = 0;
	m_spawnScale = 0.1f;

	m_numSpawnedItems = 0;
	m_maxNumItemsToHaveActive = 1;
	m_shouldSpawnOnGround = true;
}

ItemSpawner::~ItemSpawner()
{
	m_pItemManager->RemoveItemSpawnerFromItems(this);
}

void ItemSpawner::SetPosition(vec3 pos)
{
	m_position = pos;
}

vec3 ItemSpawner::GetPosition()
{
	return m_position;
}

void ItemSpawner::SetInitialPosition(vec3 pos)
{
	m_initialiPosition = pos;
}

vec3 ItemSpawner::GetInitialPosition()
{
	return m_initialiPosition;
}

void ItemSpawner::SetFacingDirection(vec3 dir)
{
	m_spawnFacingDirection = dir;
}

vec3 ItemSpawner::GetFacingDirection()
{
	return m_spawnFacingDirection;
}

bool ItemSpawner::ShouldFollowPlayer()
{
	return m_followPlayerIntheWorld;
}

// Spawning params
void ItemSpawner::SetSpawningParams(float initialSpawnDelay, float spawnTimer, int maxNumItemsActive, vec3 spawnRandomOffset, bool shouldSpawnOnGround, vec3 groundSpawnOffset, bool followPlayerIntheWorld, bool spawnFullLoaderRange, float minDistanceFromPlayer, Biome biomeSpawn, float spawnScale)
{
	m_spawnTime = spawnTimer;
	m_maxNumItemsToHaveActive = maxNumItemsActive;
	m_spawnRandomOffset = spawnRandomOffset;
	m_shouldSpawnOnGround = shouldSpawnOnGround;
	m_groundSpawnOffset = groundSpawnOffset;
	m_followPlayerIntheWorld = followPlayerIntheWorld;
	m_spawnFullLoaderRange = spawnFullLoaderRange;
	m_minDistanceFromPlayer = minDistanceFromPlayer;
	m_biomeSpawn = biomeSpawn;
	m_spawnScale = spawnScale;

	if(initialSpawnDelay >= 0.0f)
	{
		StartSpawning(initialSpawnDelay);
	}
}

void ItemSpawner::AddItemTypeToSpawn(eItem itemType)
{
	m_vpItemTypeList.push_back(itemType);
}

// Item removed
void ItemSpawner::RemoveItemFromThisSpawner()
{
	m_numSpawnedItems -= 1;
}

// Spawning
void ItemSpawner::StartSpawning(float initialDelay)
{
	m_spawning = true;
	m_spawnCountdownTimer = initialDelay;
}

void ItemSpawner::StopSpawning()
{
	m_spawning = false;
}

eItem ItemSpawner::GetItemTypeToSpawn()
{
	int numItemTypes = (int)m_vpItemTypeList.size();

	int randomNum = GetRandomNumber(0, numItemTypes-1);

	return m_vpItemTypeList[randomNum];
}

bool ItemSpawner::GetSpawnPosition(vec3* pSpawnPosition)
{
	bool lLocationGood = false;
	int numTries = 0;
	while (lLocationGood == false && numTries < 10)
	{
		vec3 spawnPos = m_position;
		vec3 randomOffset;

		if (m_spawnFullLoaderRange)
		{
			float loaderRadius = m_pChunkManager->GetLoaderRadius();
			randomOffset = vec3(GetRandomNumber(-100, 100, 2)*0.01f*loaderRadius, GetRandomNumber(-100, 100, 2)*0.01f*loaderRadius, GetRandomNumber(-100, 100, 2)*0.01f*loaderRadius);
		}
		else
		{
			randomOffset = vec3(GetRandomNumber(-100, 100, 2)*0.01f*(m_spawnRandomOffset.x - 8.0f), GetRandomNumber(-100, 100, 2)*0.01f*(m_spawnRandomOffset.y - 8.0f), GetRandomNumber(-100, 100, 2)*0.01f*(m_spawnRandomOffset.z - 8.0f));
		}

		spawnPos += randomOffset;

		int blockX, blockY, blockZ;
		vec3 blockPos;
		Chunk* pChunk = nullptr;
		bool active = m_pChunkManager->GetBlockActiveFrom3DPosition(spawnPos.x, spawnPos.y, spawnPos.z, &blockPos, &blockX, &blockY, &blockZ, &pChunk);
		if (pChunk != nullptr && pChunk->IsSetup() && active == false)
		{
			if (m_shouldSpawnOnGround)
			{
				vec3 floorPosition;
				if (m_pChunkManager->FindClosestFloor(spawnPos, &floorPosition))
				{
					spawnPos = floorPosition + vec3(0.0f, 0.01f, 0.0f);
					spawnPos += m_groundSpawnOffset;

					Biome biome = VoxGame::GetInstance()->GetBiomeManager()->GetBiome(spawnPos);
					if (biome == m_biomeSpawn)
					{
						*pSpawnPosition = spawnPos;
						lLocationGood = true;
					}
				}
			}
		}
		
		numTries++;
	}

	return lLocationGood;
}

// Updating
void ItemSpawner::Update(float dt)
{
	m_canSpawn = (m_numSpawnedItems < m_maxNumItemsToHaveActive);

	// Update timers
	UpdateTimers(dt);

	if(m_spawning && m_canSpawn)
	{
		if(m_spawnCountdownTimer <= 0.0f)
		{
			vec3 spawnPos;
			bool spawnGood = GetSpawnPosition(&spawnPos);

			if (spawnGood)
			{
				vec3 toPlayer = spawnPos - m_pPlayer->GetCenter();
				if (length(toPlayer) > m_minDistanceFromPlayer)
				{
					eItem itemType = GetItemTypeToSpawn();
					string itemFilename = GetItemFilenameForType(itemType);

					Item* pItem = m_pItemManager->CreateItem(spawnPos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), itemFilename.c_str(), itemType, "SpawnedItem", true, false, m_spawnScale);
					pItem->SetItemSpawner(this);
					
					if (itemType == eItem_CopperVein || itemType == eItem_IronVein || itemType == eItem_SilverVein || itemType == eItem_GoldVein)
					{
						// Interaction with ore deposits
						pItem->SetMaxtInteractCount(4);
					}
					if (itemType == eItem_Chest)
					{
						int xPos = 0;
						int yPos = LootGUI::MAX_NUM_SLOTS_VERTICAL - 1;

						// Create random loot inside the chest
						eEquipment equipment = eEquipment_None;
						InventoryItem* pRandomLoot = VoxGame::GetInstance()->GetRandomLootManager()->GetRandomLootItem(&equipment);
						if (pRandomLoot != nullptr && equipment != eEquipment_None)
						{
							InventoryItem* pRandomLootItem = pItem->AddLootItem(pRandomLoot, xPos, yPos);
							pRandomLootItem->m_scale = pRandomLoot->m_scale;
							pRandomLootItem->m_offsetX = pRandomLoot->m_offsetX;
							pRandomLootItem->m_offsetY = pRandomLoot->m_offsetY;
							pRandomLootItem->m_offsetZ = pRandomLoot->m_offsetZ;
							pRandomLootItem->m_left = pRandomLoot->m_left;
							pRandomLootItem->m_right = pRandomLoot->m_right;
						}
					}

					pItem->SetRotation(vec3(0.0f, GetRandomNumber(0, 360, 2), 0.0f));

					m_numSpawnedItems += 1;

					m_spawnCountdownTimer = m_spawnTime;
				}
			}
			else
			{
				// Can't spawn an item at a good position, wait a while before we try again
				m_spawnCountdownTimer = 0.5f;
			}
		}
	}
}

void ItemSpawner::UpdateTimers(float dt)
{
	if(m_spawning && m_canSpawn)
	{
		if(m_spawnCountdownTimer > 0.0f)
		{
			m_spawnCountdownTimer -= dt;
		}
	}
}

// Rendering
void ItemSpawner::Render()
{

}

void ItemSpawner::RenderDebug()
{
	m_pRenderer->PushMatrix();
		float l_length = 0.5f;
		float l_height = 0.5f;
		float l_width = 0.5f;

		m_pRenderer->TranslateWorldMatrix(m_position.x, m_position.y, m_position.z);

		m_pRenderer->SetRenderMode(RM_WIREFRAME);
		m_pRenderer->SetCullMode(CM_NOCULL);
		m_pRenderer->SetLineWidth(1.0f);

		// TODO : Loader culling
/*		if(m_pChunkManager->IsInsideLoader(m_position) == false)
		{
			m_pRenderer->ImmediateColourAlpha(0.1f, 0.8f, 0.85f, 1.0f);
		}
		else */if(m_spawning && m_canSpawn)
		{
			m_pRenderer->ImmediateColourAlpha(0.1f, 0.8f, 0.05f, 1.0f);
		}
		else if(m_spawning)
		{
			m_pRenderer->ImmediateColourAlpha(0.8f, 0.8f, 0.05f, 1.0f);
		}
		else
		{
			m_pRenderer->ImmediateColourAlpha(0.8f, 0.1f, 0.05f, 1.0f);
		}

		m_pRenderer->EnableImmediateMode(IM_QUADS);			
			m_pRenderer->ImmediateNormal(0.0f, 0.0f, -1.0f);
			m_pRenderer->ImmediateVertex(l_length, -l_height, -l_width);
			m_pRenderer->ImmediateVertex(-l_length, -l_height, -l_width);
			m_pRenderer->ImmediateVertex(-l_length, l_height, -l_width);
			m_pRenderer->ImmediateVertex(l_length, l_height, -l_width);

			m_pRenderer->ImmediateNormal(0.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateVertex(-l_length, -l_height, l_width);
			m_pRenderer->ImmediateVertex(l_length, -l_height, l_width);
			m_pRenderer->ImmediateVertex(l_length, l_height, l_width);
			m_pRenderer->ImmediateVertex(-l_length, l_height, l_width);

			m_pRenderer->ImmediateNormal(1.0f, 0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(l_length, -l_height, l_width);
			m_pRenderer->ImmediateVertex(l_length, -l_height, -l_width);
			m_pRenderer->ImmediateVertex(l_length, l_height, -l_width);
			m_pRenderer->ImmediateVertex(l_length, l_height, l_width);

			m_pRenderer->ImmediateNormal(-1.0f, 0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(-l_length, -l_height, -l_width);
			m_pRenderer->ImmediateVertex(-l_length, -l_height, l_width);
			m_pRenderer->ImmediateVertex(-l_length, l_height, l_width);
			m_pRenderer->ImmediateVertex(-l_length, l_height, -l_width);

			m_pRenderer->ImmediateNormal(0.0f, -1.0f, 0.0f);
			m_pRenderer->ImmediateVertex(-l_length, -l_height, -l_width);
			m_pRenderer->ImmediateVertex(l_length, -l_height, -l_width);
			m_pRenderer->ImmediateVertex(l_length, -l_height, l_width);
			m_pRenderer->ImmediateVertex(-l_length, -l_height, l_width);

			m_pRenderer->ImmediateNormal(0.0f, 1.0f, 0.0f);
			m_pRenderer->ImmediateVertex(l_length, l_height, -l_width);
			m_pRenderer->ImmediateVertex(-l_length, l_height, -l_width);
			m_pRenderer->ImmediateVertex(-l_length, l_height, l_width);
			m_pRenderer->ImmediateVertex(l_length, l_height, l_width);
		m_pRenderer->DisableImmediateMode();

		// Spawn facing direction
		m_pRenderer->SetRenderMode(RM_SOLID);
		m_pRenderer->SetLineWidth(3.0f);
		m_pRenderer->EnableImmediateMode(IM_LINES);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(m_spawnFacingDirection.x*1.5f, m_spawnFacingDirection.y*1.5f, m_spawnFacingDirection.z*1.5f);
		m_pRenderer->DisableImmediateMode();

		m_pRenderer->SetCullMode(CM_BACK);
	m_pRenderer->PopMatrix();
}