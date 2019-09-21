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

#include "ItemManager.h"
#include "ItemSpawner.h"
#include "../utils/Random.h"
#include "../Lighting/LightingManager.h"
#include "../VoxGame.h"

#include <algorithm>

float ItemManager::ITEM_INTERACTION_DISTANCE = 2.5f;
float ItemManager::ITEM_INERACCTION_RADIUS_CHECK = 0.75f;


ItemManager::ItemManager(Renderer* pRenderer, ChunkManager* pChunkManager, Player* pPlayer)
{
	m_pRenderer = pRenderer;
	m_pChunkManager = pChunkManager;
	m_pPlayer = pPlayer;
	m_pQubicleBinaryManager = nullptr;
	m_pInventoryManager = nullptr;
	m_pLightingManager = nullptr;
	m_pBlockParticleManager = nullptr;

	m_numRenderItems = 0;

	// Chest
	AddItemSubSpawnData(eItem_Chest, BlockType_Default, eItem_Coin, GetItemFilenameForType(eItem_Coin), GetItemTitleForType(eItem_Coin), false, true, 0.0225f,
		GetItemFilenameForType(eItem_Coin), GetItemTextureForType(eItem_Coin), InventoryType_Item, eItem_Coin, ItemStatus_None, EquipSlot_NoSlot, ItemQuality_Common, false, false, GetItemTitleForType(eItem_Coin), GetItemDescriptionForType(eItem_Coin), 1.0f, 1.0f, 1.0f, 1);

	// Ores
	AddItemSubSpawnData(eItem_CopperVein, BlockType_Default, eItem_CopperOre, GetItemFilenameForType(eItem_CopperOre), GetItemTitleForType(eItem_CopperOre), false, true, 0.08f,
		GetItemFilenameForType(eItem_CopperOre), GetItemTextureForType(eItem_CopperOre), InventoryType_Item, eItem_CopperOre, ItemStatus_None, EquipSlot_NoSlot, ItemQuality_Common, false, false, GetItemTitleForType(eItem_CopperOre), GetItemDescriptionForType(eItem_CopperOre), 1.0f, 1.0f, 1.0f, 1);
	AddItemSubSpawnData(eItem_IronVein, BlockType_Default, eItem_IronOre, GetItemFilenameForType(eItem_IronOre), GetItemTitleForType(eItem_IronOre), false, true, 0.08f,
		GetItemFilenameForType(eItem_IronOre), GetItemTextureForType(eItem_IronOre), InventoryType_Item, eItem_IronOre, ItemStatus_None, EquipSlot_NoSlot, ItemQuality_Common, false, false, GetItemTitleForType(eItem_IronOre), GetItemDescriptionForType(eItem_IronOre), 1.0f, 1.0f, 1.0f, 1);
	AddItemSubSpawnData(eItem_SilverVein, BlockType_Default, eItem_SilverOre, GetItemFilenameForType(eItem_SilverOre), GetItemTitleForType(eItem_SilverOre), false, true, 0.08f,
		GetItemFilenameForType(eItem_SilverOre), GetItemTextureForType(eItem_SilverOre), InventoryType_Item, eItem_SilverOre, ItemStatus_None, EquipSlot_NoSlot, ItemQuality_Common, false, false, GetItemTitleForType(eItem_SilverOre), GetItemDescriptionForType(eItem_SilverOre), 1.0f, 1.0f, 1.0f, 1);
	AddItemSubSpawnData(eItem_GoldVein, BlockType_Default, eItem_GoldOre, GetItemFilenameForType(eItem_GoldOre), GetItemTitleForType(eItem_GoldOre), false, true, 0.08f,
		GetItemFilenameForType(eItem_GoldOre), GetItemTextureForType(eItem_GoldOre), InventoryType_Item, eItem_GoldOre, ItemStatus_None, EquipSlot_NoSlot, ItemQuality_Common, false, false, GetItemTitleForType(eItem_GoldOre), GetItemDescriptionForType(eItem_GoldOre), 1.0f, 1.0f, 1.0f, 1);

	// Blocks
	AddItemSubSpawnData(eItem_None, BlockType_Grass, eItem_Block_Grass, GetItemFilenameForType(eItem_Block_Grass), GetItemTitleForType(eItem_Block_Grass), false, true, 0.06f,
		GetItemFilenameForType(eItem_Block_Grass), GetItemTextureForType(eItem_Block_Grass), InventoryType_Block, eItem_Block_Grass, ItemStatus_None, EquipSlot_RightHand, ItemQuality_Common, false, false, GetItemTitleForType(eItem_Block_Grass), GetItemDescriptionForType(eItem_Block_Grass), 0.23f, 0.13f, 0.02f, 1);
	AddItemSubSpawnData(eItem_None, BlockType_Dirt, eItem_Block_Dirt, GetItemFilenameForType(eItem_Block_Dirt), GetItemTitleForType(eItem_Block_Dirt), false, true, 0.06f,
		GetItemFilenameForType(eItem_Block_Dirt), GetItemTextureForType(eItem_Block_Dirt), InventoryType_Block, eItem_Block_Dirt, ItemStatus_None, EquipSlot_RightHand, ItemQuality_Common, false, false, GetItemTitleForType(eItem_Block_Dirt), GetItemDescriptionForType(eItem_Block_Dirt), 0.23f, 0.13f, 0.02f, 1);
	AddItemSubSpawnData(eItem_None, BlockType_Stone, eItem_Block_Stone, GetItemFilenameForType(eItem_Block_Stone), GetItemTitleForType(eItem_Block_Stone), false, true, 0.06f,
		GetItemFilenameForType(eItem_Block_Stone), GetItemTextureForType(eItem_Block_Stone), InventoryType_Block, eItem_Block_Stone, ItemStatus_None, EquipSlot_RightHand, ItemQuality_Common, false, false, GetItemTitleForType(eItem_Block_Stone), GetItemDescriptionForType(eItem_Block_Stone), 0.23f, 0.13f, 0.02f, 1);
	AddItemSubSpawnData(eItem_None, BlockType_Wood, eItem_Block_Wood, GetItemFilenameForType(eItem_Block_Wood), GetItemTitleForType(eItem_Block_Wood), false, true, 0.06f,
		GetItemFilenameForType(eItem_Block_Wood), GetItemTextureForType(eItem_Block_Wood), InventoryType_Block, eItem_Block_Wood, ItemStatus_None, EquipSlot_RightHand, ItemQuality_Common, false, false, GetItemTitleForType(eItem_Block_Wood), GetItemDescriptionForType(eItem_Block_Wood), 0.23f, 0.13f, 0.02f, 1);
	AddItemSubSpawnData(eItem_None, BlockType_Leaf, eItem_Block_Leaf, GetItemFilenameForType(eItem_Block_Leaf), GetItemTitleForType(eItem_Block_Leaf), false, true, 0.06f,
		GetItemFilenameForType(eItem_Block_Leaf), GetItemTextureForType(eItem_Block_Leaf), InventoryType_Block, eItem_Block_Leaf, ItemStatus_None, EquipSlot_RightHand, ItemQuality_Common, false, false, GetItemTitleForType(eItem_Block_Leaf), GetItemDescriptionForType(eItem_Block_Leaf), 0.23f, 0.13f, 0.02f, 1);
	AddItemSubSpawnData(eItem_None, BlockType_Sand, eItem_Block_Sand, GetItemFilenameForType(eItem_Block_Sand), GetItemTitleForType(eItem_Block_Sand), false, true, 0.06f,
		GetItemFilenameForType(eItem_Block_Sand), GetItemTextureForType(eItem_Block_Sand), InventoryType_Block, eItem_Block_Sand, ItemStatus_None, EquipSlot_RightHand, ItemQuality_Common, false, false, GetItemTitleForType(eItem_Block_Sand), GetItemDescriptionForType(eItem_Block_Sand), 0.23f, 0.13f, 0.02f, 1);
	AddItemSubSpawnData(eItem_None, BlockType_Cactus, eItem_Block_Cactus, GetItemFilenameForType(eItem_Block_Cactus), GetItemTitleForType(eItem_Block_Cactus), false, true, 0.06f,
		GetItemFilenameForType(eItem_Block_Cactus), GetItemTextureForType(eItem_Block_Cactus), InventoryType_Block, eItem_Block_Cactus, ItemStatus_None, EquipSlot_RightHand, ItemQuality_Common, false, false, GetItemTitleForType(eItem_Block_Cactus), GetItemDescriptionForType(eItem_Block_Cactus), 0.23f, 0.13f, 0.02f, 1);
	AddItemSubSpawnData(eItem_None, BlockType_Rock, eItem_Block_Rock, GetItemFilenameForType(eItem_Block_Rock), GetItemTitleForType(eItem_Block_Rock), false, true, 0.06f,
		GetItemFilenameForType(eItem_Block_Rock), GetItemTextureForType(eItem_Block_Rock), InventoryType_Block, eItem_Block_Rock, ItemStatus_None, EquipSlot_RightHand, ItemQuality_Common, false, false, GetItemTitleForType(eItem_Block_Rock), GetItemDescriptionForType(eItem_Block_Rock), 0.23f, 0.13f, 0.02f, 1);
	AddItemSubSpawnData(eItem_None, BlockType_Snow, eItem_Block_Snow, GetItemFilenameForType(eItem_Block_Snow), GetItemTitleForType(eItem_Block_Snow), false, true, 0.06f,
		GetItemFilenameForType(eItem_Block_Snow), GetItemTextureForType(eItem_Block_Snow), InventoryType_Block, eItem_Block_Snow, ItemStatus_None, EquipSlot_RightHand, ItemQuality_Common, false, false, GetItemTitleForType(eItem_Block_Snow), GetItemDescriptionForType(eItem_Block_Snow), 1.0f, 1.0f, 1.0f, 1);
}

