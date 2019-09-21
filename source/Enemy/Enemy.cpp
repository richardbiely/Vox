// ******************************************************************************
// Filename:    Enemy.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 18/11/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "Enemy.h"
#include "EnemyManager.h"
#include "EnemySpawner.h"

#include "../utils/Interpolator.h"
#include "../utils/Random.h"
#include "../utils/FileUtils.h"

#include "../Lighting/LightingManager.h"
#include "../Particles/BlockParticleManager.h"
#include "../TextEffects/TextEffectsManager.h"
#include "../Items/ItemManager.h"
#include "../Projectile/ProjectileManager.h"
#include "../Projectile/Projectile.h"
#include "../GameGUI/HUD.h"
#include "../NPC/NPCManager.h"
#include "../NPC/NPC.h"
#include "../Items/Item.h"
#include "../VoxGame.h"


Enemy::Enemy(Renderer* pRenderer, ChunkManager* pChunkManager, Player* pPlayer, LightingManager* pLightingManager, BlockParticleManager* pBlockParticleManager, TextEffectsManager* pTextEffectsManager, ItemManager* pItemManager, ProjectileManager* pProjectileManager, HUD* pHUD, EnemyManager* pEnemyManager, NPCManager* pNPCManager, QubicleBinaryManager* pQubicleBinaryManager, eEnemyType enemyType)
{
	m_pRenderer = pRenderer;
	m_pChunkManager = pChunkManager;
	m_pPlayer = pPlayer;
	m_pEnemyManager = pEnemyManager;
	m_pLightingManager = pLightingManager;
	m_pBlockParticleManager = pBlockParticleManager;
	m_pTextEffectsManager = pTextEffectsManager;
	m_pItemManager = pItemManager;
	m_pProjectileManager = pProjectileManager;
	m_pHUD = pHUD;
	m_pQubicleBinaryManager = pQubicleBinaryManager;
	m_pNPCManager = pNPCManager;

	m_eEnemyType = enemyType;

	m_radius = 1.0f;

	m_gravityDirection = vec3(0.0f, -1.0f, 0.0f);

	m_forward = vec3(0.0f, 0.0f, 1.0f);
	m_targetForward = vec3(0.0f, 0.0f, 1.0f);

	for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
	{
		m_animationSpeed[i] = 1.0f;
		m_animationSpeedonlySetOnCompleteAnimation[i] = false;
		m_animationFinished[i] = false;
	}

	// Parent spawner
	m_pParentEnemySpawner = nullptr;

	// Enemy name
	m_enemyName = "Enemy";

	// Idle
	m_bIsIdle = true;
	m_waitBeforeStopMovingAnimationTimer = 0.0f;
	m_hasSetIdleAnimation = true;

	// Rendering modes
	m_outlineRender = false;

	// Movement
	m_movementSpeed = 6.0f;
	m_movementFreezeTimer = 0.0f;

	// Jumping
	m_bCanJump = true;
	m_jumpTime = 1.5f;
	m_jumpTimer = 0.0f;
	m_jumpHeight = 14.5f;

	// Combat
	m_bCanAttack = true;
	m_attackDelayTime = 0.25f;
	m_attackDelayTimer = m_attackDelayTime*2.0f;
	m_attackEnabledDelayTimer = 0.0f;
	m_bCanInteruptCombatAnim = true;
	m_attackEnabled = false;
	m_insideAttackRadius = false;
	m_attackEnabledTimer = 0.0f;
	m_attackSegmentAngle = 0.75f;
	m_attackRadius = 1.75f;
	m_attackRotation = 0.0f;
	m_movementWaitAfterAttackTimer = 0.0f;

	// Gameplay
	m_sapped = false;
	m_sappedTimer = 0.0f;
	m_sappedParticleEffectId = -1;
	m_pSappedParticleEffect = nullptr;

	// Spawning
	m_spawning = false;
	m_spawnTime = 0.0f;
	m_spawningTimer = 0.0f;
	m_spawningT = 0.0f;

	// Wander / random target
	m_bRandomTargetPositionMode = false;
	m_getRamdomTargetPositionTimer = 3.0f;
	m_leashRadius = 10.0f;

	// Stuck
	m_bUpdateStuckTimer = false;
	m_bStuck = false;
	m_stuckTimer = 5.0f;

	// Enemy light
	m_enemyLightId = -1;

	// Enemy particle effect
	m_pEnemyParticleEffect = nullptr;

	// Charging attacks
	m_bIsChargingAttack = false;
	m_chargeAmount = 0.0f;
	m_chargeTime = 1.0f;

	// Projectile hitbox
	m_eProjectileHitboxType = eProjectileHitboxType_Sphere;
	m_projectileHitboxRadius = 0.5f;
	m_projectileHitboxXLength = 0.5f;
	m_projectileHitboxYLength = 0.5f;
	m_projectileHitboxZLength = 0.5f;

	// Aggro
	m_aggro = false;
	m_aggroLookSegmentAngle = 0.7f;
	m_aggroLookRadius = 10.0f;
	m_aggroResetTime = 5.0f;
	m_aggroResetTimer = m_aggroResetTime;

	// Look at point
	m_bLookAtPoint = false;
	m_bodyTurnSpeedMultiplier = 3.5f;
	m_bodyTurnStopThreshold = 0.35f;
	m_bIsLookingAtPoint = false;

	m_maxHealth = 100.0f;
	m_health = m_maxHealth;

	m_damageTime = 0.4f;
	m_damageTimer = m_damageTime;
	m_knockbackTime = 0.4f;
	m_knockbackTimer = m_knockbackTime;

	m_pTargetNPC = nullptr;

	m_gridPositionX = 0;
	m_gridPositionY = 0;
	m_gridPositionZ = 0;

	m_pCachedGridChunk = nullptr;

	m_modelVariation = 1;

	char characterBaseFolder[128];
	char qbFilename[128];
	char ms3dFilename[128];
	char animListFilename[128];
	char facesFilename[128];
	char characterFilename[128];

	SetupEnemyForType();

	sprintf(characterBaseFolder, "media/gamedata/models");
	sprintf(qbFilename, "media/gamedata/models/%s/%s.qb", m_typeString.c_str(), m_modelNameString.c_str());
	sprintf(ms3dFilename, "media/gamedata/models/%s/%s.ms3d", m_typeString.c_str(), m_typeString.c_str());
	sprintf(animListFilename, "media/gamedata/models/%s/%s.animlist", m_typeString.c_str(), m_typeString.c_str());
	sprintf(facesFilename, "media/gamedata/models/%s/%s.faces", m_typeString.c_str(), m_modelNameString.c_str());
	sprintf(characterFilename, "media/gamedata/models/%s/%s.character", m_typeString.c_str(), m_modelNameString.c_str());

	if(m_eEnemyType == eEnemyType_Doppelganger)
	{
		char playerFolder[128];
		sprintf(playerFolder, "saves/characters/%s", m_modelNameString.c_str());
		sprintf(qbFilename, "saves/characters/%s/%s.qb", m_modelNameString.c_str(), m_modelNameString.c_str());
		sprintf(facesFilename, "saves/characters/%s/%s.faces", m_modelNameString.c_str(), m_modelNameString.c_str());
		sprintf(characterFilename, "saves/characters/%s/%s.character", m_modelNameString.c_str(), m_modelNameString.c_str());

		m_pLightingManager->AddLight(vec3(0.0f, 0.0f, 0.0f), 10.0f, 1.0f, Colour(0.0f, 0.0f, 1.0f, 1.0f), &m_enemyLightId);

		m_pEnemyParticleEffect = m_pBlockParticleManager->ImportParticleEffect("media/gamedata/particles/ghost_rain.effect", m_position, &m_enemyParticleEffectId);
		m_pEnemyParticleEffect->PlayEffect();
	}

	m_pVoxelCharacter = new VoxelCharacter(m_pRenderer, m_pQubicleBinaryManager);
	m_pVoxelCharacter->UnloadCharacter();
	m_pVoxelCharacter->Reset();
	bool useQubicleManager = false;//(m_eEnemyType != eEnemyType_Doppelganger);
	m_pVoxelCharacter->LoadVoxelCharacter(m_typeString.c_str(), qbFilename, ms3dFilename, animListFilename, facesFilename, characterFilename, characterBaseFolder, useQubicleManager);

	m_pVoxelCharacter->SetBreathingAnimationEnabled(true);
	m_pVoxelCharacter->SetWinkAnimationEnabled(true);
	m_pVoxelCharacter->SetTalkingAnimationEnabled(false);
	m_pVoxelCharacter->SetRandomMouthSelection(false);
	m_pVoxelCharacter->SetRandomLookDirection(true);
	m_pVoxelCharacter->SetWireFrameRender(false);

	m_pVoxelCharacter->PlayAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "BindPose");

	SetupWeaponsForType();

	InitEnemyForType();

	m_erase = false;
}

Enemy::~Enemy()
{
	// If we belong to a spawner, make sure we indicate that we were killed
	if(m_pParentEnemySpawner != nullptr)
	{
		m_pParentEnemySpawner->RemoveEnemyFromThisSpawner();
	}

	UnloadWeapon(true);
	UnloadWeapon(false);

	m_pVoxelCharacter->RemoveQubicleMatrix("Quiver");

	if (m_eEnemyType == eEnemyType_Doppelganger)
	{
		// Remove enemy light
		m_pLightingManager->RemoveLight(m_enemyLightId);

		// Create dying light
		unsigned int lId;
		m_pLightingManager->AddDyingLight(GetCenter(), 10.0f, 1.0f, Colour(0.0f, 0.0f, 1.0f, 1.0f), 2.0f, &lId);

		m_pEnemyParticleEffect->StopEffect();
		m_pEnemyParticleEffect->m_erase = true;
	}

	delete m_pVoxelCharacter;
}

// Erase flag
bool Enemy::GetErase()
{
	return m_erase;
}

void Enemy::SetErase(bool erase)
{
	m_erase = erase;
}

// Accessors / Setters
void Enemy::SetPosition(vec3 pos)
{
	m_position = pos;
}

vec3 Enemy::GetPosition()
{
	return m_position;
}

void Enemy::SetVelocity(vec3 vel)
{
	m_velocity = vel;
}

vec3 Enemy::GetVelocity()
{
	return m_velocity;
}

void Enemy::SetRotation(float rot)
{
	float angle = DegToRad(rot);
	m_forward = vec3(sin(angle), 0.0f, cos(angle));
	m_targetForward = m_forward;
}

float Enemy::GetRotation()
{
	float rotationAngle = acos(dot(vec3(0.0f, 0.0f, 1.0f), m_forward));
	rotationAngle = RadToDeg(rotationAngle);
	if(m_forward.x < 0.0f)
	{
		rotationAngle = (360.0f - rotationAngle);
	}

	return rotationAngle;
}

float Enemy::GetFaceLookingRotation()
{
	vec3 faceLooking = m_pVoxelCharacter->GetFaceLookingDirection();
	faceLooking.y = 0.0f;
	faceLooking = normalize(faceLooking);
	float rotationAngle = acos(dot(vec3(0.0f, 0.0f, 1.0f), faceLooking));
	rotationAngle = RadToDeg(rotationAngle);
	if(faceLooking.x < 0.0f)
	{
		rotationAngle = (360.0f - rotationAngle);
	}

	return (rotationAngle);
}

void Enemy::SetScale(float scale)
{
	m_renderScale = scale;

	m_pVoxelCharacter->SetCharacterScale(scale);

	UpdateRadius();
}

float Enemy::GetRadius()
{
	return m_radius;
}

void Enemy::UpdateRadius()
{
	m_radius = m_pVoxelCharacter->GetCharacterScale() / 0.14f;
}

vec3 Enemy::GetCenter()
{
	vec3 center = m_position + vec3(0.0f, m_radius, 0.0f);

	return center;
}

vec3 Enemy::GetAboveHeadPosition()
{
	vec3 abovePosition;

	if(m_eProjectileHitboxType == eProjectileHitboxType_Sphere)
	{
		abovePosition.y += m_projectileHitboxRadius * 1.1f;
	}
	else if(m_eProjectileHitboxType == eProjectileHitboxType_Cube)
	{
		abovePosition.y += m_projectileHitboxYLength * 1.75f;
	}

	return GetCenter() + abovePosition;
}

vec3 Enemy::GetForwardVector()
{
	return m_worldMatrix.GetForwardVector();
}

vec3 Enemy::GetRightVector()
{
	return m_worldMatrix.GetRightVector();
}

vec3 Enemy::GetUpVector()
{
	return m_worldMatrix.GetUpVector();
}

vec3 Enemy::GetTargetForward()
{
	return m_targetForward;
}

// Setup
void Enemy::SetupEnemyForType()
{
	switch(m_eEnemyType)
	{
	case eEnemyType_None:
		{
		}
		break;

	case eEnemyType_GreenSlime:
		{
			m_typeString = "Jumping";
			m_modelNameString = "GreenSlime";
		}
		break;	
	case eEnemyType_RedSlime:
		{
			m_typeString = "Jumping";
			m_modelNameString = "RedSlime";
		}
		break;
	case eEnemyType_BlueSlime:
		{
			m_typeString = "Jumping";
			m_modelNameString = "BlueSlime";
		}
		break;
	case eEnemyType_YellowSlime:
		{
			m_typeString = "Jumping";
			m_modelNameString = "YellowSlime";
		}
		break;

	case eEnemyType_NormalSkeleton:
		{
			m_typeString = "Human";
			m_modelNameString = "Skeleton1";
		}
		break;
	case eEnemyType_RangedSkeleton:
		{
			m_typeString = "Human";
			m_modelNameString = "Skeleton2";
		}
		break;
	case eEnemyType_MeleeSkeleton:
		{
			m_typeString = "Human";
			m_modelNameString = "Skeleton3";
		}
		break;
	case eEnemyType_MageSkeleton:
		{
			m_typeString = "Human";
			m_modelNameString = "Skeleton4";
		}
		break;

	case eEnemyType_IronGiant:
		{
			m_typeString = "Human";
			m_modelNameString = "IronGiant";
		}
		break;

	case eEnemyType_Mummy:
		{
			m_typeString = "Human";
			m_modelNameString = "Mummy";
		}
		break;

	case eEnemyType_WalkingZombie:
		{
			if(GetRandomNumber(0, 100) > 50.0f)
			{
				m_modelVariation = 1;
			}
			else
			{
				m_modelVariation = 2;
			}

			char variationString[64];
			sprintf(variationString, "WalkingZombie%i", m_modelVariation);

			m_typeString = "Human";
			m_modelNameString = variationString;
		}
		break;
	case eEnemyType_CrawlingZombie:
		{
			char variationString[64];
			sprintf(variationString, "CrawlingZombie%i", m_modelVariation);

			m_typeString = "Human";
			m_modelNameString = variationString;
		}
		break;

	case eEnemyType_Bee:
		{
			m_typeString = "Flying";
			m_modelNameString = "Bee";
		}
		break;
	case eEnemyType_Bat:
		{
			m_typeString = "Flying";
			m_modelNameString = "Bat";
		}
		break;
	case eEnemyType_Ghost:
		{
			m_typeString = "Flying";
			m_modelNameString = "Ghost";
		}
		break;
	case eEnemyType_Doppelganger:
		{
			m_typeString = m_pPlayer->GetType();
			m_modelNameString = m_pPlayer->GetModelName();
		}
		break;

	case eEnemyType_TargetDummy:
		{
			m_typeString = "Dummy";
			m_modelNameString = "TargetDummy";
		}
		break;

	case eEnemyType_Mimic:
		{
			m_typeString = "Jumping";
			m_modelNameString = "Mimic";
		}
		break;
		
	case eEnemyType_SnowGolem:
		{
		}
		break;
	case eEnemyType_MagmaGolem:
		{
		}
		break;
	}
}

void Enemy::SetupWeaponsForType()
{
	switch(m_eEnemyType)
	{
	case eEnemyType_None:
		{
		}
		break;

	case eEnemyType_GreenSlime:
		{
		}
		break;
	case eEnemyType_RedSlime:
		{
		}
		break;
	case eEnemyType_BlueSlime:
		{
		}
		break;
	case eEnemyType_YellowSlime:
		{
		}
		break;

	case eEnemyType_NormalSkeleton:
		{
		}
		break;
	case eEnemyType_RangedSkeleton:
		{
			LoadWeapon(true, GetEquipmentFilenameForType(eEquipment_WoodenBow));
		}
		break;
	case eEnemyType_MeleeSkeleton:
		{
			LoadWeapon(false, GetEquipmentFilenameForType(eEquipment_IronSword));
			LoadWeapon(true, GetEquipmentFilenameForType(eEquipment_IronShield));
		}
		break;
	case eEnemyType_MageSkeleton:
		{
			LoadWeapon(false, GetEquipmentFilenameForType(eEquipment_NecroStaff));
		}
		break;

	case eEnemyType_IronGiant:
		{
		}
		break;

	case eEnemyType_Mummy:
		{
		}
		break;

	case eEnemyType_WalkingZombie:
		{
		}
		break;
	
	case eEnemyType_CrawlingZombie:
		{
		}
		break;

	case eEnemyType_Bee:
		{
		}
		break;
	case eEnemyType_Bat:
		{
		}
		break;
	case eEnemyType_Ghost:
		{
		}
		break;
	case eEnemyType_Doppelganger:
		{
		}
		break;

	case eEnemyType_TargetDummy:
		{
		}
		break;

	case eEnemyType_Mimic:
		{
		}
		break;

	case eEnemyType_SnowGolem:
		{
		}
		break;
	case eEnemyType_MagmaGolem:
		{
		}
		break;
	}
}

