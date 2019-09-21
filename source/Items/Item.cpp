// ******************************************************************************
// Filename:    Item.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 12/04/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "Item.h"
#include "ItemManager.h"
#include "ItemSpawner.h"

#include <algorithm>

#include "../utils/Interpolator.h"
#include "../utils/Random.h"
#include "../Player/Player.h"
#include "../Lighting/LightingManager.h"
#include "../VoxGame.h"


Item::Item(Renderer* pRenderer, ChunkManager* pChunkManager, QubicleBinaryManager* pQubicleBinaryManager, string itemTitle, eItem itemType, float scale)
{
	m_pRenderer =  pRenderer;
	m_pChunkManager = pChunkManager;
	m_pQubicleBinaryManager = pQubicleBinaryManager;

	m_itemType = itemType;

	m_itemTitle = itemTitle;

	m_renderScale = scale;

	m_outlineRender = false;

	m_forward = vec3(0.0f, 0.0f, 1.0f);

	m_erase = false;

	m_radius = GetItemRadiusForType(m_itemType);

	m_worldCollide = true;

	m_isCollectible = false;
	m_collectionDelay = 0.0f;

	m_interactable = true;
	m_itemInteracting = false;

	m_itemPickup = false;

	m_autoDisappear = false;
	m_autoDisappearTimer = 0.0f;

	m_bCreateDyingLights = true;

	m_droppedInventoryItem = NULL;

	// Disappear animaton
	m_disappear = false;
	m_disappearDelay = 0.5f;
	m_disappearTimer = m_disappearDelay;
	m_disappearAnimationStarted = false;
	m_disappearScale = m_renderScale;

	m_interactCount = 0;
	m_maxInteractCount = 1;

	m_pParentItemSpawner = NULL;

	m_gridPositionX = 0;
	m_gridPositionY = 0;
	m_gridPositionZ = 0;

	m_collisionEnabled = false;
	m_collisionRadius = 1.0f;

	m_pCachedGridChunk = NULL;

	m_pOwningChunk = NULL;

	m_pVoxelItem = NULL;
}

Item::~Item()
{
	// If we belong to a spawner, make sure we indicate that we were killed
	if (m_pParentItemSpawner != NULL)
	{
		m_pParentItemSpawner->RemoveItemFromThisSpawner();
	}

	UnloadEffectsAndLights();

	// Delete the voxel item
	if(m_pVoxelItem != NULL)
	{
		delete m_pVoxelItem;
	}

	// Remove us from an owning chunk
	if(m_pOwningChunk != NULL)
	{
		m_pOwningChunk->RemoveItem(this);
	}

	// Clear the dropped item
	if(m_droppedInventoryItem != NULL)
	{
		delete m_droppedInventoryItem;
	}

	// Clear the bounding region data
	for(unsigned int i = 0; i < m_vpBoundingRegionList.size(); i++)
	{
		delete m_vpBoundingRegionList[i];
		m_vpBoundingRegionList[i] = NULL;
	}
	m_vpBoundingRegionList.clear();
}

// Unloading
void Item::UnloadEffectsAndLights()
{
	// Lights
	for(int i = 0; i < m_pVoxelItem->GetNumLights(); i++)
	{
		unsigned int lightId;
		vec3 lightPos;
		float lightRadius;
		float lightDiffuseMultiplier;
		Colour lightColour;
		bool connectedToSegment;
		m_pVoxelItem->GetLightParams(i, &lightId, &lightPos, &lightRadius, &lightDiffuseMultiplier, &lightColour, &connectedToSegment);

		if(lightId != -1)
		{
			m_pLightingManager->RemoveLight(lightId);
			m_pVoxelItem->SetLightingId(i, -1);

			lightPos += m_position;

			if(m_itemType != eItem_DroppedItem)
			{
				if (m_bCreateDyingLights)
				{
					unsigned int lId;
					m_pLightingManager->AddDyingLight(lightPos, lightRadius, lightDiffuseMultiplier, lightColour, 3.5f, &lId);
				}
			}
		}
	}

	// Particle Effects
	for(int i = 0; i < m_pVoxelItem->GetNumParticleEffects(); i++)
	{
		unsigned int particleEffectId;
		vec3 ParticleEffectPos;
		string effectName;
		bool connectedToSegment;
		m_pVoxelItem->GetParticleEffectParams(i, &particleEffectId, &ParticleEffectPos, &effectName, &connectedToSegment);

		if(particleEffectId != -1)
		{
			m_pBlockParticleManager->DestroyParticleEffect(particleEffectId);
			m_pVoxelItem->SetParticleEffectId(i, -1);
		}
	}
}

void Item::SetLightingManager(LightingManager* pLightingManager)
{
	m_pLightingManager = pLightingManager;
}

void Item::SetBlockParticleManager(BlockParticleManager* pBlockParticleManager)
{
	m_pBlockParticleManager = pBlockParticleManager;
}

void Item::SetPlayer(Player* pPlayer)
{
	m_pPlayer = pPlayer;
}

void Item::SetItemManager(ItemManager* pItemManager)
{
	m_pItemManager = pItemManager;
}

void Item::SetInventoryManager(InventoryManager* pInventoryManager)
{
	m_pInventoryManager = pInventoryManager;
}

void Item::SetErase(bool erase)
{
	m_erase = erase;
}

bool Item::NeedsErasing()
{
	return m_erase;
}

string Item::GetFileName()
{
	return m_fileName;
}

void Item::SetChunk(Chunk* pChunk)
{
	m_pOwningChunk = pChunk;
}

Chunk* Item::GetChunk()
{
	return m_pOwningChunk;
}

VoxelWeapon* Item::GetVoxelItem()
{
	return m_pVoxelItem;
}

// Setup
void Item::LoadItem(const char* objectFilename)
{
	if(m_pVoxelItem == NULL)
	{
		m_pVoxelItem = new VoxelWeapon(m_pRenderer, m_pQubicleBinaryManager);
	}
	
	m_pVoxelItem->SetVoxelCharacterParent(NULL);
	m_pVoxelItem->LoadWeapon(objectFilename);

	m_fileName = objectFilename;

	if(m_pVoxelItem != NULL)
	{
		// Collision bounding region		
		const QubicleMatrix* pMatrix = m_pVoxelItem->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->GetQubicleMatrix(0);
		const int numX = pMatrix->m_matrixSizeX;
		const int numY = pMatrix->m_matrixSizeY;
		const int numZ = pMatrix->m_matrixSizeZ;
		CreateBoundingRegion(vec3(0.0f, 0.0f, 0.0f), BoundingRegionType_Cube, 0.0f, numX*0.5f, numY*0.5f, numZ*0.5f, 1.0);

		UpdateCollisionRadius();

		// Lights
		for(int i = 0; i < m_pVoxelItem->GetNumLights(); i++)
		{
			unsigned int lightId;
			vec3 lightPos;
			float lightRadius;
			float lightDiffuseMultiplier;
			Colour lightColour;
			bool connectedToSegment;
			m_pVoxelItem->GetLightParams(i, &lightId, &lightPos, &lightRadius, &lightDiffuseMultiplier, &lightColour, &connectedToSegment);

			if(lightId != -1)
			{
				m_pLightingManager->RemoveLight(lightId);
				m_pVoxelItem->SetLightingId(i, -1);
			}
		}

		// Particle effects
		for(int i = 0; i < m_pVoxelItem->GetNumParticleEffects(); i++)
		{
			unsigned int particleEffectId;
			vec3 ParticleEffectPos;
			string effectName;
			bool connectedToSegment;
			m_pVoxelItem->GetParticleEffectParams(i, &particleEffectId, &ParticleEffectPos, &effectName, &connectedToSegment);

			if(particleEffectId != -1)
			{
				m_pBlockParticleManager->DestroyParticleEffect(particleEffectId);
				m_pVoxelItem->SetParticleEffectId(i, -1);
			}
		}
	}
}

