// ******************************************************************************
// Filename:    NPC.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//   An NPC is like a player, but not human controlled. Can be set to have
//   different behaviours and different sub-types of NPCs, interacts with
//   the player and also attacks enemies.
//
// Revision History:
//   Initial Revision - 27/09/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include <string>

#include "../models/modelloader.h"

#include "../Inventory/InventoryManager.h"
#include "../blocks/ChunkManager.h"
#include "../Projectile/ProjectileManager.h"

class LightingManager;
class BlockParticleManager;
class TextEffectsManager;
class ItemManager;
class Enemy;
class EnemyManager;


enum eNPCState
{
	eNPCState_Idle = 0,	
	eNPCState_MovingToWayPoint,
	eNPCState_Talking,	
	eNPCState_Working,
	eNPCState_Combat,
};

enum eNPCCombatType
{
	eNPCCombatType_None = 0,
	eNPCCombatType_MeleeSword,
	eNPCCombatType_Archer,
	eNPCCombatType_Staff,
	eNPCCombatType_FireballHands,
};

class WayPoint
{
public:
	vec3 m_position;
	float m_xLength;
	float m_yLength;
	float m_zLength;
	WayPoint* m_pNextWaypoint;
};

typedef std::vector<WayPoint*> WayPointList;

class NPC
{
public:
	/* Public methods */
	NPC(Renderer* pRenderer, ChunkManager* pChunkManager, Player* pPlayer, LightingManager* pLightingManager, BlockParticleManager* pBlockParticleManager, TextEffectsManager* pTextEffectsManager, ItemManager* pItemManager, ProjectileManager* pProjectileManager, EnemyManager* pEnemyManager, QubicleBinaryManager* pQubicleBinaryManager, string name, string typeName, string modelName, bool characterSelectScreen, bool useQubicleManager);
	~NPC();

	void SetLightingManager(LightingManager* pLightingManager);

	void SetErase(bool erase);
	bool NeedsErasing();

	// Accessors / Setters
	VoxelCharacter* GetVoxelCharacter();
	void SetName(string name);
	string GetName() const;
	void SetType(string typeName);
	string GetType() const;
	void SetModelname(string modelName);
	string GetModelName() const;
	void SetPosition(vec3 pos);
	vec3 GetPosition() const;
	void SetRotation(float rot);
	float GetRotation() const;
	void SetScale(float scale);
	float GetRadius() const;
	void UpdateRadius();
	vec3 GetCenter() const;
	vec3 GetForwardVector() const;
	vec3 GetRightVector() const;
	vec3 GetUpVector() const;

	// Front-end NPCs
	void SetFrontEndNPC(bool frontEnd);
	bool IsFrontEndNPC() const;
	void SetPlayerClass(PlayerClass selectedClass);
	PlayerClass GetPlayerClass();

	// Combat type
	void SetNPCCombatType(eNPCCombatType eNPCCombatType, bool setWeaponModel);

	// NPC setup
	void LoadWeapon(bool left, string weaponFile);
	void UnloadWeapon(bool left);

	// Equipping items
	void ImportEquippedItems(string inventoryFile);
	void EquipItem(EquipSlot equipSlot, const char* itemFilename, bool left, bool right);
	void UnequipItem(EquipSlot equipSlot, bool left, bool right);

	// Animation
	void SetAnimationSpeed(float speed, bool onlySetOnCompleteAnimation, AnimationSections section);
	float GetAnimationSpeed(AnimationSections section);
	bool AnimationFinished(AnimationSections section);

	// Skeleton bone scale
	void SetSkeletonBoneScale(float scale);
	float GetSkeletonBoneScale();

	// NPC State
	eNPCState GetState() const;

	// Movement gameplay
	void SetMoveToPlayer(bool moveToPlayer);
	void SetLookAtPositionWhenReachedTarget(bool enabled, vec3 lookAtPosition);

	// Dead
	bool IsDead() const;

	// Projectile hitbox
	eProjectileHitboxType GetProjectileHitboxType();
	float GetProjectileHitboxRadius() const;
	float GetProjectileHitboxXLength() const;
	float GetProjectileHitboxYLength() const;
	float GetProjectileHitboxZLength() const;
	vec3 GetProjectileHitboxCenter() const;

	// Movement
	void MoveAbsolute(vec3 direction, const float speed, bool shouldChangeForward = true);
	void StopMoving();
	void Jump();
	void SetTargetPosition(vec3 pos);
	WayPoint* AddWaypoint(vec3 pos, float xLength, float yLength, float zLength);
	void ClearWaypoints();
	void StartWaypointNavigation(int waypointIndex);
	bool IsInsideWaypoint(int waypointIndex);
	void SetMoveBackToPosition(vec3 pos);
	void SetForwards(vec3 dir);
	void SetTargetForwards(vec3 dir);
	bool HasReachedTargetPosition();