void Enemy::InitEnemyForType()
{
	switch(m_eEnemyType)
	{
	case eEnemyType_None:
		{
		}
		break;

	case eEnemyType_GreenSlime:
		{
			m_enemyName = "Green Slime";

			m_pVoxelCharacter->SetBreathingAnimationEnabled(false);
			m_pVoxelCharacter->SetWinkAnimationEnabled(false);
			m_pVoxelCharacter->SetRandomLookDirection(false);

			m_pVoxelCharacter->SetMeshAlpha(0.65f);

			m_eProjectileHitboxType = eProjectileHitboxType_Sphere;
			m_projectileHitboxRadius = 0.75f;

			m_jumpTime = 2.5f;
			m_attackRadius = 1.15f;
			m_bodyTurnStopThreshold = 0.025f;
		}
		break;
	case eEnemyType_RedSlime:
		{
			m_enemyName = "Red Slime";

			m_pVoxelCharacter->SetBreathingAnimationEnabled(false);
			m_pVoxelCharacter->SetWinkAnimationEnabled(false);
			m_pVoxelCharacter->SetRandomLookDirection(false);

			m_pVoxelCharacter->SetMeshAlpha(0.65f);

			m_eProjectileHitboxType = eProjectileHitboxType_Sphere;
			m_projectileHitboxRadius = 0.75f;

			m_jumpTime = 2.5f;
			m_attackRadius = 1.15f;
			m_bodyTurnStopThreshold = 0.025f;
		}
		break;
	case eEnemyType_BlueSlime:
		{
			m_enemyName = "Blue Slime";

			m_pVoxelCharacter->SetBreathingAnimationEnabled(false);
			m_pVoxelCharacter->SetWinkAnimationEnabled(false);
			m_pVoxelCharacter->SetRandomLookDirection(false);

			m_pVoxelCharacter->SetMeshAlpha(0.65f);

			m_eProjectileHitboxType = eProjectileHitboxType_Sphere;
			m_projectileHitboxRadius = 0.75f;

			m_jumpTime = 0.5f;
			m_jumpHeight = 10.0f;
			m_attackRadius = 1.15f;
			m_bodyTurnStopThreshold = 0.025f;
		}
		break;
	case eEnemyType_YellowSlime:
		{
			m_enemyName = "Yellow Slime";

			m_pVoxelCharacter->SetBreathingAnimationEnabled(false);
			m_pVoxelCharacter->SetWinkAnimationEnabled(false);
			m_pVoxelCharacter->SetRandomLookDirection(false);

			m_pVoxelCharacter->SetMeshAlpha(0.65f);

			m_eProjectileHitboxType = eProjectileHitboxType_Sphere;
			m_projectileHitboxRadius = 0.75f;

			m_jumpTime = 0.5f;
			m_jumpHeight = 10.0f;
			m_attackRadius = 1.215f;
			m_bodyTurnStopThreshold = 0.025f;
		}
		break;

	case eEnemyType_NormalSkeleton:
		{
			m_enemyName = "Skeleton";

			m_eProjectileHitboxType = eProjectileHitboxType_Cube;
			m_projectileHitboxXLength = 0.65f;
			m_projectileHitboxYLength = 1.25f;
			m_projectileHitboxZLength = 0.65f;
			m_projectileHitboxCenterOffset = vec3(0.0f, 0.75f, 0.0f);

			m_bodyTurnStopThreshold = 0.025f;
			m_attackRadius = 0.25f;

			m_aggroResetTime = 15.0f;
		}
		break;
	case eEnemyType_RangedSkeleton:
		{
			m_enemyName = "Skeleton Archer";

			m_eProjectileHitboxType = eProjectileHitboxType_Cube;
			m_projectileHitboxXLength = 0.65f;
			m_projectileHitboxYLength = 1.25f;
			m_projectileHitboxZLength = 0.65f;
			m_projectileHitboxCenterOffset = vec3(0.0f, 0.75f, 0.0f);

			m_bodyTurnStopThreshold = 0.025f;
			m_attackRadius = 0.25f;

			m_aggroResetTime = 15.0f;

			// Add a quiver item to the enemy ranger
			VoxelWeapon* pNewEquipment = new VoxelWeapon(m_pRenderer, m_pQubicleBinaryManager);
			pNewEquipment->SetVoxelCharacterParent(nullptr);
			pNewEquipment->LoadWeapon("media/gamedata/items/Quiver/Quiver.item", false);

			AnimatedSection* section = pNewEquipment->GetAnimatedSection(0);
			section->m_pVoxelObject->GetQubicleModel()->SetScaleAndOffsetForMatrix("Quiver", section->m_renderScale, section->m_renderOffset.x, section->m_renderOffset.y, section->m_renderOffset.z);
			QubicleMatrix* pQuiverMatrix = section->m_pVoxelObject->GetQubicleModel()->SetQubicleMatrix("Quiver");
			pQuiverMatrix->m_boneIndex = m_pVoxelCharacter->GetBodyBoneIndex();
			m_pVoxelCharacter->AddQubicleMatrix(pQuiverMatrix, false);
		}
		break;
	case eEnemyType_MeleeSkeleton:
		{
			m_enemyName = "Skeleton Knight";

			m_eProjectileHitboxType = eProjectileHitboxType_Cube;
			m_projectileHitboxXLength = 0.65f;
			m_projectileHitboxYLength = 1.25f;
			m_projectileHitboxZLength = 0.65f;
			m_projectileHitboxCenterOffset = vec3(0.0f, 0.75f, 0.0f);
		}
		break;
	case eEnemyType_MageSkeleton:
		{
			m_enemyName = "Skeleton Mage";

			m_eProjectileHitboxType = eProjectileHitboxType_Cube;
			m_projectileHitboxXLength = 0.65f;
			m_projectileHitboxYLength = 1.25f;
			m_projectileHitboxZLength = 0.65f;
			m_projectileHitboxCenterOffset = vec3(0.0f, 0.75f, 0.0f);

			m_bodyTurnStopThreshold = 0.025f;
			m_attackRadius = 0.25f;

			m_aggroResetTime = 15.0f;

			m_pVoxelCharacter->PlayAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "StaffPose");
		}
		break;

	case eEnemyType_IronGiant:
		{
			m_enemyName = "Iron Giant";

			m_eProjectileHitboxType = eProjectileHitboxType_Cube;
			m_projectileHitboxXLength = 1.65f;
			m_projectileHitboxYLength = 3.25f;
			m_projectileHitboxZLength = 1.65f;
			m_projectileHitboxCenterOffset = vec3(0.0f, 3.0f, 0.0f);

			m_bodyTurnSpeedMultiplier = 1.0f;
			m_bodyTurnStopThreshold = 0.005f;
			m_attackRadius = 0.0f;
			m_movementSpeed = 3.0f;
			m_aggroResetTime = 15.0f;

			m_radius = 3.0f;
		}
		break;

	case eEnemyType_Mummy:
		{
			m_enemyName = "Mummy";

			m_eProjectileHitboxType = eProjectileHitboxType_Cube;
			m_projectileHitboxXLength = 0.65f;
			m_projectileHitboxYLength = 1.25f;
			m_projectileHitboxZLength = 0.65f;
			m_projectileHitboxCenterOffset = vec3(0.0f, 0.75f, 0.0f);

			m_attackRadius = 0.0f;
			m_movementSpeed = 3.0f;
			m_aggroResetTime = 15.0f;
		}
		break;

	case eEnemyType_WalkingZombie:
		{
			m_enemyName = "Zombie";

			m_eProjectileHitboxType = eProjectileHitboxType_Cube;
			m_projectileHitboxXLength = 0.65f;
			m_projectileHitboxYLength = 1.25f;
			m_projectileHitboxZLength = 0.65f;
			m_projectileHitboxCenterOffset = vec3(0.0f, 0.75f, 0.0f);

			m_attackRadius = 0.0f;
			m_movementSpeed = 3.0f;
			m_aggroResetTime = 15.0f;
		}
		break;

	case eEnemyType_CrawlingZombie:
		{
			m_enemyName = "Zombie";

			m_eProjectileHitboxType = eProjectileHitboxType_Cube;
			m_projectileHitboxXLength = 0.65f;
			m_projectileHitboxYLength = 0.8f;
			m_projectileHitboxZLength = 0.65f;
			m_projectileHitboxCenterOffset = vec3(0.0f, 0.0f, 0.0f);

			m_attackRadius = 0.0f;
			m_movementSpeed = 1.0f;
			m_aggroResetTime = 15.0f;

			m_pVoxelCharacter->PlayAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "ZombieCrawl");
		}
		break;

	case eEnemyType_Bee:
		{
			m_enemyName = "Bee";

			m_eProjectileHitboxType = eProjectileHitboxType_Cube;
			m_projectileHitboxXLength = 0.6f;
			m_projectileHitboxYLength = 0.6f;
			m_projectileHitboxZLength = 0.95f;
			m_projectileHitboxCenterOffset = vec3(0.0f, 0.0f, 0.0f);

			m_pVoxelCharacter->PlayAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "Fly");
			m_attackRadius = 1.0f;
		}
		break;
	case eEnemyType_Bat:
		{
			m_enemyName = "Bat";

			m_eProjectileHitboxType = eProjectileHitboxType_Cube;
			m_projectileHitboxXLength = 0.75f;
			m_projectileHitboxYLength = 0.5f;
			m_projectileHitboxZLength = 0.5f;
			m_projectileHitboxCenterOffset = vec3(0.0f, 0.25f, 0.0f);

			m_pVoxelCharacter->PlayAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "Fly");

			m_aggroResetTime = 15.0f;
		}
		break;
	case eEnemyType_Ghost:
		{
			m_enemyName = "Ghost";

			m_pVoxelCharacter->SetMeshAlpha(0.65f);
			m_attackRadius = 0.65f;
		}
		break;
	case eEnemyType_Doppelganger:
		{
			m_enemyName = "Ghostly Doppelganger";

			m_eProjectileHitboxType = eProjectileHitboxType_Cube;
			m_projectileHitboxXLength = 0.65f;
			m_projectileHitboxYLength = 1.25f;
			m_projectileHitboxZLength = 0.65f;
			m_projectileHitboxCenterOffset = vec3(0.0f, 0.75f, 0.0f);

			m_pVoxelCharacter->SetMeshAlpha(0.65f);
			m_pVoxelCharacter->SetMeshSingleColour(1.0f, 1.0f, 1.0f);
			m_attackRadius = 0.65f;
		}
		break;

	case eEnemyType_TargetDummy:
		{
			m_enemyName = "Target Dummy";

			m_eProjectileHitboxType = eProjectileHitboxType_Cube;
			m_projectileHitboxXLength = 0.65f;
			m_projectileHitboxYLength = 1.6f;
			m_projectileHitboxZLength = 0.65f;
			m_projectileHitboxCenterOffset = vec3(0.0f, 1.0f, 0.0f);

			m_pVoxelCharacter->SetBreathingAnimationEnabled(false);
			m_pVoxelCharacter->SetWinkAnimationEnabled(false);
			m_pVoxelCharacter->SetRandomLookDirection(false);
		}
		break;

	case eEnemyType_Mimic:
		{
			m_enemyName = "Mimic";

			m_pVoxelCharacter->SetBreathingAnimationEnabled(false);
			m_pVoxelCharacter->SetWinkAnimationEnabled(false);
			m_pVoxelCharacter->SetRandomLookDirection(false);

			m_eProjectileHitboxType = eProjectileHitboxType_Cube;
			m_projectileHitboxXLength = 0.8f;
			m_projectileHitboxYLength = 0.5f;
			m_projectileHitboxZLength = 0.5f;
			m_projectileHitboxCenterOffset = vec3(0.0f, 0.0f, 0.0f);

			m_bodyTurnSpeedMultiplier = 10.0f;
			m_bodyTurnStopThreshold = 0.005f;
			m_movementSpeed = 8.0f;
			m_jumpTime = 0.5f;
			m_jumpHeight = 10.0f;
			m_attackRadius = 0.5f;
		}
		break;

	case eEnemyType_SnowGolem:
		{
			m_enemyName = "Snow Golem";

		}
		break;
	case eEnemyType_MagmaGolem:
		{
			m_enemyName = "Magma Golem";

		}
		break;
	}
}

void Enemy::LoadWeapon(bool left, string weaponFile)
{
	auto updateWeapon = [&](VoxelWeapon* weapon)
	{
		// Lights
		for (int i = 0; i < weapon->GetNumLights(); i++)
		{
			unsigned int lightId;
			vec3 lightPos;
			float lightRadius;
			float lightDiffuseMultiplier;
			Colour lightColour;
			bool connectedToSegment;
			weapon->GetLightParams(i, &lightId, &lightPos, &lightRadius, &lightDiffuseMultiplier, &lightColour, &connectedToSegment);

			if (lightId != -1)
			{
				m_pLightingManager->RemoveLight(lightId);
				weapon->SetLightingId(i, -1);
			}
		}

		// Particle effects
		for (int i = 0; i < weapon->GetNumParticleEffects(); i++)
		{
			unsigned int particleEffectId;
			vec3 ParticleEffectPos;
			string effectName;
			bool connectedToSegment;
			weapon->GetParticleEffectParams(i, &particleEffectId, &ParticleEffectPos, &effectName, &connectedToSegment);

			if (particleEffectId != -1)
			{
				m_pBlockParticleManager->DestroyParticleEffect(particleEffectId);
				weapon->SetParticleEffectId(i, -1);
			}
		}
	};

	if(left)
	{
		if(m_pVoxelCharacter->GetLeftWeapon() != nullptr)
		{
			if(m_pVoxelCharacter->IsLeftWeaponLoaded())
				updateWeapon(m_pVoxelCharacter->GetLeftWeapon());
		}
	}
	else
	{
		if(m_pVoxelCharacter->GetRightWeapon() != nullptr)
		{
			if(m_pVoxelCharacter->IsRightWeaponLoaded())
				updateWeapon(m_pVoxelCharacter->GetRightWeapon());

			// Load the weapon file
			m_pVoxelCharacter->LoadRightWeapon(weaponFile.c_str());

			// Weapon attack radius
			if(m_pVoxelCharacter->GetRightWeapon() != nullptr)
			{
				if(m_pVoxelCharacter->IsRightWeaponLoaded())
				{
					m_attackRadius = m_pVoxelCharacter->GetRightWeapon()->GetWeaponRadius();
				}
			}
		}
	}
}

void Enemy::UnloadWeapon(bool left)
{
	VoxelWeapon* pWeapon = nullptr;
	bool isWeaponLoaded = false;
	if (left)  // Left side
	{
		pWeapon = m_pVoxelCharacter->GetLeftWeapon();
		isWeaponLoaded = m_pVoxelCharacter->IsLeftWeaponLoaded();
	}
	else  // Right side
	{
		pWeapon = m_pVoxelCharacter->GetRightWeapon();
		isWeaponLoaded = m_pVoxelCharacter->IsRightWeaponLoaded();
	}

	if (pWeapon != nullptr)
	{
		if (isWeaponLoaded)
		{
			// Lights
			for (int i = 0; i < pWeapon->GetNumLights(); i++)
			{
				unsigned int lightId;
				vec3 lightPos;
				float lightRadius;
				float lightDiffuseMultiplier;
				Colour lightColour;
				bool connectedToSegment;
				pWeapon->GetLightParams(i, &lightId, &lightPos, &lightRadius, &lightDiffuseMultiplier, &lightColour, &connectedToSegment);

				if (lightId != -1)
				{
					m_pLightingManager->RemoveLight(lightId);
					pWeapon->SetLightingId(i, -1);

					if (connectedToSegment == false)
					{
						// Rotate due to characters forward vector
						float rotationAngle = acos(dot(vec3(0.0f, 0.0f, 1.0f), m_forward));
						if (m_forward.x < 0.0f)
						{
							rotationAngle = -rotationAngle;
						}
						Matrix4x4 rotationMatrix;
						rotationMatrix.SetRotation(0.0f, rotationAngle, 0.0f);
						lightPos = rotationMatrix * lightPos;

						// Translate to position
						lightPos += m_position;
					}

					float scale = m_pVoxelCharacter->GetCharacterScale();
					unsigned int lId;
					m_pLightingManager->AddDyingLight(vec3(lightPos.x, lightPos.y, lightPos.z), lightRadius * scale, lightDiffuseMultiplier, lightColour, 2.0f, &lId);
				}
			}

			// Particle Effects
			for (int i = 0; i < pWeapon->GetNumParticleEffects(); i++)
			{
				unsigned int particleEffectId;
				vec3 ParticleEffectPos;
				string effectName;
				bool connectedToSegment;
				pWeapon->GetParticleEffectParams(i, &particleEffectId, &ParticleEffectPos, &effectName, &connectedToSegment);

				if (particleEffectId != -1)
				{
					m_pBlockParticleManager->DestroyParticleEffect(particleEffectId);
					pWeapon->SetParticleEffectId(i, -1);
				}
			}
		}

		pWeapon->UnloadWeapon();

		if (left)  // Left side
		{
			m_pVoxelCharacter->UnloadLeftWeapon();
		}
		else  // Right side
		{
			m_pVoxelCharacter->UnloadRightWeapon();
		}
	}
}

