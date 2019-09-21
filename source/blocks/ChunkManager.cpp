// ******************************************************************************
// Filename:    ChunkManager.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 01/11/15
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "ChunkManager.h"
#include "BiomeManager.h"
#include "../Player/Player.h"
#include "../NPC/NPCManager.h"
#include "../Enemy/EnemyManager.h"
#include "../Items/ItemManager.h"
#include "../Particles/BlockParticleManager.h"
#include "../VoxSettings.h"
#include "../VoxGame.h"
#include "../utils/Random.h"
#include "../models/QubicleBinaryManager.h"

#include <algorithm>


ChunkManager::ChunkManager(Renderer* pRenderer, VoxSettings* pVoxSettings, QubicleBinaryManager* pQubicleBinaryManager)
{
	m_pRenderer = pRenderer;
	m_pPlayer = nullptr;
	m_pVoxSettings = pVoxSettings;
	m_pQubicleBinaryManager = pQubicleBinaryManager;

	// Chunk material
	m_chunkMaterialID = -1;
	m_pRenderer->CreateMaterial(Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f), 64, &m_chunkMaterialID);

	// Create the block colour to cblock type matching
	AddBlockColourBlockTypeMatching(59, 34, 4, BlockType_Wood);
	AddBlockColourBlockTypeMatching(82, 51, 4, BlockType_Wood);
	AddBlockColourBlockTypeMatching(87, 58, 0, BlockType_Wood);
	AddBlockColourBlockTypeMatching(25, 21, 14, BlockType_Wood);
	AddBlockColourBlockTypeMatching(30, 26, 18, BlockType_Wood);
	AddBlockColourBlockTypeMatching(132, 97, 36, BlockType_Wood);
	AddBlockColourBlockTypeMatching(55, 172, 3, BlockType_Leaf);
	AddBlockColourBlockTypeMatching(27, 82, 0, BlockType_Leaf);
	AddBlockColourBlockTypeMatching(61, 95, 24, BlockType_Leaf);
	AddBlockColourBlockTypeMatching(67, 104, 27, BlockType_Leaf);
	AddBlockColourBlockTypeMatching(121, 134, 0, BlockType_Leaf);
	AddBlockColourBlockTypeMatching(121, 134, 0, BlockType_Leaf);
	AddBlockColourBlockTypeMatching(113, 113, 1, BlockType_Leaf);
	AddBlockColourBlockTypeMatching(0, 182, 0, BlockType_Cactus);
	AddBlockColourBlockTypeMatching(34, 26, 48, BlockType_Wood); // TODO : Should be ash leaf, from ash trees
	AddBlockColourBlockTypeMatching(33, 26, 45, BlockType_Wood); // TODO : Should be ash leaf, from ash trees
	AddBlockColourBlockTypeMatching(255, 255, 255, BlockType_Snow);

	// Loader radius
	m_loaderRadius = m_pVoxSettings->m_loaderRadius;

	// Water
	m_waterHeight = 0.0f;

	// Update lock
	m_stepLockEnabled = false;
	m_updateStepLock = true;

	// Rendering modes
	m_wireframeRender = false;
	m_faceMerging = true;

	// Chunk counters
	m_numChunksLoaded = 0;
	m_numChunksRender = 0;

	// Threading
	m_updateThreadActive = true;
	m_updateThreadFinished = false;
	m_pUpdatingChunksThread = new thread(_UpdatingChunksThread, this);
}

ChunkManager::~ChunkManager()
{
	// Clear the block colour to block type matching data
	for (unsigned int i = 0; i < m_vpBlockColourTypeMatchList.size(); i++)
	{
		delete m_vpBlockColourTypeMatchList[i];
		m_vpBlockColourTypeMatchList[i] = nullptr;
	}
	m_vpBlockColourTypeMatchList.clear();

	m_stepLockEnabled = false;
	m_updateStepLock = true;
	m_updateThreadFlagLock.lock();
	m_updateThreadActive = false;
	m_updateThreadFlagLock.unlock();
	while (m_updateThreadFinished == false)
	{
#ifdef _WIN32
    		Sleep(200);
#else
    		usleep(200000);
#endif
	}
#ifdef _WIN32
	Sleep(200);
#else
	usleep(200000);
#endif
}

// Linkage
void ChunkManager::SetPlayer(Player* pPlayer)
{
	m_pPlayer = pPlayer;
}

void ChunkManager::SetNPCManager(NPCManager* pNPCManager)
{
	m_pNPCManager = pNPCManager;
}

void ChunkManager::SetEnemyManager(EnemyManager* pEnemyManager)
{
	m_pEnemyManager = pEnemyManager;
}

void ChunkManager::SetBlockParticleManager(BlockParticleManager* pBlockParticleManager)
{
	m_pBlockParticleManager = pBlockParticleManager;
}

void ChunkManager::SetItemManager(ItemManager* pItemManager)
{
	m_pItemManager = pItemManager;
}

// Scenery manager pointer
void ChunkManager::SetSceneryManager(SceneryManager* pSceneryManager)
{
	m_pSceneryManager = pSceneryManager;
}

// Biome manager
void ChunkManager::SetBiomeManager(BiomeManager* pBiomeManager)
{
	m_pBiomeManager = pBiomeManager;
}

// Initial chunk creation
void ChunkManager::InitializeChunkCreation()
{
	// Create initial chunk
	CreateNewChunk(m_pPlayer->GetGridX(), m_pPlayer->GetGridY(), m_pPlayer->GetGridZ());
}

// Chunk rendering material
unsigned int ChunkManager::GetChunkMaterialID()
{
	return m_chunkMaterialID;
}

int ChunkManager::GetNumChunksLoaded()
{
	return m_numChunksLoaded;
}

int ChunkManager::GetNumChunksRender()
{
	return m_numChunksRender;
}

// Loader radius
void ChunkManager::SetLoaderRadius(float radius)
{
	m_loaderRadius = radius;
}

float ChunkManager::GetLoaderRadius()
{
	return m_loaderRadius;
}

// Step update
void ChunkManager::SetStepLockEnabled(bool enabled)
{
	m_stepLockEnabled = enabled;
}

void ChunkManager::StepNextUpdate()
{
	m_updateStepLock = false;
}