	// Combat
	void DoDamage(float amount, Colour textColour, vec3 knockbackDirection, float knockbackAmount, bool createParticleHit);
	void CheckEnemyDamageRadius();
	void CheckProjectileDamageRadius(Projectile* pProjectile);
	void Explode();
	void Respawn();

	// Attacking
	void Attack();
	void ReleaseAttack();
	bool CanAttack() const;
	bool GetAttackEnabled() const;
	float GetAttackRadius() const;
	void SetAttackRadius(float attackRadius);
	float GetAttackRotation() const;
	float GetAttackSegmentAngle() const;
	void SetEnemyDied(Enemy* pEnemy);
	void SetTargetEnemy(Enemy* pEnemy);

	// Looking
	void SetTargetForwardToLookAtPoint(vec3 point);
	void LookAtPoint(vec3 point);
	void SetRandomLookMode();
	void SetBodyTurnStopThreshold(float threshold);
	void SetBodyTurnSpeedMultiplier(float multiplier);
	bool IsLookAtPointMode() const;
	bool IsLookingAtPoint() const;

	// Talking
	void SetNPCTalkMode(float talkDelay, float talkTime);
	void StopNPCTalkMode();

	// Working
	void SetWorking(vec3 workPointOfInterest);
	void StopWorking();

	// World
	void UpdateGridPosition();
	Chunk* GetCachedGridChunkOrFromPosition(vec3 pos) const;
	void ClearChunkCacheForChunk(Chunk* pChunk);

	// Collision
	bool CheckCollisions(vec3 positionCheck, vec3 previousPosition, vec3 *pNormal, vec3 *pMovement) const;
	bool IsBlockInFront() const;
	bool IsPushingCollisionEnabled() const;
	void SetPushingCollisionEnabled(bool pushEnabled);

	void SetCreditsNPC(bool creditsNPC);
	bool IsCreditsNPC() const;

	// Rendering Helpers
	void CalculateWorldTransformMatrix();
	void SetOutlineRender(bool outline);
	bool GetOutlineRender() const;
	void SetHoverRender(bool hover);
	bool GetHoverRender() const;
	void SetWireFrameRender(bool wireframe);
	void SetSubSelectionRender(bool subSelection);
	bool GetSubSelectionRender() const;
	vec2 GetScreenPosition() const;

	// Updating
	void UpdateWeaponLights(float dt);
	void UpdateWeaponParticleEffects(float dt);
	void UpdateLookingAndForwardTarget(float dt);
	void UpdateTalkMode(float dt);
	void UpdateWorking(float dt);
	void UpdateCombat(float dt);
	void UpdateMeleeCombat(float dt);
	void UpdateRangedCombat(float dt);
	void UpdateMovement(float dt);
	void UpdateNPCState(float dt);
	void UpdatePhysics(float dt);
	void Update(float dt);
	void UpdateScreenCoordinates2d(Camera* pCamera);
	void UpdateSubSelectionNamePicking(int pickingId, bool mousePressed);
	void UpdateAggroRadius(float dt);
	void UpdateTimers(float dt);

	// Rendering
	void Render(bool outline, bool reflection, bool silhouette);
	void RenderFace();
	void RenderWeaponTrails();
	void RenderDebug();
	void RenderForwardDebug();
	void RenderWaypointsDebug();
	void RenderSubSelection(bool outline, bool silhouette);
	void RenderSubSelectionNormal();
	void RenderSubSelectionNamePicking();
	void RenderProjectileHitboxDebug();
	void RenderAggroRadiusDebug();
	void RenderMovementPositionDebug();

protected:
	/* Protected methods */
	static void _AttackEnabledTimerFinished(void *apData);
	void AttackEnabledTimerFinished();

	static void _AttackEnabledDelayTimerFinished(void *apData);
	void AttackEnabledDelayTimerFinished();

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	Renderer* m_pRenderer;
	ChunkManager* m_pChunkManager;
	LightingManager* m_pLightingManager;
	Player* m_pPlayer;
	BlockParticleManager* m_pBlockParticleManager;
	TextEffectsManager* m_pTextEffectsManager;
	ItemManager* m_pItemManager;
	QubicleBinaryManager* m_pQubicleBinaryManager;
	ProjectileManager* m_pProjectileManager;
	EnemyManager* m_pEnemyManager;

	// Erase flag
	bool m_erase;

	// NPC name
	string m_name;
	string m_type;
	string m_modelName;

	// Player radius
	float m_radius;

	// Forward vector
	vec3 m_forward;