// Enemy spawner
void Enemy::SetEnemySpawner(EnemySpawner* pSpawner)
{
	m_pParentEnemySpawner = pSpawner;
}

void Enemy::RemoveEnemySpawner(EnemySpawner* pSpawner)
{
	if(m_pParentEnemySpawner == pSpawner)
	{
		m_pParentEnemySpawner = nullptr;
	}
}

// Animation
void Enemy::SetAnimationSpeed(float speed, bool onlySetOnCompleteAnimation, AnimationSections section)
{
	if(m_bCanAttack == false)
	{
		return;
	}

	if(section == AnimationSections_FullBody)
	{
		for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
		{
			if(m_animationSpeedonlySetOnCompleteAnimation[i] == false || m_animationFinished[i] == true)
			{
				m_animationSpeed[i] = speed;
				m_animationSpeedonlySetOnCompleteAnimation[i] = false;
			}
		}
	}
	else
	{
		m_animationSpeed[section] = speed;
		m_animationSpeedonlySetOnCompleteAnimation[section] = onlySetOnCompleteAnimation;
	}	
}

float Enemy::GetAnimationSpeed(AnimationSections section)
{
	return m_animationSpeed[section];
}

bool Enemy::AnimationFinished(AnimationSections section)
{
	return m_animationFinished[section];
}

// Looking
void Enemy::SetTargetForwardToLookAtPoint(vec3 point)
{
	vec3 toPoint = point - GetCenter();
	toPoint.y = 0.0f;
	toPoint = normalize(toPoint);

	m_targetForward = toPoint;
}

void Enemy::LookAtPoint(vec3 point)
{
	m_bLookAtPoint = true;
	m_lookPoint = point;
}

void Enemy::LookAtRandomDirection()
{
	m_forward = vec3(GetRandomNumber(-1, 1, 2), 0.0f, GetRandomNumber(-1, 1, 2));
	m_forward = normalize(m_forward);

	m_bLookAtPoint = false;
	m_targetForward = m_forward;
}

void Enemy::SetRandomLookMode()
{
	m_pVoxelCharacter->SetFaceTargetDirection(m_pVoxelCharacter->GetFaceLookingDirection());
	m_pVoxelCharacter->SetRandomLookDirection(true);
	m_pVoxelCharacter->SetFaceLookToTargetSpeedMultiplier(1.0f);
	m_bLookAtPoint = false;
}

void Enemy::SetBodyTurnStopThreshold(float threshold)
{
	m_bodyTurnStopThreshold = threshold;
}

void Enemy::SetBodyTurnSpeedMultiplier(float multiplier)
{
	m_bodyTurnSpeedMultiplier = multiplier;
}

// World
void Enemy::UpdateGridPosition()
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

	if(gridPositionX != m_gridPositionX || gridPositionY != m_gridPositionY || gridPositionZ != m_gridPositionZ || m_pCachedGridChunk == nullptr)
	{
		m_gridPositionX = gridPositionX;
		m_gridPositionY = gridPositionY;
		m_gridPositionZ = gridPositionZ;

		m_pCachedGridChunk = m_pChunkManager->GetChunk(m_gridPositionX, m_gridPositionY, m_gridPositionZ);
	}
}

Chunk* Enemy::GetCachedGridChunkOrFromPosition(vec3 pos)
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
		return nullptr;
	}
	else
	{
		return m_pCachedGridChunk;
	}
}

void Enemy::ClearChunkCacheForChunk(Chunk* pChunk)
{
	if(m_pCachedGridChunk == pChunk)
	{
		m_pCachedGridChunk = nullptr;

		// Erase if the chunk we belonged to is unloaded
		SetErase(true);
	}
}

// Collision
bool Enemy::CheckCollisions(vec3 positionCheck, vec3 previousPosition, vec3 *pNormal, vec3 *pMovement)
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
						if (pChunk == nullptr || pChunk->IsSetup() == false)
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

	if(worldCollision)
		return true;

	*pMovement = movementCache;

	return false;
}

bool Enemy::IsBlockInFront()
{
	vec3 blockPos_Check;
	int blockX_Check, blockY_Check, blockZ_Check;
	vec3 blockPos;

	// Straight in front
	vec3 checkPos = GetCenter() + normalize(m_forward) * (m_radius*2.0f);
	Chunk* pChunk = GetCachedGridChunkOrFromPosition(checkPos);
	bool active = m_pChunkManager->GetBlockActiveFrom3DPosition(checkPos.x, checkPos.y, checkPos.z, &blockPos_Check, &blockX_Check, &blockY_Check, &blockZ_Check, &pChunk);
	if(active == true)
	{
		return true;
	}

	// To the left side
	checkPos = GetCenter() + normalize(m_forward - GetRightVector()) * (m_radius*2.0f);
	pChunk = GetCachedGridChunkOrFromPosition(checkPos);
	active = m_pChunkManager->GetBlockActiveFrom3DPosition(checkPos.x, checkPos.y, checkPos.z, &blockPos_Check, &blockX_Check, &blockY_Check, &blockZ_Check, &pChunk);
	if(active == true)
	{
		return true;
	}

	// To the right side
	checkPos = GetCenter() + normalize(m_forward + GetRightVector()) * (m_radius*2.0f);
	pChunk = GetCachedGridChunkOrFromPosition(checkPos);
	active = m_pChunkManager->GetBlockActiveFrom3DPosition(checkPos.x, checkPos.y, checkPos.z, &blockPos_Check, &blockX_Check, &blockY_Check, &blockZ_Check, &pChunk);
	if(active == true)
	{
		return true;
	}

	return false;
}

// Movement
void Enemy::MoveAbsolute(vec3 direction, const float speed, bool shouldChangeForward)
{
	if(shouldChangeForward)
	{
		float speedToUse = speed;
		vec3 diff = normalize(m_forward) - normalize(direction);
		if(dot(normalize(m_forward), normalize(direction)) < 0.1f)
		{
			speedToUse *= 1.75f;
		}
		m_forward -= (diff * speedToUse);
		m_forward = normalize(m_forward);
	}

	vec3 movement = direction;

	vec3 pNormal;
	vec3 movementAmount = direction*speed;
	int numberDivision = 1;
	while(length(movementAmount) >= Chunk::BLOCK_RENDER_SIZE)
	{
		numberDivision++;
		movementAmount = direction*(speed/numberDivision);
	}

	for(int i = 0; i < numberDivision; i++)
	{
		float speedToUse = (speed / numberDivision) + ((speed / numberDivision) * i);
		vec3 posToCheck = GetCenter() + movement*speedToUse;

		if(m_eEnemyType != eEnemyType_Ghost && m_eEnemyType != eEnemyType_Doppelganger)
		{
			if(CheckCollisions(posToCheck, m_previousPosition, &pNormal, &movement))
			{
			}
		}

		m_position += (movement * speedToUse)*0.95f;
	}

	if(m_eEnemyType == eEnemyType_Bee || m_eEnemyType == eEnemyType_Bat)
	{
		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_FullBody, "Fly", 0.01f);
	}
	else if(m_eEnemyType == eEnemyType_GreenSlime || m_eEnemyType == eEnemyType_RedSlime || m_eEnemyType == eEnemyType_BlueSlime || m_eEnemyType == eEnemyType_YellowSlime)
	{
	}
	else if(m_eEnemyType == eEnemyType_Mimic)
	{
		float RandomChompTime = 0.025f + (GetRandomNumber(0, 3, 2) * 0.05f);
		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_FullBody, "MimicChomp", RandomChompTime);
	}
	if(m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger)
	{
		SetAnimationSpeed(0.35f, true, AnimationSections_FullBody);
		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_FullBody, "GhostPose", 0.01f);
	}
	else if(m_eEnemyType == eEnemyType_CrawlingZombie)
	{
		SetAnimationSpeed(0.35f, true, AnimationSections_FullBody);
		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_FullBody, "ZombieCrawl", 0.015f);
	}
	else if(m_eEnemyType == eEnemyType_Mummy || m_eEnemyType == eEnemyType_WalkingZombie)
	{
		if(m_bCanJump)
		{
			SetAnimationSpeed(0.5f, true, AnimationSections_FullBody);
		}
		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_FullBody, "MummyWalk", 0.015f);
	}
	else if(m_eEnemyType == eEnemyType_MageSkeleton)
	{
		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_Legs_Feet, "StaffRun", 0.01f);
		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, true, AnimationSections_Right_Arm_Hand, "StaffRun", 0.01f);
	}
	else
	{
		if(CanAttack())
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_Legs_Feet, "Run", 0.01f);
		}
		else if(m_bIsIdle)
		{
			if(m_bCanInteruptCombatAnim)
			{
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Head_Body, false, AnimationSections_Head_Body, "Run", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "Run", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Legs_Feet, false, AnimationSections_Legs_Feet, "Run", 0.01f);
			}
		}
	}


	m_bIsIdle = false;
}

void Enemy::StopMoving()
{
	if(m_bCanJump)
	{
		if(m_bIsIdle == false)
		{
			m_bIsIdle = true;

			if(m_eEnemyType != eEnemyType_Mimic)
			{
				SetRandomLookMode();
			}
		}

		if(m_hasSetIdleAnimation == false)
		{
			if(m_eEnemyType != eEnemyType_Bee && m_eEnemyType != eEnemyType_Bat && m_eEnemyType != eEnemyType_Ghost && m_eEnemyType != eEnemyType_Doppelganger && m_eEnemyType != eEnemyType_CrawlingZombie)
			{
				if(m_waitBeforeStopMovingAnimationTimer <= 0.0f)
				{
					if(m_eEnemyType != eEnemyType_Mimic)
					{
						if(m_eEnemyType == eEnemyType_MageSkeleton)
						{
							m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "StaffPose", 0.15f);
						}
						else
						{
							if(CanAttack())
							{
								m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "BindPose", 0.15f);
							}
							if(m_bCanInteruptCombatAnim)
							{
								m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Head_Body, false, AnimationSections_Head_Body, "BindPose", 0.15f);
								m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Legs_Feet, false, AnimationSections_Legs_Feet, "BindPose", 0.15f);
								m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "BindPose", 0.15f);
							}
						}
					}

					m_hasSetIdleAnimation = true;
				}
			}
		}
	}
}

void Enemy::Jump(bool jumpUpBlock)
{
	if(m_bCanJump == false)
	{
		return;
	}

	if(m_jumpTimer >= 0.0f)
	{
		return;
	}

	if(m_eEnemyType == eEnemyType_Bee || m_eEnemyType == eEnemyType_Bat || m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger)
	{
		return; // Flying enemies can't jump
	}

	m_bCanJump = false;
	m_jumpTimer = m_jumpTime;

	m_velocity += vec3(0.0f, m_jumpHeight, 0.0f);

	if(m_eEnemyType == eEnemyType_GreenSlime || m_eEnemyType == eEnemyType_RedSlime || m_eEnemyType == eEnemyType_BlueSlime || m_eEnemyType == eEnemyType_YellowSlime || m_eEnemyType == eEnemyType_Mimic)
	{
		if(jumpUpBlock)
		{
			// Add a little extra bounce for jumping enemies to get up blocks
			m_velocity += vec3(0.0f, 4.5f, 0.0f);
		}
	}

	if(m_eEnemyType != eEnemyType_Bee && m_eEnemyType != eEnemyType_Bat && m_eEnemyType != eEnemyType_Ghost && m_eEnemyType != eEnemyType_Doppelganger && m_eEnemyType != eEnemyType_Mimic && m_eEnemyType != eEnemyType_GreenSlime && m_eEnemyType != eEnemyType_RedSlime && m_eEnemyType != eEnemyType_BlueSlime && m_eEnemyType != eEnemyType_YellowSlime)
	{
		if(m_eEnemyType == eEnemyType_Mummy || m_eEnemyType == eEnemyType_WalkingZombie || m_eEnemyType == eEnemyType_CrawlingZombie)
		{
			SetAnimationSpeed(1.0f, false, AnimationSections_Legs_Feet);
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Legs_Feet, false, AnimationSections_Legs_Feet, "Jump", 0.01f);
		}
		else
		{
			if(CanAttack())
			{
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "Jump", 0.01f);
			}

			if(m_bCanInteruptCombatAnim)
			{
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Head_Body, false, AnimationSections_Head_Body, "Jump", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Legs_Feet, false, AnimationSections_Legs_Feet, "Jump", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "Jump", 0.01f);
			}
		}
	}

	if (m_eEnemyType == eEnemyType_Mimic)
	{
		VoxGame::GetInstance()->PlaySoundEffect3D(eSoundEffect_MimicJump, GetCenter());
	}
}

bool Enemy::CanJump()
{
	return m_bCanJump;
}

bool Enemy::IsMoving()
{
	if(m_eEnemyType == eEnemyType_GreenSlime || m_eEnemyType == eEnemyType_RedSlime)
	{
		return false;
	}

	return m_bIsIdle == false;
}

float Enemy::GetMovementSpeed()
{
	return m_movementSpeed;
}

bool Enemy::CanMoveTowardsTarget()
{
	if(m_eEnemyType == eEnemyType_TargetDummy)
	{
		return false;
	}

	if(m_eEnemyType == eEnemyType_Mimic)
	{
		// Mimic can only move towards the target position if it is in aggro, else stay still!
		if(m_aggro)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	if(m_eEnemyType == eEnemyType_NormalSkeleton || m_eEnemyType == eEnemyType_RangedSkeleton || m_eEnemyType == eEnemyType_MageSkeleton || m_eEnemyType == eEnemyType_Bat || m_eEnemyType == eEnemyType_IronGiant)
	{
		if(m_aggro)
		{
			// Stand still when attacking for ranged characters
			return false;
		}

		if(m_bRandomTargetPositionMode == false)
		{
			return false;
		}
	}

	return true;
}

// Combat
void Enemy::DoDamage(float amount, Colour textColour, vec3 knockbackDirection, float knockbackAmount, bool createParticleHit, bool shouldUpdateGUI)
{
	if(m_eEnemyType == eEnemyType_Bee || m_eEnemyType == eEnemyType_Bat || m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger)
	{
		knockbackAmount *= 0.5f;
	}

	if(m_eEnemyType == eEnemyType_TargetDummy)
	{
		knockbackAmount = 0.0f;
	}

	if (m_eEnemyType == eEnemyType_IronGiant)
	{
		knockbackAmount = 0.0f;
	}

	bool finalDeathHit = false;

	// Remove sapped
	SetSapped(false);

	if(m_damageTimer <= 0.0f)
	{
		if(m_aggro == false)
		{
			// If we are setting aggro for initial setting, reset and extend the attack delay timer
			m_attackDelayTimer = m_attackDelayTime*2.0f;
		}

		if(m_eEnemyType != eEnemyType_TargetDummy)
		{
			m_aggro = true;
			m_aggroResetTimer = m_aggroResetTime;
		}

		float healthBefore = m_health;
		float damageDone = healthBefore - m_health;

		if(m_eEnemyType != eEnemyType_TargetDummy)
		{
			m_health -= amount;
		}

		// Figure out if we are dead yet
		if(m_health <= 0.0f)
		{
			m_health = 0.0f;

			finalDeathHit = true;

			if(m_eEnemyType != eEnemyType_WalkingZombie)
			{
				// Spawn loot items
				SpawnLootItems();

				// If we are the players target enemy, reset players target
				if(m_pPlayer->GetTargetEnemy() == this)
				{
					VoxGame::GetInstance()->ReleaseEnemyTarget();
				}
			}
			else
			{
				// Spawn gibs
				SpawnGibs();
			}

			// Explode the voxel model
			Explode();
		}

		// Update the HUD
		if(shouldUpdateGUI)
		{
			m_pHUD->SetEnemyHealth(m_health, m_maxHealth, damageDone);
			m_pHUD->SetEnemyName(m_enemyName.c_str());
		}

		if(finalDeathHit == false)
		{
			// Play a hit response animation
			if(m_eEnemyType == eEnemyType_NormalSkeleton || m_eEnemyType == eEnemyType_RangedSkeleton || m_eEnemyType == eEnemyType_MeleeSkeleton || m_eEnemyType == eEnemyType_MageSkeleton || m_eEnemyType == eEnemyType_Mummy || m_eEnemyType == eEnemyType_WalkingZombie)
			{
				if (m_bIsChargingAttack == false)
				{
					SetAnimationSpeed(1.0f, false, AnimationSections_FullBody);
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "HitResponse", 0.01f);
				}
			}
		}

		// Do an animated text effect
		vec3 screenposition = GetCenter() + vec3(GetRandomNumber(-1, 1, 2)*0.25f, 0.0f, GetRandomNumber(-1, 1, 2)*0.25f);
		char damageText[32];
		sprintf(damageText, "%i", (int)amount);
		AnimatedText* lpTestTextEffect = m_pTextEffectsManager->CreateTextEffect(VoxGame::GetInstance()->GetFrontendManager()->GetTextEffectFont(), VoxGame::GetInstance()->GetFrontendManager()->GetTextEffectOutlineFont(), VoxGame::GetInstance()->GetDefaultViewport(), TextDrawMode_3D_Screen, TextEffect_FadeUp, TextDrawStyle_Outline, screenposition, textColour, Colour(0.0f, 0.0f, 0.0f), damageText, 1.0f);
		lpTestTextEffect->SetAutoDelete(true);
		lpTestTextEffect->StartEffect();

		if(createParticleHit && m_health > 0.0f)
		{
			// Do a hit particle effect
			vec3 hitParticlePos = GetCenter() - (normalize(knockbackDirection) * m_radius);
			unsigned int effectId = -1;
			BlockParticleEffect* pBlockParticleEffect = VoxGame::GetInstance()->GetBlockParticleManager()->ImportParticleEffect("media/gamedata/particles/combat_hit.effect", hitParticlePos, &effectId);
			pBlockParticleEffect->PlayEffect();
		}

		m_damageTimer = m_damageTime;

		m_movementFreezeTimer = 0.35f;
	}

	if(m_knockbackTimer <= 0.0f)
	{
		if(m_spawning == false)
		{
			//if(finalDeathHit == false) // Do we need knockback on death? converting to other enemy types causes a change in position if knockback
			{
				m_velocity += knockbackDirection * knockbackAmount;
			}
		}

		m_knockbackTimer = m_knockbackTime;
	}
}

