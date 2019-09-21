// ******************************************************************************
// Filename:    EnemySPawner.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 18/11/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "EnemySpawner.h"
#include "EnemyManager.h"
#include "Enemy.h"

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


EnemySpawner::EnemySpawner(Renderer* pRenderer, ChunkManager* pChunkManager, Player* pPlayer, EnemyManager* pEnemyManager, NPCManager* pNPCManager)
{
	m_pRenderer = pRenderer;
	m_pChunkManager = pChunkManager;
	m_pPlayer = pPlayer;
	m_pEnemyManager = pEnemyManager;
	m_pNPCManager = pNPCManager;

	m_spawning = false;
	m_canSpawn = false;
	m_spawnTime = 1.0f;
	m_followPlayerIntheWorld = false;
	m_spawnFullLoaderRange = false;
	m_minDistanceFromPlayer = 0;

	m_numSpawnedEnemies = 0;
	m_maxNumEnemiesToHaveActive = 1;
	m_shouldSpawnOnGround = true;
}

EnemySpawner::~EnemySpawner()
{
	m_pEnemyManager->RemoveEnemySpawnerFromEnemies(this);
}

void EnemySpawner::SetPosition(vec3 pos)
{
	m_position = pos;
}

vec3 EnemySpawner::GetPosition()
{
	return m_position;
}

void EnemySpawner::SetInitialPosition(vec3 pos)
{
	m_initialiPosition = pos;
}

vec3 EnemySpawner::GetInitialPosition()
{
	return m_initialiPosition;
}

void EnemySpawner::SetFacingDirection(vec3 dir)
{
	m_spawnFacingDirection = dir;
}

vec3 EnemySpawner::GetFacingDirection()
{
	return m_spawnFacingDirection;
}

bool EnemySpawner::ShouldFollowPlayer()
{
	return m_followPlayerIntheWorld;
}

// Spawning params
void EnemySpawner::SetSpawningParams(float initialSpawnDelay, float spawnTimer, int maxNumEnemiesActive, vec3 spawnRandomOffset, bool shouldSpawnOnGround, vec3 groundSpawnOffset, bool followPlayerIntheWorld, bool spawnFullLoaderRange, float minDistanceFromPlayer, Biome biomeSpawn)
{
	m_spawnTime = spawnTimer;
	m_maxNumEnemiesToHaveActive = maxNumEnemiesActive;
	m_spawnRandomOffset = spawnRandomOffset;
	m_shouldSpawnOnGround = shouldSpawnOnGround;
	m_groundSpawnOffset = groundSpawnOffset;
	m_followPlayerIntheWorld = followPlayerIntheWorld;
	m_spawnFullLoaderRange = spawnFullLoaderRange;
	m_minDistanceFromPlayer = minDistanceFromPlayer;
	m_biomeSpawn = biomeSpawn;

	if(initialSpawnDelay >= 0.0f)
	{
		StartSpawning(initialSpawnDelay);
	}
}

void EnemySpawner::AddEnemyTypeToSpawn(eEnemyType enemyType)
{
	m_vpEnemyTypeList.push_back(enemyType);
}

// Enemies killed
void EnemySpawner::RemoveEnemyFromThisSpawner()
{
	m_numSpawnedEnemies -= 1;
}

// Spawning
void EnemySpawner::StartSpawning(float initialDelay)
{
	m_spawning = true;
	m_spawnCountdownTimer = initialDelay;
}

void EnemySpawner::StopSpawning()
{
	m_spawning = false;
}

eEnemyType EnemySpawner::GetEnemyTypeToSpawn()
{
	int numEnemyTypes = (int)m_vpEnemyTypeList.size();

	int randomNum = GetRandomNumber(0, numEnemyTypes-1);

	return m_vpEnemyTypeList[randomNum];
}

bool EnemySpawner::GetSpawnPosition(vec3* pSpawnPosition)
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
		Chunk* pChunk = NULL;
		bool active = m_pChunkManager->GetBlockActiveFrom3DPosition(spawnPos.x, spawnPos.y, spawnPos.z, &blockPos, &blockX, &blockY, &blockZ, &pChunk);
		if (pChunk != NULL && pChunk->IsSetup() && active == false)
		{
			if (m_shouldSpawnOnGround)
			{
				vec3 floorPosition;
				if (m_pChunkManager->FindClosestFloor(spawnPos, &floorPosition))
				{
					spawnPos = floorPosition + vec3(0.0f, 0.01f, 0.0f);
					spawnPos += m_groundSpawnOffset;

					BiomeManager* biomeManager = VoxGame::GetInstance()->GetBiomeManager();
					Biome biome = biomeManager->GetBiome(spawnPos);
					ZoneData *pTown = NULL;
					ZoneData *pSafeZone = NULL;
					bool isInTown = biomeManager->IsInTown(spawnPos, &pTown);
					bool isInSafeZone = biomeManager->IsInSafeZone(spawnPos, &pSafeZone);
					if (biome == m_biomeSpawn && isInTown == false && isInSafeZone == false)
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
void EnemySpawner::Update(float dt)
{
	m_canSpawn = (m_numSpawnedEnemies < m_maxNumEnemiesToHaveActive);

	// Update timers
	UpdateTimers(dt);

	if(m_spawning && m_canSpawn)
	{
		if(m_spawnCountdownTimer <= 0.0f)
		{
			vec3 spawnPos;
			bool spawnGood = GetSpawnPosition(&spawnPos);
			int numEnemies = m_pEnemyManager->GetNumEnemies();

			if (spawnGood && numEnemies < EnemyManager::MAX_NUM_ENEMIES)
			{
				vec3 toPlayer = spawnPos - m_pPlayer->GetCenter();
				if (length(toPlayer) > m_minDistanceFromPlayer)
				{
					eEnemyType enemyType = GetEnemyTypeToSpawn();

					Enemy* pEnemy = m_pEnemyManager->CreateEnemy(spawnPos, enemyType, 0.08f);
					pEnemy->SetSpawningParams(spawnPos, spawnPos, 0.0f);
					pEnemy->SetTargetForwardToLookAtPoint(spawnPos + m_spawnFacingDirection);
					pEnemy->SetEnemySpawner(this);
					
					pEnemy->SetRotation(GetRandomNumber(0, 360, 2));
					pEnemy->SetTargetForwardToLookAtPoint(pEnemy->GetPosition() + pEnemy->GetTargetForward());

					m_numSpawnedEnemies += 1;

					m_spawnCountdownTimer = m_spawnTime;
				}
			}
			else
			{
				// Can't spawn an enemy at a good position, wait a while before we try again
				m_spawnCountdownTimer = 0.5f;
			}
		}
	}
}

void EnemySpawner::UpdateTimers(float dt)
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
void EnemySpawner::Render()
{

}

void EnemySpawner::RenderDebug()
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