ItemManager::~ItemManager()
{
	ClearItems();
	ClearSubSpawnData();
	ClearItemSpawners();
}

void ItemManager::SetLightingManager(LightingManager* pLightingManager)
{
	m_pLightingManager = pLightingManager;
}

void ItemManager::SetBlockParticleManager(BlockParticleManager* pBlockParticleManager)
{
	m_pBlockParticleManager = pBlockParticleManager;
}

void ItemManager::SetQubicleBinaryManager(QubicleBinaryManager* pQubicleBinaryManager)
{
	m_pQubicleBinaryManager = pQubicleBinaryManager;
}

void ItemManager::SetInventoryManager(InventoryManager* pInventoryManager)
{
	m_pInventoryManager = pInventoryManager;
}

void ItemManager::SetNPCManager(NPCManager* pNPCManager)
{
	m_pNPCManager = pNPCManager;
}

// Deletion
void ItemManager::ClearItems()
{
	for(unsigned int i = 0; i < m_vpItemList.size(); i++)
	{
		delete m_vpItemList[i];
		m_vpItemList[i] = 0;
	}
	m_vpItemList.clear();
}

void ItemManager::ClearSubSpawnData()
{
	for(unsigned int i = 0; i < m_vpItemSubSpawnDataList.size(); i++)
	{
		delete m_vpItemSubSpawnDataList[i];
		m_vpItemSubSpawnDataList[i] = 0;
	}
	m_vpItemSubSpawnDataList.clear();
}