void Enemy::CheckPlayerDamageRadius()
{
	if(m_pPlayer->GetAttackEnabled() == false)
	{
		// If attack is not enabled, just return
		return;
	}

	vec3 distance = GetCenter() - m_pPlayer->GetCenter();
	float lengthToPlayer = length(distance);

	if(lengthToPlayer <= m_radius + m_pPlayer->GetAttackRadius())
	{
		vec3 distance_minus_y = distance;
		distance_minus_y.y = 0.0f;
		vec3 direction = normalize(distance_minus_y);
		direction = normalize(direction);
		
		// Figure out the attack vector, based on the attack rotation
		float playerRotation = m_pPlayer->GetRotation();
		float attackRotation = m_pPlayer->GetAttackRotation();
		float angle = DegToRad(playerRotation + attackRotation);
		vec3 attackDirection = vec3(sin(angle), 0.0f, cos(angle));
		float dotProduct = dot(direction, attackDirection);

		if(dotProduct > m_pPlayer->GetAttackSegmentAngle()) // Check if we are within the attack segment
		{	
			vec3 knockbackDirection;
			if(m_eEnemyType == eEnemyType_Bee || m_eEnemyType == eEnemyType_Bat || m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger)
			{
				knockbackDirection = (direction*2.0f);
			}
			else
			{
				knockbackDirection = (direction*2.0f) + vec3(0.0f, 1.0f, 0.0f);
			}

			knockbackDirection = normalize(knockbackDirection);
			Colour damageColour = Colour(1.0f, 1.0f, 1.0f);

			float knockbackAmount = 16.0f;
			if (m_eEnemyType == eEnemyType_Bee || m_eEnemyType == eEnemyType_Bat || m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger)
			{
				knockbackAmount = 1.0f;
			}
			DoDamage(20.0f, damageColour, knockbackDirection, knockbackAmount, true, true);
		}
	}
}

void Enemy::CheckNPCDamageRadius()
{
	int numNPCs = m_pNPCManager->GetNumNPCs();

	for(int i = 0; i < numNPCs; i++)
	{
		NPC *pNPC = m_pNPCManager->GetNPC(i);

		if(pNPC->NeedsErasing() == true)
		{
			continue;
		}

		if(pNPC->GetAttackEnabled() == false)
		{
			continue;
		}

		vec3 distance = GetCenter() - pNPC->GetCenter();
		float lengthToNPC = length(distance);
		if(lengthToNPC <= m_radius + pNPC->GetAttackRadius())
		{
			vec3 distance_minus_y = distance;
			distance_minus_y.y = 0.0f;
			vec3 direction = normalize(distance_minus_y);
			direction = normalize(direction);

			// Figure out the attack vector, based on the attack rotation
			float NPCRotation = pNPC->GetRotation();
			float attackRotation = pNPC->GetAttackRotation();
			float angle = DegToRad(NPCRotation + attackRotation);
			vec3 attackDirection = vec3(sin(angle), 0.0f, cos(angle));
			float dotProduct = dot(direction, attackDirection);

			if(dotProduct > pNPC->GetAttackSegmentAngle()) // Check if we are within the attack segment
			{	
				vec3 knockbackDirection;
				if(m_eEnemyType == eEnemyType_Bee || m_eEnemyType == eEnemyType_Bat || m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger)
				{
					knockbackDirection = (direction*2.0f);
				}
				else
				{
					knockbackDirection = (direction*2.0f) + vec3(0.0f, 1.0f, 0.0f);
				}

				knockbackDirection = normalize(knockbackDirection);
				Colour damageColour = Colour(1.0f, 1.0f, 1.0f);

				// Set NPC target as attacker
				SetTargetNPC(pNPC);

				float knockbackAmount = 16.0f;
				if (m_eEnemyType == eEnemyType_Bee || m_eEnemyType == eEnemyType_Bat || m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger)
				{
					knockbackAmount = 1.0f;
				}
				DoDamage(20.0f, damageColour, knockbackDirection, knockbackAmount, true, false);
			}
		}
	}
}

void Enemy::CheckProjectileDamageRadius(Projectile* pProjectile)
{
	if(pProjectile->CanAttackEnemies() == false)
	{
		return;
	}

	vec3 projectileHitboxCenter = GetProjectileHitboxCenter();

	vec3 toProjectile = projectileHitboxCenter - pProjectile->GetCenter();

	bool hitByProjectile = false;
	if(m_eProjectileHitboxType == eProjectileHitboxType_Sphere)
	{
		if(length(toProjectile) < GetRadius() + pProjectile->GetRadius())
		{
			hitByProjectile = true;
		}
	}
	else if(m_eProjectileHitboxType == eProjectileHitboxType_Cube)
	{
		Plane3D planes[6];
		Matrix4x4 rotationMatrix;
		rotationMatrix.SetYRotation(DegToRad(GetRotation()));
		planes[0] = Plane3D(rotationMatrix * vec3(-1.0f, 0.0f, 0.0f), projectileHitboxCenter + (rotationMatrix * vec3(m_projectileHitboxXLength, 0.0f, 0.0f)));
		planes[1] = Plane3D(rotationMatrix * vec3(1.0f, 0.0f, 0.0f),  projectileHitboxCenter + (rotationMatrix * vec3(-m_projectileHitboxXLength, 0.0f, 0.0f)));
		planes[2] = Plane3D(rotationMatrix * vec3(0.0f, -1.0f, 0.0f), projectileHitboxCenter + (rotationMatrix * vec3(0.0f, m_projectileHitboxYLength, 0.0f)));
		planes[3] = Plane3D(rotationMatrix * vec3(0.0f, 1.0f, 0.0f),  projectileHitboxCenter + (rotationMatrix * vec3(0.0f, -m_projectileHitboxYLength, 0.0f)));
		planes[4] = Plane3D(rotationMatrix * vec3(0.0f, 0.0f, -1.0f), projectileHitboxCenter + (rotationMatrix * vec3(0.0f, 0.0f, m_projectileHitboxZLength)));
		planes[5] = Plane3D(rotationMatrix * vec3(0.0f, 0.0f, 1.0f),  projectileHitboxCenter + (rotationMatrix * vec3(0.0f, 0.0f, -m_projectileHitboxZLength)));

		float distance;
		int inside = 0;
		vec3 normal;

		for(int i = 0; i < 6; i++)
		{
			distance = planes[i].GetPointDistance(pProjectile->GetCenter());

			if (distance < -pProjectile->GetRadius())
			{
				// Outside...
			}
			else if (distance < pProjectile->GetRadius())
			{
				// Intersecting..
				inside++;
			}
			else
			{
				// Inside...
				inside++;
			}
		}

		if(inside == 6)
		{
			hitByProjectile = true;
		}
	}

	if(hitByProjectile)
	{
		vec3 knockbackDirection;
		if(m_eEnemyType == eEnemyType_Bee || m_eEnemyType == eEnemyType_Bat || m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger)
		{
			knockbackDirection = (normalize(pProjectile->GetVelocity())*2.0f);
		}
		else
		{
			knockbackDirection = (normalize(pProjectile->GetVelocity())*2.0f) + vec3(0.0f, 1.0f, 0.0f);
		}

		knockbackDirection = normalize(knockbackDirection);
		Colour damageColour = Colour(1.0f, 1.0f, 1.0f);

		// Set NPC target as attacker, if they owned the projectile
		bool playerDamage = (pProjectile->GetPlayerOwner() != nullptr);
		if(pProjectile->GetNPCOwner() != nullptr)
		{
			SetTargetNPC(pProjectile->GetNPCOwner());
		}

		float knockbackAmount = 16.0f;
		if (m_eEnemyType == eEnemyType_Bee || m_eEnemyType == eEnemyType_Bat || m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger)
		{
			knockbackAmount = 1.0f;
		}
		DoDamage(15.0f, damageColour, knockbackDirection, knockbackAmount, false, playerDamage);

		if(pProjectile->IsReturnToPlayer() == false)
		{
			pProjectile->Explode();
		}
	}
}

void Enemy::Explode()
{
	if(m_erase == true)
	{
		return;
	}

	CalculateWorldTransformMatrix();

	if(m_eEnemyType == eEnemyType_WalkingZombie)
	{
		const QubicleBinary* pQubicleModel = m_pVoxelCharacter->GetQubicleModel();

		int legsIndex = pQubicleModel->GetMatrixIndexForName("Legs");
		const QubicleMatrix* pLegsMatrix = pQubicleModel->GetQubicleMatrix(legsIndex);
		m_pBlockParticleManager->ExplodeQubicleMatrix(pLegsMatrix, m_pVoxelCharacter->GetCharacterScale(), 100, false, 1.0f, 1.0f, 1.0f, 1.0f);

		int leftFootIndex = pQubicleModel->GetMatrixIndexForName("Left_Foot");
		const QubicleMatrix* pLeftFootMatrix = pQubicleModel->GetQubicleMatrix(leftFootIndex);
		m_pBlockParticleManager->ExplodeQubicleMatrix(pLeftFootMatrix, m_pVoxelCharacter->GetCharacterScale(), 100, false, 1.0f, 1.0f, 1.0f, 1.0f);

		int rightFootIndex = pQubicleModel->GetMatrixIndexForName("Right_Foot");
		const QubicleMatrix* pRightFootMatrix = pQubicleModel->GetQubicleMatrix(rightFootIndex);
		m_pBlockParticleManager->ExplodeQubicleMatrix(pRightFootMatrix, m_pVoxelCharacter->GetCharacterScale(), 100, false, 1.0f, 1.0f, 1.0f, 1.0f);

		ConvertIntoOtherEnemyType(eEnemyType_CrawlingZombie, 0.08f);

		m_pVoxelCharacter->PlayAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "BindPose");	
		SetAnimationSpeed(0.35f, true, AnimationSections_FullBody);
		m_pVoxelCharacter->Update(0.01f, m_animationSpeed);
		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "ZombieCrawl", 0.15f);	

		m_movementFreezeTimer = 0.5f;

		return;
	}

	for(int explodeCounter = 0; explodeCounter < 3; explodeCounter++)
	{
		const QubicleBinary* pQubicleModel = nullptr;
		if(explodeCounter == 0)
		{
			pQubicleModel = m_pVoxelCharacter->GetQubicleModel();
		}
		else if(explodeCounter == 1)
		{
			if(m_pVoxelCharacter->GetRightWeapon() != nullptr)
			{
				if(m_pVoxelCharacter->IsRightWeaponLoaded())
				{
					for(int animatedSectionsIndex = 0; animatedSectionsIndex < m_pVoxelCharacter->GetRightWeapon()->GetNumAimatedSections(); animatedSectionsIndex++)
					{
						AnimatedSection* pAnimatedSection = m_pVoxelCharacter->GetRightWeapon()->GetAnimatedSection(animatedSectionsIndex);
						pQubicleModel = pAnimatedSection->m_pVoxelObject->GetQubicleModel();
					}
				}
			}
		}
		else if(explodeCounter == 2)
		{
			if(m_pVoxelCharacter->GetLeftWeapon() != nullptr)
			{
				if(m_pVoxelCharacter->IsLeftWeaponLoaded())
				{
					for(int animatedSectionsIndex = 0; animatedSectionsIndex < m_pVoxelCharacter->GetLeftWeapon()->GetNumAimatedSections(); animatedSectionsIndex++)
					{
						AnimatedSection* pAnimatedSection = m_pVoxelCharacter->GetLeftWeapon()->GetAnimatedSection(animatedSectionsIndex);
						pQubicleModel = pAnimatedSection->m_pVoxelObject->GetQubicleModel();
					}
				}
			}
		}

		if(pQubicleModel != nullptr)
		{
			m_pBlockParticleManager->ExplodeQubicleBinary(pQubicleModel, m_pVoxelCharacter->GetCharacterScale(), 50);
		}
	}

	// Signal to the player and NPC manager that this enemy died, clear linkage
	m_pPlayer->SetEnemyDied(this);
	m_pNPCManager->SetEnemyDied(this);

	// Signal to the projectile manager that this enemy died, clear linkage
	m_pProjectileManager->ClearProjectileOwnerForEnemy(this);

	// Give the player experience
	m_pPlayer->GetPlayerStats()->GiveExperience(10);

	if (m_eEnemyType == eEnemyType_Mimic)
	{
		VoxGame::GetInstance()->PlaySoundEffect3D(eSoundEffect_MimicDie, GetCenter());
	}

	m_erase = true;
}

void Enemy::ConvertIntoOtherEnemyType(eEnemyType newEnemyType, float scale)
{
	UnloadWeapon(true);
	UnloadWeapon(false);

	m_eEnemyType = newEnemyType;

	char characterBaseFolder[128];
	char qbFilename[128];
	char ms3dFilename[128];
	char animListFilename[128];
	char facesFilename[128];
	char characterFilename[128];

	SetupEnemyForType();

	sprintf(characterBaseFolder, "media/gamedata/models");
	sprintf(qbFilename, "media/gamedata/models/%s/%s.qb", m_typeString.c_str(), m_modelNameString.c_str());
	sprintf(ms3dFilename, "media/gamedata/models/%s/%s.ms3d", m_typeString.c_str(), m_typeString.c_str());
	sprintf(animListFilename, "media/gamedata/models/%s/%s.animlist", m_typeString.c_str(), m_typeString.c_str());
	sprintf(facesFilename, "media/gamedata/models/%s/%s.faces", m_typeString.c_str(), m_modelNameString.c_str());
	sprintf(characterFilename, "media/gamedata/models/%s/%s.character", m_typeString.c_str(), m_modelNameString.c_str());

	if(m_eEnemyType == eEnemyType_Doppelganger)
	{
		char playerFolder[128];
		sprintf(playerFolder, "saves/characters/%s", m_modelNameString.c_str());
		sprintf(qbFilename, "saves/characters/%s/%s.qb", m_modelNameString.c_str(), m_modelNameString.c_str());
		sprintf(facesFilename, "saves/characters/%s/%s.faces", m_modelNameString.c_str(), m_modelNameString.c_str());
		sprintf(characterFilename, "saves/characters/%s/%s.character", m_modelNameString.c_str(), m_modelNameString.c_str());
	}

	m_pVoxelCharacter->UnloadCharacter();
	m_pVoxelCharacter->Reset();
	bool useQubicleManager = (m_eEnemyType != eEnemyType_Doppelganger);
	m_pVoxelCharacter->LoadVoxelCharacter(m_typeString.c_str(), qbFilename, ms3dFilename, animListFilename, facesFilename, characterFilename, characterBaseFolder, useQubicleManager);

	m_pVoxelCharacter->SetBreathingAnimationEnabled(true);
	m_pVoxelCharacter->SetWinkAnimationEnabled(true);
	m_pVoxelCharacter->SetTalkingAnimationEnabled(false);
	m_pVoxelCharacter->SetRandomMouthSelection(false);
	m_pVoxelCharacter->SetRandomLookDirection(true);
	m_pVoxelCharacter->SetWireFrameRender(false);

	m_pVoxelCharacter->PlayAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "BindPose");	

	SetupWeaponsForType();

	InitEnemyForType();

	SetScale(scale);

	SetLeashParameters(GetCenter(), m_leashRadius);

	m_health = m_maxHealth;
}

