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

#include <algorithm>

#include "Chunk.h"
#include "ChunkManager.h"
#include "BiomeManager.h"
#include "BlocksEnum.h"
#include "../Player/Player.h"
#include "../scenery/SceneryManager.h"
#include "../models/QubicleBinary.h"
#include "../utils/Random.h"
#include "../simplex/simplexnoise.h"
#include "../VoxSettings.h"
#include "../VoxGame.h"
#include "../Items/ItemManager.h"

// A chunk cube is double this render size, since we create from - to + for each axis.
const float Chunk::BLOCK_RENDER_SIZE = 0.5f;
// The chunk radius is an approximation of a sphere that will enclose totally our cuboid. (Used for culling)
const float Chunk::CHUNK_RADIUS = sqrt(((CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f)*(CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f))*2.0f) / 2.0f + ((Chunk::BLOCK_RENDER_SIZE*2.0f)*2.0f);


Chunk::Chunk(Renderer* pRenderer, ChunkManager* pChunkManager, VoxSettings* pVoxSettings)
{
	m_pRenderer = pRenderer;
	m_pChunkManager = pChunkManager;
	m_pPlayer = NULL;
	m_pVoxSettings = pVoxSettings;

	Initialize();
}

Chunk::~Chunk()
{
	Unload();

	delete m_colour;
	delete m_blockType;
}

// Player pointer
void Chunk::SetPlayer(Player* pPlayer)
{
	m_pPlayer = pPlayer;
}

// Scenery manager pointer
void Chunk::SetSceneryManager(SceneryManager* pSceneryManager)
{
	m_pSceneryManager = pSceneryManager;
}

// Biome manager
void Chunk::SetBiomeManager(BiomeManager* pBiomeManager)
{
	m_pBiomeManager = pBiomeManager;
}

// Initialize
void Chunk::Initialize()
{
	// Neighbours
	m_numNeighbours = 0;
	m_pxMinus = NULL;
	m_pxPlus = NULL;
	m_pyMinus = NULL;
	m_pyPlus = NULL;
	m_pzMinus = NULL;
	m_pzPlus = NULL;

	// Flag for change during a batch update
	m_chunkChangedDuringBatchUpdate = false;

	// Grid
	m_gridX = 0;
	m_gridY = 0;
	m_gridZ = 0;

	// Flags
	m_emptyChunk = false;
	m_surroundedChunk = false;
	m_x_minus_full = false;
	m_x_plus_full = false;
	m_y_minus_full = false;
	m_y_plus_full = false;
	m_z_minus_full = false;
	m_z_plus_full = false;

	// Setup and creation
	m_created = false;
	m_setup = false;
	m_isUnloading = false;
	m_rebuild = false;
	m_rebuildNeighours = false;
	m_isRebuildingMesh = false;
	m_deleteCachedMesh = false;

	// Counters
	m_numRebuilds = 0;

	// Mesh
	m_pMesh = NULL;
	m_pCachedMesh = NULL;

	// Blocks data
	m_colour = new unsigned int[CHUNK_SIZE_CUBED];
	m_blockType = new BlockType[CHUNK_SIZE_CUBED];
	for (int i = 0; i < CHUNK_SIZE_CUBED; i++)
	{
		m_colour[i] = 0;
		m_blockType[i] = BlockType_Default;
	}
}

// Creation and destruction
void Chunk::SetCreated(bool created)
{
	m_created = created;
}

bool Chunk::IsCreated()
{
	return m_created;
}

void Chunk::Unload()
{
	m_isUnloading = true;

	if (m_pMesh != NULL)
	{
		m_pRenderer->ClearMesh(m_pMesh);
		m_pMesh = NULL;
	}

	if (m_setup == true)
	{
		// If we are already setup, when we unload, also tell our neighbours to update their flags
		Chunk* pChunkXMinus = m_pChunkManager->GetChunk(m_gridX - 1, m_gridY, m_gridZ);
		Chunk* pChunkXPlus = m_pChunkManager->GetChunk(m_gridX + 1, m_gridY, m_gridZ);
		Chunk* pChunkYMinus = m_pChunkManager->GetChunk(m_gridX, m_gridY - 1, m_gridZ);
		Chunk* pChunkYPlus = m_pChunkManager->GetChunk(m_gridX, m_gridY + 1, m_gridZ);
		Chunk* pChunkZMinus = m_pChunkManager->GetChunk(m_gridX, m_gridY, m_gridZ - 1);
		Chunk* pChunkZPlus = m_pChunkManager->GetChunk(m_gridX, m_gridY, m_gridZ + 1);

		if (pChunkXMinus != NULL && pChunkXMinus->IsSetup() == true)
			pChunkXMinus->UpdateSurroundedFlag();
		if (pChunkXPlus != NULL && pChunkXPlus->IsSetup() == true)
			pChunkXPlus->UpdateSurroundedFlag();
		if (pChunkYMinus != NULL && pChunkYMinus->IsSetup() == true)
			pChunkYMinus->UpdateSurroundedFlag();
		if (pChunkYPlus != NULL && pChunkYPlus->IsSetup() == true)
			pChunkYPlus->UpdateSurroundedFlag();
		if (pChunkZMinus != NULL && pChunkZMinus->IsSetup() == true)
			pChunkZMinus->UpdateSurroundedFlag();
		if (pChunkZPlus != NULL && pChunkZPlus->IsSetup() == true)
			pChunkZPlus->UpdateSurroundedFlag();
	}

	RemoveItems();
}

