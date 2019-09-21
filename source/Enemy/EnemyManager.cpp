// ******************************************************************************
// Filename:    EnemyManager.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 18/11/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "EnemyManager.h"

#include "../utils/Random.h"
#include "../Lighting/LightingManager.h"
#include "../Particles/BlockParticleManager.h"
#include "../TextEffects/TextEffectsManager.h"
#include "../Items/ItemManager.h"
#include "../Projectile/ProjectileManager.h"
#include "../VoxGame.h"
#include "../GameGUI/HUD.h"


EnemyManager::EnemyManager(Renderer* pRenderer, ChunkManager* pChunkManager, Player* pPlayer)
{
	m_pRenderer = pRenderer;
	m_pChunkManager = pChunkManager;
	m_pPlayer = pPlayer;

	m_numRenderEnemies = 0;
}

void EnemyManager::SetLightingManager(LightingManager* pLightingManager)
{
	m_pLightingManager = pLightingManager;
}

void EnemyManager::SetBlockParticleManager(BlockParticleManager* pBlockParticleManager)
{
	m_pBlockParticleManager = pBlockParticleManager;
}

void EnemyManager::SetTextEffectsManager(TextEffectsManager* pTextEffectsManager)
{
	m_pTextEffectsManager = pTextEffectsManager;
}

void EnemyManager::SetItemManager(ItemManager* pItemManager)
{
	m_pItemManager = pItemManager;
}

void EnemyManager::SetProjectileManager(ProjectileManager* pProjectileManager)
{
	m_pProjectileManager = pProjectileManager;
}

void EnemyManager::SetHUD(HUD* pHUD)
{
	m_pHUD = pHUD;
}

void EnemyManager::SetQubicleBinaryManager(QubicleBinaryManager* pQubicleBinaryManager)
{
	m_pQubicleBinaryManager = pQubicleBinaryManager;
}

void EnemyManager::SetNPCManager(NPCManager* pNPCManager)
{
	m_pNPCManager = pNPCManager;
}

EnemyManager::~EnemyManager()
{
	ClearEnemies();
	ClearEnemySpawners();
}

// Clearing
void EnemyManager::ClearEnemies()
{
	// Enemy list
	m_enemyMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		delete m_vpEnemyList[i];
		m_vpEnemyList[i] = 0;
	}
	m_vpEnemyList.clear();
	m_enemyMutex.unlock();

	// Create list
	for(unsigned int i = 0; i < m_vpEnemyCreateList.size(); i++)
	{
		delete m_vpEnemyCreateList[i];
		m_vpEnemyCreateList[i] = 0;
	}
	m_vpEnemyCreateList.clear();
}

void EnemyManager::ClearEnemyChunkCacheForChunk(Chunk* pChunk)
{
	m_enemyMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		m_vpEnemyList[i]->ClearChunkCacheForChunk(pChunk);
	}
	m_enemyMutex.unlock();
}

void EnemyManager::ClearEnemySpawners()
{
	m_enemySpawnerMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemySpawnerList.size(); i++)
	{
		delete m_vpEnemySpawnerList[i];
		m_vpEnemySpawnerList[i] = 0;
	}
	m_vpEnemySpawnerList.clear();
	m_enemySpawnerMutex.unlock();
}

void EnemyManager::RemoveEnemySpawnerFromEnemies(EnemySpawner* pSpawner)
{
	m_enemyMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		m_vpEnemyList[i]->RemoveEnemySpawner(pSpawner);
	}
	m_enemyMutex.unlock();

	for(unsigned int i = 0; i < m_vpEnemyCreateList.size(); i++)
	{
		m_vpEnemyCreateList[i]->RemoveEnemySpawner(pSpawner);
	}
}