void Enemy::Respawn()
{
	m_health = m_maxHealth;
}

bool Enemy::CanCheckAggroRange()
{
	if(m_eEnemyType == eEnemyType_Mimic)
	{
		if (m_aggro == false)
		{
			return false;
		}
	}

	return true;
}

void Enemy::SetNPCDied(NPC* pNPC)
{
	if(m_pTargetNPC == pNPC)
	{
		SetTargetNPC(nullptr);
	}
}

void Enemy::SetTargetNPC(NPC* pTargetNPC)
{
	m_pTargetNPC = pTargetNPC;
}

// Gameplay
void Enemy::SetSapped(bool sapped)
{
	if(m_sapped == false && sapped)
	{
		// Turn on
		m_pSappedParticleEffect = m_pBlockParticleManager->ImportParticleEffect("media/gamedata/particles/sapped.effect", GetAboveHeadPosition(), &m_sappedParticleEffectId);
		m_pSappedParticleEffect->PlayEffect();

		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "BindPose", 0.1f);
	}
	else if(m_sapped && sapped == false)
	{
		// Turn off
		m_pSappedParticleEffect->StopEffect();
		m_pSappedParticleEffect->m_erase = true;
		m_pSappedParticleEffect = nullptr;
		m_sappedParticleEffectId = -1;
		m_sappedTimer = 0.0f;
	}

	m_sapped = sapped;

	if(m_sapped)
	{
		// Refresh sapped timer
		m_sappedTimer = 10.0f;
	}
}

bool Enemy::IsSapped()
{
	return m_sapped;
}

// Spawning
void Enemy::SetSpawningParams(vec3 start, vec3 end, float time)
{
	m_spawning = true;
	m_spawnStartPosition = start;
	m_spawnEndPosition = end;
	m_spawnTime = time;
	m_spawningTimer = time;

	if(m_eEnemyType == eEnemyType_WalkingZombie || m_eEnemyType == eEnemyType_NormalSkeleton || m_eEnemyType == eEnemyType_MeleeSkeleton || m_eEnemyType == eEnemyType_RangedSkeleton || m_eEnemyType == eEnemyType_MageSkeleton)
	{
		m_pVoxelCharacter->Update(0.01f, m_animationSpeed);
		SetAnimationSpeed(0.75f, true, AnimationSections_FullBody);
		m_pVoxelCharacter->PlayAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "ZombieSpawn");
	}
	else if(m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger)
	{
		m_pVoxelCharacter->Update(0.01f, m_animationSpeed);
		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "GhostPose", time);
	}

	Interpolator::GetInstance()->AddFloatInterpolation(&m_spawningT, 0.0f, 1.0f, time, -100.0f);
}

// Wander / Random target
void Enemy::SetRandomTargetMode(bool randomTargetMode)
{
	m_bRandomTargetPositionMode = randomTargetMode;
}

void Enemy::GetNewRandomTargetPosition()
{
	m_getRamdomTargetPositionTimer = 3.5f;
	float randomDistance = (GetRandomNumber(30, 100, 1) * 0.01f)*m_leashRadius;
	m_randomTargetPosition = m_leashPosition + (normalize(vec3(GetRandomNumber(-1, 1, 2), 0.0f, GetRandomNumber(-1, 1, 2)))*randomDistance);

	m_bStuck = false;
	m_bUpdateStuckTimer = false;
	m_stuckTimer = 5.0f;
}

void Enemy::ResetRandomTargetPosition()
{
	m_randomTargetPosition = m_position;
}

void Enemy::SetLeashParameters(vec3 leashPosition, float leashRadius)
{
	m_leashPosition = leashPosition;
	m_leashRadius = leashRadius;
}

// Attacking
void Enemy::Attack()
{
	bool doAttack = false;

	if(m_attackDelayTimer <= 0.0f)
	{
		if(m_eEnemyType == eEnemyType_Bee)
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "StingAttack", 0.01f);
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Head_Body, false, AnimationSections_Head_Body, "StingAttack", 0.01f);

			m_attackSegmentAngle = 0.75f;
			m_attackDelayTime = 1.0f;
			float attackTime = 0.60f;
			float startRotation = 0.0f;
			float endRotation = 0.0f;
			float easingRotation = 0.0f;

			m_attackEnabled = true;
			m_attackEnabledTimer = 0.0f;
			m_attackEnabledDelayTimer = 0.3f;
			m_attackRotation = startRotation;
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledTimer, 0.0f, attackTime, attackTime, 0.0f, nullptr, _AttackEnabledTimerFinished, this);
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledDelayTimer, m_attackEnabledDelayTimer, 0.0f, m_attackEnabledDelayTimer, 0.0f, nullptr, _AttackEnabledDelayTimerFinished, this);
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackRotation, startRotation, endRotation, attackTime, easingRotation);

			doAttack = true;
		}
		else if(m_eEnemyType == eEnemyType_Bat)
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "StingAttack", 0.1f);
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Head_Body, false, AnimationSections_Head_Body, "StingAttack", 0.1f);

			m_attackDelayTime = 1.75f + GetRandomNumber(-100, 50, 2) * 0.005f;

			m_attackEnabledDelayTimer = 0.35f;
			float attackTime = 0.60f;
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledDelayTimer, m_attackEnabledDelayTimer, 0.0f, m_attackEnabledDelayTimer, 0.0f, nullptr, _AttackEnabledDelayTimerFinished, this);
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledTimer, 0.0f, attackTime, attackTime, 0.0f, nullptr, _AttackEnabledTimerFinished, this);

			doAttack = true;
		}
		else if(m_eEnemyType == eEnemyType_GreenSlime || m_eEnemyType == eEnemyType_RedSlime || m_eEnemyType == eEnemyType_BlueSlime || m_eEnemyType == eEnemyType_YellowSlime)
		{
		}
		else if(m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger)
		{
		}
		else if(m_eEnemyType == eEnemyType_Mimic)
		{
		}
		else if(m_eEnemyType == eEnemyType_IronGiant)
		{
			m_attackDelayTime = 1.0f;

			m_attackEnabledDelayTimer = 0.15f;
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledDelayTimer, m_attackEnabledDelayTimer, 0.0f, m_attackEnabledDelayTimer, 0.0f, nullptr, _AttackEnabledDelayTimerFinished, this);

			doAttack = true;
		}
		else if(m_eEnemyType == eEnemyType_NormalSkeleton)
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_FullBody, "SwordAttack2", 0.01f);

			m_attackDelayTime = 1.75f + GetRandomNumber(-50, 25, 2) * 0.005f;

			m_attackEnabledDelayTimer = 0.15f;
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledDelayTimer, m_attackEnabledDelayTimer, 0.0f, m_attackEnabledDelayTimer, 0.0f, nullptr, _AttackEnabledDelayTimerFinished, this);

			doAttack = true;
		}
		else if(m_eEnemyType == eEnemyType_MageSkeleton)
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "StaffAttack", 0.01f);

			m_attackDelayTime = 1.0f + GetRandomNumber(-100, 50, 2) * 0.005f;

			m_attackEnabledDelayTimer = 0.15f;
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledDelayTimer, m_attackEnabledDelayTimer, 0.0f, m_attackEnabledDelayTimer, 0.0f, nullptr, _AttackEnabledDelayTimerFinished, this);

			doAttack = true;
		}
		else if(m_eEnemyType == eEnemyType_RangedSkeleton)
		{
			if(m_bIsChargingAttack == false)
			{
				m_bIsChargingAttack = true;
				m_chargeAmount = 0.0f;

				LoadWeapon(false, "media/gamedata/items/Arrow/ArrowHold.item");

				if(m_bIsIdle)
				{
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "BowCharge", 0.2f);
				}
				else
				{
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Head_Body, false, AnimationSections_Head_Body, "BowCharge", 0.2f);
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "BowCharge", 0.2f);
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "BowCharge", 0.2f);
				}

				doAttack = true;
			}
		}
		else if(m_eEnemyType == eEnemyType_MeleeSkeleton)
		{
			static int counter = 0;

			float attackTime = 0.0f;
			float startRotation = 0.0f;
			float endRotation = 0.0f;
			float easingRotation = 0.0f;

			if(counter == 2)
			{
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "SwordAttack3", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "SwordAttack3", 0.01f);
				counter = 0;
				m_bCanInteruptCombatAnim = false;

				m_attackSegmentAngle = 0.60f;
				m_attackEnabledDelayTimer = 0.0f;
				attackTime = 0.6f;
				startRotation = 360.0f;
				endRotation = -45.0f;
				easingRotation = -75.0f;
			}
			else
			{
				if(counter == 0)
				{
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_FullBody, "SwordAttack1", 0.01f);
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "SwordAttack1", 0.01f);

					m_attackSegmentAngle = 0.75f;
					m_attackEnabledDelayTimer = 0.1f;
					attackTime = 0.4f;
					startRotation = -20.0f;
					endRotation = -20.0f;
					easingRotation = 0.0f;
				}
				else
				{
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_FullBody, "SwordAttack2", 0.01f);
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "SwordAttack2", 0.01f);

					m_attackSegmentAngle = 0.75f;
					m_attackEnabledDelayTimer = 0.2f;
					attackTime = 0.4f;
					startRotation = -10.0f;
					endRotation = -10.0f;
					easingRotation = 0.0f;
				}
				counter++;
			}

			m_movementWaitAfterAttackTimer = 0.5f;
			m_attackDelayTime = 0.35f;
			m_attackEnabled = true;
			m_attackEnabledTimer = 0.0f;
			m_attackRotation = startRotation;
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledTimer, 0.0f, attackTime, attackTime, 0.0f, nullptr, _AttackEnabledTimerFinished, this);
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledDelayTimer, m_attackEnabledDelayTimer, 0.0f, m_attackEnabledDelayTimer, 0.0f, nullptr, _AttackEnabledDelayTimerFinished, this);
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackRotation, startRotation, endRotation, attackTime, easingRotation);

			// Start weapon trails
			if(m_pVoxelCharacter->GetRightWeapon())
			{
				m_pVoxelCharacter->GetRightWeapon()->StartWeaponTrails();
			}

			doAttack = true;
		}
	}

	// Only setup the delay timer and animation speed change if we are actually going to attack
	if(doAttack)
	{
		m_animationFinished[AnimationSections_Right_Arm_Hand] = true;
		SetAnimationSpeed(1.0f, true, AnimationSections_FullBody);

		m_attackDelayTimer = m_attackDelayTime;

		m_bCanAttack = false;
	}
}

void Enemy::ReleaseAttack()
{
	if(m_bIsChargingAttack == true)
	{
		UnloadWeapon(false);

		Projectile* pProjectile = m_pProjectileManager->CreateProjectile(m_chargeSpawnPosition, m_chargeSpawnVelocity, 0.0f, "media/gamedata/items/Arrow/arrow.item", 0.08f);	
		pProjectile->SetProjectileType(false, true, true);
		pProjectile->SetOwner(nullptr, nullptr, this);

		m_bIsChargingAttack = false;
		m_chargeAmount = 0.0f;

		m_attackDelayTimer = m_attackDelayTime*2.0f;

		if(m_bIsIdle)
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "BindPose", 0.2f);
		}
		else
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Head_Body, false, AnimationSections_Head_Body, "BindPose", 0.2f);
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "BindPose", 0.2f);
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "BindPose", 0.2f);
		}
	}
}

bool Enemy::CanAttack()
{
	return m_bCanAttack;
}

bool Enemy::GetAttackEnabled()
{
	return m_attackEnabled && (m_attackEnabledDelayTimer <= 0.0f);
}

float Enemy::GetAttackRadius()
{
	return m_attackRadius;
}

float Enemy::GetAttackRotation()
{
	return m_attackRotation;
}

float Enemy::GetAttackSegmentAngle()
{
	return m_attackSegmentAngle;
}

// Projectile hitbox
eProjectileHitboxType Enemy::GetProjectileHitboxType()
{
	return m_eProjectileHitboxType;
}

float Enemy::GetProjectileHitboxRadius()
{
	return m_projectileHitboxRadius;
}

float Enemy::GetProjectileHitboxXLength()
{
	return m_projectileHitboxXLength;
}

float Enemy::GetProjectileHitboxYLength()
{
	return m_projectileHitboxYLength;
}

float Enemy::GetProjectileHitboxZLength()
{
	return m_projectileHitboxZLength;
}

vec3 Enemy::GetProjectileHitboxCenter()
{
	return GetCenter() + m_projectileHitboxCenterOffset;
}

// Spawn loot items
void Enemy::SpawnLootItems()
{
	// Create the random ingredients drops
	eItem item;
	int quantity;
	VoxGame::GetInstance()->GetRandomLootManager()->GetSpawnedIngredientItemForEnemy(m_eEnemyType, &item, &quantity);
	if (quantity != 0 && item != eItem_None)
	{
		for (int i = 0; i < quantity; i++)
		{
			float radius = GetRadius();
			float angle = DegToRad(GetRandomNumber(0, 360, 1));
			vec3 ItemPosition = GetCenter() + vec3(cos(angle) * radius, 0.0f, sin(angle) * radius);
			vec3 gravity = vec3(0.0f, -1.0f, 0.0f);
			gravity = normalize(gravity);

			Item* pItem = m_pItemManager->CreateItem(GetCenter(), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), GetItemFilenameForType(item).c_str(), eItem_DroppedItem, GetItemTitleForType(item).c_str(), true, false, 0.08f);

			if (pItem != nullptr)
			{
				pItem->SetGravityDirection(gravity);
				vec3 vel = ItemPosition - GetCenter();
				pItem->SetVelocity(normalize(vel)*(float)GetRandomNumber(2, 4, 2) + vec3(0.0f, 9.5f + GetRandomNumber(-2, 4, 2), 0.0f));
				pItem->SetRotation(vec3(0.0f, GetRandomNumber(0, 360, 2), 0.0f));
				pItem->SetAngularVelocity(vec3(0.0f, 90.0f, 0.0f));
				pItem->SetCollisionEnabled(false);

				InventoryItem* pInventoryItem = VoxGame::GetInstance()->GetInventoryManager()->CreateInventoryItemForCrafting(item, 1, ItemQuality_Common);
				pItem->SetDroppedItem(pInventoryItem);

				int numY = pItem->GetVoxelItem()->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->GetQubicleMatrix(0)->m_matrixSizeY;
				pItem->GetVoxelItem()->SetRenderOffset(vec3(0.0f, numY*0.5f, 0.0f));
			}
		}
	}

	// Create random loot
	if (m_eEnemyType == eEnemyType_Mimic)
	{
		eEquipment equipment = eEquipment_None;
		InventoryItem* pRandomLoot = VoxGame::GetInstance()->GetRandomLootManager()->GetRandomLootItem(&equipment);

		if (pRandomLoot != nullptr && equipment != eEquipment_None)
		{
			float radius = GetRadius();
			float angle = DegToRad(GetRandomNumber(0, 360, 1));
			vec3 ItemPosition = GetCenter() + vec3(cos(angle) * radius, 0.0f, sin(angle) * radius);
			vec3 gravity = vec3(0.0f, -1.0f, 0.0f);
			gravity = normalize(gravity);

			Item* pItem = m_pItemManager->CreateItem(GetCenter(), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), GetEquipmentFilenameForType(equipment).c_str(), eItem_DroppedItem, GetEquipmentTitleForType(equipment).c_str(), true, false, 0.08f);

			if (pItem != nullptr)
			{
				pItem->SetGravityDirection(gravity);
				vec3 vel = ItemPosition - GetCenter();
				pItem->SetVelocity(normalize(vel)*(float)GetRandomNumber(2, 4, 2) + vec3(0.0f, 9.5f + GetRandomNumber(-2, 4, 2), 0.0f));
				pItem->SetRotation(vec3(0.0f, GetRandomNumber(0, 360, 2), 0.0f));
				pItem->SetAngularVelocity(vec3(0.0f, 90.0f, 0.0f));
				pItem->SetCollisionEnabled(false);

				pItem->SetDroppedItem(pRandomLoot);

				int numY = pItem->GetVoxelItem()->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->GetQubicleMatrix(0)->m_matrixSizeY;
				pItem->GetVoxelItem()->SetRenderOffset(vec3(0.0f, numY*0.5f, 0.0f));
			}
		}
	}

	// Create items from the killed enemy (coins, hearts)
	int numItems = GetRandomNumber(3, 8);
	for(int i = 0; i < numItems; i++)
	{
		float scale = 0.15f;
		float lifeTime = 1.0f;
		float r = 0.13f;
		float g = 0.65f;
		float b = 1.0f;
		float a = 1.0f;

		float radius = GetRadius();
		float angle = DegToRad(GetRandomNumber(0, 360, 1));
		vec3 ItemPosition = GetCenter() + vec3(cos(angle) * radius, 0.0f, sin(angle) * radius);

		vec3 gravity = vec3(0.0f, -1.0f, 0.0f);
		gravity = normalize(gravity);
		Item* pItem = nullptr;
		if(GetRandomNumber(0, 100) > 90)
		{
			pItem = m_pItemManager->CreateItem(GetCenter(), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), GetItemFilenameForType(eItem_Heart).c_str(), eItem_Heart, GetItemTitleForType(eItem_Heart).c_str(), false, true, 0.03f);
		}
		else
		{
			pItem = m_pItemManager->CreateItem(GetCenter(), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), GetItemFilenameForType(eItem_Coin).c_str(), eItem_Coin, GetItemTitleForType(eItem_Coin).c_str(), false, true, 0.0225f);
		}

		if(pItem != nullptr)
		{
			pItem->SetGravityDirection(gravity);
			vec3 vel = ItemPosition - GetCenter();
			pItem->SetVelocity(normalize(vel)*(float)GetRandomNumber(2, 4, 2) + vec3(0.0f, 9.5f+GetRandomNumber(-2, 4, 2), 0.0f));
			pItem->SetRotation(vec3(0.0f, GetRandomNumber(0, 360, 2), 0.0f));
			pItem->SetAngularVelocity(vec3(0.0f, 90.0f, 0.0f));

			pItem->SetAutoDisappear(20.0f + (GetRandomNumber(-20, 20, 1) * 0.2f));
			pItem->SetCollisionEnabled(false);
		}
	}
}

