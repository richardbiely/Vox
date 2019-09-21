// ******************************************************************************
// Filename:    NPC.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 27/09/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma warning(disable: 4800) // Disable 'casting to bool' warning C4800

#include "NPC.h"

#include "../utils/Interpolator.h"
#include "../utils/Random.h"

#include "../Lighting/LightingManager.h"
#include "../Particles/BlockParticleManager.h"
#include "../TextEffects/TextEffectsManager.h"
#include "../Items/ItemManager.h"
#include "../Enemy/Enemy.h"
#include "../Enemy/EnemyManager.h"
#include "../Projectile/ProjectileManager.h"
#include "../Projectile/Projectile.h"
#include "../VoxGame.h"

#include <fstream>
#include <ostream>
#include <iostream>
#include <string>
using namespace std;


NPC::NPC(Renderer* pRenderer, ChunkManager* pChunkManager, Player* pPlayer, LightingManager* pLightingManager, BlockParticleManager* pBlockParticleManager, TextEffectsManager* pTextEffectsManager, ItemManager* pItemManager, ProjectileManager* pProjectileManager, EnemyManager* pEnemyManager, QubicleBinaryManager* pQubicleBinaryManager, string name, string typeName, string modelName, bool characterSelectScreen, bool useQubicleManager)
{
	m_pRenderer = pRenderer;
	m_pChunkManager = pChunkManager;
	m_pPlayer = pPlayer;
	m_pLightingManager = pLightingManager;
	m_pBlockParticleManager = pBlockParticleManager;
	m_pTextEffectsManager = pTextEffectsManager;
	m_pItemManager = pItemManager;
	m_pQubicleBinaryManager = pQubicleBinaryManager;
	m_pProjectileManager = pProjectileManager;
	m_pEnemyManager = pEnemyManager;

	m_erase = false;

	m_name = name;

	m_pVoxelCharacter = new VoxelCharacter(m_pRenderer, m_pQubicleBinaryManager);
	m_pCharacterBackup = new QubicleBinary(m_pRenderer);

	m_radius = 1.0f;

	m_gravityDirection = vec3(0.0f, -1.0f, 0.0f);

	m_forward = vec3(0.0f, 0.0f, 1.0f);
	m_targetForward = vec3(0.0f, 0.0f, 1.0f);

	m_gridPositionX = 0;
	m_gridPositionY = 0;
	m_gridPositionZ = 0;

	m_pCachedGridChunk = NULL;

	m_outlineRender = false;
	m_hoverRender = false;
	m_subSelectionRender = false;

	// Animation params
	for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
	{
		m_animationSpeed[i] = 1.0f;
		m_animationSpeedonlySetOnCompleteAnimation[i] = false;
		m_animationFinished[i] = false;
	}
	m_animationTimer = 0.0f;

	// Loading character files
	m_type = typeName;
	m_modelName = modelName;

	// NPC State
	m_eNPCState = eNPCState_Idle;
	m_ePreviousNPCState = eNPCState_Idle;

	// Combat type
	m_eNPCCombatType = eNPCCombatType_None;

	// Frontend NPC
	m_frontEndNPC = false;

	// Player class for frontend lineup
	m_selectedClass = PlayerClass_Debug;

	// Movement gameplay params
	m_moveToPlayer = false;
	m_hasReachedTargetPosition = false;
	m_lookAtPositionWhenReachTarget = false;

	// Projectile hitbox
	m_eProjectileHitboxType = eProjectileHitboxType_Cube;
	m_projectileHitboxXLength = 0.65f;
	m_projectileHitboxYLength = 1.25f;
	m_projectileHitboxZLength = 0.65f;
	m_projectileHitboxCenterOffset = vec3(0.0f, 0.75f, 0.0f);

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
	m_aggroRadius = 12.0f;
	m_updateAggroRadiusTimer = 0.0f;
	m_attackCounter = 0;

	// Health
	m_maxHealth = 100.0f;
	m_health = m_maxHealth;
	m_maxMagic = 100.0f;
	m_magic = m_maxMagic;

	m_damageTime = 0.4f;
	m_damageTimer = m_damageTime;
	m_knockbackTime = 0.4f;
	m_knockbackTimer = m_knockbackTime;

	// Charging attacks
	m_bIsChargingAttack = false;
	m_chargeAmount = 0.0f;
	m_chargeTime = 1.0f;

	// Idle
	m_bIsIdle = true;
	m_waitBeforeStopMovingAnimationTimer = 0.0f;
	m_hasSetIdleAnimation = true;

	m_movementSpeed = 3.75f;
	m_maxMovementSpeed = 7.0f;
	m_minMovementSpeed = 1.0f;

	m_currentWaypointIndex = 0;

	// Jumping
	m_bCanJump = true;
	m_jumpTime = 1.0f;
	m_jumpTimer = 0.0f;
	m_jumpHeight = 14.5f;

	// Look at point
	m_bLookAtPoint = false;
	m_bodyTurnSpeedMultiplier = 3.5f;
	m_bodyTurnStopThreshold = 0.35f;
	m_bIsLookingAtPoint = false;

	// Talking
	m_talkMode = false;
	m_talkDelay = 0.0f;
	m_talkTime = 0.0f;
	m_talkTimer = 0.0f;

	// Working
	m_isWorking = false;
	m_workingAnimationWaitTimer = 0.0f;
	m_workingAnimationDelay = 0.45f;
	m_workingRepetitionBeforeLongDelay = 3;
	m_workingRepetitionCounter = 0;
	m_createdAnvilHitParticleEffect = false;

	// Push collision enabled
	m_bPushingCollisionEnabled = true;

	// Is this NPC a credits NPC, if so, behave differently
	m_isCreditsNPC = false;

	m_pTargetEnemy = NULL;
		
	char characterBaseFolder[128];
	char qbFilename[128];
	char ms3dFilename[128];
	char animListFilename[128];
	char facesFilename[128];
	char characterFilename[128];

	if(characterSelectScreen)
	{
		sprintf(characterBaseFolder, "media/gamedata/models");
		sprintf(qbFilename, "saves/characters/%s/%s.qb", modelName.c_str(), modelName.c_str());
		sprintf(ms3dFilename, "media/gamedata/models/%s/%s.ms3d", m_type.c_str(), m_type.c_str());
		sprintf(animListFilename, "media/gamedata/models/%s/%s.animlist", m_type.c_str(), m_type.c_str());
		sprintf(facesFilename, "saves/characters/%s/%s.faces", modelName.c_str(), modelName.c_str());
		sprintf(characterFilename, "saves/characters/%s/%s.character", modelName.c_str(), modelName.c_str());
	}
	else
	{
		sprintf(characterBaseFolder, "media/gamedata/models");
		sprintf(qbFilename, "media/gamedata/models/%s/%s.qb", m_type.c_str(), modelName.c_str());
		sprintf(ms3dFilename, "media/gamedata/models/%s/%s.ms3d", m_type.c_str(), m_type.c_str());
		sprintf(animListFilename, "media/gamedata/models/%s/%s.animlist", m_type.c_str(), m_type.c_str());
		sprintf(facesFilename, "media/gamedata/models/%s/%s.faces", m_type.c_str(), modelName.c_str());
		sprintf(characterFilename, "media/gamedata/models/%s/%s.character", m_type.c_str(), modelName.c_str());
	}

	m_pVoxelCharacter->UnloadCharacter();
	m_pVoxelCharacter->Reset();
	m_pVoxelCharacter->LoadVoxelCharacter(m_type.c_str(), qbFilename, ms3dFilename, animListFilename, facesFilename, characterFilename, characterBaseFolder, useQubicleManager);

	m_pVoxelCharacter->SetBreathingAnimationEnabled(true);
	m_pVoxelCharacter->SetWinkAnimationEnabled(true);
	m_pVoxelCharacter->SetTalkingAnimationEnabled(false);
	m_pVoxelCharacter->SetRandomMouthSelection(true);
	m_pVoxelCharacter->SetRandomLookDirection(true);
	m_pVoxelCharacter->SetWireFrameRender(false);

	m_pVoxelCharacter->PlayAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "BindPose");

	UnloadWeapon(true);
	UnloadWeapon(false);

	if(m_pCharacterBackup != NULL)
	{
		delete m_pCharacterBackup;
		m_pCharacterBackup = new QubicleBinary(m_pRenderer);
	}

	m_pCharacterBackup->Import(qbFilename, true);

	UpdateRadius();
}

NPC::~NPC()
{
	ClearWaypoints();

	UnloadWeapon(true);
	UnloadWeapon(false);

	m_pVoxelCharacter->RemoveQubicleMatrix("Quiver");

	if (m_pCharacterBackup != NULL)
	{
		m_pCharacterBackup->SetNullLinkage(m_pVoxelCharacter->GetQubicleModel());
	}
	delete m_pVoxelCharacter;
	if (m_pCharacterBackup != NULL)
	{
		delete m_pCharacterBackup;
	}
}

void NPC::SetLightingManager(LightingManager* pLightingManager)
{
	m_pLightingManager = pLightingManager;
}

void NPC::SetErase(bool erase)
{
	m_erase = erase;
}

bool NPC::NeedsErasing()
{
	return m_erase;
}

VoxelCharacter* NPC::GetVoxelCharacter()
{
	return m_pVoxelCharacter;
}

void NPC::SetName(string name)
{
	m_name = name;
}

string NPC::GetName() const
{
	return m_name;
}

void NPC::SetType(string typeName)
{
	m_type = typeName;
}

string NPC::GetType() const
{
	return m_type;
}

void NPC::SetModelname(string modelName)
{
	m_modelName = modelName;
}

string NPC::GetModelName() const
{
	return m_modelName;
}

void NPC::SetPosition(vec3 pos)
{
	m_position = pos;
}

vec3 NPC::GetPosition() const
{
	return m_position;
}

void NPC::SetRotation(float rot)
{
	float angle = DegToRad(rot);
	m_forward = vec3(sin(angle), 0.0f, cos(angle));
}

float NPC::GetRotation() const
{
	float rotationAngle = acos(dot(vec3(0.0f, 0.0f, 1.0f), m_forward));
	rotationAngle = RadToDeg(rotationAngle);
	if(m_forward.x < 0.0f)
	{
		rotationAngle = (360.0f - rotationAngle);
	}

	return rotationAngle;
}

void NPC::SetScale(float scale)
{
	m_pVoxelCharacter->SetCharacterScale(scale);

	UpdateRadius();
}

float NPC::GetRadius() const
{
	return m_radius;
}

void NPC::UpdateRadius()
{
	m_radius = m_pVoxelCharacter->GetCharacterScale() / 0.14f;
}

vec3 NPC::GetCenter() const
{
	vec3 center = m_position + vec3(0.0f, m_radius, 0.0f);

	return center;
}

vec3 NPC::GetForwardVector() const
{
	return m_worldMatrix.GetForwardVector();
}

vec3 NPC::GetRightVector() const
{
	return m_worldMatrix.GetRightVector();
}

vec3 NPC::GetUpVector() const
{
	return m_worldMatrix.GetUpVector();
}