void Chunk::Setup()
{
	ChunkStorageLoader* pChunkStorage = m_pChunkManager->GetChunkStorage(m_gridX, m_gridY, m_gridZ, false);

	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			float xPosition = m_position.x + x;
			float zPosition = m_position.z + z;

			Biome biome = VoxGame::GetInstance()->GetBiomeManager()->GetBiome(vec3(xPosition, 0.0f, zPosition));

			// Get the 
			float noise = octave_noise_2d(m_pVoxSettings->m_landscapeOctaves, m_pVoxSettings->m_landscapePersistence, m_pVoxSettings->m_landscapeScale, xPosition, zPosition);
			float noiseNormalized = ((noise + 1.0f) * 0.5f);
			float noiseHeight = noiseNormalized * CHUNK_SIZE;

			// Multiple by mountain ratio
			float mountainNoise = octave_noise_2d(m_pVoxSettings->m_mountainOctaves, m_pVoxSettings->m_mountainPersistence, m_pVoxSettings->m_mountainScale, xPosition, zPosition);
			float mountainNoiseNormalise = (mountainNoise + 1.0f) * 0.5f;
			float mountainMultiplier = m_pVoxSettings->m_mountainMultiplier * mountainNoiseNormalise;
			noiseHeight *= mountainMultiplier;

			// Smooth out for towns
			float townMultiplier = VoxGame::GetInstance()->GetBiomeManager()->GetTowMultiplier(vec3(xPosition, 0.0f, zPosition));
			noiseHeight *= townMultiplier;

			if (m_gridY < 0)
			{
				noiseHeight = CHUNK_SIZE;
			}

			for (int y = 0; y < CHUNK_SIZE; y++)
			{
				float yPosition = m_position.y + y;

				if (pChunkStorage != NULL && pChunkStorage->m_blockSet[x][y][z] == true)
				{
					SetColour(x, y, z, pChunkStorage->m_colour[x][y][z]);
				}
				else
				{
					if (y + (m_gridY*CHUNK_SIZE) < noiseHeight)
					{
						float colorNoise = octave_noise_3d(4.0f, 0.3f, 0.005f, xPosition, yPosition, zPosition);
						float colorNoiseNormalized = ((colorNoise + 1.0f) * 0.5f);

						float red = 0.65f;
						float green = 0.80f;
						float blue = 0.00f;
						float alpha = 1.0f;
						BlockType blockType = BlockType_Default;

						m_pBiomeManager->GetChunkColourAndBlockType(xPosition, yPosition, zPosition, noise, colorNoiseNormalized, &red, &green, &blue, &blockType);
						
						SetColour(x, y, z, red, green, blue, alpha);
						SetBlockType(x, y, z, blockType);
					}
				}
			}

			// Tree generation
			if (m_gridY >= 0) // Only above ground
			{
				// Trees
				if ((GetRandomNumber(0, 2000) >= 2000))
				{
					float minTreeHeight = 0.0f;
					if (biome == Biome_GrassLand)
					{
						minTreeHeight = 0.5f;
					}
					else if (biome == Biome_Desert)
					{
						minTreeHeight = 0.0f;
					}
					else if (biome == Biome_AshLand)
					{
						minTreeHeight = 0.25f;
					}

					if (noiseNormalized >= minTreeHeight)
					{
						vec3 treePos = vec3(xPosition, noiseHeight, zPosition);

						if (biome == Biome_GrassLand)
						{
							m_pChunkManager->ImportQubicleBinary("media/gamedata/terrain/plains/smalltree.qb", treePos, QubicleImportDirection_Normal);
						}
						else if (biome == Biome_Desert)
						{
							m_pChunkManager->ImportQubicleBinary("media/gamedata/terrain/desert/cactus1.qb", treePos, QubicleImportDirection_Normal);
						}
						else if (biome == Biome_Tundra)
						{
							m_pChunkManager->ImportQubicleBinary("media/gamedata/terrain/tundra/tundra_tree1.qb", treePos, QubicleImportDirection_Normal);
						}
						else if (biome == Biome_AshLand)
						{
							m_pChunkManager->ImportQubicleBinary("media/gamedata/terrain/ashlands/ashtree1.qb", treePos, QubicleImportDirection_Normal);
						}
					}
				}

				// Scenery
				// TODO : Create scenery using poisson disc and also using instance manager.
				//if ((GetRandomNumber(0, 1000) >= 995))
				//{
				//	if (noiseNormalized >= 0.5f)
				//	{
				//		vec3 pos = vec3(xPosition, noiseHeight, zPosition);
				//		m_pSceneryManager->AddSceneryObject("flower", "media/gamedata/terrain/plains/flower1.qb", pos, vec3(0.0f, 0.0f, 0.0f), QubicleImportDirection_Normal, QubicleImportDirection_Normal, 0.08f, GetRandomNumber(0, 360, 2));
				//	}
				//}
			}
		}
	}

	// Remove the chunk storage loader since we no longer need it
	if (pChunkStorage != NULL)
	{
		m_pChunkManager->RemoveChunkStorageLoader(pChunkStorage);
	}

	m_setup = true;

	SetNeedsRebuild(true, true);
}

bool Chunk::IsSetup()
{
	return m_setup;
}

bool Chunk::IsUnloading()
{
	return m_isUnloading;
}

// Saving and loading
void Chunk::SaveChunk()
{
	// TODO : Write Chunk::SaveChunk()
}

void Chunk::LoadChunk()
{
	// TODO : Write Chunk::LoadChunk()
}

// Position
void Chunk::SetPosition(vec3 pos)
{
	m_position = pos;
}

vec3 Chunk::GetPosition()
{
	return m_position;
}

// Neighbours
int Chunk::GetNumNeighbours()
{
	return m_numNeighbours;
}

void Chunk::SetNumNeighbours(int neighbours)
{
	m_numNeighbours = neighbours;
}

Chunk* Chunk::GetxMinus()
{
	return m_pxMinus;
}

Chunk* Chunk::GetxPlus()
{
	return m_pxPlus;
}

Chunk* Chunk::GetyMinus()
{
	return m_pyMinus;
}

Chunk* Chunk::GetyPlus()
{
	return m_pyPlus;
}

Chunk* Chunk::GetzMinus()
{
	return m_pzMinus;
}

Chunk* Chunk::GetzPlus()
{
	return m_pzPlus;
}

void Chunk::SetxMinus(Chunk* pChunk)
{
	m_pxMinus = pChunk;
}

void Chunk::SetxPlus(Chunk* pChunk)
{
	m_pxPlus = pChunk;
}

void Chunk::SetyMinus(Chunk* pChunk)
{
	m_pyMinus = pChunk;
}

void Chunk::SetyPlus(Chunk* pChunk)
{
	m_pyPlus = pChunk;
}

void Chunk::SetzMinus(Chunk* pChunk)
{
	m_pzMinus = pChunk;
}

void Chunk::SetzPlus(Chunk* pChunk)
{
	m_pzPlus = pChunk;
}

// Grid
void Chunk::SetGrid(int x, int y, int z)
{
	m_gridX = x;
	m_gridY = y;
	m_gridZ = z;
}

int Chunk::GetGridX() const
{
	return m_gridX;
}

int Chunk::GetGridY() const
{
	return m_gridY;
}

int Chunk::GetGridZ() const
{
	return m_gridZ;
}

// Batch update
void Chunk::StartBatchUpdate()
{
	m_chunkChangedDuringBatchUpdate = false;
}

void Chunk::StopBatchUpdate()
{
	if (m_chunkChangedDuringBatchUpdate)
	{
		SetNeedsRebuild(true, true);
	}
}

// Active
bool Chunk::GetActive(int x, int y, int z)
{
	unsigned colour = m_colour[x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQUARED];
	unsigned int alpha = (colour & 0xFF000000) >> 24;
	unsigned int blue = (colour & 0x00FF0000) >> 16;
	unsigned int green = (colour & 0x0000FF00) >> 8;
	unsigned int red = (colour & 0x000000FF);

	if (alpha == 0)
	{
		return false;
	}

	return true;
}

// Inside chunk
bool Chunk::IsInsideChunk(vec3 pos)
{
	if ((pos.x < m_position.x - BLOCK_RENDER_SIZE) || (pos.x - m_position.x > CHUNK_SIZE * (BLOCK_RENDER_SIZE*2.0f) - BLOCK_RENDER_SIZE))
		return false;

	if ((pos.y < m_position.y - BLOCK_RENDER_SIZE) || (pos.y - m_position.y > CHUNK_SIZE * (BLOCK_RENDER_SIZE*2.0f) - BLOCK_RENDER_SIZE))
		return false;

	if ((pos.z < m_position.z - BLOCK_RENDER_SIZE) || (pos.z - m_position.z > CHUNK_SIZE * (BLOCK_RENDER_SIZE*2.0f) - BLOCK_RENDER_SIZE))
		return false;

	return true;
}

// Items
void Chunk::AddItem(Item* pItem)
{
	m_itemMutexLock.lock();
	m_vpItemList.push_back(pItem);
	m_itemMutexLock.unlock();
}

void Chunk::RemoveItem(Item* pItem)
{
	m_itemMutexLock.lock();
	vector<Item*>::iterator iter = std::find(m_vpItemList.begin(), m_vpItemList.end(), pItem);
	if (iter != m_vpItemList.end())
	{
		m_vpItemList.erase(iter);
	}
	m_itemMutexLock.unlock();
}