// Creation
Enemy* EnemyManager::CreateEnemy(vec3 position, eEnemyType enemyType, float scale)
{
	Enemy* pNewEnemy = new Enemy(m_pRenderer, m_pChunkManager, m_pPlayer, m_pLightingManager, m_pBlockParticleManager, m_pTextEffectsManager, m_pItemManager, m_pProjectileManager, m_pHUD, this, m_pNPCManager, m_pQubicleBinaryManager, enemyType);

	pNewEnemy->SetPosition(position);
	pNewEnemy->SetLeashParameters(position, 15.0f);
	pNewEnemy->ResetRandomTargetPosition();
	if(enemyType != eEnemyType_Mimic)
	{
		pNewEnemy->SetRandomTargetMode(true);
	}
	pNewEnemy->SetScale(scale);

	m_vpEnemyCreateList.push_back(pNewEnemy);

	return pNewEnemy;
}

void EnemyManager::CreateEnemyCamp(vec3 campPosition)
{
	Enemy* pEnemy1 = CreateEnemy(campPosition + vec3(2.0f, 0.0f, 0.0f), eEnemyType_NormalSkeleton, 0.08f);
	pEnemy1->SetRandomTargetMode(false);
	pEnemy1->SetTargetForwardToLookAtPoint(campPosition);
	Enemy* pEnemy2 = CreateEnemy(campPosition + vec3(-1.0f, 0.0f, -3.0f), eEnemyType_MeleeSkeleton, 0.08f);
	pEnemy2->SetRandomTargetMode(false);
	pEnemy2->SetTargetForwardToLookAtPoint(campPosition);
	Enemy* pEnemy3 = CreateEnemy(campPosition + vec3(-1.3f, 0.0f, 2.5f), eEnemyType_MageSkeleton, 0.08f);
	pEnemy3->SetRandomTargetMode(false);
	pEnemy3->SetTargetForwardToLookAtPoint(campPosition);
	Enemy* pEnemy4 = CreateEnemy(campPosition + vec3(-3.3f, 0.0f, 0.5f), eEnemyType_RangedSkeleton, 0.08f);
	pEnemy4->SetRandomTargetMode(false);
	pEnemy4->SetTargetForwardToLookAtPoint(campPosition);

	Item* pCampFire = m_pItemManager->CreateItem(campPosition, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 45.0f, 0.0f), "media/gamedata/items/CampFire/CampFire.item", eItem_CampFire, "Camp Fire", false, false, 0.08f);
}

EnemySpawner* EnemyManager::CreateEnemySpawner(vec3 position, vec3 direction)
{
	EnemySpawner* pEnemySpawner = new EnemySpawner(m_pRenderer, m_pChunkManager, m_pPlayer, this, m_pNPCManager);

	pEnemySpawner->SetPosition(position);
	pEnemySpawner->SetInitialPosition(position);
	pEnemySpawner->SetFacingDirection(normalize(direction));

	m_vpEnemySpawnerList.push_back(pEnemySpawner);

	return pEnemySpawner;
}

// Get number of enemies
int EnemyManager::GetNumEnemies()
{
	m_enemyMutex.lock();
	int numEnemies =  (int)m_vpEnemyList.size();
	m_enemyMutex.unlock();

	return numEnemies;
}

int EnemyManager::GetNumRenderEnemies()
{
	return m_numRenderEnemies;
}

Enemy* EnemyManager::GetEnemy(int index)
{
	m_enemyMutex.lock();
	Enemy* pReturn =  m_vpEnemyList[index];
	m_enemyMutex.unlock();

	return pReturn;
}