// Chunk Creation
void ChunkManager::CreateNewChunk(int x, int y, int z)
{
	ChunkCoordKeys coordKeys;
	coordKeys.x = x;
	coordKeys.y = y;
	coordKeys.z = z;

	// Create a new chunk at this grid position
	Chunk* pNewChunk = new Chunk(m_pRenderer, this, m_pVoxSettings);
	pNewChunk->SetPlayer(m_pPlayer);
	pNewChunk->SetSceneryManager(m_pSceneryManager);
	pNewChunk->SetBiomeManager(m_pBiomeManager);

	float xPos = x * (Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f);
	float yPos = y * (Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f);
	float zPos = z * (Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f);

	pNewChunk->SetPosition(vec3(xPos, yPos, zPos));
	pNewChunk->SetGrid(coordKeys.x, coordKeys.y, coordKeys.z);

	m_ChunkMapMutexLock.lock();
	m_chunksMap[coordKeys] = pNewChunk;
	m_ChunkMapMutexLock.unlock();

	pNewChunk->Setup();
	pNewChunk->SetNeedsRebuild(false, true);
	pNewChunk->RebuildMesh();
	pNewChunk->CompleteMesh();
	pNewChunk->SetCreated(true);

	UpdateChunkNeighbours(pNewChunk, x, y, z);
}

void ChunkManager::UpdateChunkNeighbours(Chunk* pChunk, int x, int y, int z)
{
	pChunk->SetNumNeighbours(0);

	Chunk* pChunkXMinus = GetChunk(x - 1, y, z);
	Chunk* pChunkXPlus = GetChunk(x + 1, y, z);
	Chunk* pChunkYMinus = GetChunk(x, y - 1, z);
	Chunk* pChunkYPlus = GetChunk(x, y + 1, z);
	Chunk* pChunkZMinus = GetChunk(x, y, z - 1);
	Chunk* pChunkZPlus = GetChunk(x, y, z + 1);

	if (pChunkXMinus)
	{
		pChunk->SetNumNeighbours(pChunk->GetNumNeighbours() + 1);
		pChunk->SetxMinus(pChunkXMinus);
		if (pChunkXMinus->GetxPlus() == nullptr)
		{
			pChunkXMinus->SetNumNeighbours(pChunkXMinus->GetNumNeighbours() + 1);
			pChunkXMinus->SetxPlus(pChunk);
		}
	}
	if (pChunkXPlus)
	{
		pChunk->SetNumNeighbours(pChunk->GetNumNeighbours() + 1);
		pChunk->SetxPlus(pChunkXPlus);
		if (pChunkXPlus->GetxMinus() == nullptr)
		{
			pChunkXPlus->SetNumNeighbours(pChunkXPlus->GetNumNeighbours() + 1);
			pChunkXPlus->SetxMinus(pChunk);
		}
	}
	if (pChunkYMinus)
	{
		pChunk->SetNumNeighbours(pChunk->GetNumNeighbours() + 1);
		pChunk->SetyMinus(pChunkYMinus);
		if (pChunkYMinus->GetyPlus() == nullptr)
		{
			pChunkYMinus->SetNumNeighbours(pChunkYMinus->GetNumNeighbours() + 1);
			pChunkYMinus->SetyPlus(pChunk);
		}
	}
	if (pChunkYPlus)
	{
		pChunk->SetNumNeighbours(pChunk->GetNumNeighbours() + 1);
		pChunk->SetyPlus(pChunkYPlus);
		if (pChunkYPlus->GetyMinus() == nullptr)
		{
			pChunkYPlus->SetNumNeighbours(pChunkYPlus->GetNumNeighbours() + 1);
			pChunkYPlus->SetyMinus(pChunk);
		}
	}
	if (pChunkZMinus)
	{
		pChunk->SetNumNeighbours(pChunk->GetNumNeighbours() + 1);
		pChunk->SetzMinus(pChunkZMinus);
		if (pChunkZMinus->GetzPlus() == nullptr)
		{
			pChunkZMinus->SetNumNeighbours(pChunkZMinus->GetNumNeighbours() + 1);
			pChunkZMinus->SetzPlus(pChunk);
		}
	}
	if (pChunkZPlus)
	{
		pChunk->SetNumNeighbours(pChunk->GetNumNeighbours() + 1);
		pChunk->SetzPlus(pChunkZPlus);
		if (pChunkZPlus->GetzMinus() == nullptr)
		{
			pChunkZPlus->SetNumNeighbours(pChunkZPlus->GetNumNeighbours() + 1);
			pChunkZPlus->SetzMinus(pChunk);
		}
	}
}

void ChunkManager::UnloadChunk(Chunk* pChunk)
{
	ChunkCoordKeys coordKeys;
	coordKeys.x = pChunk->GetGridX();
	coordKeys.y = pChunk->GetGridY();
	coordKeys.z = pChunk->GetGridZ();

	Chunk* pChunkXMinus = GetChunk(coordKeys.x - 1, coordKeys.y, coordKeys.z);
	Chunk* pChunkXPlus = GetChunk(coordKeys.x + 1, coordKeys.y, coordKeys.z);
	Chunk* pChunkYMinus = GetChunk(coordKeys.x, coordKeys.y - 1, coordKeys.z);
	Chunk* pChunkYPlus = GetChunk(coordKeys.x, coordKeys.y + 1, coordKeys.z);
	Chunk* pChunkZMinus = GetChunk(coordKeys.x, coordKeys.y, coordKeys.z - 1);
	Chunk* pChunkZPlus = GetChunk(coordKeys.x, coordKeys.y, coordKeys.z + 1);

	if (pChunkXMinus)
	{
		if (pChunkXMinus->GetxPlus())
		{
			pChunkXMinus->SetNumNeighbours(pChunkXMinus->GetNumNeighbours() - 1);
			pChunkXMinus->SetxPlus(nullptr);
		}
	}
	if (pChunkXPlus)
	{
		if (pChunkXPlus->GetxMinus())
		{
			pChunkXPlus->SetNumNeighbours(pChunkXPlus->GetNumNeighbours() - 1);
			pChunkXPlus->SetxMinus(nullptr);
		}
	}
	if (pChunkYMinus)
	{
		if (pChunkYMinus->GetyPlus())
		{
			pChunkYMinus->SetNumNeighbours(pChunkYMinus->GetNumNeighbours() - 1);
			pChunkYMinus->SetyPlus(nullptr);
		}
	}
	if (pChunkYPlus)
	{
		if (pChunkYPlus->GetyMinus())
		{
			pChunkYPlus->SetNumNeighbours(pChunkYPlus->GetNumNeighbours() - 1);
			pChunkYPlus->SetyMinus(nullptr);
		}
	}
	if (pChunkZMinus)
	{
		if (pChunkZMinus->GetzPlus())
		{
			pChunkZMinus->SetNumNeighbours(pChunkZMinus->GetNumNeighbours() - 1);
			pChunkZMinus->SetzPlus(nullptr);
		}
	}
	if (pChunkZPlus)
	{
		if (pChunkZPlus->GetzMinus())
		{
			pChunkZPlus->SetNumNeighbours(pChunkZPlus->GetNumNeighbours() - 1);
			pChunkZPlus->SetzMinus(nullptr);
		}
	}

	// Remove from map
	m_ChunkMapMutexLock.lock();
	map<ChunkCoordKeys, Chunk*>::iterator it = m_chunksMap.find(coordKeys);
	if (it != m_chunksMap.end())
	{
		m_chunksMap.erase(coordKeys);
	}
	m_ChunkMapMutexLock.unlock();

	// Clear chunk linkage
	m_updateThreadFlagLock.lock();
	if (m_updateThreadActive)
	{
		if (m_pPlayer != nullptr)
		{
			m_pPlayer->ClearChunkCacheForChunk(pChunk);
		}
		if (m_pNPCManager != nullptr)
		{
			m_pNPCManager->ClearNPCChunkCacheForChunk(pChunk);
		}
		if (m_pEnemyManager)
		{
			m_pEnemyManager->ClearEnemyChunkCacheForChunk(pChunk);
		}
		if (m_pBlockParticleManager)
		{
			m_pBlockParticleManager->ClearParticleChunkCacheForChunk(pChunk);
		}
	}
	m_updateThreadFlagLock.unlock();

	// Unload and delete
	pChunk->Unload();
	delete pChunk;
}