// Item spawner
void Item::SetItemSpawner(ItemSpawner* pSpawner)
{
	m_pParentItemSpawner = pSpawner;
}

void Item::RemoveItemSpawner(ItemSpawner* pSpawner)
{
	if (m_pParentItemSpawner == pSpawner)
	{
		m_pParentItemSpawner = NULL;
	}
}

// Accessors / Setters
void Item::SetPosition(vec3 pos)
{
	m_position = pos;
}

vec3 Item::GetPosition()
{
	return m_position;
}

void Item::SetVelocity(vec3 vel)
{
	m_velocity = vel;
}

vec3 Item::GetVelocity()
{
	return m_velocity;
}

void Item::SetRotation(vec3 rot)
{
	m_rotation = rot;
}

vec3 Item::GetRotation()
{
	return m_rotation;
}

void Item::SetAngularVelocity(vec3 angvel)
{
	m_angularVelocity = angvel;
}

vec3 Item::GetAngularVelocity()
{
	return m_angularVelocity;
}

void Item::SetGravityDirection(vec3 dir)
{
	m_gravityDirection = dir;

	if(length(m_gravityDirection) >= 0.0001f)
	{
		m_gravityDirection = normalize(m_gravityDirection);
	}
}

vec3 Item::GetGravityDirection()
{
	return m_gravityDirection;
}

float Item::GetScale()
{
	return m_renderScale;
}

float Item::GetRadius()
{
	return m_radius;
}

vec3 Item::GetCenter()
{
	vec3 center = m_position + (m_pVoxelItem->GetCenter()*GetScale());
	return center;
}

// Inventory item params
void Item::SetDroppedItem(const char* filename, const char* iconFilename, InventoryType itemType, eItem item, ItemStatus status, EquipSlot equipSlot, ItemQuality itemQuality, bool left, bool right, const char* title, const char* description, float r, float g, float b, int quantity)
{
	if(m_droppedInventoryItem == NULL)
	{
		m_droppedInventoryItem = new InventoryItem();
	}

	m_droppedInventoryItem->m_filename = filename;
	m_droppedInventoryItem->m_Iconfilename = iconFilename;
	m_droppedInventoryItem->m_title = title;
	m_droppedInventoryItem->m_description = description;

	m_droppedInventoryItem->m_itemType = itemType;

	m_droppedInventoryItem->m_item = item;

	m_droppedInventoryItem->m_status = status;

	m_droppedInventoryItem->m_equipSlot = equipSlot;

	m_droppedInventoryItem->m_itemQuality = itemQuality;

	m_droppedInventoryItem->m_left = left;
	m_droppedInventoryItem->m_right = right;

	m_droppedInventoryItem->m_placementR = r;
	m_droppedInventoryItem->m_placementG = g;
	m_droppedInventoryItem->m_placementB = b;

	m_droppedInventoryItem->m_lootSlotX = -1;
	m_droppedInventoryItem->m_lootSlotY = -1;

	m_droppedInventoryItem->m_equipped = false;

	m_droppedInventoryItem->m_scale = 1.0f;
	m_droppedInventoryItem->m_offsetX = 0.0f;
	m_droppedInventoryItem->m_offsetY = 0.0f;
	m_droppedInventoryItem->m_offsetZ = 0.0f;

	m_droppedInventoryItem->m_quantity = quantity;

	m_droppedInventoryItem->m_remove = false;
}

void Item::SetDroppedItem(InventoryItem* pItem)
{
	SetDroppedItem(pItem->m_filename.c_str(), pItem->m_Iconfilename.c_str(), pItem->m_itemType, pItem->m_item, pItem->m_status, pItem->m_equipSlot, pItem->m_itemQuality,
				   pItem->m_left, pItem->m_right, pItem->m_title.c_str(), pItem->m_description.c_str(), pItem->m_placementR, pItem->m_placementG, pItem->m_placementB, pItem->m_quantity);
}

InventoryItem* Item::GetDroppedInventoryItem()
{
	return m_droppedInventoryItem;
}

// Collectible
bool Item::IsCollectible()
{
	return m_isCollectible && m_collectionDelay <= 0.0f;
}

void Item::SetIsCollectible(bool collect)
{
	m_isCollectible = collect;
}

void Item::SetCollectionDelay(float delay)
{
	m_collectionDelay = delay;
}

bool Item::IsItemPickedUp()
{
	return m_itemPickup;
}

void Item::SetPickupGotoPosition(vec3 pickupPos)
{
	m_autoDisappear = false;
	m_itemPickup = true;
	m_pickupPos = pickupPos;
}

// Auto disappear
void Item::SetAutoDisappear(float disappearTime)
{
	m_autoDisappear = true;
	m_autoDisappearTimer = disappearTime;
}

// Animation
bool Item::IsStillAnimating()
{
	for (int animatedSectionsIndex = 0; animatedSectionsIndex < m_pVoxelItem->GetNumAimatedSections(); animatedSectionsIndex++)
	{
		if (m_pVoxelItem->HasSubSectionAnimationFinished(animatedSectionsIndex) == false)
		{
			return true;
		}
	}

	return false;
}

// Rendering helpers
void Item::SetOutlineRender(bool outline)
{
	m_outlineRender = outline;
}

bool Item::IsOutlineRender()
{
	return m_outlineRender;
}

void Item::SetWireFrameRender(bool wireframe)
{
	if(m_pVoxelItem != NULL)
	{
		m_pVoxelItem->SetWireFrameRender(wireframe);
	}
}

void Item::CalculateWorldTransformMatrix()
{
	m_worldMatrix.LoadIdentity();
	m_worldMatrix.SetRotation(DegToRad(m_rotation.x), DegToRad(m_rotation.y), DegToRad(m_rotation.z));
	m_worldMatrix.SetTranslation(m_position);

	for(unsigned int i = 0; i < m_vpBoundingRegionList.size(); i++)
	{
		Matrix4x4 justParentRotation;
		justParentRotation.SetRotation(DegToRad(m_rotation.x), DegToRad(m_rotation.y), DegToRad(m_rotation.z));

		m_vpBoundingRegionList[i]->UpdatePlanes(justParentRotation, m_renderScale);
	}
}

