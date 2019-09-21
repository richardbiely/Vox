// ******************************************************************************
// Filename:    ChunkManager.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//   ChunkManager handles all the chunk operations and management of the
//   individual chunks, deciding which chunks to load, unload, create, update
//   and also the rendering of the chunks.
//
// Revision History:
//   Initial Revision - 01/11/15
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include "../Renderer/Renderer.h"
#include "../models/QubicleBinary.h"
#include "Chunk.h"
#include "BlocksEnum.h"

#include <map>
using namespace std;

#include "../tinythread/tinythread.h"
using namespace tthread;

class Player;
class NPCManager;
class EnemyManager;
class BlockParticleManager;
class ItemManager;
class SceneryManager;
class VoxSettings;
class QubicleBinaryManager;
class BiomeManager;

struct ChunkCoordKeys {
	int x;
	int y;
	int z;
};

inline bool const operator==(const ChunkCoordKeys& l, const ChunkCoordKeys& r) {
	return l.x == r.x && l.y == r.y && l.z == r.z;
};

inline bool const operator<(const ChunkCoordKeys& l, const ChunkCoordKeys& r) {
	if (l.x < r.x)  return true;
	if (l.x > r.x)  return false;

	if (l.y < r.y)  return true;
	if (l.y > r.y)  return false;

	if (l.z < r.z)  return true;
	if (l.z > r.z)  return false;

	return false;
};

typedef std::vector<Chunk*> ChunkList;
typedef std::vector<ChunkCoordKeys> ChunkCoordKeysList;

enum QubicleImportDirection
{
	QubicleImportDirection_Normal = 0,
	QubicleImportDirection_MirrorX,
	QubicleImportDirection_MirrorY,
	QubicleImportDirection_MirrorZ,
	QubicleImportDirection_RotateY90,
	QubicleImportDirection_RotateY180,
	QubicleImportDirection_RotateY270,
	QubicleImportDirection_RotateX90,
	QubicleImportDirection_RotateX180,
	QubicleImportDirection_RotateX270,
	QubicleImportDirection_RotateZ90,
	QubicleImportDirection_RotateZ180,
	QubicleImportDirection_RotateZ270,
};


class ChunkStorageLoader
{
public:
	int m_gridX;
	int m_gridY;
	int m_gridZ;

	vec3 m_position;

	bool m_blockSet[Chunk::CHUNK_SIZE][Chunk::CHUNK_SIZE][Chunk::CHUNK_SIZE];
	unsigned int m_colour[Chunk::CHUNK_SIZE][Chunk::CHUNK_SIZE][Chunk::CHUNK_SIZE];

	ChunkStorageLoader(int x, int y, int z)
	{
		m_gridX = x;
		m_gridY = y;
		m_gridZ = z;

		float xPos = x * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
		float yPos = y * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;
		float zPos = z * Chunk::CHUNK_SIZE * Chunk::BLOCK_RENDER_SIZE*2.0f;

		m_position = vec3(xPos, yPos, zPos);

		// Reset initial arrays
		for (int x = 0; x < Chunk::CHUNK_SIZE; x++)
		{
			for (int y = 0; y < Chunk::CHUNK_SIZE; y++)
			{
				for (int z = 0; z < Chunk::CHUNK_SIZE; z++)
				{
					m_blockSet[x][y][z] = false;
					m_colour[x][y][z] = 0;
				}
			}
		}
	}

	void SetBlockColour(int x, int y, int z, unsigned int colour)
	{
		m_blockSet[x][y][z] = true;
		m_colour[x][y][z] = colour;
	}
};

typedef std::vector<ChunkStorageLoader*> ChunkStorageLoaderList;

class BlockColourTypeMatch
{
public:
	int m_red;
	int m_green;
	int m_blue;
	BlockType m_blockType;
};

typedef std::vector<BlockColourTypeMatch*> BlockColourTypeMatchList;


class ChunkManager
{
public:
	/* Public methods */
	ChunkManager(Renderer* pRenderer, VoxSettings* pVoxSettings, QubicleBinaryManager* pQubicleBinaryManager);
	~ChunkManager();

	// Linkage
	void SetPlayer(Player* pPlayer);
	void SetNPCManager(NPCManager* pNPCManager);
	void SetEnemyManager(EnemyManager* pEnemyManager);
	void SetBlockParticleManager(BlockParticleManager* pBlockParticleManager);
	void SetItemManager(ItemManager* pItemManager);

	// Scenery manager pointer
	void SetSceneryManager(SceneryManager* pSceneryManager);

	// Biome manager
	void SetBiomeManager(BiomeManager* pBiomeManager);

	// Initial chunk creation
	void InitializeChunkCreation();