void Chunk::RemoveItems()
{
	// Delete the chunk items data
	m_itemMutexLock.lock();
	for (unsigned int i = 0; i < m_vpItemList.size(); i++)
	{
		m_vpItemList[i]->SetChunk(NULL);
		m_vpItemList[i]->SetErase(true);
	}
	m_vpItemList.clear();
	m_itemMutexLock.unlock();
}

// Block colour
void Chunk::SetColour(int x, int y, int z, float r, float g, float b, float a, bool setBlockType)
{
	if (r > 1.0f) r = 1.0f;
	if (g > 1.0f) g = 1.0f;
	if (b > 1.0f) b = 1.0f;
	if (r < 0.0f) r = 0.0f;
	if (g < 0.0f) g = 0.0f;
	if (b < 0.0f) b = 0.0f;

	unsigned int alpha = (int)(a * 255) << 24;
	unsigned int blue = (int)(b * 255) << 16;
	unsigned int green = (int)(g * 255) << 8;
	unsigned int red = (int)(r * 255);

	unsigned int colour = red + green + blue + alpha;
	SetColour(x, y, z, colour, setBlockType);
}

void Chunk::GetColour(int x, int y, int z, float* r, float* g, float* b, float* a)
{
	if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE)
		return;

	unsigned int colour = m_colour[x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQUARED];
	unsigned int alpha = (colour & 0xFF000000) >> 24;
	unsigned int blue = (colour & 0x00FF0000) >> 16;
	unsigned int green = (colour & 0x0000FF00) >> 8;
	unsigned int red = (colour & 0x000000FF);

	*r = (float)(red / 255.0f);
	*g = (float)(green / 255.0f);
	*b = (float)(blue / 255.0f);
	*a = 1.0f;
}

void Chunk::SetColour(int x, int y, int z, unsigned int colour, bool setBlockType)
{
	if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE)
		return;

	bool changed = ((m_colour[x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQUARED] == colour) == false);

	if (changed)
	{
		m_chunkChangedDuringBatchUpdate = true;
	}

	m_colour[x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQUARED] = colour;

	if (setBlockType)
	{
		unsigned int blockB = (colour & 0x00FF0000) >> 16;
		unsigned int blockG = (colour & 0x0000FF00) >> 8;
		unsigned int blockR = (colour & 0x000000FF);
		m_blockType[x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQUARED] = m_pChunkManager->SetBlockTypeBasedOnColour(blockR, blockG, blockB);
	}
}

unsigned int Chunk::GetColour(int x, int y, int z)
{
	return m_colour[x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQUARED];
}

// Block type
BlockType Chunk::GetBlockType(int x, int y, int z)
{
	return m_blockType[x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQUARED];
}

void Chunk::SetBlockType(int x, int y, int z, BlockType blockType)
{
	m_blockType[x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQUARED] = blockType;
}

// Flags
bool Chunk::IsEmpty()
{
	return m_emptyChunk;
}

bool Chunk::IsSurrounded()
{
	return m_surroundedChunk;
}

void Chunk::UpdateWallFlags()
{
	// Figure out if we have any full walls(sides) and are a completely surrounded chunk
	int x_minus = 0;
	int x_plus = 0;
	int y_minus = 0;
	int y_plus = 0;
	int z_minus = 0;
	int z_plus = 0;

	for (int y = 0; y < CHUNK_SIZE; y++)
	{
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			if (GetActive(0, y, z) == true)
			{
				x_minus++;
			}

			if (GetActive(CHUNK_SIZE - 1, y, z) == true)
			{
				x_plus++;
			}
		}
	}

	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int z = 0; z < CHUNK_SIZE; z++)
		{
			if (GetActive(x, 0, z) == true)
			{
				y_minus++;
			}

			if (GetActive(x, CHUNK_SIZE - 1, z) == true)
			{
				y_plus++;
			}
		}
	}

	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			if (GetActive(x, y, 0) == true)
			{
				z_minus++;
			}

			if (GetActive(x, y, CHUNK_SIZE - 1) == true)
			{
				z_plus++;
			}
		}
	}

	// Reset the wall flags first
	m_x_minus_full = false;
	m_x_plus_full = false;
	m_y_minus_full = false;
	m_y_plus_full = false;
	m_z_minus_full = false;
	m_z_plus_full = false;

	// Set the flags to show if we have any sides completely full
	int wallsize = CHUNK_SIZE*CHUNK_SIZE;
	if (x_minus == wallsize)
		m_x_minus_full = true;

	if (x_plus == wallsize)
		m_x_plus_full = true;

	if (y_minus == wallsize)
		m_y_minus_full = true;

	if (y_plus == wallsize)
		m_y_plus_full = true;

	if (z_minus == wallsize)
		m_z_minus_full = true;

	if (z_plus == wallsize)
		m_z_plus_full = true;
}

bool Chunk::UpdateSurroundedFlag()
{
	if (m_pChunkManager == NULL)
	{
		return false;
	}

	Chunk* pChunkXMinus = m_pChunkManager->GetChunk(m_gridX - 1, m_gridY, m_gridZ);
	Chunk* pChunkXPlus = m_pChunkManager->GetChunk(m_gridX + 1, m_gridY, m_gridZ);
	Chunk* pChunkYMinus = m_pChunkManager->GetChunk(m_gridX, m_gridY - 1, m_gridZ);
	Chunk* pChunkYPlus = m_pChunkManager->GetChunk(m_gridX, m_gridY + 1, m_gridZ);
	Chunk* pChunkZMinus = m_pChunkManager->GetChunk(m_gridX, m_gridY, m_gridZ - 1);
	Chunk* pChunkZPlus = m_pChunkManager->GetChunk(m_gridX, m_gridY, m_gridZ + 1);

	// Check our neighbor chunks
	if (pChunkXMinus != NULL && pChunkXMinus->IsSetup() == true && pChunkXMinus->m_x_plus_full &&
	    pChunkXPlus != NULL && pChunkXPlus->IsSetup() == true && pChunkXPlus->m_x_minus_full &&
	    pChunkYMinus != NULL && pChunkYMinus->IsSetup() == true && pChunkYMinus->m_y_plus_full &&
	    pChunkYPlus != NULL && pChunkYPlus->IsSetup() == true && pChunkYPlus->m_y_minus_full &&
	    pChunkZMinus != NULL && pChunkZMinus->IsSetup() == true && pChunkZMinus->m_z_plus_full &&
	    pChunkZPlus != NULL && pChunkZPlus->IsSetup() == true && pChunkZPlus->m_z_minus_full)
	{
		m_surroundedChunk = true;
	}
	else
	{
		m_surroundedChunk = false;
	}

	return true;
}

void Chunk::UpdateEmptyFlag()
{
	// Figure out if we are a completely empty chunk
	int numVerts;
	int numTriangles;
	m_pRenderer->GetMeshInformation(&numVerts, &numTriangles, m_pMesh);

	if (numVerts == 0 && numTriangles == 0)
	{
		m_emptyChunk = true;
	}
	else
	{
		m_emptyChunk = false;
	}
}