void ItemManager::ClearItemSpawners()
{
	for (unsigned int i = 0; i < m_vpItemSpawnerList.size(); i++)
	{
		delete m_vpItemSpawnerList[i];
		m_vpItemSpawnerList[i] = 0;
	}
	m_vpItemSpawnerList.clear();
}

void ItemManager::RemoveItemSpawnerFromItems(ItemSpawner* pSpawner)
{
	for (unsigned int i = 0; i < m_vpItemList.size(); i++)
	{
		m_vpItemList[i]->RemoveItemSpawner(pSpawner);
	}
}

// Item sub spawn data
void ItemManager::AddItemSubSpawnData(eItem sourceItem, BlockType sourceBlockType, eItem spawnedItem, string spawnedItemFilename, string spawnedItemTitle, bool interactable, bool collectible,float scale,
	string droppedItemFilename, string droppedItemTextureFilename, InventoryType droppedItemInventoryType, eItem droppedItemItem, ItemStatus droppedItemStatus, EquipSlot droppedItemEquipSlot, ItemQuality droppedItemQuality,
	bool droppedItemLeft, bool droppedItemRight, string droppedItemTitle, string droppedItemDescription, float droppedItemPlacementR, float droppedItemPlacementG, float droppedItemPlacementB, int droppedItemQuantity)
{
	ItemSubSpawnData* pItemSubSpawnData = new ItemSubSpawnData();

	pItemSubSpawnData->m_sourceItem = sourceItem;
	pItemSubSpawnData->m_sourceBlockType = sourceBlockType;

	pItemSubSpawnData->m_spawnedItem = spawnedItem;
	pItemSubSpawnData->m_spawnedItemFilename = spawnedItemFilename;
	pItemSubSpawnData->m_spawnedItemTitle = spawnedItemTitle;
	pItemSubSpawnData->m_collectible = collectible;
	pItemSubSpawnData->m_interactable = interactable;
	pItemSubSpawnData->m_scale = scale;

	pItemSubSpawnData->m_droppedItemFilename = droppedItemFilename;
	pItemSubSpawnData->m_droppedItemTextureFilename = droppedItemTextureFilename;
	pItemSubSpawnData->m_droppedItemInventoryType = droppedItemInventoryType;
	pItemSubSpawnData->m_droppedItemItem = droppedItemItem;
	pItemSubSpawnData->m_droppedItemStatus = droppedItemStatus;
	pItemSubSpawnData->m_droppedItemEquipSlot = droppedItemEquipSlot;
	pItemSubSpawnData->m_droppedItemQuality = droppedItemQuality;
	pItemSubSpawnData->m_droppedItemLeft = droppedItemLeft;
	pItemSubSpawnData->m_droppedItemRight = droppedItemRight;
	pItemSubSpawnData->m_droppedItemTitle = droppedItemTitle;
	pItemSubSpawnData->m_droppedItemDescription = droppedItemDescription;
	pItemSubSpawnData->m_droppedItemPlacementR = droppedItemPlacementR;
	pItemSubSpawnData->m_droppedItemPlacementG = droppedItemPlacementG;
	pItemSubSpawnData->m_droppedItemPlacementB = droppedItemPlacementB;
	pItemSubSpawnData->m_droppedItemQuantity = droppedItemQuantity;

	m_vpItemSubSpawnDataList.push_back(pItemSubSpawnData);
}