	// Chunk rendering material
	unsigned int GetChunkMaterialID();

	// Chunk counters
	int GetNumChunksLoaded();
	int GetNumChunksRender();

	// Loader radius
	void SetLoaderRadius(float radius);
	float GetLoaderRadius();

	// Step update
	void SetStepLockEnabled(bool enabled);
	void StepNextUpdate();

	// Chunk Creation
	void CreateNewChunk(int x, int y, int z);
	void UnloadChunk(Chunk* pChunk);
	void UpdateChunkNeighbours(Chunk* pChunk, int x, int y, int z);

	// Getting chunk and positional information
	void GetGridFromPosition(vec3 position, int* gridX, int* gridY, int* gridZ);
	Chunk* GetChunkFromPosition(float posX, float posY, float posZ);
	Chunk* GetChunk(int aX, int aY, int aZ);
	bool FindClosestFloor(vec3 position, vec3* floorPosition);

	// Getting the active block state given a position and chunk information
	bool GetBlockActiveFrom3DPosition(float x, float y, float z, vec3 *blockPos, int* blockX, int* blockY, int* blockZ, Chunk** pChunk);
	void GetBlockGridFrom3DPositionChunkStorage(float x, float y, float z, int* blockX, int* blockY, int* blockZ, ChunkStorageLoader* ChunkStorage);

	// Adding to chunk storage for parts of the world generation that are outside of loaded chunks
	ChunkStorageLoader* GetChunkStorage(int aX, int aY, int aZ, bool CreateIfNotExist);
	void RemoveChunkStorageLoader(ChunkStorageLoader* pChunkStorage);

	// Block colour to block type matching
	void AddBlockColourBlockTypeMatching(int r, int g, int b, BlockType blockType);
	bool CheckBlockColour(int r, int g, int b, int rCheck, int gCheck, int bCheck);
	BlockType SetBlockTypeBasedOnColour(int r, int g, int b);

	// Importing into the world chunks
	void ImportQubicleBinaryMatrix(const QubicleMatrix* pMatrix, vec3 position, QubicleImportDirection direction);
	const QubicleBinary* ImportQubicleBinary(const QubicleBinary* qubicleBinaryFile, vec3 position, QubicleImportDirection direction);
	const QubicleBinary* ImportQubicleBinary(const char* filename, vec3 position, QubicleImportDirection direction);

	// Explosions
	void CreateBlockDestroyParticleEffect(float r, float g, float b, float a, vec3 blockPosition);
	void ExplodeSphere(vec3 position, float radius);

	// Collectible block objects
	void CreateCollectibleBlock(BlockType blockType, vec3 blockPos);

	// Water
	void SetWaterHeight(float height);
	float GetWaterHeight();
	bool IsUnderWater(vec3 position);

	// Rendering modes
	void SetWireframeRender(bool wireframe);
	void SetFaceMerging(bool faceMerge);
	bool GetFaceMerging();

	// Updating
	void Update(float dt);
	static void _UpdatingChunksThread(void* pData);
	void UpdatingChunksThread();

	// Rendering
	void Render(bool shadowRender);
	void RenderWater();
	void RenderDebug();
	void Render2D(Camera* pCamera, unsigned int viewport, unsigned int font);

protected:
	/* Protected methods */

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	Renderer* m_pRenderer;
	Player* m_pPlayer;
	SceneryManager* m_pSceneryManager;
	BiomeManager* m_pBiomeManager;
	VoxSettings* m_pVoxSettings;
	QubicleBinaryManager* m_pQubicleBinaryManager;
	ItemManager* m_pItemManager;
	BlockParticleManager* m_pBlockParticleManager;
	EnemyManager* m_pEnemyManager;
	NPCManager* m_pNPCManager;

	// Chunk Material
	unsigned int m_chunkMaterialID;

	// Loader radius
	float m_loaderRadius;

	// Water
	float m_waterHeight;

	// Update step lock
	bool m_stepLockEnabled;
	bool m_updateStepLock;

	// Render modes
	bool m_wireframeRender;
	bool m_faceMerging;

	// Chunks storage
	map<ChunkCoordKeys, Chunk*> m_chunksMap;

	// Storage for modifications to chunks that are not loaded yet
	ChunkStorageLoaderList m_vpChunkStorageList;
	mutex m_chunkStorageListLock;

	// Block colour to type matching boundaries
	BlockColourTypeMatchList m_vpBlockColourTypeMatchList;

	// Chunk counters
	int m_numChunksLoaded;
	int m_numChunksRender;

	// Threading
	thread* m_pUpdatingChunksThread;
	mutex m_ChunkMapMutexLock;
	bool m_updateThreadActive;
	mutex m_updateThreadFlagLock;
	bool m_updateThreadFinished;
};