// Front-end NPCs
void NPC::SetFrontEndNPC(bool frontEnd)
{
	m_frontEndNPC = frontEnd;
}

bool NPC::IsFrontEndNPC() const
{
	return m_frontEndNPC;
}

void NPC::SetPlayerClass(PlayerClass selectedClass)
{
	m_selectedClass = selectedClass;
}

PlayerClass NPC::GetPlayerClass()
{
	return m_selectedClass;
}

// Combat type
void NPC::SetNPCCombatType(eNPCCombatType eNPCCombatType, bool setWeaponModel)
{
	m_eNPCCombatType = eNPCCombatType;

	if(setWeaponModel)
	{
		UnloadWeapon(false);
		UnloadWeapon(true);

		SetBodyTurnStopThreshold(0.01f);
		SetBodyTurnSpeedMultiplier(6.0f);

		switch(m_eNPCCombatType)
		{
		case eNPCCombatType_None: { break; }
		case eNPCCombatType_MeleeSword:
			{
				LoadWeapon(false, GetEquipmentFilenameForType(eEquipment_IronSword));
				break;
			}
		case eNPCCombatType_Archer:
			{
				LoadWeapon(true, GetEquipmentFilenameForType(eEquipment_WoodenBow));

				// Add a quiver item to the enemy ranger
				VoxelWeapon* pNewEquipment = new VoxelWeapon(m_pRenderer, m_pQubicleBinaryManager);
				pNewEquipment->SetVoxelCharacterParent(NULL);
				pNewEquipment->LoadWeapon("media/gamedata/items/Quiver/Quiver.item", false);

				pNewEquipment->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->SetScaleAndOffsetForMatrix("Quiver", pNewEquipment->GetAnimatedSection(0)->m_renderScale, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.x, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.y, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.z);
				QubicleMatrix* pQuiverMatrix = pNewEquipment->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->SetQubicleMatrix("Quiver");
				pQuiverMatrix->m_boneIndex = m_pVoxelCharacter->GetBodyBoneIndex();
				m_pVoxelCharacter->AddQubicleMatrix(pQuiverMatrix, false);

				break;
			}
		case eNPCCombatType_Staff:
			{
				LoadWeapon(false, GetEquipmentFilenameForType(eEquipment_MageStaff));
				break;
			}
		case eNPCCombatType_FireballHands:
			{
				LoadWeapon(false, GetEquipmentFilenameForType(eEquipment_FireballHandRight));
				LoadWeapon(true, GetEquipmentFilenameForType(eEquipment_FireballHandLeft));
				break;
			}
		}
	}
}

void NPC::LoadWeapon(bool left, string weaponFile)
{
	if(left)
	{
		if(m_pVoxelCharacter->GetLeftWeapon() != NULL)
		{
			if(m_pVoxelCharacter->IsLeftWeaponLoaded())
			{
				// Lights
				for(int i = 0; i < m_pVoxelCharacter->GetLeftWeapon()->GetNumLights(); i++)
				{
					unsigned int lightId;
					vec3 lightPos;
					float lightRadius;
					float lightDiffuseMultiplier;
					Colour lightColour;
					bool connectedToSegment;
					m_pVoxelCharacter->GetLeftWeapon()->GetLightParams(i, &lightId, &lightPos, &lightRadius, &lightDiffuseMultiplier, &lightColour, &connectedToSegment);

					if(lightId != -1)
					{
						m_pLightingManager->RemoveLight(lightId);
						m_pVoxelCharacter->GetLeftWeapon()->SetLightingId(i, -1);
					}
				}

				// Particle effects
				for(int i = 0; i < m_pVoxelCharacter->GetLeftWeapon()->GetNumParticleEffects(); i++)
				{
					unsigned int particleEffectId;
					vec3 ParticleEffectPos;
					string effectName;
					bool connectedToSegment;
					m_pVoxelCharacter->GetLeftWeapon()->GetParticleEffectParams(i, &particleEffectId, &ParticleEffectPos, &effectName, &connectedToSegment);

					if(particleEffectId != -1)
					{
						m_pBlockParticleManager->DestroyParticleEffect(particleEffectId);
						m_pVoxelCharacter->GetLeftWeapon()->SetParticleEffectId(i, -1);
					}
				}
			}

			// Load the weapon file
			m_pVoxelCharacter->LoadLeftWeapon(weaponFile.c_str());
		}
	}
	else
	{
		if(m_pVoxelCharacter->GetRightWeapon() != NULL)
		{
			if(m_pVoxelCharacter->IsRightWeaponLoaded())
			{
				// Lights
				for(int i = 0; i < m_pVoxelCharacter->GetRightWeapon()->GetNumLights(); i++)
				{
					unsigned int lightId;
					vec3 lightPos;
					float lightRadius;
					float lightDiffuseMultiplier;
					Colour lightColour;
					bool connectedToSegment;
					m_pVoxelCharacter->GetRightWeapon()->GetLightParams(i, &lightId, &lightPos, &lightRadius, &lightDiffuseMultiplier, &lightColour, &connectedToSegment);

					if(lightId != -1)
					{
						m_pLightingManager->RemoveLight(lightId);
						m_pVoxelCharacter->GetRightWeapon()->SetLightingId(i, -1);
					}
				}

				// Particle effects
				for(int i = 0; i < m_pVoxelCharacter->GetRightWeapon()->GetNumParticleEffects(); i++)
				{
					unsigned int particleEffectId;
					vec3 ParticleEffectPos;
					string effectName;
					bool connectedToSegment;
					m_pVoxelCharacter->GetRightWeapon()->GetParticleEffectParams(i, &particleEffectId, &ParticleEffectPos, &effectName, &connectedToSegment);

					if(particleEffectId != -1)
					{
						m_pBlockParticleManager->DestroyParticleEffect(particleEffectId);
						m_pVoxelCharacter->GetRightWeapon()->SetParticleEffectId(i, -1);
					}
				}
			}

			// Load the weapon file
			m_pVoxelCharacter->LoadRightWeapon(weaponFile.c_str());

			// Weapon attack radius
			if(m_pVoxelCharacter->GetRightWeapon() != NULL)
			{
				if(m_pVoxelCharacter->IsRightWeaponLoaded())
				{
					m_attackRadius = m_pVoxelCharacter->GetRightWeapon()->GetWeaponRadius();
				}
			}
		}
	}
}