// Item type
eItem Item::GetItemType()
{
	return m_itemType;
}

// Item title
const char* Item::GetItemTitle()
{
	return m_itemTitle.c_str();
}

// Should we create dying lights when we unload
void Item::SetCreateDyingLights(bool dyingLights)
{
	m_bCreateDyingLights = dyingLights;
}

// Grid
void Item::UpdateGridPosition()
{
	int gridPositionX = (int)((m_position.x + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);
	int gridPositionY = (int)((m_position.y + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);
	int gridPositionZ = (int)((m_position.z + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);

	if(m_position.x <= -0.5f)
		gridPositionX -= 1;
	if(m_position.y <= -0.5f)
		gridPositionY -= 1;
	if(m_position.z <= -0.5f)
		gridPositionZ -= 1;

	if(gridPositionX != m_gridPositionX || gridPositionY != m_gridPositionY || gridPositionZ != m_gridPositionZ || m_pCachedGridChunk == NULL)
	{
		m_gridPositionX = gridPositionX;
		m_gridPositionY = gridPositionY;
		m_gridPositionZ = gridPositionZ;

		m_pCachedGridChunk = m_pChunkManager->GetChunk(m_gridPositionX, m_gridPositionY, m_gridPositionZ);
	}
}

Chunk* Item::GetCachedGridChunkOrFromPosition(vec3 pos)
{
	// First check if the position is in the same grid as the cached chunk
	int gridPositionX = (int)((pos.x + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);
	int gridPositionY = (int)((pos.y + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);
	int gridPositionZ = (int)((pos.z + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);

	if(pos.x <= -0.5f)
		gridPositionX -= 1;
	if(pos.y <= -0.5f)
		gridPositionY -= 1;
	if(pos.z <= -0.5f)
		gridPositionZ -= 1;

	if(gridPositionX != m_gridPositionX || gridPositionY != m_gridPositionY || gridPositionZ != m_gridPositionZ)
	{
		return NULL;
	}
	else
	{
		return m_pCachedGridChunk;
	}
}

// Loot items
int Item::GetNumLootItems()
{
	return (int)m_vpInventoryItemList.size();
}

InventoryItem* Item::GetLootItem(int index)
{
	return m_vpInventoryItemList[index];
}

InventoryItem* Item::AddLootItem(InventoryItem* pItem, int slotX, int slotY)
{
	if(pItem != NULL)
	{
		InventoryItem* pAddedLootItem = AddLootItem(pItem->m_filename.c_str(), pItem->m_Iconfilename.c_str(), pItem->m_itemType, pItem->m_item, pItem->m_status, pItem->m_equipSlot, pItem->m_itemQuality, pItem->m_title.c_str(), pItem->m_description.c_str(), pItem->m_left, pItem->m_right, pItem->m_placementR, pItem->m_placementG, pItem->m_placementB, pItem->m_quantity, slotX, slotY);

		for(int i = 0; i < (int)pItem->m_vpStatAttributes.size(); i++)
		{
			pAddedLootItem->AddStatAttribute(pItem->m_vpStatAttributes[i]->GetType(), pItem->m_vpStatAttributes[i]->GetModifyAmount());
		}

		return pAddedLootItem;
	}

	return NULL;
}

InventoryItem* Item::AddLootItem(const char* filename, const char* iconFilename, InventoryType itemType, eItem item, ItemStatus status, EquipSlot equipSlot, ItemQuality itemQuality, const char* title, const char* description, bool left, bool right, float r, float g, float b, int quantity, int slotX, int slotY)
{
	bool addToExistingItem = false;
	bool createNewitem = true;
	if(quantity != -1)
	{
		addToExistingItem = true;
	}

	if(addToExistingItem)
	{
		for(unsigned int i = 0; i < m_vpInventoryItemList.size(); i++)
		{
			if(strcmp(title, m_vpInventoryItemList[i]->m_title.c_str()) == 0)
			{
				createNewitem = false;

				m_vpInventoryItemList[i]->m_quantity += quantity;

				return m_vpInventoryItemList[i];
			}
		}
	}

	if(createNewitem)
	{
		InventoryItem* pNewItem = new InventoryItem();

		pNewItem->m_filename = filename;
		pNewItem->m_Iconfilename = iconFilename;
		pNewItem->m_title = title;
		pNewItem->m_description = description;

		pNewItem->m_itemType = itemType;
		pNewItem->m_item = item;
		pNewItem->m_status = status;
		pNewItem->m_equipSlot = equipSlot;
		pNewItem->m_itemQuality = itemQuality;

		pNewItem->m_left = left;
		pNewItem->m_right = right;

		pNewItem->m_placementR = r;
		pNewItem->m_placementG = g;
		pNewItem->m_placementB = b;

		pNewItem->m_lootSlotX = slotX;
		pNewItem->m_lootSlotY = slotY;

		pNewItem->m_equipped = false;

		pNewItem->m_quantity = quantity;

		pNewItem->m_remove = false;

		m_vpInventoryItemList.push_back(pNewItem);

		return pNewItem;
	}

	return NULL;
}

bool needs_erasing(InventoryItem* aI)
{
	bool needsErase = aI->m_remove;

	if(needsErase == true)
	{
		delete aI;
	}

	return needsErase;
}

void Item::RemoveLootItem(InventoryItem* pInventoryItem)
{
	for(unsigned int i = 0; i < m_vpInventoryItemList.size(); i++)
	{
		if(m_vpInventoryItemList[i] == pInventoryItem)
		{
			pInventoryItem->m_remove = true;
		}
	}

	m_vpInventoryItemList.erase( remove_if(m_vpInventoryItemList.begin(), m_vpInventoryItemList.end(), needs_erasing), m_vpInventoryItemList.end() );
}

void Item::ClearLootItems()
{
	for(unsigned int i = 0; i < m_vpInventoryItemList.size(); i++)
	{
		//RemoveLootItem(m_vpInventoryItemList[i]);

		delete m_vpInventoryItemList[i];
		m_vpInventoryItemList[i] = 0;
	}
	m_vpInventoryItemList.clear();
}

InventoryItemList Item::GetLootItemList()
{
	return m_vpInventoryItemList;
}

// Interaction position
void Item::SetInteractionPositionOffset(vec3 offset)
{
	m_interactionPositionOffset = offset;
}

vec3 Item::GetInteractionPosition()
{
	return GetCenter() + m_interactionPositionOffset;
}

// World collision
void Item::SetWorldCollide(bool collide)
{
	m_worldCollide = collide;
}

bool Item::CheckCollisions(vec3 positionCheck, vec3 previousPosition, vec3 *pNormal, vec3 *pMovement)
{
	float radius = GetRadius();

	vec3 movementCache = *pMovement;

	// World collisions
	bool worldCollision = false;

	vec3 floorPosition;
	if (m_pChunkManager->FindClosestFloor(positionCheck, &floorPosition) == false)
	{
		*pMovement = vec3(0.0f, 0.0f, 0.0f);
		return true;
	}
	else
	{
		int blockX, blockY, blockZ;
		vec3 blockPos;
		int numChecks = 1 + (int)(radius / (Chunk::BLOCK_RENDER_SIZE* 2.0f));
		for (int x = -numChecks; x <= numChecks; x++)
		{
			for (int y = -numChecks; y <= numChecks; y++)
			{
				for (int z = -numChecks; z <= numChecks; z++)
				{
					*pNormal = vec3(0.0f, 0.0f, 0.0f);

					Chunk* pChunk = GetCachedGridChunkOrFromPosition(positionCheck + vec3((Chunk::BLOCK_RENDER_SIZE*2.0f)*x, (Chunk::BLOCK_RENDER_SIZE*2.0f)*y, (Chunk::BLOCK_RENDER_SIZE*2.0f)*z));
					bool active = m_pChunkManager->GetBlockActiveFrom3DPosition(positionCheck.x + ((Chunk::BLOCK_RENDER_SIZE*2.0f)*x), positionCheck.y + ((Chunk::BLOCK_RENDER_SIZE*2.0f)*y), positionCheck.z + ((Chunk::BLOCK_RENDER_SIZE*2.0f)*z), &blockPos, &blockX, &blockY, &blockZ, &pChunk);

					if (active == false)
					{
						if (pChunk == NULL || pChunk->IsSetup() == false)
						{
							*pMovement = vec3(0.0f, 0.0f, 0.0f);
							worldCollision = false;
						}
					}
					else if (active == true)
					{
						Plane3D planes[6];
						planes[0] = Plane3D(vec3(-1.0f, 0.0f, 0.0f), vec3(Chunk::BLOCK_RENDER_SIZE, 0.0f, 0.0f));
						planes[1] = Plane3D(vec3(1.0f, 0.0f, 0.0f), vec3(-Chunk::BLOCK_RENDER_SIZE, 0.0f, 0.0f));
						planes[2] = Plane3D(vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, Chunk::BLOCK_RENDER_SIZE, 0.0f));
						planes[3] = Plane3D(vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, -Chunk::BLOCK_RENDER_SIZE, 0.0f));
						planes[4] = Plane3D(vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 0.0f, Chunk::BLOCK_RENDER_SIZE));
						planes[5] = Plane3D(vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, -Chunk::BLOCK_RENDER_SIZE));

						float distance;
						int inside = 0;
						bool insideCache[6];

						for (int i = 0; i < 6; i++)
						{
							vec3 pointToCheck = blockPos - previousPosition;
							distance = planes[i].GetPointDistance(pointToCheck);

							if (distance < -radius)
							{
								// Outside...
								insideCache[i] = false;
							}
							else if (distance < radius)
							{
								// Intersecting..
								insideCache[i] = true;
							}
							else
							{
								// Inside...
								insideCache[i] = true;
							}
						}

						for (int i = 0; i < 6; i++)
						{
							vec3 pointToCheck = blockPos - positionCheck;
							distance = planes[i].GetPointDistance(pointToCheck);

							if (distance < -radius)
							{
								// Outside...
							}
							else if (distance < radius)
							{
								// Intersecting..
								inside++;
								if (insideCache[i] == false)
								{
									*pNormal += planes[i].mNormal;
								}
							}
							else
							{
								// Inside...
								inside++;
								if (insideCache[i] == false)
								{
									*pNormal += planes[i].mNormal;
								}
							}
						}

						if (inside == 6)
						{
							if (length(*pNormal) <= 1.0f)
							{
								if (length(*pNormal) > 0.0f)
								{
									*pNormal = normalize(*pNormal);
								}

								float dotResult = dot(*pNormal, *pMovement);
								*pNormal *= dotResult;

								*pMovement -= *pNormal;

								worldCollision = true;
							}
						}
					}
				}
			}
		}
	}

	if (worldCollision)
		return true;

	*pMovement = movementCache;

	return false;
}

// Bounding collision region
void Item::CreateBoundingRegion(vec3 origin, BoundingRegionType boundingType, float radius, float xWidth, float yWidth, float zWidth, float scale)
{
	if(m_vpBoundingRegionList.size() >= 1)
	{
		return;
	}

	SetCollisionEnabled(true);

	BoundingRegion* lpNewRegion = new BoundingRegion();

	lpNewRegion->m_origin = origin;
	lpNewRegion->m_boundingType = boundingType;
	lpNewRegion->m_radius = radius;
	lpNewRegion->m_x_length = xWidth;
	lpNewRegion->m_y_length = yWidth;
	lpNewRegion->m_z_length = zWidth;
	lpNewRegion->m_scale = scale;

	Matrix4x4 transformMatrix;
	lpNewRegion->UpdatePlanes(transformMatrix, 1.0f);

	m_vpBoundingRegionList.push_back(lpNewRegion);
}

void Item::UpdateBoundingRegion(int index, vec3 origin, BoundingRegionType boundingType, float radius, float xWidth, float yWidth, float zWidth, float scale)
{
	if(m_vpBoundingRegionList.size() == 0)
	{
		CreateBoundingRegion(origin, boundingType, radius, xWidth, yWidth, zWidth, scale);
	}

	if(index >= (int)(m_vpBoundingRegionList.size()))
	{
		return;
	}

	BoundingRegion* lpRegion = m_vpBoundingRegionList[index];

	lpRegion->m_origin = origin;
	lpRegion->m_boundingType = boundingType;
	lpRegion->m_radius = radius;
	lpRegion->m_x_length = xWidth;
	lpRegion->m_y_length = yWidth;
	lpRegion->m_z_length = zWidth;
	lpRegion->m_scale = scale;

	Matrix4x4 transformMatrix;
	lpRegion->UpdatePlanes(transformMatrix, 1.0f);
}

void Item::AddBoundingRegion(vec3 origin, BoundingRegionType boundingType, float radius, float xWidth, float yWidth, float zWidth, float scale)
{
	BoundingRegion* lpNewRegion = new BoundingRegion();

	lpNewRegion->m_origin = origin;
	lpNewRegion->m_boundingType = boundingType;
	lpNewRegion->m_radius = radius;
	lpNewRegion->m_x_length = xWidth;
	lpNewRegion->m_y_length = yWidth;
	lpNewRegion->m_z_length = zWidth;
	lpNewRegion->m_scale = scale;

	Matrix4x4 transformMatrix;
	lpNewRegion->UpdatePlanes(transformMatrix, 1.0f);

	m_vpBoundingRegionList.push_back(lpNewRegion);
}

BoundingRegionList Item::GetBoundingRegionList()
{
	return m_vpBoundingRegionList;
}

// Explode
void Item::Explode()
{
	CalculateWorldTransformMatrix();

	if(m_pVoxelItem != NULL)
	{
		for(int animatedSectionsIndex = 0; animatedSectionsIndex < m_pVoxelItem->GetNumAimatedSections(); animatedSectionsIndex++)
		{
			AnimatedSection* pAnimatedSection = m_pVoxelItem->GetAnimatedSection(animatedSectionsIndex);
			QubicleBinary* pQubicleModel = pAnimatedSection->m_pVoxelObject->GetQubicleModel();
			m_pBlockParticleManager->ExplodeQubicleBinary(pQubicleModel, m_renderScale, 100);
		}
	}
}

bool Item::IsColliding(vec3 center, vec3 previousCenter, float radius, vec3 *pNormal, vec3 *pMovement)
{
	bool colliding = false;

	center = GetCenter() - center;
	previousCenter = GetCenter() - previousCenter;

	*pNormal = vec3(0.0f, 0.0f, 0.0f);

	for(unsigned int i = 0; i < m_vpBoundingRegionList.size(); i++)
	{
		BoundingRegion* pRegion = m_vpBoundingRegionList[i];

		if(pRegion->m_boundingType == BoundingRegionType_Sphere)
		{
			vec3 spriteCenter = GetCenter()*pRegion->m_scale; // Should this be used?
			vec3 toRegion = center + (pRegion->m_origin * m_renderScale);
			float lengthSize = length(toRegion);

			if(lengthSize <= pRegion->m_radius*pRegion->m_scale + radius)
			{
				vec3 toPrevious = previousCenter + pRegion->m_origin;
				*pNormal = -normalize(toPrevious);

				colliding = true;
			}
		}

		if(pRegion->m_boundingType == BoundingRegionType_Cube)
		{
			float distance;
			int inside = 0;
			bool insideCache[6];

			for(int i = 0; i < 6; i++)
			{
				distance = pRegion->m_planes[i].GetPointDistance(previousCenter + (pRegion->m_origin * m_renderScale));

				if (distance < -radius)
				{
					// Outside...
					insideCache[i] = false;
				}
				else if (distance < radius)
				{
					// Intersecting..
					insideCache[i] = true;
				}
				else
				{
					// Inside...
					insideCache[i] = true;
				}
			}

			for(int i = 0; i < 6; i++)
			{
				distance = pRegion->m_planes[i].GetPointDistance(center + (pRegion->m_origin * m_renderScale));

				if (distance < -radius)
				{
					// Outside...
				}
				else if (distance < radius)
				{
					// Intersecting..
					inside++;
					if(insideCache[i] == false)
					{
						*pNormal += pRegion->m_planes[i].mNormal;
					}
				}
				else
				{
					// Inside...
					inside++;
					if(insideCache[i] == false)
					{
						*pNormal += pRegion->m_planes[i].mNormal;
					}
				}
			}

			if(inside == 6)
			{
				if(length(*pNormal) <= 1.0f)
				{
					if(length(*pNormal) > 0.0f)
					{
						*pNormal = normalize(*pNormal);
					}

					colliding = true;
				}
			}
		}
	}

	if(colliding)
	{
		float dotResult = dot(*pNormal, *pMovement);
		*pNormal *= dotResult;

		*pMovement -= *pNormal;
	}

	return colliding;
}

void Item::SetCollisionEnabled(bool set)
{
	m_collisionEnabled = set;
}

bool Item::IsCollisionEnabled()
{
	return m_collisionEnabled;
}

void Item::UpdateCollisionRadius()
{
	const QubicleMatrix* pMatrix = m_pVoxelItem->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->GetQubicleMatrix(0);
	const int numX = pMatrix->m_matrixSizeX;
	const int numY = pMatrix->m_matrixSizeY;
	const int numZ = pMatrix->m_matrixSizeZ;

	int max = numX;
	if (numY > max)
	{
		max = numY;
	}
	if (numZ > max)
	{
		max = numZ;
	}

	m_collisionRadius = max * (Chunk::BLOCK_RENDER_SIZE*1.75f);
	m_collisionRadius *= m_renderScale;
	m_collisionRadius *= m_pVoxelItem->GetRenderScale();
}

float Item::GetCollisionRadius()
{
	return m_collisionRadius;
}

// Interaction
void Item::SetInteractable(bool interactable)
{
	m_interactable = interactable;
}

bool Item::IsInteractable()
{
	return m_interactable;
}

bool Item::IsInteracting()
{
	return m_itemInteracting;
}

void Item::Interact()
{
	m_interactCount++;
	bool needsErase = false;
	bool spawnSubItems = false;
	bool createHitEffect = false;
	bool createCrumbleBlockParticles = false;

	if(m_interactCount < m_maxInteractCount)
	{
		bool changeItemModel = false;

		char itemFilename[64];
		if(m_itemType == eItem_CopperVein)
		{
			changeItemModel = true;
			spawnSubItems = true;
			createHitEffect = true;
			createCrumbleBlockParticles = true;
			sprintf(itemFilename, "media/gamedata/items/CopperVein/CopperVein%i.item", m_interactCount);
		}
		if(m_itemType == eItem_IronVein)
		{
			changeItemModel = true;
			spawnSubItems = true;
			createHitEffect = true;
			createCrumbleBlockParticles = true;
			sprintf(itemFilename, "media/gamedata/items/IronVein/IronVein%i.item", m_interactCount);
		}
		if(m_itemType == eItem_SilverVein)
		{
			changeItemModel = true;
			spawnSubItems = true;
			createHitEffect = true;
			createCrumbleBlockParticles = true;
			sprintf(itemFilename, "media/gamedata/items/SilverVein/SilverVein%i.item", m_interactCount);
		}
		if(m_itemType == eItem_GoldVein)
		{
			changeItemModel = true;
			spawnSubItems = true;
			createHitEffect = true;
			createCrumbleBlockParticles = true;
			sprintf(itemFilename, "media/gamedata/items/GoldVein/GoldVein%i.item", m_interactCount);
		}
		
		if(changeItemModel)
		{
			delete m_pVoxelItem;
			m_pVoxelItem = new VoxelWeapon(m_pRenderer, m_pQubicleBinaryManager);
			m_pVoxelItem->SetVoxelCharacterParent(NULL);
			m_pVoxelItem->LoadWeapon(itemFilename, false);
		}
	}
	else
	{
		if(m_itemType == eItem_CopperVein || m_itemType == eItem_IronVein || m_itemType == eItem_SilverVein || m_itemType == eItem_GoldVein)
		{
			needsErase = true;
			spawnSubItems = true;
			createHitEffect = true;
		}
	}

	// Chest open/close animation
	if(m_itemType == eItem_Chest)
	{
		m_pVoxelItem->StartSubSectionAnimation();

		if(m_itemInteracting)
		{
			m_itemInteracting = false;
		}
		else
		{
			if(m_interactCount == m_maxInteractCount)
			{
				SpawnSubItems();
			}

			m_itemInteracting = true;
		}
	}

	// Sitting in chair
	//if(m_itemType == eItem_Chair)
	//{
	//	m_pPlayer->SetPosition(m_position + vec3(0.0f, 0.01f, 0.0f));
	//	m_pPlayer->SetRotation(m_rotation.y);
	//	m_pPlayer->SetSitting(true);
	//}

	// Spawn sub items
	if(spawnSubItems)
	{
		SpawnSubItems();
	}
	
	// Crumble particle effects
	if(createCrumbleBlockParticles)
	{
		// Create some block particle effects for the crumbling ore veins
		int numCrubleBlocks = 10;
		for(int i = 0; i < numCrubleBlocks; i++)
		{
			float startScale = m_renderScale;
			float endScale = m_renderScale;
			startScale *= GetRandomNumber(90, 100, 2) * 0.01f;
			endScale *= GetRandomNumber(25, 75, 2) * 0.01f;
			float lifeTime = 4.0f + GetRandomNumber(-100, 200, 1) * 0.0075f;
			vec3 gravity = vec3(0.0f, -1.0f, 0.0f);
			vec3 pointOrigin = vec3(0.0f, 0.0f, 0.0f);
			float r = 0.49f; float g = 0.44f; float b = 0.44f; float a = 1.0f;
			vec3 spawnPos = GetCenter() + vec3(GetRandomNumber(-1, 1, 2)*0.5f, GetRandomNumber(-1, 1, 2)*0.5f, GetRandomNumber(-1, 1, 2)*0.5f);
			BlockParticle* pParticle = m_pBlockParticleManager->CreateBlockParticle(spawnPos, spawnPos, gravity, 1.5f, pointOrigin, startScale, 0.0f, endScale, 0.0f, r, g, b, a, 0.0f, 0.0f, 0.0f, 0.0f, r, g, b, a, 0.0f, 0.0f, 0.0f, 0.0f, lifeTime, 0.0f, 0.0f, 0.0f, vec3(0.0f, 3.0f, 0.0f), vec3(1.85f, 3.0f, 1.85f), vec3(0.0f, 0.0f, 0.0f), vec3(180.0f, 180.0f, 180.0f), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, vec3(0.0f, 0.0f, 0.0f), true, false, false, false, NULL);
			if(pParticle != NULL)
			{
				pParticle->m_allowFloorSliding = true;
			}
		}
	}

	// Do a hit particle effect
	if(createHitEffect)
	{
		vec3 direction = GetCenter() - m_pPlayer->GetCenter();
		vec3 hitParticlePos = GetCenter() - (normalize(direction) * (m_radius*0.5f));
		unsigned int effectId = -1;
		BlockParticleEffect* pBlockParticleEffect = VoxGame::GetInstance()->GetBlockParticleManager()->ImportParticleEffect("media/gamedata/particles/combat_hit.effect", hitParticlePos, &effectId);
		pBlockParticleEffect->PlayEffect();
	}

	if(needsErase)
	{
		// Explode the item
		Explode();

		// Erase
		SetErase(true);
	}
}

void Item::SpawnSubItems()
{
	int numItems = GetRandomNumber(2, 4);

	if(m_itemType == eItem_Chest)
	{
		numItems += 3; // Spawn more coins
	}

	for(int i = 0; i < numItems; i++)
	{
		float scale = 0.15f;
		float lifeTime = 1.0f;
		float r = 0.13f;
		float g = 0.65f;
		float b = 1.0f;
		float a = 1.0f;

		float radius = 0.5f;
		//float angle = DegToRad(((float)i/(float)numItems) * 360.0f);
		float angle = DegToRad(GetRandomNumber(0, 360, 1));
		vec3 ItemPosition = GetCenter() + vec3(cos(angle) * radius, 0.0f, sin(angle) * radius);

		vec3 gravity = vec3(0.0f, -1.0f, 0.0f);
		gravity = normalize(gravity);
		Item* pItem = NULL;
		ItemSubSpawnData *pItemSubSpawnData = m_pItemManager->GetItemSubSpawnData(m_itemType);
		if(pItemSubSpawnData != NULL)
		{
			pItem = m_pItemManager->CreateItem(GetCenter(), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), pItemSubSpawnData->m_spawnedItemFilename.c_str(), pItemSubSpawnData->m_spawnedItem, pItemSubSpawnData->m_spawnedItemTitle.c_str(), pItemSubSpawnData->m_interactable, pItemSubSpawnData->m_collectible, pItemSubSpawnData->m_scale);

			if(pItem != NULL)
			{
				pItem->SetGravityDirection(gravity);
				vec3 vel = ItemPosition - GetCenter();
				pItem->SetVelocity(normalize(vel)*(float)GetRandomNumber(2, 4, 2) + vec3(0.0f, 9.5f+GetRandomNumber(-2, 4, 2), 0.0f));
				pItem->SetRotation(vec3(0.0f, GetRandomNumber(0, 360, 2), 0.0f));
				pItem->SetAngularVelocity(vec3(0.0f, 90.0f, 0.0f));

				if(pItemSubSpawnData->m_droppedItemItem != eItem_Coin)
				{
					pItem->SetDroppedItem(pItemSubSpawnData->m_droppedItemFilename.c_str(), pItemSubSpawnData->m_droppedItemTextureFilename.c_str(), pItemSubSpawnData->m_droppedItemInventoryType, pItemSubSpawnData->m_droppedItemItem, pItemSubSpawnData->m_droppedItemStatus, pItemSubSpawnData->m_droppedItemEquipSlot, pItemSubSpawnData->m_droppedItemQuality, pItemSubSpawnData->m_droppedItemLeft, pItemSubSpawnData->m_droppedItemRight, pItemSubSpawnData->m_droppedItemTitle.c_str(), pItemSubSpawnData->m_droppedItemDescription.c_str(), pItemSubSpawnData->m_droppedItemPlacementR, pItemSubSpawnData->m_droppedItemPlacementG, pItemSubSpawnData->m_droppedItemPlacementB, pItemSubSpawnData->m_droppedItemQuantity);
				}

				pItem->SetAutoDisappear(20.0f + (GetRandomNumber(-20, 20, 1) * 0.2f));
			}
		}
	}
}

void Item::SetCurrentInteractCount(int currentInteract)
{
	m_interactCount = currentInteract;
}

int Item::GetCurrentInteractCount()
{
	return m_interactCount;
}

void Item::SetMaxtInteractCount(int maxInteract)
{
	m_maxInteractCount = maxInteract;
}

int Item::GetMaxInteractCount()
{
	return m_maxInteractCount;
}

// Update
void Item::Update(float dt)
{
	if(m_erase)
	{
		return;
	}

	if(m_pVoxelItem != NULL)
	{
		m_pVoxelItem->Update(dt);
	}

	// Update grid position
	UpdateGridPosition();

	// Update timers
	UpdateTimers(dt);

	// Update player magnet
	UpdatePlayerMagnet(dt);

	// If we don't belong to a chunk
	if(m_pCachedGridChunk == NULL)
	{
		return;
	}

	// Make sure that an owning chunk knows about us
	if(m_pOwningChunk == NULL || m_pOwningChunk->IsInsideChunk(m_position) == false)
	{
		if(m_pOwningChunk != NULL)
		{
			m_pOwningChunk->RemoveItem(this);
		}

		m_pOwningChunk = m_pChunkManager->GetChunkFromPosition(m_position.x, m_position.y, m_position.z);

		if(m_pOwningChunk != NULL)
		{
			m_pOwningChunk->AddItem(this);
		}
		else
		{
			//SetErase(true);
		}

		return;
	}

	// Auto disappear
	if(m_autoDisappear)
	{
		if(m_autoDisappearTimer <= 0.0f)
		{
			SetErase(true);

			return;
		}
	}

	// Update physics
	UpdatePhysics(dt);
}

void Item::UpdatePhysics(float dt)
{
	vec3 acceleration = (m_gravityDirection * 9.81f) * 4.0f;

	// Integrate velocity
	m_velocity += acceleration * dt;

	// Integrate angular velocity and rotation
	vec3 angularAcceleration(0.0f, 0.0f, 0.0f);
	m_angularVelocity += angularAcceleration * dt;
	m_rotation += m_angularVelocity * dt;

	if (m_worldCollide)
	{
		int blockX, blockY, blockZ;
		vec3 blockPos;

		// Check collision
		{
			vec3 velocityToUse = m_velocity;
			vec3 velAmount = velocityToUse*dt;
			vec3 pNormal;
			int numberDivision = 1;
			while (length(velAmount) >= 1.0f)
			{
				numberDivision++;
				velAmount = velocityToUse*(dt / numberDivision);
			}
			for (int i = 0; i < numberDivision; i++)
			{
				float dtToUse = (dt / numberDivision) + ((dt / numberDivision) * i);
				vec3 posToCheck = GetCenter() + velocityToUse*dtToUse;
				bool stepUp = false;
				if (CheckCollisions(posToCheck, m_previousPosition, &pNormal, &velAmount))
				{
					// Reset velocity, we don't have any bounce
					m_velocity = vec3(0.0f, 0.0f, 0.0f);
					velocityToUse = vec3(0.0f, 0.0f, 0.0f);
				}
			}

			// Integrate position
			m_position += velocityToUse * dt;
		}

		// Owning chunks
		if (m_pOwningChunk != NULL && m_pOwningChunk->IsSetup() && m_pOwningChunk->IsInsideChunk(m_position))
		{
			Chunk* pChunk = GetCachedGridChunkOrFromPosition(m_position);
			bool active = m_pChunkManager->GetBlockActiveFrom3DPosition(m_position.x, m_position.y, m_position.z, &blockPos, &blockX, &blockY, &blockZ, &pChunk);

			if (active == true)
			{
				// Roll back the integration, since we will intersect the block otherwise
				m_position -= m_velocity * dt;

				m_velocity = vec3(0.0f, 0.0f, 0.0f);
			}
		}
		else
		{
			if (m_pOwningChunk != NULL)
			{
				m_pOwningChunk->RemoveItem(this);
			}

			m_pOwningChunk = m_pChunkManager->GetChunkFromPosition(m_position.x, m_position.y, m_position.z);

			if (m_pOwningChunk != NULL)
			{
				m_pOwningChunk->AddItem(this);
			}

			if (m_pOwningChunk == NULL)
			{
				m_position -= m_velocity * dt;
				m_velocity = vec3(0.0f, 0.0f, 0.0f);
			}
		}
	}
	else
	{
		// Integrate position
		m_position += m_velocity * dt;
	}

	m_previousPosition = GetCenter();
}

void Item::UpdateTimers(float dt)
{
	if(m_isCollectible)
	{
		m_collectionDelay -= dt;
	}

	// Disappear for pickup
	if(m_disappear)
	{
		if(m_disappearTimer > 0.0f)
		{
			m_disappearTimer -= dt;
		}
	}

	// Auto disappear
	if(m_autoDisappear)
	{
		if(m_autoDisappearTimer > 0.0f)
		{
			m_autoDisappearTimer -= dt;
		}
	}
}

void Item::UpdatePlayerMagnet(float dt)
{
	if(IsItemPickedUp())
	{
		if(m_disappear)
		{
			m_renderScale = m_disappearScale;

			if(m_disappearTimer <= 0.0f)
			{
				if(m_disappearAnimationStarted == false)
				{
					Interpolator::GetInstance()->AddFloatInterpolation(&m_disappearScale, m_disappearScale, 0.0f, 0.5f, -100.0f, NULL, _PickupAnimationFinished, this);

					m_disappearAnimationStarted = true;
				}
			}
		}
		else
		{
			vec3 diff = m_pickupPos - m_position;
			float lengthSize = length(diff);

			if(lengthSize < 0.01f)
			{
				m_disappear = true;
				m_disappearTimer = m_disappearDelay;
				m_disappearScale = m_renderScale;
			}
			else
			{
				diff = normalize(diff);

				m_position += diff * lengthSize*0.25f;
			}
		}
	}
	else
	{
		if(IsCollectible())
		{
			if(m_droppedInventoryItem == NULL || m_pInventoryManager->CanAddInventoryItem(GetItemTitle(), GetItemType(), 1))
			{
				float yAdditionalMaagnetOffset = 0.5f;

				// Magnet towards the player
				if(m_pPlayer->IsDead() == false && length(m_pPlayer->GetCenter() - GetCenter()) < m_pPlayer->GetRadius() + 4.0f)
				{
					vec3 toPlayer = (m_pPlayer->GetCenter()+vec3(0.0f, yAdditionalMaagnetOffset, 0.0f)) - GetCenter();
					SetGravityDirection(toPlayer);
					SetVelocity(normalize(toPlayer)*(20.0f/length(toPlayer)));
					SetWorldCollide(false);
				}
				else
				{
					SetGravityDirection(vec3(0.0f, -1.0f, 0.0f));
					SetWorldCollide(true);
				}

				// Check if we have been picked up by the player
				if(m_pPlayer->IsDead() == false && length((m_pPlayer->GetCenter()+vec3(0.0f, yAdditionalMaagnetOffset, 0.0f)) - GetCenter()) < m_pPlayer->GetRadius())
				{
					SetPickupGotoPosition(m_pPlayer->GetCenter() + vec3(0.0f, 2.5f, 0.0f));
					SetVelocity(vec3(0.0f, 0.0f, 0.0f));
					SetGravityDirection(vec3(0.0f, 0.0f, 0.0f));

					if(m_itemType == eItem_Heart)
					{
						m_pPlayer->GiveHealth(10.0f);
					}

					if(m_itemType == eItem_Coin)
					{
						m_pPlayer->GiveCoins(1);
					}

					if(m_droppedInventoryItem != NULL)
					{
						m_pInventoryManager->AddInventoryItem(m_droppedInventoryItem, -1, -1);
					}
				}
			}
		}
	}
}

void Item::UpdateItemLights(float dt)
{
	if(m_erase)
	{
		return;
	}

	if(m_pVoxelItem != NULL)
	{
		for(int i = 0; i < m_pVoxelItem->GetNumLights(); i++)
		{
			unsigned int lightId;
			vec3 lightPos;
			float lightRadius;
			float lightDiffuseMultiplier;
			Colour lightColour;
			bool connectedToSegment;
			m_pVoxelItem->GetLightParams(i, &lightId, &lightPos, &lightRadius, &lightDiffuseMultiplier, &lightColour, &connectedToSegment);

			if(lightId == -1)
			{
				m_pLightingManager->AddLight(vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f, 1.0f), &lightId);
				m_pVoxelItem->SetLightingId(i, lightId);
			}

			if(connectedToSegment == false)
			{
				lightPos *= m_renderScale;

				// Rotate due to characters forward vector
				float rotationAngle = acos(dot(vec3(0.0f, 0.0f, 1.0f), m_forward));
				if(m_forward.x < 0.0f)
				{
					rotationAngle = -rotationAngle;
				}
				Matrix4x4 rotationMatrix;
				rotationMatrix.SetRotation(0.0f, rotationAngle, 0.0f);
				lightPos = rotationMatrix * lightPos;

				// Translate to position
				lightPos += m_position;
			}

			m_pLightingManager->UpdateLightPosition(lightId, lightPos);
			m_pLightingManager->UpdateLightRadius(lightId, lightRadius * m_renderScale);
			m_pLightingManager->UpdateLightDiffuseMultiplier(lightId, lightDiffuseMultiplier);
			m_pLightingManager->UpdateLightColour(lightId, lightColour);
		}
	}
}