// Get enemy based on cursor position
Enemy* EnemyManager::GetCursorEnemy(Camera* pGameCamera, int cursorX, int cursorY)
{
	// Project the cursor into the world,
	m_pRenderer->PushMatrix();
		m_pRenderer->SetProjectionMode(PM_PERSPECTIVE, VoxGame::GetInstance()->GetDefaultViewport());
		pGameCamera->Look();

		vec3 worldPos1 = m_pRenderer->GetWorldProjectionFromScreenCoordinates(cursorX, cursorY, 0.5f);
		vec3 worldPos2 = m_pRenderer->GetWorldProjectionFromScreenCoordinates(cursorX, cursorY, 0.9999f);
	m_pRenderer->PopMatrix();

	// Check if our projected mouse line is colliding with any enemies
	bool collides = false;
	int interations = 0;
	float increments = Chunk::BLOCK_RENDER_SIZE*0.1f;
	vec3 worldNormal = normalize(worldPos2 - worldPos1);
	while(collides == false && interations < 100)
	{
		vec3 newPos = worldPos1 + worldNormal * increments;

		m_enemyMutex.lock();
		for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
		{
			Enemy* pEnemy = m_vpEnemyList[i];

			vec3 diff = pEnemy->GetCenter() - newPos;
			float diff_length = length(diff);
			if(diff_length < pEnemy->GetRadius()*2.5f)
			{
				m_enemyMutex.unlock();
				return pEnemy;
			}
		}
		m_enemyMutex.unlock();

		interations++;
		increments += Chunk::BLOCK_RENDER_SIZE;
	}

	return nullptr;
}

// Rendering Helpers
void EnemyManager::SetWireFrameRender(bool wireframe)
{
	m_enemyMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		Enemy* pEnemy = m_vpEnemyList[i];

		pEnemy->SetWireFrameRender(wireframe);
	}
	m_enemyMutex.unlock();
}

// Gameplay
void EnemyManager::RemoveSappedFromEnemies(Enemy *pEnemyToBeSapped)
{
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		Enemy* pEnemy = m_vpEnemyList[i];

		if(pEnemy == pEnemyToBeSapped)
		{
			continue;
		}

		pEnemy->SetSapped(false);
	}
}

// Collision
void EnemyManager::PushCollisions(Enemy* pPushingEnemy, vec3 position, float radius)
{
	EnemyList::iterator enemy_iterator;
	m_enemyMutex.lock();
	for(enemy_iterator = m_vpEnemyList.begin(); enemy_iterator != m_vpEnemyList.end(); ++enemy_iterator)
	{
		Enemy* pEnemy = (*enemy_iterator);

		if(pEnemy == pPushingEnemy)
		{
			continue;  // Can't push ourselves
		}

		vec3 distance = pEnemy->GetCenter() - position;
		float lengthResult = length(distance) - radius - pEnemy->GetRadius();

		if(lengthResult < 0.0f)
		{
			vec3 pushVector = distance;
			pushVector.y = 0.0f;  // Don't push in Y direction
			pushVector *= lengthResult;
			pEnemy->SetPosition(pEnemy->GetPosition() - pushVector);
		}
	}
	m_enemyMutex.unlock();
}

// Updating
void EnemyManager::UpdateWeaponLights(float dt)
{
	m_enemyMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		Enemy* pEnemy = m_vpEnemyList[i];

		// TODO : Loader radius culling
		//if(m_pChunkManager->IsInsideLoader(pEnemy->GetCenter()) == false)
		//{
		//	continue;
		//}

		pEnemy->UpdateWeaponLights(dt);
	}
	m_enemyMutex.unlock();
}

void EnemyManager::UpdateWeaponParticleEffects(float dt)
{
	m_enemyMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		Enemy* pEnemy = m_vpEnemyList[i];

		// TODO : Loader radius culling
		//if(m_pChunkManager->IsInsideLoader(pEnemy->GetCenter()) == false)
		//{
		//	continue;
		//}

		pEnemy->UpdateWeaponParticleEffects(dt);
	}
	m_enemyMutex.unlock();
}

bool needs_erasing(Enemy* aE)
{
	bool needsErase = aE->GetErase();

	if(needsErase == true)
	{
		delete aE;
	}

	return needsErase;
}