	// Target forward / looking vector
	vec3 m_targetForward;

	// Player position and movement variables
	vec3 m_position;
	vec3 m_previousPosition;
	vec3 m_velocity;
	vec3 m_gravityDirection;

	// NPC's world matrix
	Matrix4x4 m_worldMatrix;

	// NPC State
	eNPCState m_eNPCState;
	eNPCState m_ePreviousNPCState;

	// Combat type
	eNPCCombatType m_eNPCCombatType;

	// Frontend NPC flag
	bool m_frontEndNPC;

	// What class are we for the front-end character lineup
	PlayerClass m_selectedClass;

	// Idle flag
	bool m_bIsIdle;
	float m_waitBeforeStopMovingAnimationTimer;
	bool m_hasSetIdleAnimation;
	
	// Movement gameplay params
	bool m_moveToPlayer;
	vec3 m_moveBackToPosition;
	vec3 m_lookAtPositionWhenReachedTargetPosition;
	bool m_lookAtPositionWhenReachTarget;
	bool m_hasReachedTargetPosition;

	// Projectile hitbox
	eProjectileHitboxType m_eProjectileHitboxType;
	float m_projectileHitboxRadius;
	float m_projectileHitboxXLength;
	float m_projectileHitboxYLength;
	float m_projectileHitboxZLength;
	vec3 m_projectileHitboxCenterOffset;

	// Movement
	vec3 m_targetPosition;
	WayPointList m_vpWayPointList;
	int m_currentWaypointIndex;

	// Movement params
	float m_movementSpeed;
	float m_maxMovementSpeed;
	float m_minMovementSpeed;

	// Jumping params
	bool m_bCanJump;
	float m_jumpTime;
	float m_jumpTimer;
	float m_jumpHeight;

	// Damage and knockback timers
	float m_damageTime;
	float m_damageTimer;
	float m_knockbackTime;
	float m_knockbackTimer;

	// Game properties
	float m_maxHealth;
	float m_health;
	float m_maxMagic;
	float m_magic;

	// Combat
	bool m_bCanAttack;
	float m_attackDelayTime;
	float m_attackDelayTimer;
	bool m_bCanInteruptCombatAnim;
	bool m_attackEnabled;
	bool m_insideAttackRadius;
	float m_attackEnabledTimer;
	float m_attackEnabledDelayTimer;
	float m_attackSegmentAngle;
	float m_attackRadius;
	float m_attackRotation;
	float m_movementWaitAfterAttackTimer;
	float m_aggroRadius;
	float m_updateAggroRadiusTimer;
	int m_attackCounter;

	// Charging attacks
	bool m_bIsChargingAttack;
	float m_chargeAmount;
	float m_chargeTime;
	vec3 m_chargeSpawnVelocity;
	vec3 m_chargeSpawnPosition;

	// Look at point
	bool m_bLookAtPoint;
	vec3 m_lookPoint;
	bool m_bIsLookingAtPoint;

	// Turn speed multiplier
	float m_bodyTurnSpeedMultiplier;

	// Turn stop threshold
	float m_bodyTurnStopThreshold;

	// Talking
	bool m_talkMode;
	float m_talkDelay;
	float m_talkTime;
	float m_talkTimer;

	// Working
	bool m_isWorking;
	float m_workingAnimationWaitTimer;
	float m_workingAnimationDelay;
	int m_workingRepetitionBeforeLongDelay;
	int m_workingRepetitionCounter;
	vec3 m_lookAtWorkPoint;
	bool m_createdAnvilHitParticleEffect;

	// Push collision enabled
	bool m_bPushingCollisionEnabled;

	// Is this NPC a credits NPC, if so, behave differently
	bool m_isCreditsNPC;

	// Grid position
	int m_gridPositionX;
	int m_gridPositionY;
	int m_gridPositionZ;

	// Cached chunk from grid position
	Chunk* m_pCachedGridChunk;

	// Rendering modes
	bool m_outlineRender;
	bool m_hoverRender;
	bool m_subSelectionRender;

	// 2d screen position, for billboard rendering
	vec2 m_screenPosition;

	// Sub selection of body parts - During character creation screen
	string m_subSelectionSelected;
	string m_subSelectionHover;

	// Animation params
	float m_animationSpeed[AnimationSections_NUMSECTIONS];
	bool m_animationSpeedonlySetOnCompleteAnimation[AnimationSections_NUMSECTIONS];
	bool m_animationFinished[AnimationSections_NUMSECTIONS];
	float m_animationTimer;

	// Target enemy
	Enemy* m_pTargetEnemy;

	// Voxel character
	VoxelCharacter* m_pVoxelCharacter;
	QubicleBinary* m_pCharacterBackup;
};