void Item::UpdateItemParticleEffects(float dt)
{
	if(m_erase)
	{
		return;
	}

	if(m_pVoxelItem != NULL)
	{
		for(int i = 0; i < m_pVoxelItem->GetNumParticleEffects(); i++)
		{
			unsigned int particleEffectId;
			vec3 ParticleEffectPos;
			string effectName;
			bool connectedToSegment;
			m_pVoxelItem->GetParticleEffectParams(i, &particleEffectId, &ParticleEffectPos, &effectName, &connectedToSegment);

			if(particleEffectId == -1)
			{
				m_pBlockParticleManager->ImportParticleEffect(effectName, ParticleEffectPos, &particleEffectId);
				m_pVoxelItem->SetParticleEffectId(i, particleEffectId);
			}

			if(connectedToSegment == false)
			{
				ParticleEffectPos *= m_renderScale;

				// Rotate due to characters forward vector
				float rotationAngle = acos(dot(vec3(0.0f, 0.0f, 1.0f), m_forward));
				if(m_forward.x < 0.0f)
				{
					rotationAngle = -rotationAngle;
				}
				Matrix4x4 rotationMatrix;
				rotationMatrix.SetRotation(0.0f, rotationAngle, 0.0f);
				ParticleEffectPos = rotationMatrix * ParticleEffectPos;

				// Translate to position
				ParticleEffectPos += m_position;
			}

			m_pBlockParticleManager->UpdateParticleEffectPosition(particleEffectId, ParticleEffectPos, ParticleEffectPos);
		}
	}
}