void EnemyManager::Update(float dt)
{
	// Update all enemy spawners
	m_enemySpawnerMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemySpawnerList.size(); i++)
	{
		EnemySpawner* pEnemySpawner = m_vpEnemySpawnerList[i];

		if(pEnemySpawner->ShouldFollowPlayer())
		{
			pEnemySpawner->SetPosition(m_pPlayer->GetCenter() + pEnemySpawner->GetInitialPosition());
		}

		// TODO : Loader radius culling
		//if(m_pChunkManager->IsInsideLoader(pEnemySpawner->GetPosition()) == false)
		//{
		//	continue;
		//}

		pEnemySpawner->Update(dt);
	}
	m_enemySpawnerMutex.unlock();

	// Add any enemies on the create list to the main list and then clear the create list
	EnemyList::iterator add_iterator;
	for(add_iterator = m_vpEnemyCreateList.begin(); add_iterator != m_vpEnemyCreateList.end(); ++add_iterator)
	{
		Enemy* pEnemy = (*add_iterator);

		m_enemyMutex.lock();
		m_vpEnemyList.push_back(pEnemy);
		m_enemyMutex.unlock();
	}
	m_vpEnemyCreateList.clear();

	// Erase any dead enemies
	m_enemyMutex.lock();
	m_vpEnemyList.erase( remove_if(m_vpEnemyList.begin(), m_vpEnemyList.end(), needs_erasing), m_vpEnemyList.end() );

	// Update all enemies
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		Enemy* pEnemy = m_vpEnemyList[i];

		// TODO : Loader radius culling
		//if(m_pChunkManager->IsInsideLoader(pEnemy->GetCenter()) == false)
		//{
		//	continue;
		//}

		pEnemy->Update(dt);

		m_enemyMutex.unlock();

		// Allow NPCs to push each other away (simple collision).
		PushCollisions(pEnemy, pEnemy->GetCenter(), pEnemy->GetRadius());

		m_enemyMutex.lock();
	}
	m_enemyMutex.unlock();

	// Update weapon lights
	UpdateWeaponLights(dt);

	// Update weapon particle effects
	UpdateWeaponParticleEffects(dt);

	// Update the players attack checking
	UpdateEnemyPlayerAttackCheck(dt);

	// Check the projectile collisions
	UpdateEnemyProjectileCheck(dt);
}

void EnemyManager::UpdateEnemyPlayerAttackCheck(float dt)
{
	if(m_pPlayer->IsDead() == true)
	{
		return;
	}

	m_enemyMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		Enemy* pEnemy = m_vpEnemyList[i];

		if(pEnemy->GetErase() == true)
		{
			continue;
		}

		// TODO : Loader radius culling
		//if(m_pChunkManager->IsInsideLoader(pEnemy->GetCenter()) == false)
		//{
		//	continue;
		//}

		m_pPlayer->CheckEnemyDamageRadius(pEnemy);
	}
	m_enemyMutex.unlock();
}

void EnemyManager::UpdateEnemyProjectileCheck(float dt)
{
	m_enemyMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		Enemy* pEnemy = m_vpEnemyList[i];

		if(pEnemy->GetErase() == true)
		{
			continue;
		}

		// TODO : Loader radius culling
		//if(m_pChunkManager->IsInsideLoader(pEnemy->GetCenter()) == false)
		//{
		//	continue;
		//}

		for(int j = 0; j < m_pProjectileManager->GetNumProjectiles(); j++)
		{
			Projectile* pProjectile = m_pProjectileManager->GetProjectile(j);

			if(pProjectile != nullptr && pProjectile->GetErase() == false)
			{
				pEnemy->CheckProjectileDamageRadius(pProjectile);
			}
		}
	}
	m_enemyMutex.unlock();
}

void EnemyManager::CalculateWorldTransformMatrix()
{
	m_enemyMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		Enemy* pEnemy = m_vpEnemyList[i];

		pEnemy->CalculateWorldTransformMatrix();
	}
	m_enemyMutex.unlock();
}