void NPC::UnloadWeapon(bool left)
{
	VoxelWeapon* pWeapon = NULL;
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

	if (pWeapon != NULL)
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

					if (m_frontEndNPC == false)
					{
						float scale = m_pVoxelCharacter->GetCharacterScale();
						unsigned int lId;
						m_pLightingManager->AddDyingLight(vec3(lightPos.x, lightPos.y, lightPos.z), lightRadius * scale, lightDiffuseMultiplier, lightColour, 2.0f, &lId);
					}
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

// Equipping items
void NPC::ImportEquippedItems(string inventoryFile)
{
	ifstream importFile;
	importFile.open(inventoryFile.c_str(), ios_base::binary);

	if(importFile.is_open())
	{
		char input[128];
		char * pEnd;

		// Import the stored inventory items
		importFile.getline(input, 128, '|');
		int numInventory = strtol(input, &pEnd, 10);

		for(int i = 0; i < numInventory; i++)
		{
			importFile.getline(input, 128, '|');
			int itemValid = strtol(input, &pEnd, 10);

			if(itemValid == 1)
			{
				char filename[128];
				importFile.getline(filename, 128, '|');

				char iconFilename[128];
				importFile.getline(iconFilename, 128, '|');

				importFile.getline(input, 128, '|');
				int itemType = strtol(input, &pEnd, 10);

				importFile.getline(input, 128, '|');
				int item = strtol(input, &pEnd, 10);

				importFile.getline(input, 128, '|');
				int status = strtol(input, &pEnd, 10);

				importFile.getline(input, 128, '|');
				int equipSlot = strtol(input, &pEnd, 10);

				importFile.getline(input, 128, '|');
				int itemQuality = strtol(input, &pEnd, 10);

				importFile.getline(input, 128, '|');
				int left = strtol(input, &pEnd, 10);

				importFile.getline(input, 128, '|');
				int right = strtol(input, &pEnd, 10);

				char title[128];
				importFile.getline(title, 128, '|');

				char description[128];
				importFile.getline(description, 128, '|');

				float r;
				float g;
				float b;
				importFile.getline(input, 128, '|');
				r = (float)strtod(input, &pEnd);
				importFile.getline(input, 128, '|');
				g = (float)strtod(input, &pEnd);
				importFile.getline(input, 128, '|');
				b = (float)strtod(input, &pEnd);

				float scale;
				float offsetX;
				float offsetY;
				float offsetZ;

				importFile.getline(input, 128, '|');
				scale = (float)strtod(input, &pEnd);
				importFile.getline(input, 128, '|');
				offsetX = (float)strtod(input, &pEnd);
				importFile.getline(input, 128, '|');
				offsetY = (float)strtod(input, &pEnd);
				importFile.getline(input, 128, '|');
				offsetZ = (float)strtod(input, &pEnd);

				importFile.getline(input, 128, '|');
				int quantity = strtol(input, &pEnd, 10);

				// Also import the stats attributes
				int numStatAttributes;
				importFile.getline(input, 128, '|');
				numStatAttributes = strtol(input, &pEnd, 10);
				for(int k = 0; k < numStatAttributes; k++)
				{
					int type;
					int amount;
					importFile.getline(input, 128, '|');
					type = strtol(input, &pEnd, 10);

					importFile.getline(input, 128, '|');
					amount = strtol(input, &pEnd, 10);
				}
			}
		}

		// Import the equipped items
		for(int i = 0; i < EquipSlot_NumSlots; i++)
		{
			importFile.getline(input, 128, '|');
			int itemValid = strtol(input, &pEnd, 10);

			if(itemValid == 1)
			{
				char filename[128];
				importFile.getline(filename, 128, '|');

				char iconFilename[128];
				importFile.getline(iconFilename, 128, '|');

				importFile.getline(input, 128, '|');
				int itemType = strtol(input, &pEnd, 10);

				importFile.getline(input, 128, '|');
				int item = strtol(input, &pEnd, 10);

				importFile.getline(input, 128, '|');
				int status = strtol(input, &pEnd, 10);

				importFile.getline(input, 128, '|');
				int equipSlot = strtol(input, &pEnd, 10);

				importFile.getline(input, 128, '|');
				int itemQuality = strtol(input, &pEnd, 10);

				importFile.getline(input, 128, '|');
				int left = strtol(input, &pEnd, 10);

				importFile.getline(input, 128, '|');
				int right = strtol(input, &pEnd, 10);

				char title[128];
				importFile.getline(title, 128, '|');

				char description[128];
				importFile.getline(description, 128, '|');

				float r;
				float g;
				float b;
				importFile.getline(input, 128, '|');
				r = (float)strtod(input, &pEnd);
				importFile.getline(input, 128, '|');
				g = (float)strtod(input, &pEnd);
				importFile.getline(input, 128, '|');
				b = (float)strtod(input, &pEnd);

				float scale;
				float offsetX;
				float offsetY;
				float offsetZ;

				importFile.getline(input, 128, '|');
				scale = (float)strtod(input, &pEnd);
				importFile.getline(input, 128, '|');
				offsetX = (float)strtod(input, &pEnd);
				importFile.getline(input, 128, '|');
				offsetY = (float)strtod(input, &pEnd);
				importFile.getline(input, 128, '|');
				offsetZ = (float)strtod(input, &pEnd);

				importFile.getline(input, 128, '|');
				int quantity = strtol(input, &pEnd, 10);

				// Also import the stats attributes
				int numStatAttributes;
				importFile.getline(input, 128, '|');
				numStatAttributes = strtol(input, &pEnd, 10);
				for(int k = 0; k < numStatAttributes; k++)
				{
					int type;
					int amount;
					importFile.getline(input, 128, '|');
					type = strtol(input, &pEnd, 10);

					importFile.getline(input, 128, '|');
					amount = strtol(input, &pEnd, 10);
				}

				EquipItem((EquipSlot)equipSlot, filename, (bool)left,(bool)right);
			}
		}

		// Import the number of coins
		importFile.getline(input, 128, '|');
	}
}

void NPC::EquipItem(EquipSlot equipSlot, const char* itemFilename, bool left, bool right)
{
	switch(equipSlot)
	{
	case EquipSlot_LeftHand:
		{
			LoadWeapon(true, itemFilename);
		}
		break;
	case EquipSlot_RightHand:
		{
			LoadWeapon(false, itemFilename);
		}
		break;
	case EquipSlot_Head:
		{
			if(m_pVoxelCharacter != NULL)
			{
				VoxelWeapon* pNewEquipment = new VoxelWeapon(m_pRenderer, m_pQubicleBinaryManager);
				pNewEquipment->SetVoxelCharacterParent(NULL);
				pNewEquipment->LoadWeapon(itemFilename, false);

				pNewEquipment->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->SetScaleAndOffsetForMatrix("Helm", pNewEquipment->GetAnimatedSection(0)->m_renderScale, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.x, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.y, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.z);
				QubicleMatrix* pHelmMatrix = pNewEquipment->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->SetQubicleMatrix("Helm");
				pHelmMatrix->m_boneIndex = m_pVoxelCharacter->GetHeadBoneIndex();
				m_pVoxelCharacter->AddQubicleMatrix(pHelmMatrix, false);
			}
		}
		break;
	case EquipSlot_Shoulders:
		{
			if(m_pVoxelCharacter != NULL)
			{
				VoxelWeapon* pNewEquipment = new VoxelWeapon(m_pRenderer, m_pQubicleBinaryManager);
				pNewEquipment->SetVoxelCharacterParent(NULL);
				pNewEquipment->LoadWeapon(itemFilename, false);

				if(right)
				{
					pNewEquipment->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->SetScaleAndOffsetForMatrix("Right_Shoulder_Armor", pNewEquipment->GetAnimatedSection(0)->m_renderScale, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.x, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.y, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.z);
					QubicleMatrix* pRightShoulderMatrix = pNewEquipment->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->SetQubicleMatrix("Right_Shoulder_Armor");
					pRightShoulderMatrix->m_boneIndex = m_pVoxelCharacter->GetRightShoulderBoneIndex();
					m_pVoxelCharacter->AddQubicleMatrix(pRightShoulderMatrix, false);
				}

				if(left)
				{
					pNewEquipment->GetAnimatedSection(1)->m_pVoxelObject->GetQubicleModel()->SetScaleAndOffsetForMatrix("Left_Shoulder_Armor", pNewEquipment->GetAnimatedSection(1)->m_renderScale, pNewEquipment->GetAnimatedSection(1)->m_renderOffset.x, pNewEquipment->GetAnimatedSection(1)->m_renderOffset.y, pNewEquipment->GetAnimatedSection(1)->m_renderOffset.z);				
					QubicleMatrix* pLeftShoulderMatrix = pNewEquipment->GetAnimatedSection(1)->m_pVoxelObject->GetQubicleModel()->SetQubicleMatrix("Left_Shoulder_Armor");
					pLeftShoulderMatrix->m_boneIndex = m_pVoxelCharacter->GetLeftShoulderBoneIndex();				
					m_pVoxelCharacter->AddQubicleMatrix(pLeftShoulderMatrix, false);
				}
			}
		}
		break;
	case EquipSlot_Body:
		{
			if(m_pVoxelCharacter != NULL)
			{
				VoxelWeapon* pNewEquipment = new VoxelWeapon(m_pRenderer, m_pQubicleBinaryManager);
				pNewEquipment->SetVoxelCharacterParent(NULL);
				pNewEquipment->LoadWeapon(itemFilename, false);

				pNewEquipment->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->SetScaleAndOffsetForMatrix("Body", pNewEquipment->GetAnimatedSection(0)->m_renderScale, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.x, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.y, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.z);
				QubicleMatrix* pBodyMatrix = pNewEquipment->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->SetQubicleMatrix("Body");
				pBodyMatrix->m_boneIndex = m_pVoxelCharacter->GetBodyBoneIndex();
				m_pVoxelCharacter->AddQubicleMatrix(pBodyMatrix, false);
			}
		}
		break;
	case EquipSlot_Legs:
		{
			if(m_pVoxelCharacter != NULL)
			{
				VoxelWeapon* pNewEquipment = new VoxelWeapon(m_pRenderer, m_pQubicleBinaryManager);
				pNewEquipment->SetVoxelCharacterParent(NULL);
				pNewEquipment->LoadWeapon(itemFilename, false);

				pNewEquipment->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->SetScaleAndOffsetForMatrix("Legs", pNewEquipment->GetAnimatedSection(0)->m_renderScale, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.x, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.y, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.z);
				QubicleMatrix* pLegsMatrix = pNewEquipment->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->SetQubicleMatrix("Legs");
				pLegsMatrix->m_boneIndex = m_pVoxelCharacter->GetLegsBoneIndex();
				m_pVoxelCharacter->AddQubicleMatrix(pLegsMatrix, false);
			}
		}
		break;
	case EquipSlot_Hand:
		{
			if(m_pVoxelCharacter != NULL)
			{
				VoxelWeapon* pNewEquipment = new VoxelWeapon(m_pRenderer, m_pQubicleBinaryManager);
				pNewEquipment->SetVoxelCharacterParent(NULL);
				pNewEquipment->LoadWeapon(itemFilename, false);

				if(right)
				{
					pNewEquipment->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->SetScaleAndOffsetForMatrix("Right_Hand", pNewEquipment->GetAnimatedSection(0)->m_renderScale, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.x, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.y, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.z);
					QubicleMatrix* pRightHandMatrix = pNewEquipment->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->SetQubicleMatrix("Right_Hand");
					pRightHandMatrix->m_boneIndex = m_pVoxelCharacter->GetRightHandBoneIndex();
					m_pVoxelCharacter->AddQubicleMatrix(pRightHandMatrix, false);
				}

				if(left)
				{
					pNewEquipment->GetAnimatedSection(1)->m_pVoxelObject->GetQubicleModel()->SetScaleAndOffsetForMatrix("Left_Hand", pNewEquipment->GetAnimatedSection(1)->m_renderScale, pNewEquipment->GetAnimatedSection(1)->m_renderOffset.x, pNewEquipment->GetAnimatedSection(1)->m_renderOffset.y, pNewEquipment->GetAnimatedSection(1)->m_renderOffset.z);				
					QubicleMatrix* pLeftHandMatrix = pNewEquipment->GetAnimatedSection(1)->m_pVoxelObject->GetQubicleModel()->SetQubicleMatrix("Left_Hand");
					pLeftHandMatrix->m_boneIndex = m_pVoxelCharacter->GetLeftHandBoneIndex();				
					m_pVoxelCharacter->AddQubicleMatrix(pLeftHandMatrix, false);
				}
			}
		}
		break;
	case EquipSlot_Feet:
		{
			if(m_pVoxelCharacter != NULL)
			{
				VoxelWeapon* pNewEquipment = new VoxelWeapon(m_pRenderer, m_pQubicleBinaryManager);
				pNewEquipment->SetVoxelCharacterParent(NULL);
				pNewEquipment->LoadWeapon(itemFilename, false);

				if(right)
				{
					pNewEquipment->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->SetScaleAndOffsetForMatrix("Right_Foot", pNewEquipment->GetAnimatedSection(0)->m_renderScale, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.x, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.y, pNewEquipment->GetAnimatedSection(0)->m_renderOffset.z);
					QubicleMatrix* pRightFootMatrix = pNewEquipment->GetAnimatedSection(0)->m_pVoxelObject->GetQubicleModel()->SetQubicleMatrix("Right_Foot");
					pRightFootMatrix->m_boneIndex = m_pVoxelCharacter->GetRightFootBoneIndex();
					m_pVoxelCharacter->AddQubicleMatrix(pRightFootMatrix, false);
				}

				if(left)
				{
					pNewEquipment->GetAnimatedSection(1)->m_pVoxelObject->GetQubicleModel()->SetScaleAndOffsetForMatrix("Left_Foot", pNewEquipment->GetAnimatedSection(1)->m_renderScale, pNewEquipment->GetAnimatedSection(1)->m_renderOffset.x, pNewEquipment->GetAnimatedSection(1)->m_renderOffset.y, pNewEquipment->GetAnimatedSection(1)->m_renderOffset.z);				
					QubicleMatrix* pLeftFootMatrix = pNewEquipment->GetAnimatedSection(1)->m_pVoxelObject->GetQubicleModel()->SetQubicleMatrix("Left_Foot");
					pLeftFootMatrix->m_boneIndex = m_pVoxelCharacter->GetLeftFootBoneIndex();				
					m_pVoxelCharacter->AddQubicleMatrix(pLeftFootMatrix, false);
				}
			}
		}
		break;
	case EquipSlot_Accessory1:
		{
		}
		break;
	case EquipSlot_Accessory2:
		{
		}
		break;
	}
}

void NPC::UnequipItem(EquipSlot equipSlot, bool left, bool right)
{
	switch(equipSlot)
	{
	case EquipSlot_LeftHand:
		{
			UnloadWeapon(true);

			m_pVoxelCharacter->RemoveQubicleMatrix("Quiver");
		}
		break;
	case EquipSlot_RightHand:
		{
			UnloadWeapon(false);
		}
		break;
	case EquipSlot_Head:
		{
			m_pVoxelCharacter->RemoveQubicleMatrix("Helm");
		}
		break;
	case EquipSlot_Shoulders:
		{
			m_pVoxelCharacter->RemoveQubicleMatrix("Right_Shoulder_Armor");
			m_pVoxelCharacter->RemoveQubicleMatrix("Left_Shoulder_Armor");
		}
		break;
	case EquipSlot_Body:
		{
			QubicleMatrix* pBodyMatrix = m_pCharacterBackup->SetQubicleMatrix("Body");
			pBodyMatrix->m_boneIndex = m_pVoxelCharacter->GetBodyBoneIndex();
			m_pVoxelCharacter->AddQubicleMatrix(pBodyMatrix, false);

			char characterFilename[128];
			sprintf(characterFilename, "media/gamedata/characters/%s/%s.character", m_type.c_str(), m_modelName.c_str());
			m_pVoxelCharacter->ResetMatrixParamsFromCharacterFile(characterFilename, "Body");
		}
		break;
	case EquipSlot_Legs:
		{
			QubicleMatrix* pLegsMatrix = m_pCharacterBackup->SetQubicleMatrix("Legs");
			pLegsMatrix->m_boneIndex = m_pVoxelCharacter->GetLegsBoneIndex();
			m_pVoxelCharacter->AddQubicleMatrix(pLegsMatrix, false);

			char characterFilename[128];
			sprintf(characterFilename, "media/gamedata/characters/%s/%s.character", m_type.c_str(), m_modelName.c_str());
			m_pVoxelCharacter->ResetMatrixParamsFromCharacterFile(characterFilename, "Legs");
		}
		break;
	case EquipSlot_Hand:
		{
			if (right)
			{
				QubicleMatrix* pRightHandMatrix = m_pCharacterBackup->SetQubicleMatrix("Right_Hand");
				pRightHandMatrix->m_boneIndex = m_pVoxelCharacter->GetRightHandBoneIndex();
				m_pVoxelCharacter->AddQubicleMatrix(pRightHandMatrix, false);
			}

			if (left)
			{
				QubicleMatrix* pLeftHandMatrix = m_pCharacterBackup->SetQubicleMatrix("Left_Hand");
				pLeftHandMatrix->m_boneIndex = m_pVoxelCharacter->GetLeftHandBoneIndex();
				m_pVoxelCharacter->AddQubicleMatrix(pLeftHandMatrix, false);
			}

			char characterFilename[128];
			sprintf(characterFilename, "media/gamedata/characters/%s/%s.character", m_type.c_str(), m_modelName.c_str());
			m_pVoxelCharacter->ResetMatrixParamsFromCharacterFile(characterFilename, "Right_Hand");
			m_pVoxelCharacter->ResetMatrixParamsFromCharacterFile(characterFilename, "Left_Hand");
		}
		break;
	case EquipSlot_Feet:
		{
			if (right)
			{
				QubicleMatrix* pRightFootMatrix = m_pCharacterBackup->SetQubicleMatrix("Right_Foot");
				pRightFootMatrix->m_boneIndex = m_pVoxelCharacter->GetRightFootBoneIndex();
				m_pVoxelCharacter->AddQubicleMatrix(pRightFootMatrix, false);
			}

			if (left)
			{
				QubicleMatrix* pLeftFootMatrix = m_pCharacterBackup->SetQubicleMatrix("Left_Foot");
				pLeftFootMatrix->m_boneIndex = m_pVoxelCharacter->GetLeftFootBoneIndex();
				m_pVoxelCharacter->AddQubicleMatrix(pLeftFootMatrix, false);
			}

			char characterFilename[128];
			sprintf(characterFilename, "media/gamedata/characters/%s/%s.character", m_type.c_str(), m_modelName.c_str());
			m_pVoxelCharacter->ResetMatrixParamsFromCharacterFile(characterFilename, "Right_Foot");
			m_pVoxelCharacter->ResetMatrixParamsFromCharacterFile(characterFilename, "Left_Foot");
		}
		break;
	case EquipSlot_Accessory1:
		{
		}
		break;
	case EquipSlot_Accessory2:
		{
		}
		break;
	}
}

// Animation
void NPC::SetAnimationSpeed(float speed, bool onlySetOnCompleteAnimation, AnimationSections section)
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

float NPC::GetAnimationSpeed(AnimationSections section)
{
	return m_animationSpeed[section];
}

bool NPC::AnimationFinished(AnimationSections section)
{
	return m_animationFinished[section];
}

// Skeleton bone scale
void NPC::SetSkeletonBoneScale(float scale)
{
	if(m_pVoxelCharacter)
	{
		m_pVoxelCharacter->SetBoneScale(scale);
	}
}

float NPC::GetSkeletonBoneScale()
{
	if(m_pVoxelCharacter)
	{
		return m_pVoxelCharacter->GetBoneScale().x; // x,y,z all the same
	}

	return 1.0f;
}

// NPC State
eNPCState NPC::GetState() const
{
	return m_eNPCState;
}

// Movement gameplay
void NPC::SetMoveToPlayer(bool moveToPlayer)
{
	m_moveToPlayer = moveToPlayer;
}

void NPC::SetLookAtPositionWhenReachedTarget(bool enabled, vec3 lookAtPosition)
{
	m_lookAtPositionWhenReachTarget = enabled;
	m_lookAtPositionWhenReachedTargetPosition = lookAtPosition;
}

// Dead
bool NPC::IsDead() const
{
	// TODO : IsDead()
	return false;
}

// Projectile hitbox
eProjectileHitboxType NPC::GetProjectileHitboxType()
{
	return m_eProjectileHitboxType;
}

float NPC::GetProjectileHitboxRadius() const
{
	return m_projectileHitboxRadius;
}

float NPC::GetProjectileHitboxXLength() const
{
	return m_projectileHitboxXLength;
}

float NPC::GetProjectileHitboxYLength() const
{
	return m_projectileHitboxYLength;
}

float NPC::GetProjectileHitboxZLength() const
{
	return m_projectileHitboxZLength;
}

vec3 NPC::GetProjectileHitboxCenter() const
{
	return GetCenter() + m_projectileHitboxCenterOffset;
}

// Movement
void NPC::MoveAbsolute(vec3 direction, const float speed, bool shouldChangeForward)
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

		if(CheckCollisions(posToCheck, m_previousPosition, &pNormal, &movement))
		{
		}

		m_position += (movement * speedToUse)*0.95f;
	}

	if(CanAttack())
	{
		if(m_eNPCCombatType == eNPCCombatType_Staff)
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_Legs_Feet, "StaffRun", 0.01f);
		}
		else
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_Legs_Feet, "Run", 0.01f);
		}
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

	m_bIsIdle = false;
}