// Rendering
void Item::Render(bool outline, bool reflection, bool silhouette)
{
	if(m_erase == true)
	{
		return;
	}

	if(m_pOwningChunk == NULL || m_pOwningChunk->IsUnloading())
	{
		return;
	}

	if(m_pVoxelItem != NULL)
	{
		Colour OutlineColour(1.0f, 1.0f, 0.0f, 1.0f);

		m_pRenderer->PushMatrix();
			m_pRenderer->MultiplyWorldMatrix(m_worldMatrix);
			m_pRenderer->ScaleWorldMatrix(m_renderScale, m_renderScale, m_renderScale);
			m_pVoxelItem->Render(outline, reflection, silhouette, OutlineColour);
		m_pRenderer->PopMatrix();
	}
}

void Item::RenderDebug()
{
	if(m_erase == true)
	{
		return;
	}

	if(m_pOwningChunk == NULL || m_pOwningChunk->IsUnloading())
	{
		return;
	}

	m_pRenderer->PushMatrix();
		m_pRenderer->TranslateWorldMatrix(GetCenter().x, GetCenter().y, GetCenter().z);

		m_pRenderer->PushMatrix();
			m_pRenderer->SetLineWidth(1.0f);
			m_pRenderer->SetRenderMode(RM_WIREFRAME);
			m_pRenderer->RotateWorldMatrix(GetRotation().x, GetRotation().y, GetRotation().z);

			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
			m_pRenderer->DrawSphere(m_radius, 20, 20);

			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
			m_pRenderer->DrawSphere(m_collisionRadius, 20, 20);
		m_pRenderer->PopMatrix();
	m_pRenderer->PopMatrix();

	// Render interaction point
	m_pRenderer->PushMatrix();
		float l_length = 0.5f;
		m_pRenderer->TranslateWorldMatrix(GetInteractionPosition().x, GetInteractionPosition().y, GetInteractionPosition().z);

		m_pRenderer->SetRenderMode(RM_SOLID);		
		m_pRenderer->SetLineWidth(2.0f);
		m_pRenderer->EnableImmediateMode(IM_LINES);
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
			m_pRenderer->ImmediateVertex(-l_length, 0.0f, 0.0f);
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
			m_pRenderer->ImmediateVertex(l_length, 0.0f, 0.0f);
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, -l_length, 0.0f);
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, l_length, 0.0f);
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, -l_length);
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, l_length);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();
	
	// Render link to owning chunk
	if(m_pOwningChunk != NULL)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->EnableImmediateMode(IM_LINES);
			m_pRenderer->ImmediateVertex(m_position.x, m_position.y, m_position.z);
			m_pRenderer->ImmediateVertex(m_pOwningChunk->GetPosition().x + Chunk::BLOCK_RENDER_SIZE*Chunk::CHUNK_SIZE, m_pOwningChunk->GetPosition().y + Chunk::BLOCK_RENDER_SIZE*Chunk::CHUNK_SIZE, m_pOwningChunk->GetPosition().z + Chunk::BLOCK_RENDER_SIZE*Chunk::CHUNK_SIZE);
			m_pRenderer->DisableImmediateMode();
		m_pRenderer->PopMatrix();
	}

	// Render collision regions
	RenderCollisionRegions();
}

void Item::RenderCollisionRegions()
{
	for(unsigned int i = 0; i < m_vpBoundingRegionList.size(); i++)
	{
		BoundingRegion* pRegion = m_vpBoundingRegionList[i];

		m_pRenderer->PushMatrix();
			m_pRenderer->TranslateWorldMatrix(GetCenter().x, GetCenter().y, GetCenter().z);

			Matrix4x4 justParentRotation;
			justParentRotation.SetRotation(DegToRad(m_rotation.x), DegToRad(m_rotation.y), DegToRad(m_rotation.z));
			m_pRenderer->MultiplyWorldMatrix(justParentRotation);

			m_pRenderer->ScaleWorldMatrix(m_renderScale, m_renderScale, m_renderScale);
			pRegion->Render(m_pRenderer);
		m_pRenderer->PopMatrix();
	}
}

void Item::_PickupAnimationFinished(void *apData)
{
	Item* lpItem = (Item*)apData;
	lpItem->PickupAnimationFinished();
}

void Item::PickupAnimationFinished()
{
	SetErase(true);
}