// Rendering
void EnemyManager::Render(bool outline, bool reflection, bool silhouette, bool shadow)
{
	m_numRenderEnemies = 0;

	m_enemyMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		Enemy* pEnemy = m_vpEnemyList[i];

		if(silhouette && pEnemy->GetOutlineRender() == false)
		{
			continue; // Don't render silhouette unless we are rendering outline
		}

		// TODO : Loader radius culling
		//if(m_pChunkManager->IsInsideLoader(pEnemy->GetCenter()) == false)
		//{
		//	continue;
		//}

		// Fog
		float toCamera = length(VoxGame::GetInstance()->GetGameCamera()->GetPosition() - pEnemy->GetCenter());
		if (toCamera > m_pChunkManager->GetLoaderRadius() + (Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE*5.0f))
		{
			continue;
		}
		if (toCamera > m_pChunkManager->GetLoaderRadius() - Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE*3.0f)
		{
			m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
		}

		if(shadow || m_pRenderer->SphereInFrustum(VoxGame::GetInstance()->GetDefaultViewport(), pEnemy->GetCenter(), pEnemy->GetRadius()))
		{
			pEnemy->Render(outline, reflection, silhouette);

			m_numRenderEnemies++;
		}

		m_pRenderer->DisableTransparency();
	}
	m_enemyMutex.unlock();
}

void EnemyManager::RenderFaces()
{
	m_enemyMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		Enemy* pEnemy = m_vpEnemyList[i];

		// TODO : Loader radius culling
		//if(m_pChunkManager->IsInsideLoader(pEnemy->GetCenter()) == false)
		//{
		//	continue;
		//}

		if(m_pRenderer->SphereInFrustum(VoxGame::GetInstance()->GetDefaultViewport(), pEnemy->GetCenter(), pEnemy->GetRadius()))
		{
			pEnemy->RenderFace();
		}		
	}
	m_enemyMutex.unlock();
}

void EnemyManager::RenderOutlineEnemies()
{
	m_enemyMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		Enemy* pEnemy = m_vpEnemyList[i];

		if(pEnemy->GetOutlineRender() == false)
		{
			continue;
		}

		// TODO : Loader radius culling
		//if(m_pChunkManager->IsInsideLoader(pEnemy->GetCenter()) == false)
		//{
		//	continue;
		//}

		if(m_pRenderer->SphereInFrustum(VoxGame::GetInstance()->GetDefaultViewport(), pEnemy->GetCenter(), pEnemy->GetRadius()))
		{

			pEnemy->Render(true, false, false);
		}
	}
	m_enemyMutex.unlock();
}

void EnemyManager::RenderWeaponTrails()
{
	m_enemyMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		Enemy* pEnemy = m_vpEnemyList[i];

		// TODO : Loader radius culling
		//if(m_pChunkManager->IsInsideLoader(pEnemy->GetCenter()) == false)
		//{
		//	continue;
		//}

		if(m_pRenderer->SphereInFrustum(VoxGame::GetInstance()->GetDefaultViewport(), pEnemy->GetCenter(), pEnemy->GetRadius()))
		{
			pEnemy->RenderWeaponTrails();
		}		
	}
	m_enemyMutex.unlock();
}

void EnemyManager::RenderDebug()
{
	// Enemies
	m_enemyMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemyList.size(); i++)
	{
		Enemy* pEnemy = m_vpEnemyList[i];

		// TODO : Loader radius culling
		//if(m_pChunkManager->IsInsideLoader(pEnemy->GetCenter()) == false)
		//{
		//	continue;
		//}

		if(m_pRenderer->SphereInFrustum(VoxGame::GetInstance()->GetDefaultViewport(), pEnemy->GetCenter(), pEnemy->GetRadius()))
		{
			pEnemy->RenderDebug();
		}
	}
	m_enemyMutex.unlock();

	// Enemy spawners
	m_enemySpawnerMutex.lock();
	for(unsigned int i = 0; i < m_vpEnemySpawnerList.size(); i++)
	{
		EnemySpawner* pEnemySpawner = m_vpEnemySpawnerList[i];

		pEnemySpawner->RenderDebug();
	}
	m_enemySpawnerMutex.unlock();
}