void Enemy::SpawnGibs()
{
	int numGibs = 4;
	for(int i = 0; i < numGibs; i++)
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
		Item* pItem = nullptr;
		if(i == 0)
		{
			pItem = m_pItemManager->CreateItem(GetCenter(), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), "media/gamedata/items/ZombieGibs/SpineGib.item", eItem_Gib, "Gib1", false, false, 0.08f);
		}
		else if(i == 1)
		{
			pItem = m_pItemManager->CreateItem(GetCenter(), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), "media/gamedata/items/ZombieGibs/FootGib.item", eItem_Gib, "Gib2", false, false, 0.08f);
		}
		else if(i == 2)
		{
			pItem = m_pItemManager->CreateItem(GetCenter(), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), "media/gamedata/items/ZombieGibs/Hand2Gib.item", eItem_Gib, "Gib3", false, false, 0.08f);
		}
		else if(i == 3)
		{
			pItem = m_pItemManager->CreateItem(GetCenter(), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), "media/gamedata/items/ZombieGibs/LegsGib.item", eItem_Gib, "Gib4", false, false, 0.08f);
		}

		if(pItem != nullptr)
		{
			pItem->SetGravityDirection(gravity);
			vec3 vel = ItemPosition - GetCenter();
			pItem->SetVelocity(normalize(vel)*(float)GetRandomNumber(3, 7, 2) + vec3(0.0f, 5.0f+GetRandomNumber(-2, 5, 2), 0.0f));
			pItem->SetRotation(vec3(0.0f, GetRandomNumber(0, 360, 2), 0.0f));
			//pItem->SetAngularVelocity(vec3(0.0f, 90.0f, 0.0f));
			pItem->SetCollisionEnabled(false);
			pItem->SetAutoDisappear(10.0f + (GetRandomNumber(-20, 20, 1) * 0.2f));
		}
	}
}

// Rendering Helpers
void Enemy::CalculateWorldTransformMatrix()
{
	vec3 lForward = m_forward;
	vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
	if(lForward.x == 0.0f && lForward.z == 0.0f)
	{
		if(lForward.y < 0.0f)
		{
			lUp = vec3(1.0f, 0.0f, 0.0f);
		}
		else
		{
			lUp = vec3(-1.0f, 0.0f, 0.0f);
		}
	}
	vec3 lRight = normalize(cross(lUp, lForward));

	lUp = normalize(cross(lForward, lRight));

	float lMatrix[16] =
	{
		lRight.x, lRight.y, lRight.z, 0.0f,
		lUp.x, lUp.y, lUp.z, 0.0f,
		lForward.x, lForward.y, lForward.z, 0.0f,
		m_position.x, m_position.y, m_position.z, 1.0f
	};

	m_worldMatrix.SetValues(lMatrix);
}

void Enemy::SetOutlineRender(bool outline)
{
	m_outlineRender = outline;
}

bool Enemy::GetOutlineRender()
{
	return m_outlineRender;
}

void Enemy::SetWireFrameRender(bool wireframe)
{
	if(m_pVoxelCharacter != nullptr)
	{
		m_pVoxelCharacter->SetWireFrameRender(wireframe);
	}
}

// Updating
void Enemy::UpdateWeaponLights(float dt)
{
	for (int i = 0; i < 2; i++)
	{
		VoxelWeapon* pWeapon = nullptr;
		bool isWeaponLoaded = false;
		if (i == 0)  // Right side
		{
			pWeapon = m_pVoxelCharacter->GetRightWeapon();
			isWeaponLoaded = m_pVoxelCharacter->IsRightWeaponLoaded();
		}
		else  // Left side
		{
			pWeapon = m_pVoxelCharacter->GetLeftWeapon();
			isWeaponLoaded = m_pVoxelCharacter->IsLeftWeaponLoaded();
		}

		if (pWeapon != nullptr)
		{
			if (isWeaponLoaded)
			{
				for (int i = 0; i < pWeapon->GetNumLights(); i++)
				{
					unsigned int lightId;
					vec3 lightPos;
					float lightRadius;
					float lightDiffuseMultiplier;
					Colour lightColour;
					bool connectedToSegment;
					pWeapon->GetLightParams(i, &lightId, &lightPos, &lightRadius, &lightDiffuseMultiplier, &lightColour, &connectedToSegment);

					if (lightId == -1)
					{
						m_pLightingManager->AddLight(vec3(0.0f), 0.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f, 1.0f), &lightId);
						pWeapon->SetLightingId(i, lightId);
					}

					if (connectedToSegment == false)
					{
						// Rotate due to characters forward vector
						float rotationAngle = acos(dot(vec3(0.0f, 0.0f, 1.0f), m_forward));
						if (m_forward.x < 0.0f)
						{
							rotationAngle = -rotationAngle;
						}
						Matrix4x4 rotationMatrix;
						rotationMatrix.SetRotation(0.0f, rotationAngle, 0.0f);
						lightPos = rotationMatrix * lightPos;

						// Translate to position
						lightPos += m_position;
					}

					float scale = m_pVoxelCharacter->GetCharacterScale();

					m_pLightingManager->UpdateLightPosition(lightId, vec3(lightPos.x, lightPos.y, lightPos.z));
					m_pLightingManager->UpdateLightRadius(lightId, lightRadius * scale);
					m_pLightingManager->UpdateLightDiffuseMultiplier(lightId, lightDiffuseMultiplier);
					m_pLightingManager->UpdateLightColour(lightId, lightColour);
				}
			}
		}
	}
}

void Enemy::UpdateWeaponParticleEffects(float dt)
{
	for (int i = 0; i < 2; i++)
	{
		VoxelWeapon* pWeapon = nullptr;
		bool isWeaponLoaded = false;
		if (i == 0)  // Right side
		{
			pWeapon = m_pVoxelCharacter->GetRightWeapon();
			isWeaponLoaded = m_pVoxelCharacter->IsRightWeaponLoaded();
		}
		else  // Left side
		{
			pWeapon = m_pVoxelCharacter->GetLeftWeapon();
			isWeaponLoaded = m_pVoxelCharacter->IsLeftWeaponLoaded();
		}

		if (pWeapon != nullptr)
		{
			if (isWeaponLoaded)
			{
				for (int i = 0; i < pWeapon->GetNumParticleEffects(); i++)
				{
					unsigned int particleEffectId;
					vec3 ParticleEffectPos;
					vec3 ParticleEffectPos_NoWorldOffset;
					string effectName;
					bool connectedToSegment;
					pWeapon->GetParticleEffectParams(i, &particleEffectId, &ParticleEffectPos, &effectName, &connectedToSegment);

					if (particleEffectId == -1)
					{
						m_pBlockParticleManager->ImportParticleEffect(effectName, vec3(ParticleEffectPos.x, ParticleEffectPos.y, ParticleEffectPos.z), &particleEffectId);
						pWeapon->SetParticleEffectId(i, particleEffectId);
						m_pBlockParticleManager->SetRenderNoWoldOffsetViewport(particleEffectId, true);
					}

					ParticleEffectPos_NoWorldOffset = ParticleEffectPos;
					if (connectedToSegment == false)
					{
						// Rotate due to characters forward vector
						float rotationAngle = acos(dot(vec3(0.0f, 0.0f, 1.0f), m_forward));
						if (m_forward.x < 0.0f)
						{
							rotationAngle = -rotationAngle;
						}
						Matrix4x4 rotationMatrix;
						rotationMatrix.SetRotation(0.0f, rotationAngle, 0.0f);
						ParticleEffectPos = rotationMatrix * ParticleEffectPos;

						// Translate to position
						ParticleEffectPos += m_position;
					}

					m_pBlockParticleManager->UpdateParticleEffectPosition(particleEffectId, ParticleEffectPos, ParticleEffectPos_NoWorldOffset);
				}
			}
		}
	}
}

void Enemy::Update(float dt)
{
	// Update grid position
	UpdateGridPosition();

	// Update timers
	UpdateTimers(dt);

	// Update spawning
	UpdateSpawning(dt);

	// Update look at point
	UpdateLookingAndForwardTarget(dt);

	// Update movement
	UpdateMovement(dt);

	// Update combat
	UpdateCombat(dt);

	// Update gameplay
	UpdateGameplay(dt);

	// Check for player attack damage
	CheckPlayerDamageRadius();

	// Check for NPC attack damage
	CheckNPCDamageRadius();

	if(m_pVoxelCharacter != nullptr)
	{
		m_pVoxelCharacter->Update(dt, m_animationSpeed);
		m_pVoxelCharacter->SetWeaponTrailsOriginMatrix(dt, m_worldMatrix);

		for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
		{
			m_animationFinished[i] = m_pVoxelCharacter->HasAnimationFinished((AnimationSections)i);
		}
	}

	// Reset the canAttack flag if our weapon arm animation is completed
	if(m_bCanAttack == false && (m_animationFinished[AnimationSections_Right_Arm_Hand] == true) || (m_eEnemyType == eEnemyType_Bee || m_eEnemyType == eEnemyType_Bat || m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger))
	{
		// Stop weapon trails
		if(m_pVoxelCharacter != nullptr)
		{
			if(m_pVoxelCharacter->GetRightWeapon())
			{
				if(m_pVoxelCharacter->IsRightWeaponLoaded())
				{
					m_pVoxelCharacter->GetRightWeapon()->StopWeaponTrails();
				}
			}
		}

		m_bCanAttack = true;
		m_bCanInteruptCombatAnim = true;
	}

	// Charging - figure out trajectory and velocity for projectile
	if(m_bIsChargingAttack)
	{
		m_chargeAmount += dt / m_chargeTime;

		if(m_chargeAmount > 1.0f)
		{
			m_chargeAmount = 1.0f;
		}
	}

	if(m_eEnemyType == eEnemyType_RangedSkeleton)
	{
		m_chargeSpawnPosition = GetCenter() + (m_forward*0.75f) + (GetUpVector()*0.5f);

		float liftAmount = 1.75f * m_chargeAmount;
		float powerAmount = 45.0f * m_chargeAmount;

		//if(m_pTargetEnemy)
		{
			// Player target
			vec3 toTarget;
			if(m_pTargetNPC != nullptr)
			{
				toTarget = m_pTargetNPC->GetCenter() - GetCenter();
			}
			else
			{
				toTarget = m_pPlayer->GetCenter() - GetCenter();
			}

			float toTargetDistance = length(toTarget);
			liftAmount += toTargetDistance * 0.20f;
			m_chargeSpawnVelocity = (normalize(toTarget) * powerAmount) + vec3(0.0f, liftAmount, 0.0f);
		}
	}

	// Update physics
	UpdatePhysics(dt);

	// Update the enemy light
	if (m_eEnemyType == eEnemyType_Doppelganger)
	{
		m_pLightingManager->UpdateLightPosition(m_enemyLightId, GetCenter());

		m_pEnemyParticleEffect->SetPosition(GetCenter() + vec3(0.0f, 0.75f, 0.0f));
	}
}

void Enemy::UpdatePhysics(float dt)
{
	// Gravity modifications for flying creatures
	vec3 acceleration;
	if (m_eEnemyType != eEnemyType_Bee && m_eEnemyType != eEnemyType_Bat && m_eEnemyType != eEnemyType_Ghost && m_eEnemyType != eEnemyType_Doppelganger)
	{
		if (m_spawning == false)
		{
			acceleration += (m_gravityDirection * 9.81f) * 5.0f;
		}
	}

	// Integrate velocity
	m_velocity += acceleration * dt;

	if (m_eEnemyType != eEnemyType_Ghost && m_eEnemyType != eEnemyType_Doppelganger)
	{
		if (m_spawning == false)
		{
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

						if (velocityToUse.y <= 0.0f)
						{
							if (m_bCanJump == false)
							{
								m_bCanJump = true;
							}
						}
					}
				}

				// Integrate position
				m_position += velocityToUse * dt;
			}
		}
	}

	// Drag for flying enemies
	if (m_eEnemyType == eEnemyType_Bee || m_eEnemyType == eEnemyType_Bat || m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger)
	{
		m_velocity *= 0.9975f;
	}

	if (m_eEnemyType != eEnemyType_TargetDummy && m_eEnemyType != eEnemyType_Mimic)
	{
		vec3 currentPos = m_position;
		vec3 difference = (currentPos - m_previousPosition);
		difference.y = 0.0f;
		float lengthResult = length(difference);
		if (lengthResult <= 0.025f)
		{
			m_bUpdateStuckTimer = true;
		}
		else
		{
			m_bUpdateStuckTimer = false;
			m_stuckTimer = 5.0f;
		}
	}

	m_previousPosition = GetCenter();
}

void Enemy::UpdateLookingAndForwardTarget(float dt)
{
	if(m_sapped)
	{
		return;
	}

	vec3 lootAt = m_lookPoint;
	if(m_eEnemyType == eEnemyType_Bee || m_eEnemyType == eEnemyType_Bat || m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger)
	{
		lootAt += vec3(0.0f, 0.5f, 0.0f);
	}

	vec3 toPoint = lootAt - GetCenter();
	toPoint = normalize(toPoint);

	if(m_bLookAtPoint)
	{
		m_targetForward = toPoint;

		if(m_eEnemyType != eEnemyType_Bee && m_eEnemyType != eEnemyType_Bat && m_eEnemyType != eEnemyType_Ghost && m_eEnemyType != eEnemyType_Doppelganger)
		{
			m_targetForward.y = 0.0f;
		}
	}

	m_targetForward = normalize(m_targetForward);

	if(length(m_forward - m_targetForward) <= m_bodyTurnStopThreshold)
	{
		m_bIsLookingAtPoint = true;
	}
	else
	{
		m_bIsLookingAtPoint = false;

		vec3 targetForwardToUse = m_targetForward;
		float dotToTargetForward = dot(m_targetForward, m_forward);
		if(dotToTargetForward <= -1.0f || dotToTargetForward >= 1.0f)
		{
			targetForwardToUse = cross(m_targetForward, vec3(0.0f, 1.0f, 0.0f));
		}

		vec3 toTarget = targetForwardToUse - m_forward;
		m_forward += (toTarget * dt) * m_bodyTurnSpeedMultiplier;
		m_forward = normalize(m_forward);
	}

	if(m_pVoxelCharacter != nullptr)
	{
		if(m_bLookAtPoint)
		{
			float angleMissing = dot(m_targetForward, m_forward);
			float angle = acos(angleMissing);

			if(angle < -1.0f)
				angle = -1.0f;
			if(angle > 1.0f)
				angle = 1.0f;

			if(RadToDeg(angle) > 80.0f)
				angle = DegToRad(80.0f);

			float tempX = (float)sin( (angle) );
			float tempZ = (float)cos( (angle) );

			vec3 crossProduct = cross(m_targetForward, m_forward);

			if(crossProduct.y > 0.0f)
				tempX = -tempX;

			if((m_bRandomTargetPositionMode && m_aggro == false) || m_eEnemyType == eEnemyType_Doppelganger)
			{
				toPoint.y = 0.0f;
			}

			vec3 lookDirection = vec3(tempX, toPoint.y, tempZ);

			m_pVoxelCharacter->SetFaceTargetDirection(lookDirection);
			m_pVoxelCharacter->SetRandomLookDirection(false);
			m_pVoxelCharacter->SetFaceLookToTargetSpeedMultiplier(4.0f);
		}
	}
}