ItemSubSpawnData* ItemManager::GetItemSubSpawnData(eItem sourceItem)
{
	for(unsigned int i = 0; i < m_vpItemSubSpawnDataList.size(); i++)
	{
		if(m_vpItemSubSpawnDataList[i]->m_sourceItem == sourceItem)
		{
			return m_vpItemSubSpawnDataList[i];
		}
	}

	return nullptr;
}

ItemSubSpawnData* ItemManager::GetItemSubSpawnData(BlockType sourceBlockType)
{
	for(unsigned int i = 0; i < m_vpItemSubSpawnDataList.size(); i++)
	{
		if(m_vpItemSubSpawnDataList[i]->m_sourceBlockType == sourceBlockType)
		{
			return m_vpItemSubSpawnDataList[i];
		}
	}

	return nullptr;
}

// Creation
Item* ItemManager::CreateItem(vec3 position, vec3 velocity, vec3 rotation, const char* objectFilename, eItem itemType, const char* itemTitle, bool interactable, bool collectible, float scale)
{
	Item* pNewItem = new Item(m_pRenderer, m_pChunkManager, m_pQubicleBinaryManager, itemTitle, itemType, scale);
	pNewItem->LoadItem(objectFilename);

	pNewItem->SetLightingManager(m_pLightingManager);
	pNewItem->SetBlockParticleManager(m_pBlockParticleManager);
	pNewItem->SetPlayer(m_pPlayer);
	pNewItem->SetItemManager(this);
	pNewItem->SetInventoryManager(m_pInventoryManager);

	vec3 gravityDir = vec3(0.0f, -1.0f, 0.0f);
	pNewItem->SetPosition(position);
	pNewItem->SetRotation(rotation);
	pNewItem->SetGravityDirection(gravityDir);
	pNewItem->SetVelocity(velocity);

	pNewItem->SetIsCollectible(collectible);
	pNewItem->SetCollectionDelay(GetRandomNumber(1, 2, 2));

	pNewItem->SetInteractable(interactable);

	m_vpItemList.push_back(pNewItem);

	return pNewItem;
}

void ItemManager::RemoveItem(const char* itemTitle)
{
	for(unsigned int i = 0; i < m_vpItemList.size(); i++)
	{
		if(m_vpItemList[i]->NeedsErasing())
		{
			continue;
		}

		if(strcmp(m_vpItemList[i]->GetItemTitle(), itemTitle) == 0)
		{
			m_vpItemList[i]->SetErase(true);
		}
	}
}

ItemSpawner* ItemManager::CreateItemSpawner(vec3 position, vec3 direction)
{
	ItemSpawner* pItemSpawner = new ItemSpawner(m_pRenderer, m_pChunkManager, m_pPlayer, this, m_pNPCManager);

	pItemSpawner->SetPosition(position);
	pItemSpawner->SetInitialPosition(position);
	pItemSpawner->SetFacingDirection(normalize(direction));

	m_vpItemSpawnerList.push_back(pItemSpawner);

	return pItemSpawner;
}