// Getting chunk and positional information
void ChunkManager::GetGridFromPosition(vec3 position, int* gridX, int* gridY, int* gridZ)
{
	*gridX = (int)((position.x + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);
	*gridY = (int)((position.y + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);
	*gridZ = (int)((position.z + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);

	if (position.x <= -0.5f)
		*gridX -= 1;
	if (position.y <= -0.5f)
		*gridY -= 1;
	if (position.z <= -0.5f)
		*gridZ -= 1;
}

Chunk* ChunkManager::GetChunkFromPosition(float posX, float posY, float posZ)
{
	int gridX = (int)((posX + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);
	int gridY = (int)((posY + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);
	int gridZ = (int)((posZ + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);

	if (posX <= -0.5f)
		gridX -= 1;
	if (posY <= -0.5f)
		gridY -= 1;
	if (posZ <= -0.5f)
		gridZ -= 1;

	return GetChunk(gridX, gridY, gridZ);
}

Chunk* ChunkManager::GetChunk(int aX, int aY, int aZ)
{
	ChunkCoordKeys chunkKey;
	chunkKey.x = aX;
	chunkKey.y = aY;
	chunkKey.z = aZ;

	m_ChunkMapMutexLock.lock();
	map<ChunkCoordKeys, Chunk*>::iterator it = m_chunksMap.find(chunkKey);
	if (it != m_chunksMap.end())
	{
		Chunk* lpReturn = m_chunksMap[chunkKey];
		m_ChunkMapMutexLock.unlock();
		return lpReturn;
	}
	m_ChunkMapMutexLock.unlock();

	return nullptr;
}

bool ChunkManager::FindClosestFloor(vec3 position, vec3* floorPosition)
{
	int blockX, blockY, blockZ;
	vec3 blockPos;

	bool collides = false;
	int iterations = 1;
	while (collides == false && iterations < 100)
	{
		vec3 testPos = position - (vec3(0.0f, Chunk::BLOCK_RENDER_SIZE, 0.0f) * (float)iterations);

		Chunk* pChunk = nullptr;
		bool active = GetBlockActiveFrom3DPosition(testPos.x, testPos.y, testPos.z, &blockPos, &blockX, &blockY, &blockZ, &pChunk);

		if (pChunk != nullptr && pChunk->IsSetup() && pChunk->NeedsRebuild() == false && active == true)
		{
			collides = true;

			*floorPosition = blockPos + vec3(0.0f, Chunk::BLOCK_RENDER_SIZE, 0.0f);
			(*floorPosition).x = position.x;
			(*floorPosition).z = position.z;

			return true;
		}

		iterations++;
	}

	return collides;
}

// Getting the active block state given a position and chunk information
bool ChunkManager::GetBlockActiveFrom3DPosition(float x, float y, float z, vec3 *blockPos, int* blockX, int* blockY, int* blockZ, Chunk** pChunk)
{
	if (*pChunk == nullptr)
	{
		*pChunk = GetChunkFromPosition(x, y, z);

		if (*pChunk == nullptr)
		{
			return false;
		}
	}

	(*blockX) = (int)((abs(x) + Chunk::BLOCK_RENDER_SIZE) / (Chunk::BLOCK_RENDER_SIZE*2.0f));
	(*blockY) = (int)((abs(y) + Chunk::BLOCK_RENDER_SIZE) / (Chunk::BLOCK_RENDER_SIZE*2.0f));
	(*blockZ) = (int)((abs(z) + Chunk::BLOCK_RENDER_SIZE) / (Chunk::BLOCK_RENDER_SIZE*2.0f));

	(*blockX) = (*blockX) % Chunk::CHUNK_SIZE;
	(*blockY) = (*blockY) % Chunk::CHUNK_SIZE;
	(*blockZ) = (*blockZ) % Chunk::CHUNK_SIZE;

	(*blockPos).x = (*pChunk)->GetPosition().x + (*blockX) * (Chunk::BLOCK_RENDER_SIZE*2.0f);
	(*blockPos).y = (*pChunk)->GetPosition().y + (*blockY) * (Chunk::BLOCK_RENDER_SIZE*2.0f);
	(*blockPos).z = (*pChunk)->GetPosition().z + (*blockZ) * (Chunk::BLOCK_RENDER_SIZE*2.0f);

	if (x < 0.0f)
	{
		if ((*blockX) == 0)
		{
			(*blockPos).x = (*pChunk)->GetPosition().x;
		}
		else
		{
			(*blockPos).x = (*pChunk)->GetPosition().x - ((*blockX) * (Chunk::BLOCK_RENDER_SIZE*2.0f)) + (Chunk::CHUNK_SIZE * (Chunk::BLOCK_RENDER_SIZE*2.0f));

			(*blockX) = (Chunk::CHUNK_SIZE) - (*blockX);
		}
	}
	if (y < 0.0f)
	{
		if ((*blockY) == 0)
		{
			(*blockPos).y = (*pChunk)->GetPosition().y;
		}
		else
		{
			(*blockPos).y = (*pChunk)->GetPosition().y - ((*blockY) * (Chunk::BLOCK_RENDER_SIZE*2.0f)) + (Chunk::CHUNK_SIZE * (Chunk::BLOCK_RENDER_SIZE*2.0f));

			(*blockY) = (Chunk::CHUNK_SIZE) - (*blockY);
		}
	}
	if (z < 0.0f)
	{
		if ((*blockZ) == 0)
		{
			(*blockPos).z = (*pChunk)->GetPosition().z;
		}
		else
		{
			(*blockPos).z = (*pChunk)->GetPosition().z - ((*blockZ) * (Chunk::BLOCK_RENDER_SIZE*2.0f)) + (Chunk::CHUNK_SIZE * (Chunk::BLOCK_RENDER_SIZE*2.0f));

			(*blockZ) = (Chunk::CHUNK_SIZE) - (*blockZ);
		}
	}

	return (*pChunk)->GetActive((*blockX), (*blockY), (*blockZ));
}

void ChunkManager::GetBlockGridFrom3DPositionChunkStorage(float x, float y, float z, int* blockX, int* blockY, int* blockZ, ChunkStorageLoader* ChunkStorage)
{
	(*blockX) = (int)((abs(x) + Chunk::BLOCK_RENDER_SIZE) / (Chunk::BLOCK_RENDER_SIZE*2.0f));
	(*blockY) = (int)((abs(y) + Chunk::BLOCK_RENDER_SIZE) / (Chunk::BLOCK_RENDER_SIZE*2.0f));
	(*blockZ) = (int)((abs(z) + Chunk::BLOCK_RENDER_SIZE) / (Chunk::BLOCK_RENDER_SIZE*2.0f));

	(*blockX) = (*blockX) % Chunk::CHUNK_SIZE;
	(*blockY) = (*blockY) % Chunk::CHUNK_SIZE;
	(*blockZ) = (*blockZ) % Chunk::CHUNK_SIZE;

	if (x < 0.0f)
	{
		if ((*blockX) == 0)
		{
		}
		else
		{
			(*blockX) = (Chunk::CHUNK_SIZE) - (*blockX);
		}
	}
	if (y < 0.0f)
	{
		if ((*blockY) == 0)
		{
		}
		else
		{
			(*blockY) = (Chunk::CHUNK_SIZE) - (*blockY);
		}
	}
	if (z < 0.0f)
	{
		if ((*blockZ) == 0)
		{
		}
		else
		{
			(*blockZ) = (Chunk::CHUNK_SIZE) - (*blockZ);
		}
	}
}

// Adding to chunk storage for parts of the world generation that are outside of loaded chunks
ChunkStorageLoader* ChunkManager::GetChunkStorage(int aX, int aY, int aZ, bool CreateIfNotExist)
{
	m_chunkStorageListLock.lock();
	for (unsigned int i = 0; i < m_vpChunkStorageList.size(); i++)
	{
		if (m_vpChunkStorageList[i]->m_gridX == aX && m_vpChunkStorageList[i]->m_gridY == aY && m_vpChunkStorageList[i]->m_gridZ == aZ)
		{
			m_chunkStorageListLock.unlock();

			// Found and existing chunk storage, return it
			return m_vpChunkStorageList[i];
		}
	}
	m_chunkStorageListLock.unlock();

	// No storage found, create a new one
	if (CreateIfNotExist)
	{
		ChunkStorageLoader* pNewStorage = new ChunkStorageLoader(aX, aY, aZ);
		pNewStorage->m_gridX = aX;
		pNewStorage->m_gridY = aY;
		pNewStorage->m_gridZ = aZ;

		m_chunkStorageListLock.lock();
		m_vpChunkStorageList.push_back(pNewStorage);
		m_chunkStorageListLock.unlock();

		return pNewStorage;
	}

	return nullptr;
}

void ChunkManager::RemoveChunkStorageLoader(ChunkStorageLoader* pChunkStorage)
{
	m_chunkStorageListLock.lock();
	ChunkStorageLoaderList::iterator iter = std::find(m_vpChunkStorageList.begin(), m_vpChunkStorageList.end(), pChunkStorage);
	if (iter != m_vpChunkStorageList.end())
	{
		m_vpChunkStorageList.erase(iter);
	}
	m_chunkStorageListLock.unlock();

	delete pChunkStorage;
	pChunkStorage = nullptr;
}


// Block colour to block type matching
void ChunkManager::AddBlockColourBlockTypeMatching(int r, int g, int b, BlockType blockType)
{
	BlockColourTypeMatch* pMatch = new BlockColourTypeMatch();
	pMatch->m_red = r;
	pMatch->m_green = g;
	pMatch->m_blue = b;
	pMatch->m_blockType = blockType;

	m_vpBlockColourTypeMatchList.push_back(pMatch);
}

bool ChunkManager::CheckBlockColour(int r, int g, int b, int rCheck, int gCheck, int bCheck)
{
	if (r != rCheck)
	{
		return false;
	}

	if (g != gCheck)
	{
		return false;
	}

	if (b != bCheck)
	{
		return false;
	}

	return true;
}

BlockType ChunkManager::SetBlockTypeBasedOnColour(int r, int g, int b)
{
	for (int i = 0; i < m_vpBlockColourTypeMatchList.size(); i++)
	{
		BlockColourTypeMatch* pMatch = m_vpBlockColourTypeMatchList[i];

		if (CheckBlockColour(pMatch->m_red, pMatch->m_green, pMatch->m_blue, r, g, b))
		{
			return pMatch->m_blockType;
		}
	}

	return BlockType_Default;	
}

// Importing into the world chunks
void ChunkManager::ImportQubicleBinaryMatrix(const QubicleMatrix* pMatrix, vec3 position, QubicleImportDirection direction)
{
	bool mirrorX = false;
	bool mirrorY = false;
	bool mirrorZ = false;
	bool flipXZ = false;
	bool flipXY = false;
	bool flipYZ = false;

	switch (direction)
	{
	case QubicleImportDirection_Normal: {  } break;
	case QubicleImportDirection_MirrorX: { mirrorX = true; } break;
	case QubicleImportDirection_MirrorY: { mirrorY = true; } break;
	case QubicleImportDirection_MirrorZ: { mirrorZ = true; } break;
	case QubicleImportDirection_RotateY90: { mirrorX = true; flipXZ = true; } break;
	case QubicleImportDirection_RotateY180: { mirrorX = true; mirrorZ = true; } break;
	case QubicleImportDirection_RotateY270: { mirrorZ = true; flipXZ = true; } break;
	case QubicleImportDirection_RotateX90: { mirrorZ = true; flipYZ = true; } break;
	case QubicleImportDirection_RotateX180: { mirrorZ = true; mirrorY = true; } break;
	case QubicleImportDirection_RotateX270: { mirrorY = true; flipYZ = true; } break;
	case QubicleImportDirection_RotateZ90: { mirrorY = true; flipXY = true; } break;
	case QubicleImportDirection_RotateZ180: { mirrorX = true; mirrorY = true; } break;
	case QubicleImportDirection_RotateZ270: { mirrorX = true; flipXY = true; } break;
	}

	ChunkList vChunkBatchUpdateList;

	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
	float a = 1.0f;

	unsigned int xValueToUse = pMatrix->m_matrixSizeX;
	unsigned int yValueToUse = pMatrix->m_matrixSizeY;
	unsigned int zValueToUse = pMatrix->m_matrixSizeZ;
	if (flipXZ)
	{
		xValueToUse = pMatrix->m_matrixSizeZ;
		zValueToUse = pMatrix->m_matrixSizeX;
	}
	if (flipXY)
	{
		xValueToUse = pMatrix->m_matrixSizeY;
		yValueToUse = pMatrix->m_matrixSizeX;
	}
	if (flipYZ)
	{
		yValueToUse = pMatrix->m_matrixSizeZ;
		zValueToUse = pMatrix->m_matrixSizeY;
	}

	int xPosition = 0;
	if (mirrorX)
		xPosition = xValueToUse - 1;

	for (unsigned int x = 0; x < xValueToUse; x++)
	{
		int yPosition = 0;
		if (mirrorY)
			yPosition = yValueToUse - 1;

		for (unsigned int y = 0; y < yValueToUse; y++)
		{
			int zPosition = 0;
			if (mirrorZ)
				zPosition = zValueToUse - 1;

			for (unsigned int z = 0; z < zValueToUse; z++)
			{
				int xPosition_modified = xPosition;
				int yPosition_modified = yPosition;
				int zPosition_modified = zPosition;
				if (flipXZ)
				{
					xPosition_modified = zPosition;
					zPosition_modified = xPosition;
				}
				if (flipXY)
				{
					xPosition_modified = yPosition;
					yPosition_modified = xPosition;
				}
				if (flipYZ)
				{
					yPosition_modified = zPosition;
					zPosition_modified = yPosition;
				}

				if (pMatrix->GetActive(xPosition_modified, yPosition_modified, zPosition_modified) == false)
				{
					// Do nothing
				}
				else
				{
					unsigned int colour = pMatrix->GetColourCompact(xPosition_modified, yPosition_modified, zPosition_modified);

					vec3 blockPos = position - vec3((xValueToUse + 0.05f)*0.5f, 0.0f, (zValueToUse + 0.05f)*0.5f) + vec3(x*Chunk::BLOCK_RENDER_SIZE*2.0f, y*Chunk::BLOCK_RENDER_SIZE*2.0f, z*Chunk::BLOCK_RENDER_SIZE*2.0f);

					vec3 blockPosition;
					int blockX, blockY, blockZ;
					Chunk* pChunk = nullptr;
					bool blockActive = GetBlockActiveFrom3DPosition(blockPos.x, blockPos.y, blockPos.z, &blockPosition, &blockX, &blockY, &blockZ, &pChunk);

					if (pChunk != nullptr)
					{
						// Set the block colour (and also set the block type since we are importing a world scenery object)
						pChunk->SetColour(blockX, blockY, blockZ, colour, true);

						// Add to batch update list (no duplicates)
						bool found = false;
						for (int i = 0; i < (int)vChunkBatchUpdateList.size() && found == false; i++)
						{
							if (vChunkBatchUpdateList[i] == pChunk)
							{
								found = true;
							}
						}
						if (found == false)
						{
							vChunkBatchUpdateList.push_back(pChunk);
							pChunk->StartBatchUpdate();
						}
					}
					else
					{
						// Add to the chunk storage
						int gridX;
						int gridY;
						int gridZ;
						GetGridFromPosition(blockPos, &gridX, &gridY, &gridZ);
						ChunkStorageLoader* pStorage = GetChunkStorage(gridX, gridY, gridZ, true);

						if (pStorage != nullptr)
						{
							GetBlockGridFrom3DPositionChunkStorage(blockPos.x, blockPos.y, blockPos.z, &blockX, &blockY, &blockZ, pStorage);

							pStorage->SetBlockColour(blockX, blockY, blockZ, colour);
						}
					}
				}

				if (mirrorZ)
					zPosition--;
				else
					zPosition++;
			}

			if (mirrorY)
				yPosition--;
			else
				yPosition++;
		}

		if (mirrorX)
			xPosition--;
		else
			xPosition++;
	}

	for (int i = 0; i < (int)vChunkBatchUpdateList.size(); i++)
	{
		vChunkBatchUpdateList[i]->StopBatchUpdate();
	}
	vChunkBatchUpdateList.clear();
}

const QubicleBinary* ChunkManager::ImportQubicleBinary(const QubicleBinary* qubicleBinaryFile, vec3 position, QubicleImportDirection direction)
{
	int numMatrices = qubicleBinaryFile->GetNumMatrices();

	for (int i = 0; i < numMatrices; i++)
	{
		const QubicleMatrix* pMatrix = qubicleBinaryFile->GetQubicleMatrix(i);

		ImportQubicleBinaryMatrix(pMatrix, position, direction);
	}

	return qubicleBinaryFile;
}

const QubicleBinary* ChunkManager::ImportQubicleBinary(const char* filename, vec3 position, QubicleImportDirection direction)
{
	QubicleBinary* qubicleBinaryFile = m_pQubicleBinaryManager->GetQubicleBinaryFile(filename, true);
	if (qubicleBinaryFile != nullptr)
	{
		return ImportQubicleBinary(qubicleBinaryFile, position, direction);
	}

	return nullptr;
}

// Explosions
void ChunkManager::CreateBlockDestroyParticleEffect(float r, float g, float b, float a, vec3 blockPosition)
{
	for (int i = 0; i < 8; i++)
	{
		float size = Chunk::BLOCK_RENDER_SIZE*0.5f;
		float scale = 0.3f + (GetRandomNumber(-1, 1, 4)*0.2f);
		vec3 addition;
		if (i == 0) addition = vec3(-size, size, -size);
		if (i == 1) addition = vec3(size, size, -size);
		if (i == 2) addition = vec3(-size, size, size);
		if (i == 3) addition = vec3(size, size, size);
		if (i == 4) addition = vec3(-size, -size, -size);
		if (i == 5) addition = vec3(size, -size, -size);
		if (i == 6) addition = vec3(-size, -size, size);
		if (i == 7) addition = vec3(size, -size, size);

		float lifeTime = 6.5f + GetRandomNumber(-1, 1, 1) * 0.75f;

		vec3 gravityDir = vec3(0.0f, -1.0f, 0.0f);
		vec3 pointOrigin = vec3(0.0f, 0.0f, 0.0f);
		BlockParticle* pParticle = m_pBlockParticleManager->CreateBlockParticle(blockPosition + addition, blockPosition + addition, gravityDir, 2.5f, pointOrigin, scale, 0.0f, scale, 0.0f,
			r, g, b, a, 0.0f, 0.0f, 0.0f, 0.0f, r, g, b, a, 0.0f, 0.0f, 0.0f, 0.0f, lifeTime, 0.0f, 0.0f, 0.0f, vec3(0.0f, 7.0f, 0.0f),
			vec3(3.0f, 2.0f, 3.0f), vec3(GetRandomNumber(-360, 360, 2), GetRandomNumber(-360, 360, 2), GetRandomNumber(-360, 360, 2)),
			vec3(180.0f, 180.0f, 180.0f), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, vec3(0.0f, 0.0f, 0.0f), true, false, false, false, nullptr);
	}
}

void ChunkManager::ExplodeSphere(vec3 position, float radius)
{
	float startx = position.x - radius;
	float starty = position.y - radius;
	float startz = position.z - radius;
	float endx = position.x + radius;
	float endy = position.y + radius;
	float endz = position.z + radius;

	ChunkList vChunkBatchUpdateList;

	for (float x = startx; x < endx; x += Chunk::BLOCK_RENDER_SIZE)
	{
		for (float y = starty; y < endy; y += Chunk::BLOCK_RENDER_SIZE)
		{
			for (float z = startz; z < endz; z += Chunk::BLOCK_RENDER_SIZE)
			{
				vec3 blockPosition;
				int blockX, blockY, blockZ;
				Chunk* pChunk = nullptr;
				bool active = GetBlockActiveFrom3DPosition(x, y, z, &blockPosition, &blockX, &blockY, &blockZ, &pChunk);

				float distance = length(blockPosition - position);

				if (pChunk != nullptr)
				{
					if (distance <= radius)
					{
						if (active)
						{
							float r;
							float g;
							float b;
							float a;
							// Store the colour for particle effect later
							pChunk->GetColour(blockX, blockY, blockZ, &r, &g, &b, &a);

							// Remove the block from being active
							pChunk->SetColour(blockX, blockY, blockZ, 0);

							// Create particle effect
							if (GetRandomNumber(0, 100, 2) > 75.0f)
							{
								CreateBlockDestroyParticleEffect(r, g, b, a, blockPosition);
							}

							// Create the collectible block item
							if (GetRandomNumber(0, 100, 2) > 75.0f)
							{
								BlockType blockType = pChunk->GetBlockType(blockX, blockY, blockZ);
								CreateCollectibleBlock(blockType, blockPosition);
							}

							// Add to batch update list (no duplicates)
							bool found = false;
							for (int i = 0; i < (int)vChunkBatchUpdateList.size() && found == false; i++)
							{
								if (vChunkBatchUpdateList[i] == pChunk)
								{
									found = true;
								}
							}
							if (found == false)
							{
								vChunkBatchUpdateList.push_back(pChunk);
								pChunk->StartBatchUpdate();
							}
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < (int)vChunkBatchUpdateList.size(); i++)
	{
		vChunkBatchUpdateList[i]->StopBatchUpdate();
	}
	vChunkBatchUpdateList.clear();
}

// Collectible block objects
void ChunkManager::CreateCollectibleBlock(BlockType blockType, vec3 blockPos)
{
	Item* pItem = nullptr;

	ItemSubSpawnData *pItemSubSpawnData = m_pItemManager->GetItemSubSpawnData(blockType);
	if (pItemSubSpawnData != nullptr)
	{
		pItem = m_pItemManager->CreateItem(blockPos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), pItemSubSpawnData->m_spawnedItemFilename.c_str(), pItemSubSpawnData->m_spawnedItem, pItemSubSpawnData->m_spawnedItemTitle.c_str(), pItemSubSpawnData->m_interactable, pItemSubSpawnData->m_collectible, pItemSubSpawnData->m_scale);

		if (pItem != nullptr)
		{
			float radius = 1.5f;
			float angle = DegToRad((float)GetRandomNumber(0, 360, 2));
			vec3 ItemPosition = blockPos + vec3(cos(angle) * radius, 0.0f, sin(angle) * radius);

			pItem->SetGravityDirection(vec3(0.0f, -1.0f, 0.0f));
			vec3 vel = ItemPosition - blockPos;
			pItem->SetVelocity(normalize(vel)*(float)GetRandomNumber(0, 1, 2) + vec3(GetRandomNumber(-1, 1, 2), 1.0f + GetRandomNumber(2, 5, 2), GetRandomNumber(-1, 1, 2)));
			pItem->SetRotation(vec3(0.0f, GetRandomNumber(0, 360, 2), 0.0f));
			pItem->SetAngularVelocity(vec3(0.0f, 90.0f, 0.0f));

			pItem->SetDroppedItem(pItemSubSpawnData->m_droppedItemFilename.c_str(), pItemSubSpawnData->m_droppedItemTextureFilename.c_str(), pItemSubSpawnData->m_droppedItemInventoryType, pItemSubSpawnData->m_droppedItemItem, pItemSubSpawnData->m_droppedItemStatus, pItemSubSpawnData->m_droppedItemEquipSlot, pItemSubSpawnData->m_droppedItemQuality, pItemSubSpawnData->m_droppedItemLeft, pItemSubSpawnData->m_droppedItemRight, pItemSubSpawnData->m_droppedItemTitle.c_str(), pItemSubSpawnData->m_droppedItemDescription.c_str(), pItemSubSpawnData->m_droppedItemPlacementR, pItemSubSpawnData->m_droppedItemPlacementG, pItemSubSpawnData->m_droppedItemPlacementB, pItemSubSpawnData->m_droppedItemQuantity);
			pItem->SetAutoDisappear(20.0f + (GetRandomNumber(-20, 20, 1) * 0.2f));
			pItem->SetCollisionEnabled(false);
		}
	}
}

// Water
void ChunkManager::SetWaterHeight(float height)
{
	m_waterHeight = height;
}

float ChunkManager::GetWaterHeight()
{
	return m_waterHeight;
}

bool ChunkManager::IsUnderWater(vec3 position)
{
	if (m_pVoxSettings->m_waterRendering == false)
	{
		return false;
	}

	if (VoxGame::GetInstance()->GetGameMode() == GameMode_FrontEnd)
	{
		return false;
	}

	if(position.y <= m_waterHeight)
	{
		return true;
	}

	return false;
}

// Rendering modes
void ChunkManager::SetWireframeRender(bool wireframe)
{
	m_wireframeRender = wireframe;
}

void ChunkManager::SetFaceMerging(bool faceMerge)
{
	m_faceMerging = faceMerge;
}

bool ChunkManager::GetFaceMerging()
{
	return m_faceMerging;
}

// Updating
void ChunkManager::Update(float dt)
{
	m_numChunksLoaded = (int)m_chunksMap.size();
}

void ChunkManager::_UpdatingChunksThread(void* pData)
{
	ChunkManager* lpChunkManager = (ChunkManager*)pData;
	lpChunkManager->UpdatingChunksThread();
}

void ChunkManager::UpdatingChunksThread()
{
	while (m_updateThreadActive)
	{
		while (m_pPlayer == nullptr)
		{
#ifdef _WIN32
			Sleep(100);
#else
			usleep(100000);
#endif
		}

		while (m_stepLockEnabled == true && m_updateStepLock == true)
		{
#ifdef _WIN32
			Sleep(100);
#else
			usleep(100000);
#endif
		}

		ChunkList updateChunkList;
		ChunkCoordKeysList addChunkList;
		ChunkList rebuildChunkList;
		ChunkList unloadChunkList;

		m_ChunkMapMutexLock.lock();
		typedef map<ChunkCoordKeys, Chunk*>::iterator it_type;
		for (it_type iterator = m_chunksMap.begin(); iterator != m_chunksMap.end(); iterator++)
		{
			Chunk* pChunk = iterator->second;

			updateChunkList.push_back(pChunk);
		}
		m_ChunkMapMutexLock.unlock();

		// Updating chunks
		int numAddedChunks = 0;
		int MAX_NUM_CHUNKS_ADD = 10;
		sort(updateChunkList.begin(), updateChunkList.end(), Chunk::ClosestToCamera);
		for (unsigned int i = 0; i < (int)updateChunkList.size(); i++)
		{
			Chunk* pChunk = updateChunkList[i];

			if (pChunk != nullptr)
			{
				pChunk->Update(0.01f);

				int gridX = pChunk->GetGridX();
				int gridY = pChunk->GetGridY();
				int gridZ = pChunk->GetGridZ();

				float xPos = gridX * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
				float yPos = gridY * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
				float zPos = gridZ * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;

				vec3 chunkCenter = vec3(xPos, yPos, zPos) + vec3(Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE);
				vec3 distanceVec = chunkCenter - m_pPlayer->GetCenter();
				float lengthValue = length(distanceVec);

				if (lengthValue > m_loaderRadius)
				{
					unloadChunkList.push_back(pChunk);
				}
				else
				{
					if (numAddedChunks < MAX_NUM_CHUNKS_ADD)
					{
						// Check neighbours
						if (pChunk->GetNumNeighbours() < 6 && (pChunk->IsEmpty() == false) || (gridY == 0))
						{
							if (pChunk->GetxMinus() == nullptr)
							{
								ChunkCoordKeys coordKey;
								coordKey.x = gridX - 1;
								coordKey.y = gridY;
								coordKey.z = gridZ;
								float xPos = coordKey.x * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
								float yPos = coordKey.y * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
								float zPos = coordKey.z * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;

								vec3 chunkCenter = vec3(xPos, yPos, zPos) + vec3(Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE);
								vec3 distanceVec = chunkCenter - m_pPlayer->GetCenter();
								float lengthValue = length(distanceVec);

								if (lengthValue <= m_loaderRadius)
								{
									addChunkList.push_back(coordKey);
									numAddedChunks++;
								}
							}
							if (pChunk->GetxPlus() == nullptr)
							{
								ChunkCoordKeys coordKey;
								coordKey.x = gridX + 1;
								coordKey.y = gridY;
								coordKey.z = gridZ;
								float xPos = coordKey.x * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
								float yPos = coordKey.y * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
								float zPos = coordKey.z * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;

								vec3 chunkCenter = vec3(xPos, yPos, zPos) + vec3(Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE);
								vec3 distanceVec = chunkCenter - m_pPlayer->GetCenter();
								float lengthValue = length(distanceVec);

								if (lengthValue <= m_loaderRadius)
								{
									addChunkList.push_back(coordKey);
									numAddedChunks++;
								}
							}
							if (pChunk->GetyMinus() == nullptr)
							{
								ChunkCoordKeys coordKey;
								coordKey.x = gridX;
								coordKey.y = gridY - 1;
								coordKey.z = gridZ;
								float xPos = coordKey.x * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
								float yPos = coordKey.y * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
								float zPos = coordKey.z * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;

								vec3 chunkCenter = vec3(xPos, yPos, zPos) + vec3(Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE);
								vec3 distanceVec = chunkCenter - m_pPlayer->GetCenter();
								float lengthValue = length(distanceVec);

								if (lengthValue <= m_loaderRadius)
								{
									addChunkList.push_back(coordKey);
									numAddedChunks++;
								}
							}
							if (pChunk->GetyPlus() == nullptr)
							{
								ChunkCoordKeys coordKey;
								coordKey.x = gridX;
								coordKey.y = gridY + 1;
								coordKey.z = gridZ;
								float xPos = coordKey.x * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
								float yPos = coordKey.y * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
								float zPos = coordKey.z * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;

								vec3 chunkCenter = vec3(xPos, yPos, zPos) + vec3(Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE);
								vec3 distanceVec = chunkCenter - m_pPlayer->GetCenter();
								float lengthValue = length(distanceVec);

								if (lengthValue <= m_loaderRadius)
								{
									addChunkList.push_back(coordKey);
									numAddedChunks++;
								}
							}
							if (pChunk->GetzMinus() == nullptr)
							{
								ChunkCoordKeys coordKey;
								coordKey.x = gridX;
								coordKey.y = gridY;
								coordKey.z = gridZ - 1;
								float xPos = coordKey.x * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
								float yPos = coordKey.y * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
								float zPos = coordKey.z * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;

								vec3 chunkCenter = vec3(xPos, yPos, zPos) + vec3(Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE);
								vec3 distanceVec = chunkCenter - m_pPlayer->GetCenter();
								float lengthValue = length(distanceVec);

								if (lengthValue <= m_loaderRadius)
								{
									addChunkList.push_back(coordKey);
									numAddedChunks++;
								}
							}
							if (pChunk->GetzPlus() == nullptr)
							{
								ChunkCoordKeys coordKey;
								coordKey.x = gridX;
								coordKey.y = gridY;
								coordKey.z = gridZ + 1;
								float xPos = coordKey.x * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
								float yPos = coordKey.y * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
								float zPos = coordKey.z * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;

								vec3 chunkCenter = vec3(xPos, yPos, zPos) + vec3(Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE);
								vec3 distanceVec = chunkCenter - m_pPlayer->GetCenter();
								float lengthValue = length(distanceVec);

								if (lengthValue <= m_loaderRadius)
								{
									addChunkList.push_back(coordKey);
									numAddedChunks++;
								}
							}
						}
					}
				}
			}
		}
		updateChunkList.clear();

		// Adding chunks
		for (unsigned int i = 0; i < (int)addChunkList.size(); i++)
		{
			ChunkCoordKeys coordKey = addChunkList[i];
			Chunk* pChunk = GetChunk(coordKey.x, coordKey.y, coordKey.z);

			if (pChunk == nullptr)
			{
				CreateNewChunk(coordKey.x, coordKey.y, coordKey.z);
			}
			else
			{
				UpdateChunkNeighbours(pChunk, coordKey.x, coordKey.y, coordKey.z);
			}
		}
		addChunkList.clear();

		// Unloading chunks
		for (unsigned int i = 0; i < (int)unloadChunkList.size(); i++)
		{
			Chunk* pChunk = unloadChunkList[i];

			UnloadChunk(pChunk);
		}
		unloadChunkList.clear();

		// Check for rebuild chunks
		m_ChunkMapMutexLock.lock();
		for (it_type iterator = m_chunksMap.begin(); iterator != m_chunksMap.end(); iterator++)
		{
			Chunk* pChunk = iterator->second;

			if (pChunk != nullptr)
			{
				if (pChunk->NeedsRebuild())
				{
					rebuildChunkList.push_back(pChunk);
				}
			}
		}
		m_ChunkMapMutexLock.unlock();

		// Rebuilding chunks
		int numRebuildChunks = 0;
		int MAX_NUM_CHUNKS_REBUILD = 30;
		for (unsigned int i = 0; i < (int)rebuildChunkList.size() && numRebuildChunks < MAX_NUM_CHUNKS_REBUILD; i++)
		{
			Chunk* pChunk = rebuildChunkList[i];

			pChunk->SwitchToCachedMesh();
			pChunk->RebuildMesh();
			pChunk->CompleteMesh();
			pChunk->UndoCachedMesh();

			numRebuildChunks++;
		}
		rebuildChunkList.clear();

		if (m_stepLockEnabled == true && m_updateStepLock == false)
		{
			m_updateStepLock = true;
		}

#ifdef _WIN32
		Sleep(10);
#else
		usleep(10000);
#endif
	}

	m_updateThreadFinished = true;
}

// Rendering
void ChunkManager::Render(bool shadowRender)
{
	if (shadowRender == false)
	{
		m_numChunksRender = 0;
	}

	m_pRenderer->StartMeshRender();

	// Store cull mode
	CullMode cullMode = m_pRenderer->GetCullMode();

	if (m_wireframeRender)
	{
		m_pRenderer->SetLineWidth(1.0f);
		m_pRenderer->SetRenderMode(RM_WIREFRAME);
	}
	else
	{
		m_pRenderer->SetRenderMode(RM_SOLID);
	}

	m_pRenderer->PushMatrix();
		m_ChunkMapMutexLock.lock();
		typedef map<ChunkCoordKeys, Chunk*>::iterator it_type;
		for (it_type iterator = m_chunksMap.begin(); iterator != m_chunksMap.end(); iterator++)
		{
			Chunk* pChunk = iterator->second;

			if (pChunk != nullptr && pChunk->IsCreated() && pChunk->IsSetup() && pChunk->IsUnloading() == false && pChunk->IsEmpty() == false && pChunk->IsSurrounded() == false)
			{
				vec3 chunkCenter = pChunk->GetPosition() + vec3((Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE) - Chunk::BLOCK_RENDER_SIZE, (Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE) - Chunk::BLOCK_RENDER_SIZE, (Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE) - Chunk::BLOCK_RENDER_SIZE);

				if (shadowRender == true || m_pRenderer->SphereInFrustum(VoxGame::GetInstance()->GetDefaultViewport(), chunkCenter, Chunk::CHUNK_RADIUS))
				{
					// Fog
					if (VoxGame::GetInstance()->GetGameMode() != GameMode_FrontEnd)
					{
						vec3 chunkCenter = pChunk->GetPosition() + vec3((Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE) - Chunk::BLOCK_RENDER_SIZE, (Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE) - Chunk::BLOCK_RENDER_SIZE, (Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE) - Chunk::BLOCK_RENDER_SIZE);
						float toCamera = length(VoxGame::GetInstance()->GetGameCamera()->GetPosition() - chunkCenter);
						if (toCamera > GetLoaderRadius() + (Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE*5.0f))
						{
							continue;
						}
						if (toCamera > GetLoaderRadius() - Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE*3.0f)
						{
							m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
						}
					}

					pChunk->Render();

					if (shadowRender == false)
					{
						m_numChunksRender++;
					}

					m_pRenderer->DisableTransparency();
				}
			}
		}
		m_ChunkMapMutexLock.unlock();
	m_pRenderer->PopMatrix();

	// Restore cull mode
	m_pRenderer->SetCullMode(cullMode);

	m_pRenderer->EndMeshRender();
}

void ChunkManager::RenderWater()
{
	m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
	m_pRenderer->EnableMaterial(m_chunkMaterialID);

	float waterDistance = 500.0f;

	m_pRenderer->SetCullMode(CM_NOCULL);

	m_pRenderer->SetRenderMode(RM_SOLID);
	m_pRenderer->EnableImmediateMode(IM_QUADS);
		m_pRenderer->ImmediateVertex(-(float)waterDistance, m_waterHeight, -(float)waterDistance);
		m_pRenderer->ImmediateVertex(-(float)waterDistance, m_waterHeight, (float)waterDistance);
		m_pRenderer->ImmediateVertex((float)waterDistance, m_waterHeight, (float)waterDistance);
		m_pRenderer->ImmediateVertex((float)waterDistance, m_waterHeight, -(float)waterDistance);
	m_pRenderer->DisableImmediateMode();

	m_pRenderer->SetCullMode(CM_BACK);

	m_pRenderer->DisableTransparency();
}

void ChunkManager::RenderDebug()
{
	m_pRenderer->SetRenderMode(RM_SOLID);

	m_ChunkMapMutexLock.lock();
	typedef map<ChunkCoordKeys, Chunk*>::iterator it_type;
	for (it_type iterator = m_chunksMap.begin(); iterator != m_chunksMap.end(); iterator++)
	{
		Chunk* pChunk = iterator->second;

		if (pChunk != nullptr && pChunk->IsCreated())
		{
			pChunk->RenderDebug();
		}
	}
	m_ChunkMapMutexLock.unlock();
}

void ChunkManager::Render2D(Camera* pCamera, unsigned int viewport, unsigned int font)
{
	m_ChunkMapMutexLock.lock();
	typedef map<ChunkCoordKeys, Chunk*>::iterator it_type;
	for (it_type iterator = m_chunksMap.begin(); iterator != m_chunksMap.end(); iterator++)
	{
		Chunk* pChunk = iterator->second;

		if (pChunk != nullptr && pChunk->IsCreated())
		{
			pChunk->Render2D(pCamera, viewport, font);
		}
	}
	m_ChunkMapMutexLock.unlock();
}