void Enemy::UpdateCombat(float dt)
{
	if(m_sapped)
	{
		return;
	}

	if(m_eEnemyType == eEnemyType_TargetDummy)
	{
		return;
	}

	bool aggroCancel = false;
	float radius = 0.0f;
	vec3 center;
	if(m_pTargetNPC != nullptr)
	{
		aggroCancel = m_pTargetNPC->IsDead();
		radius = m_pTargetNPC->GetRadius();
		center = m_pTargetNPC->GetCenter();
	}
	else
	{
		aggroCancel = m_pPlayer->IsDead();
		radius = m_pPlayer->GetRadius();
		center = m_pPlayer->GetCenter();
	}

	if(aggroCancel)
	{
		if(m_aggro == true)
		{
			m_aggro = false;

			if(m_eEnemyType != eEnemyType_Mimic)
			{
				SetRandomLookMode();
			}
		}
	}
	else
	{
		// Check the aggro area and radius
		if(CanCheckAggroRange())
		{
			vec3 distance = center - GetCenter();
			float lengthToPlayer = length(distance);
			if(lengthToPlayer <= m_aggroLookRadius + radius)
			{
				vec3 distance_minus_y = distance;
				distance_minus_y.y = 0.0f;
				vec3 direction = normalize(distance_minus_y);
				direction = normalize(direction);

				// Figure out the look vector, based on the look rotation
				float rotation = GetRotation();
				float faceLookingAngle = GetFaceLookingRotation();
				float angle = DegToRad(rotation + faceLookingAngle);
				vec3 attackDirection = vec3(sin(angle), 0.0f, cos(angle));
				float dotProduct = dot(direction, attackDirection);

				if(dotProduct > m_aggroLookSegmentAngle) // Check if we are within the look segment
				{	
					if(m_aggro == false)
					{
						// If we are setting aggro for initial setting, reset and extend the attack delay timer
						m_attackDelayTimer = m_attackDelayTime*2.0f;
					}

					m_aggro = true;
					m_aggroResetTimer = m_aggroResetTime;

					if(m_eEnemyType == eEnemyType_IronGiant || m_eEnemyType == eEnemyType_RangedSkeleton || m_eEnemyType == eEnemyType_MageSkeleton || m_eEnemyType == eEnemyType_Bat)
					{
						StopMoving();
					}
				}
			}
		}
	}

	if(m_eEnemyType == eEnemyType_RangedSkeleton || m_eEnemyType == eEnemyType_NormalSkeleton || m_eEnemyType == eEnemyType_MageSkeleton || m_eEnemyType == eEnemyType_Bat || m_eEnemyType == eEnemyType_IronGiant)
	{
		// Update ranged combat
		UpdateRangedCombat(dt);
	}
	else if(m_eEnemyType == eEnemyType_GreenSlime || m_eEnemyType == eEnemyType_RedSlime || m_eEnemyType == eEnemyType_BlueSlime || m_eEnemyType == eEnemyType_YellowSlime || m_eEnemyType == eEnemyType_Mummy || m_eEnemyType == eEnemyType_WalkingZombie || m_eEnemyType == eEnemyType_CrawlingZombie || m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger || m_eEnemyType == eEnemyType_Mimic)
	{
		// Update touch combat
		UpdateTouchCombat(dt);
	}
	else if(m_eEnemyType == eEnemyType_MeleeSkeleton || m_eEnemyType == eEnemyType_Bee)
	{
		// Update melee combat
		UpdateMeleeCombat(dt);
	}
}

void Enemy::UpdateGameplay(float dt)
{
	if(m_sapped)
	{
		if(m_pSappedParticleEffect != nullptr)
		{
			m_pSappedParticleEffect->SetPosition(GetAboveHeadPosition());
		}
	}
}

void Enemy::UpdateMeleeCombat(float dt)
{
	if(m_pTargetNPC != nullptr)
	{
		if(m_pTargetNPC->IsDead())
		{
			return;
		}
	}
	else
	{
		if(m_pPlayer->IsDead())
		{
			return;
		}
	}

	if(m_aggro)
	{
		vec3 targetPos;
		float radius = 0.0f;
		if(m_pTargetNPC != nullptr)
		{
			targetPos = m_pTargetNPC->GetCenter();
			radius = m_pTargetNPC->GetRadius();
		}
		else
		{
			targetPos = m_pPlayer->GetCenter();
			radius = m_pPlayer->GetRadius();
		}

		vec3 toTarget = targetPos - GetCenter();
		toTarget.y = 0.0f;
		float lengthToTarget = length(toTarget);

		m_lookPoint = targetPos;
		m_bLookAtPoint = true;

		if(lengthToTarget <= m_attackRadius + radius)
		{
			m_insideAttackRadius = true;

			if(m_bCanAttack)
			{
				Attack();
			}
		}
		else
		{
			m_insideAttackRadius = false;
		}
	}
}

void Enemy::UpdateRangedCombat(float dt)
{
	if(m_pTargetNPC != nullptr)
	{
		if(m_pTargetNPC->IsDead())
		{
			return;
		}
	}
	else
	{
		if(m_pPlayer->IsDead())
		{
			return;
		}
	}

	if(m_aggro)
	{
		m_insideAttackRadius = true;

		if(m_bCanAttack)
		{
			Attack();
		}

		if(m_bIsChargingAttack)
		{
			if(m_chargeAmount >= 1.0f)
			{
				ReleaseAttack();
			}
		}
	}
}

void Enemy::UpdateTouchCombat(float dt)
{
	if(m_pTargetNPC != nullptr)
	{
		if(m_pTargetNPC->IsDead())
		{
			return;
		}
	}
	else
	{
		if(m_pPlayer->IsDead())
		{
			return;
		}
	}

	vec3 targetPos;
	float radius = 0.0f;
	eProjectileHitboxType hitBoxType;
	vec3 hitBoxCenter;
	float targetRotation = 0.0f;
	float hitboxX = 0.0f;
	float hitboxY = 0.0f;
	float hitboxZ = 0.0f;
	if(m_pTargetNPC != nullptr)
	{
		targetPos = m_pTargetNPC->GetCenter();
		radius = m_pTargetNPC->GetRadius();
		hitBoxType = m_pTargetNPC->GetProjectileHitboxType();
		hitBoxCenter = m_pTargetNPC->GetProjectileHitboxCenter();
		targetRotation = m_pTargetNPC->GetRotation();
		hitboxX = m_pTargetNPC->GetProjectileHitboxXLength();
		hitboxY = m_pTargetNPC->GetProjectileHitboxYLength();
		hitboxZ = m_pTargetNPC->GetProjectileHitboxZLength();
	}
	else
	{
		targetPos = m_pPlayer->GetCenter();
		radius = m_pPlayer->GetRadius();
		hitBoxType = m_pPlayer->GetProjectileHitboxType();
		hitBoxCenter = m_pPlayer->GetProjectileHitboxCenter();
		targetRotation = m_pPlayer->GetRotation();
		hitboxX = m_pPlayer->GetProjectileHitboxXLength();
		hitboxY = m_pPlayer->GetProjectileHitboxYLength();
		hitboxZ = m_pPlayer->GetProjectileHitboxZLength();
	}

	vec3 toTarget = targetPos - GetCenter();
	float lengthToTarget = length(toTarget);

	if(m_aggro)
	{
		m_lookPoint = targetPos;
		m_bLookAtPoint = true;
	}

	bool touchTarget = false;
	if(hitBoxType == eProjectileHitboxType_Sphere)
	{
		if(lengthToTarget <= m_attackRadius + radius)
		{
			touchTarget = true;
		}
	}
	else if(hitBoxType == eProjectileHitboxType_Cube)
	{
		Plane3D planes[6];
		Matrix4x4 rotationMatrix;
		rotationMatrix.SetYRotation(DegToRad(targetRotation));
		planes[0] = Plane3D(rotationMatrix * vec3(-1.0f, 0.0f, 0.0f), hitBoxCenter + (rotationMatrix * vec3(hitboxX, 0.0f, 0.0f)));
		planes[1] = Plane3D(rotationMatrix * vec3(1.0f, 0.0f, 0.0f),  hitBoxCenter + (rotationMatrix * vec3(-hitboxX, 0.0f, 0.0f)));
		planes[2] = Plane3D(rotationMatrix * vec3(0.0f, -1.0f, 0.0f), hitBoxCenter + (rotationMatrix * vec3(0.0f, hitboxY, 0.0f)));
		planes[3] = Plane3D(rotationMatrix * vec3(0.0f, 1.0f, 0.0f),  hitBoxCenter + (rotationMatrix * vec3(0.0f, -hitboxY, 0.0f)));
		planes[4] = Plane3D(rotationMatrix * vec3(0.0f, 0.0f, -1.0f), hitBoxCenter + (rotationMatrix * vec3(0.0f, 0.0f, hitboxZ)));
		planes[5] = Plane3D(rotationMatrix * vec3(0.0f, 0.0f, 1.0f),  hitBoxCenter + (rotationMatrix * vec3(0.0f, 0.0f, -hitboxZ)));

		float distance;
		int inside = 0;
		vec3 normal;

		for(int i = 0; i < 6; i++)
		{
			distance = planes[i].GetPointDistance(GetCenter());

			if (distance < -GetRadius())
			{
				// Outside...
			}
			else if (distance < GetRadius())
			{
				// Intersecting..
				inside++;
			}
			else
			{
				// Inside...
				inside++;
			}
		}

		if(inside == 6)
		{
			touchTarget = true;
		}
	}

	if(touchTarget)
	{
		vec3 knockbackDirection = vec3(toTarget.x, 0.0f, toTarget.z)*2.0f + vec3(0.0f, 1.0f, 0.0f);
		knockbackDirection = normalize(knockbackDirection);
		Colour damageColour = Colour(1.0f, 1.0f, 1.0f);

		float knockbackAmount = 16.0f;
		if(m_pTargetNPC != nullptr)
		{
			// Collision damage for NPC
			m_pTargetNPC->SetTargetEnemy(this);
			m_pTargetNPC->DoDamage(15.0f, damageColour, knockbackDirection, knockbackAmount, false);
		}
		else
		{
			// Collision damage for player
			m_pPlayer->DoDamage(15.0f, damageColour, knockbackDirection, knockbackAmount, false);
		}

		m_velocity.x = 0.0f;
		m_velocity.z = 0.0f;
		m_aggro = true;
		m_aggroResetTimer = m_aggroResetTime;
	}
}

void Enemy::UpdateMovement(float dt)
{
	if(m_spawning)
	{
		// Can't move if spawning
		return;
	}

	if(m_sapped)
	{
		// Can't move if sapped
		return;
	}

	if(m_movementFreezeTimer >= 0.0f)
	{
		// Can't move if movement freeze time is set
		return;
	}

	vec3 targetPos;

	if(m_aggro)
	{
		if(m_pTargetNPC != nullptr)
		{
			targetPos = m_pTargetNPC->GetCenter();
		}
		else
		{
			targetPos = m_pPlayer->GetCenter();
		}

		if(m_eEnemyType == eEnemyType_Bat || m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger)
		{
			targetPos += vec3(0.0f, 0.5f, 0.0f);
		}
	}
	else
	{
		if(m_bRandomTargetPositionMode)
		{
			targetPos = m_randomTargetPosition;
		}
		else
		{
			targetPos = GetCenter();
		}
	}
	
	if(m_eEnemyType == eEnemyType_TargetDummy)
	{
		targetPos = GetCenter();
	}

	vec3 toTarget = targetPos - GetCenter();
	if(m_eEnemyType != eEnemyType_Bee && m_eEnemyType != eEnemyType_Bat && m_eEnemyType != eEnemyType_Ghost && m_eEnemyType != eEnemyType_Doppelganger)
	{
		toTarget.y = 0.0f;
	}
	float lengthToTarget = length(toTarget);

	bool lReachedTarget = false;
	if(m_pTargetNPC != nullptr)
	{
		lReachedTarget = (lengthToTarget < (m_attackRadius*0.99f) + m_pTargetNPC->GetRadius());
	}
	else
	{
		lReachedTarget = (lengthToTarget < (m_attackRadius*0.99f) + m_pPlayer->GetRadius());
	}

	if(m_bIsIdle && lReachedTarget == false)
	{
		m_hasSetIdleAnimation = false;
		m_waitBeforeStopMovingAnimationTimer = 0.25f;
	}

	if(lReachedTarget && m_bCanJump == true)
	{
		StopMoving();

		// Random wander and getting new target timer
		if(m_bRandomTargetPositionMode && m_aggro == false)
		{
			if(m_getRamdomTargetPositionTimer <= 0.0f)
			{
				GetNewRandomTargetPosition();
			}
			else
			{
				m_getRamdomTargetPositionTimer -= dt;
			}
		}
	}
	else
	{
		LookAtPoint(targetPos);

		bool shouldStopMovingUntilJump = false;
		if(IsBlockInFront())
		{
			if(m_eEnemyType != eEnemyType_CrawlingZombie)
			{
				if(m_jumpTimer > 0.0f)
				{
					shouldStopMovingUntilJump = true;
					StopMoving();
				}

				Jump(true);
			}
		}

		if(m_eEnemyType == eEnemyType_GreenSlime || m_eEnemyType == eEnemyType_RedSlime || m_eEnemyType == eEnemyType_BlueSlime || m_eEnemyType == eEnemyType_YellowSlime || m_eEnemyType == eEnemyType_Mimic)
		{
			if(m_aggro || m_eEnemyType != eEnemyType_Mimic)
			{
				Jump(false);
			}
		}
		else
		{
			if(CanMoveTowardsTarget())
			{
				if(shouldStopMovingUntilJump == false)
				{
					if(m_movementWaitAfterAttackTimer <= 0.0f)
					{
						vec3 toTarget = targetPos - m_position;
						vec3 movementDirection = toTarget;
						if(m_eEnemyType != eEnemyType_Bee && m_eEnemyType != eEnemyType_Bat && m_eEnemyType != eEnemyType_Ghost && m_eEnemyType != eEnemyType_Doppelganger)
						{
							movementDirection.y = 0.0f;
						}
						movementDirection = normalize(movementDirection);

						float movementSpeed = (m_movementSpeed * dt);
						if (movementSpeed > 0.5f)
						{
							movementSpeed = 0.5f;
						}
						float animationSpeed = 0.75f;

						for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
						{
							SetAnimationSpeed(animationSpeed, false, (AnimationSections)i);
						}

						// Iron giants walk sloooooow
						if(m_eEnemyType == eEnemyType_IronGiant)
						{
							for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
							{
								SetAnimationSpeed(0.5f, false, (AnimationSections)i);
							}
						}

						MoveAbsolute(movementDirection, movementSpeed);
					}
				}
			}
		}
	}

	if(m_eEnemyType == eEnemyType_GreenSlime || m_eEnemyType == eEnemyType_RedSlime || m_eEnemyType == eEnemyType_BlueSlime || m_eEnemyType == eEnemyType_YellowSlime || m_eEnemyType == eEnemyType_Mimic)
	{
		if(m_bCanJump == false)
		{
			float movementSpeed = (m_movementSpeed * dt);
			if (movementSpeed > 0.5f)
			{
				movementSpeed = 0.5f;
			}
			MoveAbsolute(m_forward, movementSpeed);
		}
	}
}

void Enemy::UpdateSpawning(float dt)
{
	if(m_spawning)
	{
		if(m_eEnemyType == eEnemyType_Ghost || m_eEnemyType == eEnemyType_Doppelganger)
		{
			vec3 diff = m_spawnEndPosition - m_spawnStartPosition;

			m_position = m_spawnStartPosition + (diff*m_spawningT);

			// Spawning scale
			m_pVoxelCharacter->SetCharacterScale(m_renderScale * m_spawningT);

			SetRotation(720.0f * m_spawningT);
		}
	}
}