// Get number of items
int ItemManager::GetNumItems()
{
	int numItems =  (int)m_vpItemList.size();

	return numItems;
}

int ItemManager::GetNumRenderItems()
{
	return m_numRenderItems;
}

// Collision detection
bool ItemManager::CheckCollisions(vec3 center, vec3 previousCenter, float radius, vec3 *pNormal, vec3 *pMovement)
{
	bool colliding = false;
	for(unsigned int i = 0; i < m_vpItemList.size() && colliding == false; i++)
	{
		//if(m_pChunkManager->IsInsideLoader(m_vpItemList[i]->GetCenter()) == false)
		//{
		//	continue;
		//}

		if(m_vpItemList[i]->IsCollisionEnabled())
		{
			vec3 toPlayer = m_vpItemList[i]->GetCenter() - m_pPlayer->GetCenter();

			if(length(toPlayer) < radius + m_vpItemList[i]->GetCollisionRadius())
			{
				m_vpItemList[i]->CalculateWorldTransformMatrix();

				if(m_vpItemList[i]->IsColliding(center, previousCenter, radius, pNormal, pMovement))
				{
					colliding = true;
				}
			}
		}
	}

	return colliding;
}

// Player item interaction
Item* ItemManager::CheckItemPlayerInteraction()
{
	Item* pInteractingItem = nullptr;
	float maxDistance = 99.9f;
	float maxDotProduct = 0.0f;

	// Check if any item are within interaction range
	ItemList::iterator item_iterator;
	for(item_iterator = m_vpItemList.begin(); item_iterator != m_vpItemList.end(); ++item_iterator)
	{
		Item* pItem = (*item_iterator);

		if(pItem->NeedsErasing())
		{
			continue;
		}

		if(pItem->IsInteractable() == false)
		{
			// Don't test non-interactable items
			continue;
		}

		vec3 distance = pItem->GetCenter() - m_pPlayer->GetCenter();
		vec3 direction = normalize(distance);
		vec3 facing = m_pPlayer->GetForwardVector();

		distance.y = 0.0f;
		float lengthSize = length(distance);

		//if(lengthSize < maxDistance)
		{
			float dotProduct = dot(direction, facing);

			if(lengthSize > pItem->GetRadius() && lengthSize < ITEM_INTERACTION_DISTANCE + pItem->GetRadius())
			{
				// Make sure we are facing
				if(dotProduct > ITEM_INERACCTION_RADIUS_CHECK)
				{
					if(dotProduct > maxDotProduct)
					{
						maxDotProduct = dotProduct;
						//maxDistance = lengthSize;
						pInteractingItem = pItem;
					}
				}
			}
		}
	}

	return pInteractingItem;
}

bool needs_erasing(Item* aI)
{
	bool needsErase = aI->NeedsErasing();

	if(needsErase == true)
	{
		delete aI;
	}

	return needsErase;
}

// Rendering Helpers
void ItemManager::CalculateWorldTransformMatrix()
{
	for (unsigned int i = 0; i < m_vpItemList.size(); i++)
	{
		Item* pItem = m_vpItemList[i];

		pItem->CalculateWorldTransformMatrix();
	}
}

void ItemManager::SetWireFrameRender(bool wireframe)
{
	for(unsigned int i = 0; i < m_vpItemList.size(); i++)
	{
		Item* pItem = m_vpItemList[i];

		pItem->SetWireFrameRender(wireframe);
	}
}

// Update
void ItemManager::Update(float dt)
{
	// Update all item spawners
	for (unsigned int i = 0; i < m_vpItemSpawnerList.size(); i++)
	{
		ItemSpawner* pItemSpawner = m_vpItemSpawnerList[i];

		if (pItemSpawner->ShouldFollowPlayer())
		{
			pItemSpawner->SetPosition(m_pPlayer->GetCenter() + pItemSpawner->GetInitialPosition());
		}

		// TODO : Loader radius culling
		//if(m_pChunkManager->IsInsideLoader(pItemSpawner->GetPosition()) == false)
		//{
		//	continue;
		//}

		pItemSpawner->Update(dt);
	}

	// Remove any items that need to be erased
	m_vpItemList.erase( remove_if(m_vpItemList.begin(), m_vpItemList.end(), needs_erasing), m_vpItemList.end() );

	UpdateHoverItems();

	for(unsigned int i = 0; i < m_vpItemList.size(); i++)
	{
		Item* pItem = m_vpItemList[i];

		if(pItem->NeedsErasing())
		{
			continue;
		}

		pItem->Update(dt);
	}
}