void NPC::StopMoving()
{
	if(m_isCreditsNPC)
	{
		return;
	}

	if(m_bCanJump)
	{
		if(m_bIsIdle == false)
		{
			m_bIsIdle = true;
			
			SetRandomLookMode();
		}
		
		if(m_hasSetIdleAnimation == false)
		{
			if(m_waitBeforeStopMovingAnimationTimer <= 0.0f)
			{
				if(CanAttack())
				{
					if(m_eNPCCombatType == eNPCCombatType_Staff)
					{
						m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "StaffPose", 0.15f);
					}
					else
					{
						m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "BindPose", 0.15f);
					}
				}
				if(m_bCanInteruptCombatAnim)
				{
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Head_Body, false, AnimationSections_Head_Body, "BindPose", 0.15f);
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Legs_Feet, false, AnimationSections_Legs_Feet, "BindPose", 0.15f);
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "BindPose", 0.15f);
				}

				m_hasSetIdleAnimation = true;
			}
		}
	}
}

void NPC::Jump()
{
	if(m_bCanJump == false)
	{
		return;
	}

	if(m_jumpTimer >= 0.0f)
	{
		return;
	}

	m_bCanJump = false;
	m_jumpTimer = m_jumpTime;

	m_velocity += vec3(0.0f, m_jumpHeight, 0.0f);

	if(CanAttack())
	{
		SetAnimationSpeed(1.0f, false, AnimationSections_Right_Arm_Hand);
		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "Jump", 0.01f);
	}

	if(m_bCanInteruptCombatAnim)
	{
		SetAnimationSpeed(1.0f, false, AnimationSections_Head_Body);
		SetAnimationSpeed(1.0f, false, AnimationSections_Legs_Feet);
		SetAnimationSpeed(1.0f, false, AnimationSections_Left_Arm_Hand);
		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Head_Body, false, AnimationSections_Head_Body, "Jump", 0.01f);
		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Legs_Feet, false, AnimationSections_Legs_Feet, "Jump", 0.01f);
		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "Jump", 0.01f);
	}
}

void NPC::SetTargetPosition(vec3 pos)
{
	m_targetPosition = pos;
}

WayPoint* NPC::AddWaypoint(vec3 pos, float xLength, float yLength, float zLength)
{
	WayPoint* pNewWayPoint = new WayPoint();
	pNewWayPoint->m_position = pos;
	pNewWayPoint->m_xLength = xLength;
	pNewWayPoint->m_yLength = yLength;
	pNewWayPoint->m_zLength = zLength;
	pNewWayPoint->m_pNextWaypoint = NULL;

	// Connect to previous waypoint
	if(m_vpWayPointList.size() > 0)
	{
		m_vpWayPointList[m_vpWayPointList.size()-1]->m_pNextWaypoint = pNewWayPoint;
	}

	m_vpWayPointList.push_back(pNewWayPoint);

	return pNewWayPoint;
}

void NPC::ClearWaypoints()
{
	for(unsigned int i = 0; i < m_vpWayPointList.size(); i++)
	{
		delete m_vpWayPointList[i];
		m_vpWayPointList[i] = 0;
	}
	m_vpWayPointList.clear();
}

void NPC::StartWaypointNavigation(int waypointIndex)
{
	if((int)m_vpWayPointList.size() > 0 && waypointIndex < (int)m_vpWayPointList.size())
	{
		m_targetPosition = m_vpWayPointList[waypointIndex]->m_position;
		m_currentWaypointIndex = waypointIndex;
		m_eNPCState = eNPCState_MovingToWayPoint;
	}
}

bool NPC::IsInsideWaypoint(int waypointIndex)
{
	if(waypointIndex < (int)m_vpWayPointList.size())
	{
		WayPoint* pWaypoint = m_vpWayPointList[waypointIndex];

		float lowX = pWaypoint->m_position.x - pWaypoint->m_xLength;
		float lowY = pWaypoint->m_position.y - pWaypoint->m_yLength;
		float lowZ = pWaypoint->m_position.z - pWaypoint->m_zLength;
		float highX = pWaypoint->m_position.x + pWaypoint->m_xLength;
		float highY = pWaypoint->m_position.y + pWaypoint->m_yLength;
		float highZ = pWaypoint->m_position.z + pWaypoint->m_zLength;

		vec3 pos = GetCenter();
		if(pos.x < lowX || pos.x > highX || pos.y < lowY || pos.y > highY || pos.z < lowZ || pos.z > highZ)
		{
			return false;
		}
		
		return true;
	}

	return false;
}

void NPC::SetMoveBackToPosition(vec3 pos)
{
	m_moveBackToPosition = pos;
}

void NPC::SetForwards(vec3 dir)
{
	m_forward = normalize(dir);
}

void NPC::SetTargetForwards(vec3 dir)
{
	m_targetForward = normalize(dir);
}

bool NPC::HasReachedTargetPosition()
{
	return m_hasReachedTargetPosition;
}