void Enemy::UpdateTimers(float dt)
{
	// Sapped
	if(m_sapped)
	{
		if(m_sappedTimer <= 0.0f)
		{
			SetSapped(false);
		}
		else
		{
			m_sappedTimer -= dt;
		}
	}
	else
	{
		// Aggro reset timer
		if(m_aggro)
		{
			if(m_aggroResetTimer <= 0.0f)
			{
				if (m_bCanJump == true)
				{
					m_aggro = false;

					m_bIsChargingAttack = false;
					m_chargeAmount = 0.0f;

					if (m_eEnemyType != eEnemyType_GreenSlime && m_eEnemyType != eEnemyType_RedSlime && m_eEnemyType != eEnemyType_BlueSlime && m_eEnemyType != eEnemyType_YellowSlime && m_eEnemyType != eEnemyType_Mimic)
					{
						SetRandomLookMode();
					}

					if (m_eEnemyType == eEnemyType_RangedSkeleton)
					{
						m_bIsChargingAttack = false;
						m_chargeAmount = 0.0f;

						UnloadWeapon(false);

						if (m_bIsIdle)
						{
							m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "BindPose", 0.2f);
						}
						else
						{
							m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Head_Body, false, AnimationSections_Head_Body, "BindPose", 0.2f);
							m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "BindPose", 0.2f);
							m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "BindPose", 0.2f);
						}
					}
				}
			}
			else
			{
				m_aggroResetTimer -= dt;
			}
		}

		// Attack delay
		if(m_insideAttackRadius)
		{
			if(m_attackDelayTimer > 0.0f)
			{
				m_attackDelayTimer -= dt;
			}
		}

		// Movement wait after attack
		if(m_movementWaitAfterAttackTimer > 0.0f)
		{
			m_movementWaitAfterAttackTimer -= dt;
		}

		// Jump timer
		if(m_jumpTimer >= 0.0f)
		{
			m_jumpTimer -= dt;
		}

		// Movement freeze
		if(m_movementFreezeTimer >= 0.0f)
		{
			m_movementFreezeTimer -= dt;
		}

		// Spawning
		if(m_spawning)
		{
			if(m_spawningTimer >= 0.0f)
			{
				m_spawningTimer -= dt;
			}
			else
			{
				if(m_eEnemyType == eEnemyType_Doppelganger)
				{
					SetAnimationSpeed(0.35f, true, AnimationSections_FullBody);
				}

				m_pVoxelCharacter->SetCharacterScale(m_renderScale);

				SetLeashParameters(GetCenter(), m_leashRadius);
				ResetRandomTargetPosition();

				m_spawning = false;
			}
		}

		// Idle animation wait timer
		if(m_waitBeforeStopMovingAnimationTimer >= 0.0f)
		{
			m_waitBeforeStopMovingAnimationTimer -= dt;
		}

		// Stuck timer
		if(m_bUpdateStuckTimer)
		{
			if(m_stuckTimer > 0.0f)
			{
				m_stuckTimer -= dt;
			}
			else
			{
				if(m_aggro == false)
				{
					m_bStuck = true;
					GetNewRandomTargetPosition();
				}
			}
		}
	}

	// Damage and knockback timers
	if(m_damageTimer > 0.0f)
	{
		m_damageTimer -= dt;
	}
	if(m_knockbackTimer > 0.0f)
	{
		m_knockbackTimer -= dt;
	}
}

// Rendering
void Enemy::Render(bool outline, bool reflection, bool silhouette)
{
	if(m_pVoxelCharacter != nullptr)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->MultiplyWorldMatrix(m_worldMatrix);

			Colour OulineColour(1.0f, 0.0f, 0.0f, 1.0f);

			m_pVoxelCharacter->Render(outline, reflection, silhouette, OulineColour, false);
			m_pVoxelCharacter->RenderWeapons(outline, reflection, silhouette, OulineColour);
		m_pRenderer->PopMatrix();
	}
}

void Enemy::RenderFace()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->MultiplyWorldMatrix(m_worldMatrix);

		m_pVoxelCharacter->RenderFace();
	m_pRenderer->PopMatrix();
}

void Enemy::RenderWeaponTrails()
{
	if(m_pVoxelCharacter != nullptr)
	{
		m_pRenderer->PushMatrix();
			m_pVoxelCharacter->RenderWeaponTrails();
		m_pRenderer->PopMatrix();
	}
}

void Enemy::RenderDebug()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->TranslateWorldMatrix(GetCenter().x, GetCenter().y, GetCenter().z);

		m_pRenderer->PushMatrix();
			m_pRenderer->SetLineWidth(1.0f);
			m_pRenderer->SetRenderMode(RM_WIREFRAME);
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);

			m_pRenderer->RotateWorldMatrix(0.0f, GetRotation(), 0.0f);

			m_pRenderer->DrawSphere(m_radius, 20, 20);
		m_pRenderer->PopMatrix();

		// Attack segment
		if(m_attackEnabled && (m_attackEnabledDelayTimer <= 0.0f))
		{
			m_pRenderer->PushMatrix();
				float angle = acos(m_attackSegmentAngle);

				m_pRenderer->ImmediateColourAlpha(1.0f, 0.15f, 0.05f, 1.0f);
				m_pRenderer->RotateWorldMatrix(0.0f, GetRotation()-90.0f+m_attackRotation, 0.0f);
				m_pRenderer->TranslateWorldMatrix(0.0f, 0.25f, 0.0f);
				m_pRenderer->DrawCircleSector(m_attackRadius, angle, 10);
			m_pRenderer->PopMatrix();
		}
	m_pRenderer->PopMatrix();
	
	// Bones
	if(m_pVoxelCharacter != nullptr)
	{
		m_pRenderer->PushMatrix();
		m_pRenderer->MultiplyWorldMatrix(m_worldMatrix);

		m_pVoxelCharacter->RenderBones();
		m_pRenderer->PopMatrix();
	}

	// Forwards
	RenderForwardDebug();

	// Random position
	RenderRandomPositionDebug();

	// Charging attack
	RenderChargingAttackDebug();

	// Looking segment for aggro checking
	RenderLookingAreaDebug();

	// Projectile hit box
	RenderProjectileHitboxDebug();
}

void Enemy::RenderForwardDebug()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->TranslateWorldMatrix(GetCenter().x, GetCenter().y, GetCenter().z);
		
		m_pRenderer->SetRenderMode(RM_SOLID);
		m_pRenderer->SetLineWidth(3.0f);
		m_pRenderer->EnableImmediateMode(IM_LINES);
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(m_forward.x*1.5f, m_forward.y*1.5f, m_forward.z*1.5f);

			m_pRenderer->ImmediateColourAlpha(0.0f, 1.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(m_targetForward.x*1.5f, m_targetForward.y*1.5f, m_targetForward.z*1.5f);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();

	if(m_pVoxelCharacter != nullptr)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->MultiplyWorldMatrix(m_worldMatrix);
			m_pVoxelCharacter->RenderFacingDebug();
		m_pRenderer->PopMatrix();
	}
}

void Enemy::RenderRandomPositionDebug()
{
	if(m_bRandomTargetPositionMode == false)
	{
		return;
	}

	m_pRenderer->PushMatrix();
		m_pRenderer->SetRenderMode(RM_SOLID);

		m_pRenderer->SetLineWidth(1.0f);
		m_pRenderer->EnableImmediateMode(IM_LINES);
		m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
		m_pRenderer->ImmediateVertex(GetCenter().x, GetCenter().y, GetCenter().z);
		m_pRenderer->ImmediateVertex(m_randomTargetPosition.x, m_randomTargetPosition.y, m_randomTargetPosition.z);

		m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
		m_pRenderer->ImmediateVertex(m_randomTargetPosition.x, m_randomTargetPosition.y-0.5f, m_randomTargetPosition.z);
		m_pRenderer->ImmediateVertex(m_randomTargetPosition.x, m_randomTargetPosition.y+0.5f, m_randomTargetPosition.z);
		m_pRenderer->DisableImmediateMode();

		m_pRenderer->PushMatrix();
			m_pRenderer->TranslateWorldMatrix(m_leashPosition.x, m_leashPosition.y, m_leashPosition.z);
			m_pRenderer->DrawLineCircle(m_leashRadius, 20);
		m_pRenderer->PopMatrix();
	m_pRenderer->PopMatrix();
}

void Enemy::RenderChargingAttackDebug()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->TranslateWorldMatrix(GetCenter().x, GetCenter().y, GetCenter().z);
		m_pRenderer->TranslateWorldMatrix(0.0f, 0.5f, 0.0f);

		m_pRenderer->ScaleWorldMatrix(m_pVoxelCharacter->GetCharacterScale(), m_pVoxelCharacter->GetCharacterScale(), m_pVoxelCharacter->GetCharacterScale());

		float chargingMultiply = m_chargeAmount * 30.0f;
		m_pRenderer->SetRenderMode(RM_SOLID);
		m_pRenderer->SetLineWidth(3.0f);
		m_pRenderer->EnableImmediateMode(IM_LINES);
			m_pRenderer->ImmediateColourAlpha(0.0f, 1.0f, 1.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(m_forward.x*chargingMultiply, m_forward.y*chargingMultiply, m_forward.z*chargingMultiply);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();

	m_pRenderer->PushMatrix();
		vec3 startPos = m_chargeSpawnPosition;
		vec3 accel = ((m_gravityDirection * 9.81f)*2.5f);
		vec3 vel = m_chargeSpawnVelocity;
		vec3 pos1 = startPos;
		vec3 pos2 = pos1;
		float deltaTime = 0.04f;
		m_pRenderer->EnableImmediateMode(IM_LINES);
		m_pRenderer->ImmediateColourAlpha(0.0f, 1.0f, 1.0f, 1.0f);
			for(int i = 0; i < 50; i++)
			{
				vel += accel * deltaTime;
				pos2 += vel * deltaTime;

				m_pRenderer->ImmediateVertex(pos1.x, pos1.y, pos1.z);
				m_pRenderer->ImmediateVertex(pos2.x, pos2.y, pos2.z);

				pos1 = pos2;
			}
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();
}

void Enemy::RenderLookingAreaDebug()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->TranslateWorldMatrix(GetCenter().x, GetCenter().y, GetCenter().z);

		m_pRenderer->SetLineWidth(1.0f);
		m_pRenderer->SetRenderMode(RM_WIREFRAME);

		if(m_aggro)
		{
			m_pRenderer->ImmediateColourAlpha(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else
		{
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
		}		

		float facelookingAngle = GetFaceLookingRotation();
		float angle = acos(m_aggroLookSegmentAngle);
		m_pRenderer->RotateWorldMatrix(0.0f, GetRotation()-90.0f+facelookingAngle, 0.0f);
		m_pRenderer->DrawCircleSector(m_aggroLookRadius, angle, 10);
	m_pRenderer->PopMatrix();
}

void Enemy::RenderProjectileHitboxDebug()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->TranslateWorldMatrix(GetProjectileHitboxCenter().x, GetProjectileHitboxCenter().y, GetProjectileHitboxCenter().z);

		m_pRenderer->SetLineWidth(1.0f);
		m_pRenderer->SetRenderMode(RM_WIREFRAME);
		m_pRenderer->ImmediateColourAlpha(0.0f, 1.0f, 1.0f, 1.0f);

		if(m_eProjectileHitboxType == eProjectileHitboxType_Sphere)
		{
			m_pRenderer->DrawSphere(m_projectileHitboxRadius, 20, 20);
		}
		else if(m_eProjectileHitboxType == eProjectileHitboxType_Cube)
		{
			float l_length = m_projectileHitboxXLength;
			float l_height = m_projectileHitboxYLength;
			float l_width = m_projectileHitboxZLength;

			m_pRenderer->SetRenderMode(RM_WIREFRAME);
			m_pRenderer->SetCullMode(CM_NOCULL);
			m_pRenderer->SetLineWidth(1.0f);

			m_pRenderer->RotateWorldMatrix(0.0f, GetRotation(), 0.0f);

			m_pRenderer->EnableImmediateMode(IM_QUADS);
				m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
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

			m_pRenderer->SetCullMode(CM_BACK);
		}
	m_pRenderer->PopMatrix();
}

// Callback functions
void Enemy::_AttackEnabledTimerFinished(void *apData)
{
	Enemy* lpEnemy = (Enemy*)apData;
	lpEnemy->AttackEnabledTimerFinished();
}

void Enemy::AttackEnabledTimerFinished()
{
	m_attackEnabled = false;

	if(m_eEnemyType == eEnemyType_Bee || m_eEnemyType == eEnemyType_Bat)
	{
		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "Fly", 0.05f);
	}
}

void Enemy::_AttackEnabledDelayTimerFinished(void *apData)
{
	Enemy* lpEnemy = (Enemy*)apData;
	lpEnemy->AttackEnabledDelayTimerFinished();
}

void Enemy::AttackEnabledDelayTimerFinished()
{
	if(m_erase)
	{
		return;
	}

	m_attackEnabledDelayTimer = 0.0f;

	if(m_eEnemyType == eEnemyType_NormalSkeleton)
	{
		vec3 boneSpawnPosition = GetCenter() + (m_forward*0.75f) + (GetRightVector()*-0.4f) + (GetUpVector()*0.5f);

		vec3 toTarget;
		if(m_pTargetNPC != nullptr)
		{
			toTarget = m_pTargetNPC->GetCenter() - GetCenter();
		}
		else
		{
			toTarget = m_pPlayer->GetCenter() - GetCenter();
		}
		float liftAmount = 1.75f * (length(toTarget)*0.065f);
		vec3 boneSpawnVelocity = (normalize(toTarget) * 15.0f) + vec3(0.0f, liftAmount, 0.0f);

		Projectile* pProjectile = m_pProjectileManager->CreateProjectile(boneSpawnPosition, boneSpawnVelocity, 0.0f, "media/gamedata/items/Bone/Bone.item", 0.04f);	
		pProjectile->SetProjectileType(false, true, true);
		pProjectile->SetOwner(nullptr, nullptr, this);
		pProjectile->SetGravityMultiplier(0.35f);
	}
	else if(m_eEnemyType == eEnemyType_MageSkeleton)
	{
		vec3 fireballSpawnPosition = GetCenter() + (m_forward*0.75f) + (GetRightVector()*-0.4f) + (GetUpVector()*0.25f);

		float powerAmount = 25.0f;
		vec3 toTarget;
		if(m_pTargetNPC != nullptr)
		{
			toTarget = m_pTargetNPC->GetCenter() - GetCenter();
		}
		else
		{
			toTarget = m_pPlayer->GetCenter() - GetCenter();
		}
		vec3 fireballSpawnVelocity = (normalize(toTarget) * powerAmount);

		Projectile* pProjectile = m_pProjectileManager->CreateProjectile(fireballSpawnPosition, fireballSpawnVelocity, 0.0f, "media/gamedata/items/Fireball/Fireball.item", 0.04f);	
		pProjectile->SetProjectileType(false, true, true);
		pProjectile->SetOwner(nullptr, nullptr, this);
		pProjectile->SetGravityMultiplier(0.0f);
	}
	else if(m_eEnemyType == eEnemyType_IronGiant)
	{
		vec3 fireballSpawnPosition = GetCenter() + (m_forward*0.75f) + (GetUpVector()*5.0f);

		float powerAmount = 25.0f;
		vec3 toTarget;
		if(m_pTargetNPC != nullptr)
		{
			toTarget = m_pTargetNPC->GetCenter() - fireballSpawnPosition;
		}
		else
		{
			toTarget = m_pPlayer->GetCenter() - fireballSpawnPosition;
		}
		vec3 fireballSpawnVelocity = (normalize(toTarget) * powerAmount);

		Projectile* pProjectile = m_pProjectileManager->CreateProjectile(fireballSpawnPosition, fireballSpawnVelocity, 0.0f, "media/gamedata/items/Fireball/Fireball.item", 0.04f);	
		pProjectile->SetProjectileType(false, true, true);
		pProjectile->SetOwner(nullptr, nullptr, this);
		pProjectile->SetGravityMultiplier(0.0f);
	}
	else if(m_eEnemyType == eEnemyType_Bat)
	{
		vec3 boneSpawnPosition = GetCenter() + (m_forward*1.25f);

		vec3 toTarget;
		if(m_pTargetNPC != nullptr)
		{
			toTarget = m_pTargetNPC->GetCenter() - GetCenter();
		}
		else
		{
			toTarget = m_pPlayer->GetCenter() - GetCenter();
		}
		vec3 boneSpawnVelocity = (normalize(toTarget) * 15.0f);

		Projectile* pProjectile = m_pProjectileManager->CreateProjectile(boneSpawnPosition, boneSpawnVelocity, 0.0f, "media/gamedata/items/Batshot/Batshot.item", 0.06f);	
		pProjectile->SetProjectileType(false, true, true);
		pProjectile->SetOwner(nullptr, nullptr, this);
		pProjectile->SetGravityMultiplier(0.0f);
	}
}