void ItemManager::UpdateItemLights(float dt)
{
	for(unsigned int i = 0; i < m_vpItemList.size(); i++)
	{
		Item* pItem = m_vpItemList[i];

		if(pItem->NeedsErasing())
		{
			continue;
		}

		pItem->UpdateItemLights(dt);
	}
}

void ItemManager::UpdateItemParticleEffects(float dt)
{
	for(unsigned int i = 0; i < m_vpItemList.size(); i++)
	{
		Item* pItem = m_vpItemList[i];

		if(pItem->NeedsErasing())
		{
			continue;
		}

		pItem->UpdateItemParticleEffects(dt);
	}
}

void ItemManager::UpdateHoverItems()
{
	Item* pHoverItem = nullptr;
	float maxDistance = 99.9f;
	float maxDotProduct = 0.0f;

	// Check if any item are within interaction range
	ItemList::iterator item_iterator;
	for(item_iterator = m_vpItemList.begin(); item_iterator != m_vpItemList.end(); ++item_iterator)
	{
		Item* pItem = (*item_iterator);

		if(pItem->NeedsErasing())
		{
			continue;
		}

		pItem->SetOutlineRender(false);

		if(pItem->IsInteractable() == false)
		{
			// Don't hover highlight non-interactable items
			continue;
		}
		if(pItem->IsCollectible() == true)
		{
			// Don't hover highlight collectible items
			continue;
		}
		
		vec3 distance = pItem->GetCenter() - m_pPlayer->GetCenter();
		vec3 direction = normalize(distance);
		vec3 facing = m_pPlayer->GetForwardVector();

		distance.y = 0.0f;
		float lengthSize = length(distance);

		//if(length < maxDistance)
		{
			float dotProduct = dot(direction, facing);

			if(lengthSize > pItem->GetRadius() && lengthSize < ITEM_INTERACTION_DISTANCE + pItem->GetRadius())
			{
				// Make sure we are facing
				if(dotProduct > ITEM_INERACCTION_RADIUS_CHECK)
				{
					if(dotProduct > maxDotProduct)
					{
						maxDotProduct = dotProduct;
						//maxDistance = lengthSize;
						pHoverItem = pItem;
					}
				}
			}
		}
	}

	if(pHoverItem != nullptr)
	{
		pHoverItem->SetOutlineRender(true);
	}
}

// Rendering
void ItemManager::Render(bool outline, bool reflection, bool silhouette, bool shadow)
{
	m_numRenderItems = 0;

	for(unsigned int i = 0; i < m_vpItemList.size(); i++)
	{
		Item* pItem = m_vpItemList[i];

		if(pItem->NeedsErasing())
		{
			continue;
		}

		if((outline || silhouette) && pItem->IsOutlineRender() == false)
		{
			continue;
		}

		// Fog
		float toCamera = length(VoxGame::GetInstance()->GetGameCamera()->GetPosition() - pItem->GetCenter());
		if(toCamera > m_pChunkManager->GetLoaderRadius() + (Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE*5.0f))
		{
			continue;
		}
		if(toCamera > m_pChunkManager->GetLoaderRadius() - Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE*3.0f)
		{
			m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
		}

		if(shadow || m_pRenderer->SphereInFrustum(VoxGame::GetInstance()->GetDefaultViewport(), pItem->GetCenter(), pItem->GetRadius()))
		{
			pItem->Render(outline, reflection, silhouette);

			m_numRenderItems++;
		}

		m_pRenderer->DisableTransparency();
	}
}

void ItemManager::RenderDebug()
{
	// Items
	for(unsigned int i = 0; i < m_vpItemList.size(); i++)
	{
		Item* pItem = m_vpItemList[i];

		if(pItem->NeedsErasing())
		{
			continue;
		}

		if(m_pRenderer->SphereInFrustum(VoxGame::GetInstance()->GetDefaultViewport(), pItem->GetCenter(), pItem->GetRadius()))
		{
			pItem->RenderDebug();
		}
	}

	// Item spawners
	for (unsigned int i = 0; i < m_vpItemSpawnerList.size(); i++)
	{
		ItemSpawner* pItemSpawner = m_vpItemSpawnerList[i];

		pItemSpawner->RenderDebug();
	}
}