// Combat
void NPC::DoDamage(float amount, Colour textColour, vec3 knockbackDirection, float knockbackAmount, bool createParticleHit)
{
	if(m_damageTimer <= 0.0f)
	{
		float healthBefore = m_health;
		float damageDone = healthBefore - m_health;

		m_health -= amount;

		// Figure out if we are dead yet
		if(m_health <= 0.0f)
		{
			m_health = 0.0f;

			Explode();
		}

		// Update HUD player data
		VoxGame::GetInstance()->GetHUD()->UpdatePlayerData();

		// Play a hit response animation
		m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "HitResponse", 0.01f);

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
	}

	if(m_knockbackTimer <= 0.0f)
	{
		m_velocity += knockbackDirection * knockbackAmount;

		m_knockbackTimer = m_knockbackTime;
	}
}

void NPC::CheckEnemyDamageRadius()
{
	int numEnemies = m_pEnemyManager->GetNumEnemies();

	for(int i = 0; i < numEnemies; i++)
	{
		Enemy *pEnemy = m_pEnemyManager->GetEnemy(i);

		if(pEnemy->GetErase() == true)
		{
			continue;
		}

		if(pEnemy->GetAttackEnabled() == false)
		{
			continue;
		}

		vec3 distance = GetCenter() - pEnemy->GetCenter();
		float lengthToEnemy = length(distance);
		if(lengthToEnemy <= m_radius + pEnemy->GetAttackRadius())
		{
			vec3 distance_minus_y = distance;
			distance_minus_y.y = 0.0f;
			vec3 direction = normalize(distance_minus_y);
			direction = normalize(direction);

			// Figure out the attack vector, based on the attack rotation
			float enemyRotation = pEnemy->GetRotation();
			float attackRotation = pEnemy->GetAttackRotation();
			float angle = DegToRad(enemyRotation + attackRotation);
			vec3 attackDirection = vec3(sin(angle), 0.0f, cos(angle));
			float dotProduct = dot(direction, attackDirection);

			if(dotProduct > pEnemy->GetAttackSegmentAngle()) // Check if we are within the attack segment
			{	
				vec3 knockbackDirection = (direction*2.0f) + vec3(0.0f, 1.0f, 0.0f);

				knockbackDirection = normalize(knockbackDirection);
				Colour damageColour = Colour(1.0f, 1.0f, 1.0f);

				// Set NPC target as attacker
				SetTargetEnemy(pEnemy);

				float knockbackAmount = 16.0f;
				DoDamage(20.0f, damageColour, knockbackDirection, knockbackAmount, true);
			}
		}
	}
}

void NPC::CheckProjectileDamageRadius(Projectile* pProjectile)
{
	if(pProjectile->CanAttackNPCs() == false)
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
		vec3 knockbackDirection = (normalize(pProjectile->GetVelocity())*2.0f) + vec3(0.0f, 1.0f, 0.0f);
		knockbackDirection = normalize(knockbackDirection);
		Colour damageColour = Colour(1.0f, 1.0f, 1.0f);

		if(pProjectile->GetEnemyOwner() != NULL)
		{
			// Set NPC target as attacker
			SetTargetEnemy(pProjectile->GetEnemyOwner());
		}

		float knockbackAmount = 16.0f;
		DoDamage(15.0f, damageColour, knockbackDirection, knockbackAmount, false);

		pProjectile->Explode();
	}
}

void NPC::Explode()
{
	if (IsDead())
	{
		return;
	}

	// TODO : NPC Dead
	//m_dead = true;

	m_bIsChargingAttack = false;
	m_chargeAmount = 0.0f;

	CalculateWorldTransformMatrix();

	// Explode the qubicle binary voxel file
	for (int explodeCounter = 0; explodeCounter < 3; explodeCounter++)
	{
		const QubicleBinary* pQubicleModel = NULL;
		int spawnChance = 100;
		if (explodeCounter == 0)
		{
			pQubicleModel = m_pVoxelCharacter->GetQubicleModel();
			spawnChance = 50;
		}
		else if (explodeCounter == 1)
		{
			if (m_pVoxelCharacter->GetRightWeapon() != NULL)
			{
				if (m_pVoxelCharacter->IsRightWeaponLoaded())
				{
					for (int animatedSectionsIndex = 0; animatedSectionsIndex < m_pVoxelCharacter->GetRightWeapon()->GetNumAimatedSections(); animatedSectionsIndex++)
					{
						AnimatedSection* pAnimatedSection = m_pVoxelCharacter->GetRightWeapon()->GetAnimatedSection(animatedSectionsIndex);
						pQubicleModel = pAnimatedSection->m_pVoxelObject->GetQubicleModel();
						spawnChance = 100;
					}
				}
			}
		}
		else if (explodeCounter == 2)
		{
			if (m_pVoxelCharacter->GetLeftWeapon() != NULL)
			{
				if (m_pVoxelCharacter->IsLeftWeaponLoaded())
				{
					for (int animatedSectionsIndex = 0; animatedSectionsIndex < m_pVoxelCharacter->GetLeftWeapon()->GetNumAimatedSections(); animatedSectionsIndex++)
					{
						AnimatedSection* pAnimatedSection = m_pVoxelCharacter->GetLeftWeapon()->GetAnimatedSection(animatedSectionsIndex);
						pQubicleModel = pAnimatedSection->m_pVoxelObject->GetQubicleModel();
						spawnChance = 100;
					}
				}
			}
		}

		if (pQubicleModel != NULL)
		{
			m_pBlockParticleManager->ExplodeQubicleBinary(pQubicleModel, m_pVoxelCharacter->GetCharacterScale(), spawnChance);
		}
	}

	// Unload weapons
	UnloadWeapon(true);
	UnloadWeapon(false);
}

void NPC::Respawn()
{
	// TODO : NPC Respawn()
}

// Attacking
void NPC::Attack()
{
	bool doAttack = false;

	if(m_attackDelayTimer <= 0.0f)
	{
		if(m_eNPCCombatType == eNPCCombatType_MeleeSword)
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

			m_movementWaitAfterAttackTimer = 0.75f;
			m_attackDelayTime = 0.15f;
			m_attackEnabled = true;
			m_attackEnabledTimer = 0.0f;
			m_attackRotation = startRotation;
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledTimer, 0.0f, attackTime, attackTime, 0.0f, NULL, _AttackEnabledTimerFinished, this);
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledDelayTimer, m_attackEnabledDelayTimer, 0.0f, m_attackEnabledDelayTimer, 0.0f, NULL, _AttackEnabledDelayTimerFinished, this);
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackRotation, startRotation, endRotation, attackTime, easingRotation);

			// Start weapon trails
			if(m_pVoxelCharacter->GetRightWeapon())
			{
				if(m_pVoxelCharacter->IsRightWeaponLoaded())
				{
					m_pVoxelCharacter->GetRightWeapon()->StartWeaponTrails();
				}
			}

			doAttack = true;
		}
		else if(m_eNPCCombatType == eNPCCombatType_Archer)
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
			}

			doAttack = true;
		}
		else if(m_eNPCCombatType == eNPCCombatType_Staff)
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "StaffAttack", 0.01f);

			m_attackDelayTime = 1.35f + GetRandomNumber(-100, 50, 2) * 0.005f;

			m_attackEnabledDelayTimer = 0.15f;
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledDelayTimer, m_attackEnabledDelayTimer, 0.0f, m_attackEnabledDelayTimer, 0.0f, NULL, _AttackEnabledDelayTimerFinished, this);

			doAttack = true;
		}
		else if(m_eNPCCombatType == eNPCCombatType_FireballHands)
		{
			m_attackCounter++;

			if(m_attackCounter == 2)
			{
				m_attackCounter = 0;
			}

			if(m_attackCounter == 1)
			{
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "HandSpellCastRight", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "HandSpellCastRight", 0.01f);
			}
			else
			{
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "HandSpellCastLeft", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "HandSpellCastLeft", 0.01f);
			}

			m_animationFinished[AnimationSections_Right_Arm_Hand] = true;
			SetAnimationSpeed(1.5f, true, AnimationSections_FullBody);

			m_attackDelayTime = 0.75f + GetRandomNumber(-50, 50, 2) * 0.005f;
			Interpolator::GetInstance()->AddFloatInterpolation(&m_animationTimer, 0.0f, 0.3f, 0.3f, 0.0f, NULL, _AttackEnabledDelayTimerFinished, this);

			m_bCanAttack = false;

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