// Create mesh
void Chunk::CreateMesh()
{
	if (m_pMesh == NULL)
	{
		m_pMesh = m_pRenderer->CreateMesh(OGLMeshType_Textured);
	}

	int *l_merged;
	l_merged = new int[CHUNK_SIZE_CUBED];

	for (unsigned int j = 0; j < CHUNK_SIZE_CUBED; j++)
	{
		l_merged[j] = MergedSide_None;
	}

	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
	float a = 1.0f;

	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				if (GetActive(x, y, z) == false)
				{
					continue;
				}
				else
				{
					GetColour(x, y, z, &r, &g, &b, &a);

					a = 1.0f;

					vec3 p1(x - BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					vec3 p2(x + BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					vec3 p3(x + BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					vec3 p4(x - BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					vec3 p5(x + BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					vec3 p6(x - BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					vec3 p7(x - BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					vec3 p8(x + BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);

					vec3 n1;
					unsigned int v1, v2, v3, v4;
					unsigned int t1, t2, t3, t4;

					bool doXPositive = (IsMergedXPositive(l_merged, x, y, z, CHUNK_SIZE, CHUNK_SIZE) == false);
					bool doXNegative = (IsMergedXNegative(l_merged, x, y, z, CHUNK_SIZE, CHUNK_SIZE) == false);
					bool doYPositive = (IsMergedYPositive(l_merged, x, y, z, CHUNK_SIZE, CHUNK_SIZE) == false);
					bool doYNegative = (IsMergedYNegative(l_merged, x, y, z, CHUNK_SIZE, CHUNK_SIZE) == false);
					bool doZPositive = (IsMergedZPositive(l_merged, x, y, z, CHUNK_SIZE, CHUNK_SIZE) == false);
					bool doZNegative = (IsMergedZNegative(l_merged, x, y, z, CHUNK_SIZE, CHUNK_SIZE) == false);

					// Front
					if (doZPositive && ((z == CHUNK_SIZE - 1) || z < CHUNK_SIZE - 1 && GetActive(x, y, z + 1) == false))
					{
						bool addSide = true;

						if ((z == CHUNK_SIZE - 1))
						{
							Chunk* pChunk = m_pChunkManager->GetChunk(m_gridX, m_gridY, m_gridZ + 1);
							if (pChunk == NULL || pChunk->IsSetup())
							{
								addSide = pChunk != NULL && (pChunk->GetActive(x, y, 0) == false);
							}
						}

						if (addSide)
						{
							int endX = (x / CHUNK_SIZE) * CHUNK_SIZE + CHUNK_SIZE;
							int endY = (y / CHUNK_SIZE) * CHUNK_SIZE + CHUNK_SIZE;

							if (m_pChunkManager->GetFaceMerging())
							{
								UpdateMergedSide(l_merged, x, y, z, CHUNK_SIZE, CHUNK_SIZE, &p1, &p2, &p3, &p4, x, y, endX, endY, true, true, false, false);
							}

							n1 = vec3(0.0f, 0.0f, 1.0f);
							v1 = m_pRenderer->AddVertexToMesh(p1, n1, r, g, b, a, m_pMesh);
							t1 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 0.0f, m_pMesh);
							v2 = m_pRenderer->AddVertexToMesh(p2, n1, r, g, b, a, m_pMesh);
							t2 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 0.0f, m_pMesh);
							v3 = m_pRenderer->AddVertexToMesh(p3, n1, r, g, b, a, m_pMesh);
							t3 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 1.0f, m_pMesh);
							v4 = m_pRenderer->AddVertexToMesh(p4, n1, r, g, b, a, m_pMesh);
							t4 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 1.0f, m_pMesh);

							m_pRenderer->AddTriangleToMesh(v1, v2, v3, m_pMesh);
							m_pRenderer->AddTriangleToMesh(v1, v3, v4, m_pMesh);
						}
					}

					p1 = vec3(x - BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p2 = vec3(x + BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p3 = vec3(x + BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p4 = vec3(x - BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p5 = vec3(x + BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					p6 = vec3(x - BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					p7 = vec3(x - BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					p8 = vec3(x + BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);

					// Back
					if (doZNegative && ((z == 0) || (z > 0 && GetActive(x, y, z - 1) == false)))
					{
						bool addSide = true;

						if ((z == 0))
						{
							Chunk* pChunk = m_pChunkManager->GetChunk(m_gridX, m_gridY, m_gridZ - 1);
							if (pChunk == NULL || pChunk->IsSetup())
							{
								addSide = pChunk != NULL && (pChunk->GetActive(x, y, CHUNK_SIZE - 1) == false);
							}
						}

						if (addSide)
						{
							int endX = (x / CHUNK_SIZE) * CHUNK_SIZE + CHUNK_SIZE;
							int endY = (y / CHUNK_SIZE) * CHUNK_SIZE + CHUNK_SIZE;

							if (m_pChunkManager->GetFaceMerging())
							{
								UpdateMergedSide(l_merged, x, y, z, CHUNK_SIZE, CHUNK_SIZE, &p6, &p5, &p8, &p7, x, y, endX, endY, false, true, false, false);
							}

							n1 = vec3(0.0f, 0.0f, -1.0f);
							v1 = m_pRenderer->AddVertexToMesh(p5, n1, r, g, b, a, m_pMesh);
							t1 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 0.0f, m_pMesh);
							v2 = m_pRenderer->AddVertexToMesh(p6, n1, r, g, b, a, m_pMesh);
							t2 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 0.0f, m_pMesh);
							v3 = m_pRenderer->AddVertexToMesh(p7, n1, r, g, b, a, m_pMesh);
							t3 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 1.0f, m_pMesh);
							v4 = m_pRenderer->AddVertexToMesh(p8, n1, r, g, b, a, m_pMesh);
							t4 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 1.0f, m_pMesh);

							m_pRenderer->AddTriangleToMesh(v1, v2, v3, m_pMesh);
							m_pRenderer->AddTriangleToMesh(v1, v3, v4, m_pMesh);
						}
					}

					p1 = vec3(x - BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p2 = vec3(x + BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p3 = vec3(x + BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p4 = vec3(x - BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p5 = vec3(x + BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					p6 = vec3(x - BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					p7 = vec3(x - BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					p8 = vec3(x + BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);

					// Right
					if (doXPositive && ((x == CHUNK_SIZE - 1) || (x < CHUNK_SIZE - 1 && GetActive(x + 1, y, z) == false)))
					{
						bool addSide = true;

						if ((x == CHUNK_SIZE - 1))
						{
							Chunk* pChunk = m_pChunkManager->GetChunk(m_gridX + 1, m_gridY, m_gridZ);
							if (pChunk == NULL || pChunk->IsSetup())
							{
								addSide = pChunk != NULL && (pChunk->GetActive(0, y, z) == false);
							}
						}

						if (addSide)
						{
							int endX = (z / CHUNK_SIZE) * CHUNK_SIZE + CHUNK_SIZE;
							int endY = (y / CHUNK_SIZE) * CHUNK_SIZE + CHUNK_SIZE;

							if (m_pChunkManager->GetFaceMerging())
							{
								UpdateMergedSide(l_merged, x, y, z, CHUNK_SIZE, CHUNK_SIZE, &p5, &p2, &p3, &p8, z, y, endX, endY, true, false, true, false);
							}

							n1 = vec3(1.0f, 0.0f, 0.0f);
							v1 = m_pRenderer->AddVertexToMesh(p2, n1, r, g, b, a, m_pMesh);
							t1 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 0.0f, m_pMesh);
							v2 = m_pRenderer->AddVertexToMesh(p5, n1, r, g, b, a, m_pMesh);
							t2 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 0.0f, m_pMesh);
							v3 = m_pRenderer->AddVertexToMesh(p8, n1, r, g, b, a, m_pMesh);
							t3 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 1.0f, m_pMesh);
							v4 = m_pRenderer->AddVertexToMesh(p3, n1, r, g, b, a, m_pMesh);
							t4 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 1.0f, m_pMesh);

							m_pRenderer->AddTriangleToMesh(v1, v2, v3, m_pMesh);
							m_pRenderer->AddTriangleToMesh(v1, v3, v4, m_pMesh);
						}
					}

					p1 = vec3(x - BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p2 = vec3(x + BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p3 = vec3(x + BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p4 = vec3(x - BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p5 = vec3(x + BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					p6 = vec3(x - BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					p7 = vec3(x - BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					p8 = vec3(x + BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);

					// Left
					if (doXNegative && ((x == 0) || (x > 0 && GetActive(x - 1, y, z) == false)))
					{
						bool addSide = true;

						if ((x == 0))
						{
							Chunk* pChunk = m_pChunkManager->GetChunk(m_gridX - 1, m_gridY, m_gridZ);
							if (pChunk == NULL || pChunk->IsSetup())
							{
								addSide = pChunk != NULL && (pChunk->GetActive(CHUNK_SIZE - 1, y, z) == false);
							}
						}

						if (addSide)
						{
							int endX = (z / CHUNK_SIZE) * CHUNK_SIZE + CHUNK_SIZE;
							int endY = (y / CHUNK_SIZE) * CHUNK_SIZE + CHUNK_SIZE;

							if (m_pChunkManager->GetFaceMerging())
							{
								UpdateMergedSide(l_merged, x, y, z, CHUNK_SIZE, CHUNK_SIZE, &p6, &p1, &p4, &p7, z, y, endX, endY, false, false, true, false);
							}

							n1 = vec3(-1.0f, 0.0f, 0.0f);
							v1 = m_pRenderer->AddVertexToMesh(p6, n1, r, g, b, a, m_pMesh);
							t1 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 0.0f, m_pMesh);
							v2 = m_pRenderer->AddVertexToMesh(p1, n1, r, g, b, a, m_pMesh);
							t2 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 0.0f, m_pMesh);
							v3 = m_pRenderer->AddVertexToMesh(p4, n1, r, g, b, a, m_pMesh);
							t3 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 1.0f, m_pMesh);
							v4 = m_pRenderer->AddVertexToMesh(p7, n1, r, g, b, a, m_pMesh);
							t4 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 1.0f, m_pMesh);

							m_pRenderer->AddTriangleToMesh(v1, v2, v3, m_pMesh);
							m_pRenderer->AddTriangleToMesh(v1, v3, v4, m_pMesh);
						}
					}

					p1 = vec3(x - BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p2 = vec3(x + BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p3 = vec3(x + BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p4 = vec3(x - BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p5 = vec3(x + BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					p6 = vec3(x - BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					p7 = vec3(x - BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					p8 = vec3(x + BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);

					// Top
					if (doYPositive && ((y == CHUNK_SIZE - 1) || (y < CHUNK_SIZE - 1 && GetActive(x, y + 1, z) == false)))
					{
						bool addSide = true;

						if ((y == CHUNK_SIZE - 1))
						{
							Chunk* pChunk = m_pChunkManager->GetChunk(m_gridX, m_gridY + 1, m_gridZ);
							if (pChunk == NULL || pChunk->IsSetup())
							{
								addSide = pChunk != NULL && (pChunk->GetActive(x, 0, z) == false);
							}
						}

						if (addSide)
						{
							int endX = (x / CHUNK_SIZE) * CHUNK_SIZE + CHUNK_SIZE;
							int endY = (z / CHUNK_SIZE) * CHUNK_SIZE + CHUNK_SIZE;

							if (m_pChunkManager->GetFaceMerging())
							{
								UpdateMergedSide(l_merged, x, y, z, CHUNK_SIZE, CHUNK_SIZE, &p7, &p8, &p3, &p4, x, z, endX, endY, true, false, false, true);
							}

							n1 = vec3(0.0f, 1.0f, 0.0f);
							v1 = m_pRenderer->AddVertexToMesh(p4, n1, r, g, b, a, m_pMesh);
							t1 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 0.0f, m_pMesh);
							v2 = m_pRenderer->AddVertexToMesh(p3, n1, r, g, b, a, m_pMesh);
							t2 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 0.0f, m_pMesh);
							v3 = m_pRenderer->AddVertexToMesh(p8, n1, r, g, b, a, m_pMesh);
							t3 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 1.0f, m_pMesh);
							v4 = m_pRenderer->AddVertexToMesh(p7, n1, r, g, b, a, m_pMesh);
							t4 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 1.0f, m_pMesh);

							m_pRenderer->AddTriangleToMesh(v1, v2, v3, m_pMesh);
							m_pRenderer->AddTriangleToMesh(v1, v3, v4, m_pMesh);
						}
					}

					p1 = vec3(x - BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p2 = vec3(x + BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p3 = vec3(x + BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p4 = vec3(x - BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z + BLOCK_RENDER_SIZE);
					p5 = vec3(x + BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					p6 = vec3(x - BLOCK_RENDER_SIZE, y - BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					p7 = vec3(x - BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);
					p8 = vec3(x + BLOCK_RENDER_SIZE, y + BLOCK_RENDER_SIZE, z - BLOCK_RENDER_SIZE);

					// Bottom
					if (doYNegative && ((y == 0) || (y > 0 && GetActive(x, y - 1, z) == false)))
					{
						bool addSide = true;

						if ((y == 0))
						{
							Chunk* pChunk = m_pChunkManager->GetChunk(m_gridX, m_gridY - 1, m_gridZ);
							if (pChunk == NULL || pChunk->IsSetup())
							{
								addSide = pChunk != NULL && (pChunk->GetActive(x, CHUNK_SIZE - 1, z) == false);
							}
						}

						if (addSide)
						{
							int endX = (x / CHUNK_SIZE) * CHUNK_SIZE + CHUNK_SIZE;
							int endY = (z / CHUNK_SIZE) * CHUNK_SIZE + CHUNK_SIZE;

							if (m_pChunkManager->GetFaceMerging())
							{
								UpdateMergedSide(l_merged, x, y, z, CHUNK_SIZE, CHUNK_SIZE, &p6, &p5, &p2, &p1, x, z, endX, endY, false, false, false, true);
							}

							n1 = vec3(0.0f, -1.0f, 0.0f);
							v1 = m_pRenderer->AddVertexToMesh(p6, n1, r, g, b, a, m_pMesh);
							t1 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 0.0f, m_pMesh);
							v2 = m_pRenderer->AddVertexToMesh(p5, n1, r, g, b, a, m_pMesh);
							t2 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 0.0f, m_pMesh);
							v3 = m_pRenderer->AddVertexToMesh(p2, n1, r, g, b, a, m_pMesh);
							t3 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 1.0f, m_pMesh);
							v4 = m_pRenderer->AddVertexToMesh(p1, n1, r, g, b, a, m_pMesh);
							t4 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 1.0f, m_pMesh);

							m_pRenderer->AddTriangleToMesh(v1, v2, v3, m_pMesh);
							m_pRenderer->AddTriangleToMesh(v1, v3, v4, m_pMesh);
						}
					}
				}
			}
		}
	}

	// Delete the merged array
	delete [] l_merged;
}

void Chunk::CompleteMesh()
{
	m_pRenderer->FinishMesh(-1, m_pChunkManager->GetChunkMaterialID(), m_pMesh);

	UpdateEmptyFlag();

	m_isRebuildingMesh = false;
}

void Chunk::UpdateMergedSide(int *merged, int blockx, int blocky, int blockz, int width, int height, vec3 *p1, vec3 *p2, vec3 *p3, vec3 *p4, int startX, int startY, int maxX, int maxY, bool positive, bool zFace, bool xFace, bool yFace)
{
	bool doMore = true;
	unsigned int incrementX = 0;
	unsigned int incrementZ = 0;
	unsigned int incrementY = 0;

	int change = 1;
	if (positive == false)
	{
		//change = -1;
	}

	if (zFace || yFace)
	{
		incrementX = 1;
		incrementY = 1;
	}
	if (xFace)
	{
		incrementZ = 1;
		incrementY = 1;
	}

	// 1st phase
	int incrementer = 1;
	while (doMore)
	{
		if (startX + incrementer >= maxX)
		{
			doMore = false;
		}
		else
		{
			bool doPhase1Merge = true;
			float r1, r2, g1, g2, b1, b2, a1, a2;
			GetColour(blockx, blocky, blockz, &r1, &g1, &b1, &a1);
			GetColour(blockx + incrementX, blocky, blockz + incrementZ, &r2, &g2, &b2, &a2);
			//if(m_pBlocks[blockx][blocky][blockz].GetBlockType() != m_pBlocks[blockx + incrementX][blocky][blockz + incrementZ].GetBlockType())
			//{
			// Don't do any phase 1 merging if we don't have the same block type.
			//	doPhase1Merge = false;
			//	doMore = false;
			//}
			/*//else*/ if ((r1 != r2 || g1 != g2 || b1 != b2 || a1 != a2) /*&& allMerge == false*/)
			{
				// Don't do any phase 1 merging if we don't have the same colour variation
				doPhase1Merge = false;
				doMore = false;
			}
			else
			{
				if ((xFace && positive && blockx + incrementX + 1 == CHUNK_SIZE) ||
					(xFace && !positive && blockx + incrementX == 0) ||
					(yFace && positive && blocky + 1 == CHUNK_SIZE) ||
					(yFace && !positive && blocky == 0) ||
					(zFace && positive && blockz + incrementZ + 1 == CHUNK_SIZE) ||
					(zFace && !positive && blockz + incrementZ == 0))
				{
					doPhase1Merge = false;
					doMore = false;
				}
				// Don't do any phase 1 merging if we find an inactive block or already merged block in our path
				else if (xFace && positive && (blockx + incrementX + 1) < CHUNK_SIZE && GetActive(blockx + incrementX + 1, blocky, blockz + incrementZ) == true)
				{
					doPhase1Merge = false;
					doMore = false;
				}
				else if (xFace && !positive && (blockx + incrementX) > 0 && GetActive(blockx + incrementX - 1, blocky, blockz + incrementZ) == true)
				{
					doPhase1Merge = false;
					doMore = false;
				}
				else if (yFace && positive && (blocky + 1) < CHUNK_SIZE && GetActive(blockx + incrementX, blocky + 1, blockz + incrementZ) == true)
				{
					doPhase1Merge = false;
					doMore = false;
				}
				else if (yFace && !positive && blocky > 0 && GetActive(blockx + incrementX, blocky - 1, blockz + incrementZ) == true)
				{
					doPhase1Merge = false;
					doMore = false;
				}
				else if (zFace && positive && (blockz + incrementZ + 1) < CHUNK_SIZE && GetActive(blockx + incrementX, blocky, blockz + incrementZ + 1) == true)
				{
					doPhase1Merge = false;
					doMore = false;
				}
				else if (zFace && !positive && (blockz + incrementZ) > 0 && GetActive(blockx + incrementX, blocky, blockz + incrementZ - 1) == true)
				{
					doPhase1Merge = false;
					doMore = false;
				}
				else if (GetActive(blockx + incrementX, blocky, blockz + incrementZ) == false)
				{
					doPhase1Merge = false;
					doMore = false;
				}
				else
				{
					if (xFace)
					{
						doPhase1Merge = positive ? (IsMergedXPositive(merged, blockx + incrementX, blocky, blockz + incrementZ, width, height) == false) : (IsMergedXNegative(merged, blockx + incrementX, blocky, blockz + incrementZ, width, height) == false);
					}
					if (zFace)
					{
						doPhase1Merge = positive ? (IsMergedZPositive(merged, blockx + incrementX, blocky, blockz + incrementZ, width, height) == false) : (IsMergedZNegative(merged, blockx + incrementX, blocky, blockz + incrementZ, width, height) == false);
					}
					if (yFace)
					{
						doPhase1Merge = positive ? (IsMergedYPositive(merged, blockx + incrementX, blocky, blockz + incrementZ, width, height) == false) : (IsMergedYNegative(merged, blockx + incrementX, blocky, blockz + incrementZ, width, height) == false);
					}
				}

				if (doPhase1Merge)
				{
					if (zFace || yFace)
					{
						(*p2).x += change * (BLOCK_RENDER_SIZE * 2.0f);
						(*p3).x += change * (BLOCK_RENDER_SIZE * 2.0f);
					}
					if (xFace)
					{
						(*p2).z += change * (BLOCK_RENDER_SIZE * 2.0f);
						(*p3).z += change * (BLOCK_RENDER_SIZE * 2.0f);
					}

					if (positive)
					{
						if (zFace)
						{
							merged[(blockx + incrementX) + blocky*width + (blockz + incrementZ)*width*height] |= MergedSide_Z_Positive;
						}
						if (xFace)
						{
							merged[(blockx + incrementX) + blocky*width + (blockz + incrementZ)*width*height] |= MergedSide_X_Positive;
						}
						if (yFace)
						{
							merged[(blockx + incrementX) + blocky*width + (blockz + incrementZ)*width*height] |= MergedSide_Y_Positive;
						}
					}
					else
					{
						if (zFace)
						{
							merged[(blockx + incrementX) + blocky*width + (blockz + incrementZ)*width*height] |= MergedSide_Z_Negative;
						}
						if (xFace)
						{
							merged[(blockx + incrementX) + blocky*width + (blockz + incrementZ)*width*height] |= MergedSide_X_Negative;
						}
						if (yFace)
						{
							merged[(blockx + incrementX) + blocky*width + (blockz + incrementZ)*width*height] |= MergedSide_Y_Negative;
						}
					}
				}
				else
				{
					doMore = false;
				}
			}
		}

		if (zFace || yFace)
		{
			incrementX += change;
		}
		if (xFace)
		{
			incrementZ += change;
		}

		incrementer += change;
	}


	// 2nd phase
	int loop = incrementer;
	incrementer = incrementY;

	doMore = true;
	while (doMore)
	{
		if (startY + incrementer >= maxY)
		{
			doMore = false;
		}
		else
		{
			for (int i = 0; i < loop - 1; i++)
			{
				// Don't do any phase 2 merging is we have any inactive blocks or already merged blocks on the row
				if (zFace)
				{
					float r1, r2, g1, g2, b1, b2, a1, a2;
					GetColour(blockx, blocky, blockz, &r1, &g1, &b1, &a1);
					GetColour(blockx + i, blocky + incrementY, blockz, &r2, &g2, &b2, &a2);

					if (positive && (blockz + 1) < CHUNK_SIZE && GetActive(blockx + i, blocky + incrementY, blockz + 1) == true)
					{
						doMore = false;
					}
					else if (!positive && blockz > 0 && GetActive(blockx + i, blocky + incrementY, blockz - 1) == true)
					{
						doMore = false;
					}
					else if (GetActive(blockx + i, blocky + incrementY, blockz) == false || (positive ? (IsMergedZPositive(merged, blockx + i, blocky + incrementY, blockz, width, height) == true) : (IsMergedZNegative(merged, blockx + i, blocky + incrementY, blockz, width, height) == true)))
					{
						// Failed active or already merged check
						doMore = false;
					}
					/*else if(m_pBlocks[blockx][blocky][blockz].GetBlockType() != m_pBlocks[blockx + i][blocky + incrementY][blockz].GetBlockType())
					{
					// Failed block type check
					doMore = false;
					}
					*/
					else if ((r1 != r2 || g1 != g2 || b1 != b2 || a1 != a2) /*&& allMerge == false*/)
					{
						// Failed colour check
						doMore = false;
					}
				}
				if (xFace)
				{
					float r1, r2, g1, g2, b1, b2, a1, a2;
					GetColour(blockx, blocky, blockz, &r1, &g1, &b1, &a1);
					GetColour(blockx, blocky + incrementY, blockz + i, &r2, &g2, &b2, &a2);

					if (positive && (blockx + 1) < CHUNK_SIZE && GetActive(blockx + 1, blocky + incrementY, blockz + i) == true)
					{
						doMore = false;
					}
					else if (!positive && (blockx) > 0 && GetActive(blockx - 1, blocky + incrementY, blockz + i) == true)
					{
						doMore = false;
					}
					else if (GetActive(blockx, blocky + incrementY, blockz + i) == false || (positive ? (IsMergedXPositive(merged, blockx, blocky + incrementY, blockz + i, width, height) == true) : (IsMergedXNegative(merged, blockx, blocky + incrementY, blockz + i, width, height) == true)))
					{
						// Failed active or already merged check
						doMore = false;
					}
					/*else if(m_pBlocks[blockx][blocky][blockz].GetBlockType() != m_pBlocks[blockx][blocky + incrementY][blockz + i].GetBlockType())
					{
					// Failed block type check
					doMore = false;
					}
					*/
					else if ((r1 != r2 || g1 != g2 || b1 != b2 || a1 != a2) /*&& allMerge == false*/)
					{
						// Failed colour check
						doMore = false;
					}
				}
				if (yFace)
				{
					float r1, r2, g1, g2, b1, b2, a1, a2;
					GetColour(blockx, blocky, blockz, &r1, &g1, &b1, &a1);
					GetColour(blockx + i, blocky, blockz + incrementY, &r2, &g2, &b2, &a2);

					if (positive && (blocky + 1) < CHUNK_SIZE && GetActive(blockx + i, blocky + 1, blockz + incrementY) == true)
					{
						doMore = false;
					}
					else if (!positive && blocky > 0 && GetActive(blockx + i, blocky - 1, blockz + incrementY) == true)
					{
						doMore = false;
					}
					else if (GetActive(blockx + i, blocky, blockz + incrementY) == false || (positive ? (IsMergedYPositive(merged, blockx + i, blocky, blockz + incrementY, width, height) == true) : (IsMergedYNegative(merged, blockx + i, blocky, blockz + incrementY, width, height) == true)))
					{
						// Failed active or already merged check
						doMore = false;
					}
					/*else if(m_pBlocks[blockx][blocky][blockz].GetBlockType() != m_pBlocks[blockx + i][blocky][blockz + incrementY].GetBlockType())
					{
					// Failed block type check
					doMore = false;
					}
					*/
					else if ((r1 != r2 || g1 != g2 || b1 != b2 || a1 != a2) /*&& allMerge == false*/)
					{
						// Failed colour check
						doMore = false;
					}
				}
			}

			if (doMore == true)
			{
				if (zFace || xFace)
				{
					(*p3).y += change * (BLOCK_RENDER_SIZE * 2.0f);
					(*p4).y += change * (BLOCK_RENDER_SIZE * 2.0f);
				}
				if (yFace)
				{
					(*p3).z += change * (BLOCK_RENDER_SIZE * 2.0f);
					(*p4).z += change * (BLOCK_RENDER_SIZE * 2.0f);
				}

				for (int i = 0; i < loop - 1; i++)
				{
					if (positive)
					{
						if (zFace)
						{
							merged[(blockx + i) + (blocky + incrementY)*width + blockz*width*height] |= MergedSide_Z_Positive;
						}
						if (xFace)
						{
							merged[blockx + (blocky + incrementY)*width + (blockz + i)*width*height] |= MergedSide_X_Positive;
						}
						if (yFace)
						{
							merged[(blockx + i) + blocky*width + (blockz + incrementY)*width*height] |= MergedSide_Y_Positive;
						}
					}
					else
					{
						if (zFace)
						{
							merged[(blockx + i) + (blocky + incrementY)*width + blockz*width*height] |= MergedSide_Z_Negative;
						}
						if (xFace)
						{
							merged[blockx + (blocky + incrementY)*width + (blockz + i)*width*height] |= MergedSide_X_Negative;
						}
						if (yFace)
						{
							merged[(blockx + i) + blocky*width + (blockz + incrementY)*width*height] |= MergedSide_Y_Negative;
						}
					}
				}
			}
		}

		incrementY += change;
		incrementer += change;
	}
}

// Rebuild
void Chunk::RebuildMesh()
{
	m_isRebuildingMesh = true;
	if (m_pMesh != NULL)
	{
		m_pRenderer->ClearMesh(m_pMesh);
		m_pMesh = NULL;
	}

	CreateMesh();

	// Update our wall flags, so that our neighbors can check if they are surrounded
	UpdateWallFlags();
	UpdateSurroundedFlag();

	Chunk* pChunkXMinus = m_pChunkManager->GetChunk(m_gridX - 1, m_gridY, m_gridZ);
	Chunk* pChunkXPlus = m_pChunkManager->GetChunk(m_gridX + 1, m_gridY, m_gridZ);
	Chunk* pChunkYMinus = m_pChunkManager->GetChunk(m_gridX, m_gridY - 1, m_gridZ);
	Chunk* pChunkYPlus = m_pChunkManager->GetChunk(m_gridX, m_gridY + 1, m_gridZ);
	Chunk* pChunkZMinus = m_pChunkManager->GetChunk(m_gridX, m_gridY, m_gridZ - 1);
	Chunk* pChunkZPlus = m_pChunkManager->GetChunk(m_gridX, m_gridY, m_gridZ + 1);

	if (pChunkXMinus != NULL && pChunkXMinus->IsSetup() == true)
		pChunkXMinus->UpdateSurroundedFlag();
	if (pChunkXPlus != NULL && pChunkXPlus->IsSetup() == true)
		pChunkXPlus->UpdateSurroundedFlag();
	if (pChunkYMinus != NULL && pChunkYMinus->IsSetup() == true)
		pChunkYMinus->UpdateSurroundedFlag();
	if (pChunkYPlus != NULL && pChunkYPlus->IsSetup() == true)
		pChunkYPlus->UpdateSurroundedFlag();
	if (pChunkZMinus != NULL && pChunkZMinus->IsSetup() == true)
		pChunkZMinus->UpdateSurroundedFlag();
	if (pChunkZPlus != NULL && pChunkZPlus->IsSetup() == true)
		pChunkZPlus->UpdateSurroundedFlag();

	// Rebuild neighbours
	if (m_rebuildNeighours)
	{
		if (pChunkXMinus != NULL && pChunkXMinus->IsSetup() == true)
			pChunkXMinus->SetNeedsRebuild(true, false);
		if (pChunkXPlus != NULL && pChunkXPlus->IsSetup() == true)
			pChunkXPlus->SetNeedsRebuild(true, false);
		if (pChunkYMinus != NULL && pChunkYMinus->IsSetup() == true)
			pChunkYMinus->SetNeedsRebuild(true, false);
		if (pChunkYPlus != NULL && pChunkYPlus->IsSetup() == true)
			pChunkYPlus->SetNeedsRebuild(true, false);
		if (pChunkZMinus != NULL && pChunkZMinus->IsSetup() == true)
			pChunkZMinus->SetNeedsRebuild(true, false);
		if (pChunkZPlus != NULL && pChunkZPlus->IsSetup() == true)
			pChunkZPlus->SetNeedsRebuild(true, false);

		m_rebuildNeighours = false;
	}

	m_numRebuilds++;
	m_rebuild = false;
}

void Chunk::SetNeedsRebuild(bool rebuild, bool rebuildNeighours)
{
	m_rebuild = rebuild;
	m_rebuildNeighours = rebuildNeighours;
}

bool Chunk::NeedsRebuild()
{
	return m_rebuild;
}

bool Chunk::IsRebuildingMesh()
{
	return m_isRebuildingMesh;
}

void Chunk::SwitchToCachedMesh()
{
	m_pCachedMesh = m_pMesh;
	m_pMesh = NULL;
}

void Chunk::UndoCachedMesh()
{
	m_deleteCachedMesh = true;
}

// Updating
void Chunk::Update(float dt)
{

}

// Rendering
void Chunk::Render()
{
	OpenGLTriangleMesh* pMeshToUse = m_pMesh;
	if (m_pCachedMesh != NULL)
	{
		pMeshToUse = m_pCachedMesh;
	}

	if (pMeshToUse != NULL)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->TranslateWorldMatrix(m_position.x, m_position.y, m_position.z);

			// Texture manipulation (for shadow rendering)
			{
				Matrix4x4 worldMatrix;
				m_pRenderer->GetModelMatrix(&worldMatrix);

				m_pRenderer->PushTextureMatrix();
				m_pRenderer->MultiplyWorldMatrix(worldMatrix);
			}

			m_pRenderer->MeshStaticBufferRender(pMeshToUse);

			// Texture manipulation (for shadow rendering)
			{
				m_pRenderer->PopTextureMatrix();
			}
		m_pRenderer->PopMatrix();
	}

	if (m_deleteCachedMesh)
	{
		if (m_pCachedMesh != NULL)
		{
			m_pRenderer->ClearMesh(m_pCachedMesh);
			m_pCachedMesh = NULL;
		}

		m_deleteCachedMesh = false;
	}
}

void Chunk::RenderDebug()
{
	float l_length = (Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE) - 0.05f;
	float l_height = (Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE) - 0.05f;
	float l_width = (Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE) - 0.05f;

	m_pRenderer->SetRenderMode(RM_WIREFRAME);
	m_pRenderer->SetCullMode(CM_NOCULL);
	m_pRenderer->SetLineWidth(1.0f);

	m_pRenderer->PushMatrix();
		m_pRenderer->TranslateWorldMatrix(m_position.x, m_position.y, m_position.z);
		m_pRenderer->TranslateWorldMatrix(Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE);
		m_pRenderer->TranslateWorldMatrix(-Chunk::BLOCK_RENDER_SIZE, -Chunk::BLOCK_RENDER_SIZE, -Chunk::BLOCK_RENDER_SIZE);

		m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
		if (IsEmpty())
		{
			m_pRenderer->ImmediateColourAlpha(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else if (IsSurrounded())
		{
			m_pRenderer->ImmediateColourAlpha(0.0f, 1.0f, 1.0f, 1.0f);
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
	m_pRenderer->PopMatrix();

	m_pRenderer->SetCullMode(CM_BACK);
}

void Chunk::Render2D(Camera* pCamera, unsigned int viewport, unsigned int font)
{
	int winx, winy;
	vec3 centerPos = m_position + vec3(Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE, Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE);
	centerPos += vec3(-Chunk::BLOCK_RENDER_SIZE, -Chunk::BLOCK_RENDER_SIZE, -Chunk::BLOCK_RENDER_SIZE);
	m_pRenderer->PushMatrix();
		m_pRenderer->SetProjectionMode(PM_PERSPECTIVE, viewport);
		pCamera->Look();
		m_pRenderer->GetScreenCoordinatesFromWorldPosition(centerPos, &winx, &winy);
	m_pRenderer->PopMatrix();

	bool renderChunkInfo = true;
	if (renderChunkInfo)
	{
		char lLine1[64];
		sprintf(lLine1, "%i, %i, %i", m_gridX, m_gridY, m_gridZ);
		char lLine2[64];
		sprintf(lLine2, "Neighbours: %i", m_numNeighbours);
		char lLine3[64];
		sprintf(lLine3, "Empty: %s", m_emptyChunk ? "true" : "false");
		char lLine4[64];
		sprintf(lLine4, "Surrounded: %s", m_surroundedChunk ? "true" : "false");
		char lLine5[64];
		sprintf(lLine5, "Rebuilds: %i", m_numRebuilds);
		
		m_pRenderer->PushMatrix();
			m_pRenderer->SetRenderMode(RM_SOLID);
			m_pRenderer->SetProjectionMode(PM_2D, viewport);
			m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
			m_pRenderer->RenderFreeTypeText(font, (float)winx, (float)winy, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, "%s", lLine1);
			m_pRenderer->RenderFreeTypeText(font, (float)winx, (float)winy - 20.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, "%s", lLine2);
			m_pRenderer->RenderFreeTypeText(font, (float)winx, (float)winy - 40.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, "%s", lLine3);
			m_pRenderer->RenderFreeTypeText(font, (float)winx, (float)winy - 60.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, "%s", lLine4);
			m_pRenderer->RenderFreeTypeText(font, (float)winx, (float)winy - 80.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, "%s", lLine5);
		m_pRenderer->PopMatrix();
	}
}

// < Operator (Used for chunk sorting, closest to camera)
bool Chunk::operator<(const Chunk &w) const
{
	int playerX = 0;
	int playerY = 0;
	int playerZ = 0;
	if (m_pPlayer != NULL)
	{
		playerX = m_pPlayer->GetGridX();
		playerY = m_pPlayer->GetGridY();
		playerZ = m_pPlayer->GetGridZ();
	}

	int distance = abs(playerX - m_gridX) + abs(playerY - m_gridY) + abs(playerZ - m_gridZ);
	int wDistance = abs(playerX - w.GetGridX()) + abs(playerY - w.GetGridY()) + abs(playerZ - w.GetGridZ());

	return(distance < wDistance);
}

bool Chunk::ClosestToCamera(const Chunk *lhs, const Chunk *rhs)
{
	if ((*lhs) < (*rhs)) { return true; }
	if ((*rhs) < (*lhs)) { return false; }
	return (*lhs) < (*rhs);
}
