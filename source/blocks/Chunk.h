// ******************************************************************************
// Filename:    Chunk.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//   A chunk is a collection of voxel blocks that are arranged together for
//   easier manipulation and management, when a single voxel in a chunk is
//   modified the whole chunk is refreshed. Chunks are rendered together
//   as a single vertex buffer and thus each chunk can be considered a single
//   draw call to render many voxels.
//
// Revision History:
//   Initial Revision - 01/11/15
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include "BlocksEnum.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/camera.h"

#include <vector>
#include <mutex>
using namespace std;

class ChunkManager;
class Player;
class SceneryManager;
class VoxSettings;
class Item;
class BiomeManager;

typedef vector<Item*> ItemList;

class Chunk
{
public:
	/* Public methods */
	Chunk(Renderer* pRenderer, ChunkManager* pChunkManager, VoxSettings* pVoxSettings);
	~Chunk();

	// Player pointer
	void SetPlayer(Player* pPlayer);

	// Scenery manager pointer
	void SetSceneryManager(SceneryManager* pSceneryManager);

	// Biome manager
	void SetBiomeManager(BiomeManager* pBiomeManager);

	// Initialize
	void Initialize();

	// Creation and destruction
	void SetCreated(bool created);
	bool IsCreated() const;
	void Unload();
	void Setup();
	bool IsSetup() const;
	bool IsUnloading() const;

	// Saving and loading
	void SaveChunk();
	void LoadChunk();

	// Position
	void SetPosition(vec3 pos);
	vec3 GetPosition() const;

	// Neighbours
	int GetNumNeighbours();
	void SetNumNeighbours(int neighbours);
	Chunk* GetxMinus() const;
	Chunk* GetxPlus() const;
	Chunk* GetyMinus() const;
	Chunk* GetyPlus() const;
	Chunk* GetzMinus() const;
	Chunk* GetzPlus() const;
	void SetxMinus(Chunk* pChunk);
	void SetxPlus(Chunk* pChunk);
	void SetyMinus(Chunk* pChunk);
	void SetyPlus(Chunk* pChunk);
	void SetzMinus(Chunk* pChunk);
	void SetzPlus(Chunk* pChunk);

	// Grid
	void SetGrid(int x, int y, int z);
	int GetGridX() const;
	int GetGridY() const;
	int GetGridZ() const;

	// Batch update
	void StartBatchUpdate();
	void StopBatchUpdate();

	// Active
	bool GetActive(int x, int y, int z) const;

	// Inside chunk
	bool IsInsideChunk(vec3 pos) const;

	// Items
	void AddItem(Item* pItem);
	void RemoveItem(Item* pItem);
	void RemoveItems();

	// Block colour
	void SetColour(int x, int y, int z, float r, float g, float b, float a, bool setBlockType = false);
	void GetColour(int x, int y, int z, float* r, float* g, float* b, float* a);
	void SetColour(int x, int y, int z, unsigned int colour, bool setBlockType = false);
	unsigned int GetColour(int x, int y, int z) const;

	// Block type
	BlockType GetBlockType(int x, int y, int z) const;
	void SetBlockType(int x, int y, int z, BlockType blockType);

	// Flags
	bool IsEmpty() const;
	bool IsSurrounded() const;
	void UpdateWallFlags();
	bool UpdateSurroundedFlag();
	void UpdateEmptyFlag();

	// Create mesh
	void CreateMesh();
	void CompleteMesh();
	void UpdateMergedSide(int *merged, int blockx, int blocky, int blockz, int width, int height, vec3 *p1, vec3 *p2, vec3 *p3, vec3 *p4, int startX, int startY, int maxX, int maxY, bool positive, bool zFace, bool xFace, bool yFace);

	// Rebuild
	void RebuildMesh();
	void SetNeedsRebuild(bool rebuild, bool rebuildNeighours);
	bool NeedsRebuild() const;
	bool IsRebuildingMesh() const;
	void SwitchToCachedMesh();
	void UndoCachedMesh();

	// Updating
	void Update(float dt);
	
	// Rendering
	void Render();
	void RenderDebug();
	void Render2D(Camera* pCamera, unsigned int viewport, unsigned int font);

	// < Operator (Used for chunk sorting, closest to camera)
	bool operator<(const Chunk &w) const;
	static bool ClosestToCamera(const Chunk *lhs, const Chunk *rhs);

protected:
	/* Protected methods */

private:
	/* Private methods */

public:
	/* Public members */
	static const int CHUNK_SIZE = 16;
	static const int CHUNK_SIZE_SQUARED = CHUNK_SIZE * CHUNK_SIZE;
	static const int CHUNK_SIZE_CUBED = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
	static const float BLOCK_RENDER_SIZE;
	static const float CHUNK_RADIUS;

protected:
	/* Protected members */

private:
	/* Private members */
	VoxSettings* m_pVoxSettings;
	Renderer* m_pRenderer;
	ChunkManager* m_pChunkManager;
	Player* m_pPlayer;
	SceneryManager* m_pSceneryManager;
	BiomeManager* m_pBiomeManager;

	// Chunk neighbours
	int m_numNeighbours;
	Chunk* m_pxMinus;
	Chunk* m_pxPlus;
	Chunk* m_pyMinus;
	Chunk* m_pyPlus;
	Chunk* m_pzMinus;
	Chunk* m_pzPlus;

	// Flag for change during a batch update
	bool m_chunkChangedDuringBatchUpdate;

	// Grid co-ordinates
	int m_gridX;
	int m_gridY;
	int m_gridZ;

	// Chunk position
	vec3 m_position;

	// Setup and creation flags
	bool m_created;
	bool m_setup;
	bool m_isUnloading;
	bool m_rebuild;
	bool m_rebuildNeighours;
	bool m_isRebuildingMesh;
	bool m_deleteCachedMesh;

	// Counters
	int m_numRebuilds;

	// Flags for empty chunk and completely surrounded
	bool m_emptyChunk;
	bool m_surroundedChunk;

	// Used for testing if chunk completely covers neighbour chunks
	bool m_x_minus_full;
	bool m_x_plus_full;
	bool m_y_minus_full;
	bool m_y_plus_full;
	bool m_z_minus_full;
	bool m_z_plus_full;

	// The blocks colour data
	unsigned int *m_colour;

	// Block type
	BlockType *m_blockType;

	// Item list
	mutex m_itemMutexLock;
	ItemList m_vpItemList;

	// Render mesh
	OpenGLTriangleMesh* m_pMesh;
	OpenGLTriangleMesh* m_pCachedMesh;
};