void NPC::ReleaseAttack()
{
	if(m_bIsChargingAttack == true)
	{
		UnloadWeapon(false);

		Projectile* pProjectile = m_pProjectileManager->CreateProjectile(m_chargeSpawnPosition, m_chargeSpawnVelocity, 0.0f, "media/gamedata/items/Arrow/arrow.item", 0.08f);	
		pProjectile->SetProjectileType(true, false, false);
		pProjectile->SetOwner(NULL, this, NULL);

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

bool NPC::CanAttack() const
{
	return m_bCanAttack;
}

bool NPC::GetAttackEnabled() const
{
	return m_attackEnabled && (m_attackEnabledDelayTimer <= 0.0f);
}

float NPC::GetAttackRadius() const
{
	return m_attackRadius;
}

void NPC::SetAttackRadius(float attackRadius)
{
	m_attackRadius = attackRadius;
}

float NPC::GetAttackRotation() const
{
	return m_attackRotation;
}

float NPC::GetAttackSegmentAngle() const
{
	return m_attackSegmentAngle;
}

void NPC::SetEnemyDied(Enemy* pEnemy)
{
	if(m_pTargetEnemy == pEnemy)
	{
		SetTargetEnemy(NULL);

		SetRandomLookMode();
	}
}

void NPC::SetTargetEnemy(Enemy* pEnemy)
{
	m_pTargetEnemy = pEnemy;

	if(m_pTargetEnemy != NULL)
	{
		if(m_eNPCCombatType == eNPCCombatType_Archer || m_eNPCCombatType == eNPCCombatType_Staff || m_eNPCCombatType == eNPCCombatType_FireballHands)
		{
			StopMoving();
		}

		if(m_eNPCState != eNPCState_Combat)
		{
			m_ePreviousNPCState = m_eNPCState;
			m_eNPCState = eNPCState_Combat;
		}
	}
	else
	{
		m_eNPCState = m_ePreviousNPCState;

		if(m_eNPCCombatType == eNPCCombatType_Archer)
		{
			m_bIsChargingAttack = false;
			m_chargeAmount = 0.0f;

			UnloadWeapon(false);
		}

		if(m_eNPCState == eNPCState_MovingToWayPoint)
		{
			StartWaypointNavigation(m_currentWaypointIndex);
		}
		else if(m_moveToPlayer == false)
		{
			SetTargetPosition(m_moveBackToPosition);
		}
	}
}

// Looking
void NPC::SetTargetForwardToLookAtPoint(vec3 point)
{
	vec3 toPoint = point - GetCenter();
	toPoint.y = 0.0f;
	toPoint = normalize(toPoint);

	m_targetForward = toPoint;
}

void NPC::LookAtPoint(vec3 point)
{
	m_bLookAtPoint = true;
	m_lookPoint = point;
}

void NPC::SetRandomLookMode()
{
	m_pVoxelCharacter->SetFaceTargetDirection(m_pVoxelCharacter->GetFaceLookingDirection());
	m_pVoxelCharacter->SetRandomLookDirection(true);
	m_pVoxelCharacter->SetFaceLookToTargetSpeedMultiplier(1.0f);
	m_bLookAtPoint = false;
}

void NPC::SetBodyTurnStopThreshold(float threshold)
{
	m_bodyTurnStopThreshold = threshold;
}

void NPC::SetBodyTurnSpeedMultiplier(float multiplier)
{
	m_bodyTurnSpeedMultiplier = multiplier;
}

bool NPC::IsLookAtPointMode() const
{
	return m_bLookAtPoint;
}

bool NPC::IsLookingAtPoint() const
{
	return m_bIsLookingAtPoint;
}

// Talking
void NPC::SetNPCTalkMode(float talkDelay, float talkTime)
{
	m_talkMode = true;
	m_talkDelay = talkDelay;
	m_talkTime = talkTime;
	m_talkTimer = 0.0f;

	if(m_eNPCState != eNPCState_Talking)
	{
		m_ePreviousNPCState = m_eNPCState;
		m_eNPCState = eNPCState_Talking;
	}

	m_createdAnvilHitParticleEffect = true;

	m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "BindPose", 0.15f);
}

void NPC::StopNPCTalkMode()
{
	m_talkMode = false;
	m_pVoxelCharacter->SetTalkingAnimationEnabled(false);

	m_eNPCState = m_ePreviousNPCState;
}

// Working
void NPC::SetWorking(vec3 workPointOfInterest)
{
	m_isWorking = true;

	m_lookAtWorkPoint = workPointOfInterest;

	if(m_eNPCState != eNPCState_Working)
	{
		m_ePreviousNPCState = m_eNPCState;
		m_eNPCState = eNPCState_Working;
	}
}

void NPC::StopWorking()
{
	m_isWorking = false;

	m_eNPCState = m_ePreviousNPCState;
}

// World
void NPC::UpdateGridPosition()
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

Chunk* NPC::GetCachedGridChunkOrFromPosition(vec3 pos) const
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

void NPC::ClearChunkCacheForChunk(Chunk* pChunk)
{
	if(m_pCachedGridChunk == pChunk)
	{
		m_pCachedGridChunk = NULL;
	}
}

// Collision
bool NPC::CheckCollisions(vec3 positionCheck, vec3 previousPosition, vec3 *pNormal, vec3 *pMovement) const
{
	float radius = GetRadius();

	vec3 movementCache = *pMovement;

	// World collisions
	bool worldCollision = false;

	int blockX, blockY, blockZ;
	vec3 blockPos;

	vec3 floorPosition;
	if (m_pChunkManager->FindClosestFloor(positionCheck, &floorPosition) == false)
	{
		*pMovement = vec3(0.0f, 0.0f, 0.0f);
		return true;
	}
	else
	{
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

	if(worldCollision)
		return true;

	*pMovement = movementCache;

	return false;
}

bool NPC::IsBlockInFront() const
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

bool NPC::IsPushingCollisionEnabled() const
{
	return  m_bPushingCollisionEnabled;
}

void NPC::SetPushingCollisionEnabled(bool pushEnabled)
{
	 m_bPushingCollisionEnabled = pushEnabled;
}

void NPC::SetCreditsNPC(bool creditsNPC)
{
	m_isCreditsNPC = creditsNPC;
}

bool NPC::IsCreditsNPC() const
{
	return m_isCreditsNPC;
}

void NPC::CalculateWorldTransformMatrix()
{
	m_forward = normalize(m_forward);

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

void NPC::SetOutlineRender(bool outline)
{
	m_outlineRender = outline;
}

bool NPC::GetOutlineRender() const
{
	return m_outlineRender;
}

void NPC::SetHoverRender(bool hover)
{
	m_hoverRender = hover;
}

bool NPC::GetHoverRender() const
{
	return m_hoverRender;
}

void NPC::SetWireFrameRender(bool wireframe)
{
	if(m_pVoxelCharacter != NULL)
	{
		m_pVoxelCharacter->SetWireFrameRender(wireframe);
	}
}

void NPC::SetSubSelectionRender(bool subSelection)
{
	m_subSelectionRender = subSelection;
}

bool NPC::GetSubSelectionRender() const
{
	return m_subSelectionRender;
}

vec2 NPC::GetScreenPosition() const
{
	return m_screenPosition;
}

void NPC::UpdateWeaponLights(float dt)
{
	for (int i = 0; i < 2; i++)
	{
		VoxelWeapon* pWeapon = NULL;
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

		if (pWeapon != NULL)
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

void NPC::UpdateWeaponParticleEffects(float dt)
{
	for (int i = 0; i < 2; i++)
	{
		VoxelWeapon* pWeapon = NULL;
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

		if (pWeapon != NULL)
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

void NPC::UpdateLookingAndForwardTarget(float dt)
{
	vec3 toPoint = m_lookPoint - GetCenter();
	toPoint = normalize(toPoint);

	if(m_bLookAtPoint)
	{
		m_targetForward = toPoint;
		m_targetForward.y = 0.0f;
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

	if(m_pVoxelCharacter != NULL)
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

			vec3 lookDirection = vec3(tempX, toPoint.y, tempZ);

			m_pVoxelCharacter->SetFaceTargetDirection(lookDirection);
			m_pVoxelCharacter->SetRandomLookDirection(false);
			m_pVoxelCharacter->SetFaceLookToTargetSpeedMultiplier(4.0f);
		}
		else
		{
			m_pVoxelCharacter->SetRandomLookDirection(true);
			m_pVoxelCharacter->SetFaceLookToTargetSpeedMultiplier(1.0f);
		}
	}
}

void NPC::UpdateTalkMode(float dt)
{
	if(m_talkMode)
	{
		m_talkTimer += dt;
		if(m_talkTimer >= m_talkDelay)
		{
			m_pVoxelCharacter->SetTalkingAnimationEnabled(true);
		}

		if(m_talkTimer >= m_talkDelay + m_talkTime)
		{
			m_talkMode = false;
			m_pVoxelCharacter->SetTalkingAnimationEnabled(false);
		}
	}
}

void NPC::UpdateWorking(float dt)
{
	if(m_isWorking)
	{
		LookAtPoint(m_lookAtWorkPoint);

		if(m_pVoxelCharacter->HasAnimationFinished(AnimationSections_FullBody))
		{
			if(m_createdAnvilHitParticleEffect == false)
			{
				vec3 anvilHitPos = m_lookPoint + vec3(0.0f, 0.5f, 0.0f);

				unsigned int effectId = -1;
				BlockParticleEffect* pBlockParticleEffect = VoxGame::GetInstance()->GetBlockParticleManager()->ImportParticleEffect("media/gamedata/particles/anvil_hit.effect", anvilHitPos, &effectId);
				pBlockParticleEffect->PlayEffect();

				// Stop weapon trails
				if(m_pVoxelCharacter->GetRightWeapon())
				{
					if(m_pVoxelCharacter->IsRightWeaponLoaded())
					{
						m_pVoxelCharacter->GetRightWeapon()->StopWeaponTrails();
					}
				}

				m_createdAnvilHitParticleEffect = true;
			}

			if(m_workingAnimationWaitTimer <= 0.0f)
			{
				m_workingRepetitionCounter++;
				m_workingAnimationWaitTimer = m_workingAnimationDelay;

				if(m_workingRepetitionCounter == m_workingRepetitionBeforeLongDelay)
				{
					m_workingRepetitionCounter = 0;
					m_workingAnimationWaitTimer += (m_workingAnimationDelay*2.0f); // 3 times as long delay (2 + 1)
				}

				if(m_pVoxelCharacter->HasAnimationFinished(AnimationSections_FullBody))
				{
					for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
					{
						SetAnimationSpeed(1.0f, false, (AnimationSections)i);
					}

					m_pVoxelCharacter->PlayAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "AnvilWork");

					// Start weapon trails
					if(m_pVoxelCharacter->GetRightWeapon())
					{
						if(m_pVoxelCharacter->IsRightWeaponLoaded())
						{
							m_pVoxelCharacter->GetRightWeapon()->StartWeaponTrails();
						}
					}

					m_createdAnvilHitParticleEffect = false;
				}
			}
			else
			{
				m_workingAnimationWaitTimer -= dt;
			}
		}
	}
}

void NPC::UpdateCombat(float dt)
{
	if(m_eNPCCombatType == eNPCCombatType_MeleeSword)
	{
		UpdateMeleeCombat(dt);
	}
	else if(m_eNPCCombatType == eNPCCombatType_Archer)
	{
		UpdateRangedCombat(dt);
	}
	else if(m_eNPCCombatType == eNPCCombatType_Staff)
	{
		UpdateRangedCombat(dt);
	}
	else if(m_eNPCCombatType == eNPCCombatType_FireballHands)
	{
		UpdateRangedCombat(dt);
	}

	// Reset the canAttack flag if our weapon arm animation is completed
	if (m_bCanAttack == false && (m_animationFinished[AnimationSections_Right_Arm_Hand] == true))
	{
		// Stop weapon trails
		if (m_pVoxelCharacter != NULL)
		{
			if (m_pVoxelCharacter->GetRightWeapon())
			{
				if (m_pVoxelCharacter->IsRightWeaponLoaded())
				{
					m_pVoxelCharacter->GetRightWeapon()->StopWeaponTrails();
				}
			}
		}

		m_bCanAttack = true;
		m_bCanInteruptCombatAnim = true;
	}
}

void NPC::UpdateMeleeCombat(float dt)
{
	vec3 toTarget = m_targetPosition - GetCenter();
	toTarget.y = 0.0f;
	float lengthToTarget = length(toTarget);

	m_lookPoint = m_targetPosition;
	m_bLookAtPoint = true;

	if(lengthToTarget <= m_attackRadius + m_pTargetEnemy->GetRadius())
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

void NPC::UpdateRangedCombat(float dt)
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

	// Charging - figure out trajectory and velocity for projectile
	if (m_bIsChargingAttack)
	{
		m_chargeAmount += dt / m_chargeTime;

		if (m_chargeAmount > 1.0f)
		{
			m_chargeAmount = 1.0f;
		}
	}

	if (m_eNPCCombatType == eNPCCombatType_Archer && m_pTargetEnemy != NULL)
	{
		m_chargeSpawnPosition = GetCenter() + (m_forward*0.75f) + (GetUpVector()*0.5f);

		float liftAmount = 1.75f * m_chargeAmount;
		float powerAmount = 45.0f * m_chargeAmount;

		//if(m_pTargetEnemy)
		{
			// Player target
			vec3 toTarget = m_pTargetEnemy->GetCenter() - GetCenter();
			float toTargetDistance = length(toTarget);
			liftAmount += toTargetDistance * 0.20f;
			m_chargeSpawnVelocity = (normalize(toTarget) * powerAmount) + vec3(0.0f, liftAmount, 0.0f);
		}
	}
}

void NPC::UpdateMovement(float dt)
{
	if(m_eNPCState == eNPCState_Talking || m_eNPCState == eNPCState_Working)
	{
		return;
	}

	bool lReachedTarget = false;

	if(m_eNPCState == eNPCState_MovingToWayPoint)
	{
		lReachedTarget = IsInsideWaypoint(m_currentWaypointIndex);
		m_hasReachedTargetPosition = lReachedTarget;
	}
	else
	{
		float radius = 0.0f;
		if(m_pTargetEnemy != NULL)
		{
			SetTargetPosition(m_pTargetEnemy->GetCenter());
			radius = m_pTargetEnemy->GetRadius();
		}
		else
		{
			if(m_moveToPlayer)
			{
				SetTargetPosition(m_pPlayer->GetCenter());
				radius = m_pPlayer->GetRadius();
			}
			else
			{
				SetTargetPosition(m_moveBackToPosition);
				radius = GetRadius();
			}
		}

		vec3 toTarget = m_targetPosition - GetCenter();
		toTarget.y = 0;
		float lengthToTarget = length(toTarget);

		lReachedTarget = (lengthToTarget < m_attackRadius*0.99f + radius);
		m_hasReachedTargetPosition = lReachedTarget;
	}

	if(m_bIsIdle && lReachedTarget == false)
	{
		m_hasSetIdleAnimation = false;
		m_waitBeforeStopMovingAnimationTimer = 0.25f;
	}

	if(lReachedTarget && m_bCanJump == true)
	{
		if(m_eNPCState == eNPCState_MovingToWayPoint)
		{
			m_currentWaypointIndex++;
			if(m_currentWaypointIndex >= (int)m_vpWayPointList.size())
			{
				m_currentWaypointIndex = 0;
				StartWaypointNavigation(m_currentWaypointIndex);
			}
			else
			{
				StartWaypointNavigation(m_currentWaypointIndex);
			}
		}
		else
		{
			StopMoving();
		}

		if(m_lookAtPositionWhenReachTarget)
		{
			LookAtPoint(m_lookAtPositionWhenReachedTargetPosition);
			m_lookAtPositionWhenReachTarget = false;
		}
	}
	else
	{
 		LookAtPoint(m_targetPosition);

		bool shouldStopMovingUntilJump = false;
		if(IsBlockInFront())
		{
			if(m_jumpTimer > 0.0f)
			{
				shouldStopMovingUntilJump = true;
				StopMoving();
			}

			Jump();
		}

		if(shouldStopMovingUntilJump == false)
		{
			if(m_movementWaitAfterAttackTimer <= 0.0f)
			{
				if((m_eNPCCombatType != eNPCCombatType_Archer && m_eNPCCombatType != eNPCCombatType_Staff && m_eNPCCombatType != eNPCCombatType_FireballHands) || m_pTargetEnemy == NULL)
				{
					vec3 toTarget = m_targetPosition - m_position;
					vec3 movementDirection = toTarget;
					movementDirection.y = 0.0f;
					movementDirection = normalize(movementDirection);

					if(m_eNPCState == eNPCState_MovingToWayPoint)
					{
						m_movementSpeed = 3.75f;
					}
					else
					{
						vec3 movementDistance = toTarget;
						movementDistance.y = 0.0f;
						m_movementSpeed = ((length(movementDistance) + 1.5f) / 8.0f) * m_maxMovementSpeed;

						if(m_movementSpeed > m_maxMovementSpeed)
							m_movementSpeed = m_maxMovementSpeed;
						if(m_movementSpeed < m_minMovementSpeed)
							m_movementSpeed = m_minMovementSpeed;
					}

					float movementSpeed = m_movementSpeed * dt;
					if (movementSpeed > 0.5f)
					{
						movementSpeed = 0.5f;
					}
					float animationSpeed = (m_movementSpeed / m_maxMovementSpeed);

					if(m_bCanJump && m_bCanInteruptCombatAnim == false)
					{
						for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
						{
							SetAnimationSpeed(animationSpeed, true, (AnimationSections)i);
						}
					}

					MoveAbsolute(movementDirection, movementSpeed);	
				}
			}
		}
	}
}

void NPC::UpdateNPCState(float dt)
{
	switch(m_eNPCState)
	{
	case eNPCState_Idle:
		{
		}
		break;
	case eNPCState_MovingToWayPoint:
		{
		}
		break;
	case eNPCState_Talking:
		{
			UpdateTalkMode(dt);
		}
		break;
	case eNPCState_Working:
		{
			UpdateWorking(dt);
		}
		break;
	case eNPCState_Combat:
		{
			UpdateCombat(dt);
		}
		break;
	}
}

void NPC::UpdatePhysics(float dt)
{
	vec3 acceleration;
	if (IsFrontEndNPC() == false && IsCreditsNPC() == false) // Don't do gravity in front-end or credits
	{
		acceleration = (m_gravityDirection * 9.81f) * 5.0f;
	}

	// Integrate velocity
	m_velocity += acceleration * dt;

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

	m_previousPosition = GetCenter();
}

void NPC::Update(float dt)
{
	// Update grid position
	UpdateGridPosition();

	// Update normal movement
	UpdateMovement(dt);

	// Update look at point
	UpdateLookingAndForwardTarget(dt);

	// Update NPC state
	UpdateNPCState(dt);

	// Update aggro distance check for enemies to attack
	if(m_updateAggroRadiusTimer <= 0.0f)
	{
		UpdateAggroRadius(dt);
		m_updateAggroRadiusTimer = 1.0f + GetRandomNumber(0, 100, 2) * 0.01f;
	}	

	// Check for enemy attack damage
	CheckEnemyDamageRadius();

	// Update timers
	UpdateTimers(dt);

	if(m_pVoxelCharacter != NULL)
	{
		m_pVoxelCharacter->Update(dt, m_animationSpeed);
		m_pVoxelCharacter->SetWeaponTrailsOriginMatrix(dt, m_worldMatrix);

		for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
		{
			m_animationFinished[i] = m_pVoxelCharacter->HasAnimationFinished((AnimationSections)i);
		}
	}

	// Update physics
	UpdatePhysics(dt);
}

void NPC::UpdateScreenCoordinates2d(Camera* pCamera)
{
	// Get projection co-ordinates
	m_pRenderer->PushMatrix();
		m_pRenderer->SetProjectionMode(PM_PERSPECTIVE, VoxGame::GetInstance()->GetDefaultViewport());
		pCamera->Look();

		int winx, winy;
		m_pRenderer->GetScreenCoordinatesFromWorldPosition(GetCenter(), &winx, &winy);

		m_screenPosition.x = (float)winx;
		m_screenPosition.y = (float)winy;
	m_pRenderer->PopMatrix();
}

void NPC::UpdateSubSelectionNamePicking(int pickingId, bool mousePressed)
{
	if(m_pVoxelCharacter != NULL)
	{
		if(mousePressed)
		{
			m_subSelectionSelected = m_pVoxelCharacter->GetSubSelectionName(pickingId);
			VoxGame::GetInstance()->GetFrontendManager()->SetCharacterSubSelection(m_subSelectionSelected);
		}
		else
		{
			m_subSelectionHover = m_pVoxelCharacter->GetSubSelectionName(pickingId);
		}
	}	
}

void NPC::UpdateAggroRadius(float dt)
{
	if(m_pTargetEnemy != NULL)
	{
		return;
	}

	Enemy* pClosestEnemy = NULL;
	float closestDistance = 999.9f;

	int numEnemies = m_pEnemyManager->GetNumEnemies();
	for(int i = 0; i < numEnemies; i++)
	{
		Enemy* pEnemy = m_pEnemyManager->GetEnemy(i);

		if(pEnemy->GetErase())
		{
			continue;
		}

		float distance = length(GetCenter() - pEnemy->GetCenter());
		if(distance < m_aggroRadius)
		{
			if(distance < closestDistance)
			{
				closestDistance = distance;
				pClosestEnemy = pEnemy;
			}
		}
	}

	if(pClosestEnemy != NULL)
	{
		SetTargetEnemy(pClosestEnemy);
	}
}

void NPC::UpdateTimers(float dt)
{
	// Attack delay
	if(m_insideAttackRadius)
	{
		if(m_attackDelayTimer > 0.0f)
		{
			m_attackDelayTimer -= dt;
		}
	}

	// Jump timer
	if(m_jumpTimer >= 0.0f)
	{
		m_jumpTimer -= dt;
	}

	// Movement wait after attack
	if(m_movementWaitAfterAttackTimer > 0.0f)
	{
		m_movementWaitAfterAttackTimer -= dt;
	}

	// Idle animation wait timer
	if(m_waitBeforeStopMovingAnimationTimer >= 0.0f)
	{
		m_waitBeforeStopMovingAnimationTimer -= dt;
	}

	// Update aggro radius check timer
	if(m_updateAggroRadiusTimer > 0.0f)
	{
		m_updateAggroRadiusTimer -= dt;
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

void NPC::Render(bool outline, bool reflection, bool silhouette)
{
	m_pRenderer->PushMatrix();
		m_pRenderer->MultiplyWorldMatrix(m_worldMatrix);

		Colour OulineColour(1.0f, 1.0f, 0.0f, 1.0f);
		if(m_hoverRender && m_outlineRender == false)
			OulineColour = Colour(1.0f, 0.0f, 1.0f, 1.0f);

		m_pVoxelCharacter->Render(outline, reflection, silhouette, OulineColour, false);
		m_pVoxelCharacter->RenderWeapons(outline, reflection, silhouette, OulineColour);
	m_pRenderer->PopMatrix();
}

void NPC::RenderFace()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->MultiplyWorldMatrix(m_worldMatrix);

		m_pVoxelCharacter->RenderFace();
	m_pRenderer->PopMatrix();
}

void NPC::RenderWeaponTrails()
{
	m_pRenderer->PushMatrix();
		m_pVoxelCharacter->RenderWeaponTrails();
	m_pRenderer->PopMatrix();
}

void NPC::RenderDebug()
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

	if(m_pVoxelCharacter != NULL)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->MultiplyWorldMatrix(m_worldMatrix);
			//m_pRenderer->TranslateWorldMatrix(m_position.x, m_position.y, m_position.z);
			m_pVoxelCharacter->RenderFacingDebug();
		m_pRenderer->PopMatrix();
	}

	// Bones
	if(m_pVoxelCharacter != NULL)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->MultiplyWorldMatrix(m_worldMatrix);

			m_pVoxelCharacter->RenderBones();
		m_pRenderer->PopMatrix();
	}

	// Forwards debug
	RenderForwardDebug();

	// Waypoints
	RenderWaypointsDebug();

	// Projectile hitbox
	RenderProjectileHitboxDebug();

	// Aggro radius
	RenderAggroRadiusDebug();

	// Movement position
	RenderMovementPositionDebug();
}

void NPC::RenderForwardDebug()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->TranslateWorldMatrix(GetCenter().x, GetCenter().y, GetCenter().z);

		m_pRenderer->ScaleWorldMatrix(m_pVoxelCharacter->GetCharacterScale(), m_pVoxelCharacter->GetCharacterScale(), m_pVoxelCharacter->GetCharacterScale());

		m_pRenderer->SetRenderMode(RM_SOLID);
		m_pRenderer->SetLineWidth(3.0f);
		m_pRenderer->EnableImmediateMode(IM_LINES);
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(m_forward.x*15.0f, m_forward.y*15.0f, m_forward.z*15.0f);

			m_pRenderer->ImmediateColourAlpha(0.0f, 1.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(m_targetForward.x*15.0f, m_targetForward.y*15.0f, m_targetForward.z*15.0f);					
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();
}

void NPC::RenderWaypointsDebug()
{
	m_pRenderer->PushMatrix();
		if(m_eNPCState == eNPCState_MovingToWayPoint)
		{
			m_pRenderer->SetRenderMode(RM_SOLID);
			m_pRenderer->SetLineWidth(1.0f);
			m_pRenderer->EnableImmediateMode(IM_LINES);
				m_pRenderer->ImmediateColourAlpha(1.0f, 0.0f, 0.0f, 1.0f);
				m_pRenderer->ImmediateVertex(GetCenter().x, GetCenter().y+0.15f, GetCenter().z);
				m_pRenderer->ImmediateVertex(m_targetPosition.x, m_targetPosition.y+0.65f, m_targetPosition.z);
			m_pRenderer->DisableImmediateMode();

			m_pRenderer->SetLineWidth(3.0f);
				m_pRenderer->EnableImmediateMode(IM_LINES);
				m_pRenderer->ImmediateColourAlpha(0.0f, 1.0f, 1.0f, 1.0f);
				m_pRenderer->ImmediateVertex(m_targetPosition.x, m_targetPosition.y+0.1f, m_targetPosition.z);
				m_pRenderer->ImmediateVertex(m_targetPosition.x, m_targetPosition.y+0.8f, m_targetPosition.z);			
			m_pRenderer->DisableImmediateMode();
		}

		for (WayPoint* pWaypoint : m_vpWayPointList)
		{
			const float l_length = pWaypoint->m_xLength;
			const float l_height = pWaypoint->m_yLength;
			const float l_width = pWaypoint->m_zLength;

			m_pRenderer->PushMatrix();
				m_pRenderer->TranslateWorldMatrix(pWaypoint->m_position.x, pWaypoint->m_position.y, pWaypoint->m_position.z);

				m_pRenderer->SetRenderMode(RM_WIREFRAME);
				m_pRenderer->SetCullMode(CM_NOCULL);
				m_pRenderer->SetLineWidth(1.0f);
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
			m_pRenderer->PopMatrix();

			// Connecting waypoints
			if(pWaypoint->m_pNextWaypoint != NULL)
			{
				m_pRenderer->SetRenderMode(RM_SOLID);
				m_pRenderer->SetLineWidth(1.0f);
				m_pRenderer->EnableImmediateMode(IM_LINES);
					m_pRenderer->ImmediateColourAlpha(0.75f, 1.0f, 0.5f, 1.0f);
					m_pRenderer->ImmediateVertex(pWaypoint->m_position.x, pWaypoint->m_position.y, pWaypoint->m_position.z);
					m_pRenderer->ImmediateVertex(pWaypoint->m_pNextWaypoint->m_position.x, pWaypoint->m_pNextWaypoint->m_position.y, pWaypoint->m_pNextWaypoint->m_position.z);
				m_pRenderer->DisableImmediateMode();
			}
		}
		m_pRenderer->SetCullMode(CM_BACK);
	m_pRenderer->PopMatrix();
}

void NPC::RenderSubSelection(bool outline, bool silhouette)
{
	m_pRenderer->PushMatrix();
		m_pRenderer->MultiplyWorldMatrix(m_worldMatrix);

		if(m_subSelectionSelected.empty())
		{
			Colour OulineColour(1.0f, 1.0f, 0.0f, 1.0f);
			m_pVoxelCharacter->RenderSubSelection(m_subSelectionSelected, outline, silhouette, OulineColour);
		}

		if(m_subSelectionHover.empty() && m_subSelectionHover != m_subSelectionSelected)
		{
			Colour OulineColour(1.0f, 0.0f, 1.0f, 1.0f);
			m_pVoxelCharacter->RenderSubSelection(m_subSelectionHover, outline, silhouette, OulineColour);
		}
	m_pRenderer->PopMatrix();
}

void NPC::RenderSubSelectionNormal()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->MultiplyWorldMatrix(m_worldMatrix);

		Colour OulineColour(1.0f, 1.0f, 0.0f, 1.0f);

		if(m_subSelectionSelected != "Head" && m_subSelectionHover != "Head")
			m_pVoxelCharacter->RenderSubSelection("Head", false, false, OulineColour);

		if(m_subSelectionSelected != "Body" && m_subSelectionHover != "Body")
			m_pVoxelCharacter->RenderSubSelection("Body", false, false, OulineColour);

		if(m_subSelectionSelected != "Legs" && m_subSelectionHover != "Legs")
			m_pVoxelCharacter->RenderSubSelection("Legs", false, false, OulineColour);

		if(m_subSelectionSelected != "Right_Shoulder" && m_subSelectionHover != "Right_Shoulder")
			m_pVoxelCharacter->RenderSubSelection("Right_Shoulder", false, false, OulineColour);

		if(m_subSelectionSelected != "Left_Shoulder" && m_subSelectionHover != "Left_Shoulder")
			m_pVoxelCharacter->RenderSubSelection("Left_Shoulder", false, false, OulineColour);

		if(m_subSelectionSelected != "Right_Hand" && m_subSelectionHover != "Right_Hand")
			m_pVoxelCharacter->RenderSubSelection("Right_Hand", false, false, OulineColour);

		if(m_subSelectionSelected != "Left_Hand" && m_subSelectionHover != "Left_Hand")
			m_pVoxelCharacter->RenderSubSelection("Left_Hand", false, false, OulineColour);

		if(m_subSelectionSelected != "Right_Foot" && m_subSelectionHover != "Right_Foot")
			m_pVoxelCharacter->RenderSubSelection("Right_Foot", false, false, OulineColour);

		if(m_subSelectionSelected != "Left_Foot" && m_subSelectionHover != "Left_Foot")
			m_pVoxelCharacter->RenderSubSelection("Left_Foot", false, false, OulineColour);

		m_pVoxelCharacter->RenderWeapons(false, false, false, OulineColour);
	m_pRenderer->PopMatrix();
}

void NPC::RenderSubSelectionNamePicking()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->MultiplyWorldMatrix(m_worldMatrix);

		Colour OulineColour(1.0f, 1.0f, 0.0f, 1.0f);

		m_pVoxelCharacter->Render(false, false, false, OulineColour, true);
	m_pRenderer->PopMatrix();
}

void NPC::RenderProjectileHitboxDebug()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->TranslateWorldMatrix(GetProjectileHitboxCenter().x, GetProjectileHitboxCenter().y, GetProjectileHitboxCenter().z);

		m_pRenderer->SetLineWidth(1.0f);
		m_pRenderer->SetRenderMode(RM_WIREFRAME);
		m_pRenderer->ImmediateColourAlpha(0.0f, 1.0f, 1.0f, 1.0f);

		if(m_eProjectileHitboxType == eProjectileHitboxType_Sphere)
		{
			m_pRenderer->DrawSphere(m_radius, 20, 20);
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

void NPC::RenderAggroRadiusDebug()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->TranslateWorldMatrix(GetCenter().x, GetCenter().y, GetCenter().z);
		m_pRenderer->SetLineWidth(1.0f);
		m_pRenderer->SetRenderMode(RM_WIREFRAME);
		m_pRenderer->ImmediateColourAlpha(0.0f, 0.0f, 1.0f, 1.0f);

		m_pRenderer->DrawLineCircle(m_aggroRadius, 20);
	m_pRenderer->PopMatrix();
}

void NPC::RenderMovementPositionDebug()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->TranslateWorldMatrix(m_moveBackToPosition.x, m_moveBackToPosition.y, m_moveBackToPosition.z);
		m_pRenderer->SetLineWidth(1.0f);
		m_pRenderer->SetRenderMode(RM_WIREFRAME);
		m_pRenderer->ImmediateColourAlpha(0.0f, 1.0f, 1.0f, 1.0f);

		float l_length = 0.5f;

		m_pRenderer->SetLineWidth(2.0f);
		m_pRenderer->EnableImmediateMode(IM_LINES);
			m_pRenderer->ImmediateVertex(-l_length, 0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(l_length, 0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(0.0f, -l_length, 0.0f);
			m_pRenderer->ImmediateVertex(0.0f, l_length, 0.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, -l_length);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, l_length);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();
}

// Callback functions
void NPC::_AttackEnabledTimerFinished(void *apData)
{
	NPC* lpNPC = (NPC*)apData;
	lpNPC->AttackEnabledTimerFinished();
}

void NPC::AttackEnabledTimerFinished()
{
	m_attackEnabled = false;
}

void NPC::_AttackEnabledDelayTimerFinished(void *apData)
{
	NPC* lpNPC = (NPC*)apData;
	lpNPC->AttackEnabledDelayTimerFinished();
}

void NPC::AttackEnabledDelayTimerFinished()
{
	if(m_erase)
	{
		return;
	}

	if(m_pTargetEnemy == NULL)
	{
		return;
	}

	if(m_eNPCCombatType == eNPCCombatType_Staff)
	{
		vec3 fireballSpawnPosition = GetCenter() + (m_forward*0.75f) + (GetRightVector()*-0.4f) + (GetUpVector()*0.25f);

		float powerAmount = 25.0f;
		vec3 toTarget = m_pTargetEnemy->GetCenter() - GetCenter();

		vec3 fireballSpawnVelocity = (normalize(toTarget) * powerAmount);

		Projectile* pProjectile = m_pProjectileManager->CreateProjectile(fireballSpawnPosition, fireballSpawnVelocity, 0.0f, "media/gamedata/items/Fireball/Fireball.item", 0.04f);	
		pProjectile->SetProjectileType(true, false, false);
		pProjectile->SetOwner(NULL, this, NULL);
		pProjectile->SetGravityMultiplier(0.0f);
	}
	else if(m_eNPCCombatType == eNPCCombatType_FireballHands)
	{
		float powerAmount = 25.0f;
		float cameraMultiplier = 25.0f;

		vec3 spellSpawnPosition = GetCenter() + (m_forward*0.5f) + (GetUpVector()*0.0f);

		if(m_attackCounter == 1)
		{
			spellSpawnPosition += -(GetRightVector()*0.4f);
		}
		else
		{
			spellSpawnPosition += (GetRightVector()*0.4f);
		}

		vec3 toTarget = m_pTargetEnemy->GetProjectileHitboxCenter() - GetCenter();
		vec3 spellSpawnVelocity = (normalize(toTarget) * powerAmount);

		Projectile* pProjectile = m_pProjectileManager->CreateProjectile(spellSpawnPosition, spellSpawnVelocity, 0.0f, "media/gamedata/items/Fireball/FireballBlue.item", 0.05f);	
		pProjectile->SetProjectileType(true, false, false);
		pProjectile->SetOwner(NULL, this, NULL);
		pProjectile->SetGravityMultiplier(0.0f);
	}
}
