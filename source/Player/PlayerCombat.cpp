// ******************************************************************************
// Filename:    PlayerCombat.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 27/02/16
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "Player.h"
#include "../utils/Random.h"
#include "../utils/Interpolator.h"
#include "../Projectile/ProjectileManager.h"
#include "../Projectile/Projectile.h"
#include "../VoxGame.h"

// Combat
void Player::PressAttack()
{
	if (IsDead())
	{
		return;
	}

	if (IsBow())
	{
		if (m_bowAttackDelay <= 0.0f)
		{
			if (m_bIsChargingAttack == false)
			{
				m_bIsChargingAttack = true;
				m_chargeAmount = 0.0f;

				LoadWeapon(false, "media/gamedata/items/Arrow/ArrowHold.item");

				if (VoxGame::GetInstance()->GetCameraMode() == CameraMode_FirstPerson)
				{
					// Make sure we set arrow alpha if we are in first person mode
					m_pVoxelCharacter->SetMeshAlpha(0.45f, true);
				}

				if (m_bIsIdle)
				{
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "BowCharge", 0.2f);
				}
				else
				{
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Head_Body, false, AnimationSections_Head_Body, "BowCharge", 0.2f);
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "BowCharge", 0.2f);
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "BowCharge", 0.2f);
				}

				VoxGame::GetInstance()->PlaySoundEffect(eSoundEffect_BowDraw);
			}
		}
	}
	else if (IsBoomerang())
	{
		if (CanAttackRight())
		{
			if (m_bCanThrowWeapon)
			{
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_FullBody, "SwordAttack1", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "SwordAttack1", 0.01f);

				Interpolator::GetInstance()->AddFloatInterpolation(&m_animationTimer, 0.0f, 0.22f, 0.22f, 0.0f, nullptr, _AttackAnimationTimerFinished, this);

				m_bCanAttackRight = false;
				m_bCanThrowWeapon = false;

				// Start weapon trails
				if (m_pVoxelCharacter->GetRightWeapon())
				{
					if (m_pVoxelCharacter->IsRightWeaponLoaded())
					{
						m_pVoxelCharacter->GetRightWeapon()->StartWeaponTrails();
					}
				}
			}
		}
	}
	else if (IsStaff())
	{
		if (m_magic >= 10.0f)
		{
			if (CanAttackRight())
			{
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "StaffAttack", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "StaffAttack", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Head_Body, false, AnimationSections_Head_Body, "StaffAttack", 0.01f);

				m_bCanInteruptCombatAnim = false;

				Interpolator::GetInstance()->AddFloatInterpolation(&m_animationTimer, 0.0f, 0.2f, 0.2f, 0.0f, nullptr, _AttackAnimationTimerFinished, this);

				m_magic -= 10.0f;
				VoxGame::GetInstance()->GetHUD()->UpdatePlayerData();

				m_bCanAttackRight = false;
			}

			VoxGame::GetInstance()->PlaySoundEffect(eSoundEffect_FireballCast, 0.5f);
		}
	}
	else if (IsWand())
	{
	}
	else if (IsBomb())
	{
		if (CanAttackRight())
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_FullBody, "SwordAttack2", 0.01f);
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "SwordAttack2", 0.01f);

			Interpolator::GetInstance()->AddFloatInterpolation(&m_animationTimer, 0.0f, 0.25f, 0.25f, 0.0f, nullptr, _AttackAnimationTimerFinished, this);

			m_bCanAttackRight = false;
		}
	}
	else if (IsConsumable())
	{
	}
	else if (IsDagger())
	{
		InventoryItem* pRightHand = m_pInventoryManager->GetInventoryItemForEquipSlot(EquipSlot_RightHand);
		InventoryItem* pLeftHand = m_pInventoryManager->GetInventoryItemForEquipSlot(EquipSlot_LeftHand);

		if (pRightHand != nullptr && pRightHand->m_itemType == InventoryType_Weapon_Dagger && CanAttackRight())
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "SwordAttack2", 0.01f);

			Interpolator::GetInstance()->AddFloatInterpolation(&m_animationTimer, 0.0f, 0.3f, 0.3f, 0.0f, nullptr, _AttackAnimationTimerFinished, this);

			m_bCanInteruptCombatAnim = true;

			m_attackEnabledDelayTimer = 0.0f;
			m_attackSegmentAngle = 0.75f;
			float attackTime = 0.4f;
			float startRotation = -10.0f;
			float endRotation = -10.0f;
			float easingRotation = 0.0f;

			m_attackEnabled = true;
			m_attackEnabledTimer = 0.0f;
			m_attackRotation = startRotation;
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledTimer, 0.0f, attackTime, attackTime, 0.0f, nullptr, _AttackEnabledTimerFinished, this);
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledDelayTimer, m_attackEnabledDelayTimer, 0.0f, m_attackEnabledDelayTimer, 0.0f, nullptr, _AttackEnabledDelayTimerFinished, this);
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackRotation, startRotation, endRotation, attackTime, easingRotation);

			m_bCanAttackRight = false;
		}
		else if (pLeftHand != nullptr && pLeftHand->m_itemType == InventoryType_Weapon_Dagger && CanAttackLeft())
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "SwordAttack2", 0.01f);

			Interpolator::GetInstance()->AddFloatInterpolation(&m_animationTimer, 0.0f, 0.3f, 0.3f, 0.0f, nullptr, _AttackAnimationTimerFinished_Alternative, this);

			m_bCanInteruptCombatAnim = true;

			m_attackEnabledDelayTimer = 0.0f;
			m_attackSegmentAngle = 0.75f;
			float attackTime = 0.4f;
			float startRotation = -10.0f;
			float endRotation = -10.0f;
			float easingRotation = 0.0f;

			m_attackEnabled = true;
			m_attackEnabledTimer = 0.0f;
			m_attackRotation = startRotation;
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledTimer, 0.0f, attackTime, attackTime, 0.0f, nullptr, _AttackEnabledTimerFinished, this);
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledDelayTimer, m_attackEnabledDelayTimer, 0.0f, m_attackEnabledDelayTimer, 0.0f, nullptr, _AttackEnabledDelayTimerFinished, this);
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackRotation, startRotation, endRotation, attackTime, easingRotation);

			m_bCanAttackLeft = false;
		}
	}
	else if (IsHammer())
	{
	}
	else if (IsMace())
	{
	}
	else if (IsSickle())
	{
	}
	else if (IsPickaxe())
	{
		if (CanAttackRight())
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_FullBody, "Mine", 0.01f);
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "Mine", 0.01f);

			m_bCanInteruptCombatAnim = false;

			Interpolator::GetInstance()->AddFloatInterpolation(&m_animationTimer, 0.0f, 0.4f, 0.4f, 0.0f, nullptr, _AttackAnimationTimerFinished, this);

			m_bCanAttackRight = false;
		}
	}
	else if (IsAxe())
	{
	}
	else if (Is2HandedSword())
	{
		if (CanAttackRight())
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_FullBody, "2HandedSwordAttack", 0.01f);
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "2HandedSwordAttack", 0.01f);
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "2HandedSwordAttack", 0.01f);

			m_bCanInteruptCombatAnim = false;

			m_attackEnabledDelayTimer = 0.175f;
			m_attackSegmentAngle = 0.60f;
			float attackTime = 0.6f;
			float startRotation = 240.0f;
			float endRotation = -60.0f;
			float easingRotation = -50.0f;

			m_attackEnabled = true;
			m_attackEnabledTimer = 0.0f;
			m_attackRotation = startRotation;
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledTimer, 0.0f, attackTime, attackTime, 0.0f, nullptr, _AttackEnabledTimerFinished, this);
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledDelayTimer, m_attackEnabledDelayTimer, 0.0f, m_attackEnabledDelayTimer, 0.0f, nullptr, _AttackEnabledDelayTimerFinished, this);
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackRotation, startRotation, endRotation, attackTime, easingRotation);

			m_bCanAttackRight = false;
		}
	}
	else if (IsSword())
	{
		if (CanAttackRight())
		{
			float attackTime = 0.0f;
			float startRotation = 0.0f;
			float endRotation = 0.0f;
			float easingRotation = 0.0f;

			int randomAttack = GetRandomNumber(1, 3);
			if (randomAttack == 1)
			{
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_FullBody, "SwordAttack1", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Head_Body, false, AnimationSections_Head_Body, "SwordAttack1", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "SwordAttack1", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "SwordAttack1", 0.01f);

				m_bCanInteruptCombatAnim = false;

				m_attackEnabledDelayTimer = 0.1f;
				m_attackSegmentAngle = 0.75f;
				attackTime = 0.4f;
				startRotation = -20.0f;
				endRotation = -20.0f;
				easingRotation = 0.0f;
			}
			else if (randomAttack == 2)
			{
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_FullBody, "SwordAttack2", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Head_Body, false, AnimationSections_Head_Body, "SwordAttack2", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "SwordAttack2", 0.01f);
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "SwordAttack1", 0.01f);

				m_bCanInteruptCombatAnim = false;

				m_attackEnabledDelayTimer = 0.2f;
				m_attackSegmentAngle = 0.75f;
				attackTime = 0.4f;
				startRotation = -10.0f;
				endRotation = -10.0f;
				easingRotation = 0.0f;
			}
			else if (randomAttack == 3)
			{
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "SwordAttack3", 0.01f);

				m_bCanInteruptCombatAnim = false;

				m_attackEnabledDelayTimer = 0.0f;
				m_attackSegmentAngle = 0.60f;
				attackTime = 0.6f;
				startRotation = 360.0f;
				endRotation = -45.0f;
				easingRotation = -75.0f;
			}

			m_bCanAttackRight = false;

			m_attackEnabled = true;
			m_attackEnabledTimer = 0.0f;
			m_attackRotation = startRotation;
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledTimer, 0.0f, attackTime, attackTime, 0.0f, nullptr, _AttackEnabledTimerFinished, this);
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackEnabledDelayTimer, m_attackEnabledDelayTimer, 0.0f, m_attackEnabledDelayTimer, 0.0f, nullptr, _AttackEnabledDelayTimerFinished, this);
			Interpolator::GetInstance()->AddFloatInterpolation(&m_attackRotation, startRotation, endRotation, attackTime, easingRotation);
		}
	}
	else if (IsBlockPlacing())
	{
	}
	else if (IsItemPlacing())
	{
	}
	else if (IsSceneryPlacing())
	{
	}
	else if (IsSpellHands())
	{
		if (m_magic >= 5.0f)
		{
			InventoryItem* pRightHand = m_pInventoryManager->GetInventoryItemForEquipSlot(EquipSlot_RightHand);
			InventoryItem* pLeftHand = m_pInventoryManager->GetInventoryItemForEquipSlot(EquipSlot_LeftHand);

			if (pRightHand != nullptr && pRightHand->m_itemType == InventoryType_Weapon_SpellHands && CanAttackRight())
			{
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "HandSpellCastRight", 0.01f);

				Interpolator::GetInstance()->AddFloatInterpolation(&m_animationTimer, 0.0f, 0.3f, 0.3f, 0.0f, nullptr, _AttackAnimationTimerFinished, this);

				m_magic -= 5.0f;
				VoxGame::GetInstance()->GetHUD()->UpdatePlayerData();

				m_bCanAttackRight = false;
			}
			else if (pLeftHand != nullptr && pLeftHand->m_itemType == InventoryType_Weapon_SpellHands && CanAttackLeft())
			{
				m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "HandSpellCastLeft", 0.01f);

				Interpolator::GetInstance()->AddFloatInterpolation(&m_animationTimer, 0.0f, 0.3f, 0.3f, 0.0f, nullptr, _AttackAnimationTimerFinished_Alternative, this);

				m_magic -= 5.0f;
				VoxGame::GetInstance()->GetHUD()->UpdatePlayerData();

				m_bCanAttackLeft = false;
			}

			VoxGame::GetInstance()->PlaySoundEffect(eSoundEffect_FireballCast, 0.5f);
		}
	}
	else if (IsShield())
	{
	}
	else if (IsTorch())
	{
	}
}

void Player::ReleaseAttack()
{
	if (IsDead())
	{
		return;
	}

	if (IsBow())
	{
		if (m_bIsChargingAttack == true)
		{
			UnloadWeapon(false);

			Projectile* pProjectile = m_pProjectileManager->CreateProjectile(m_chargeSpawnPosition, m_chargeSpawnVelocity, 0.0f, "media/gamedata/items/Arrow/Arrow.item", 0.08f);
			pProjectile->SetProjectileType(true, false, false);
			pProjectile->SetOwner(this, nullptr, nullptr);

			m_bIsChargingAttack = false;
			m_chargeAmount = 0.0f;
			m_bowAttackDelay = 0.125f;

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

			VoxGame::GetInstance()->PlaySoundEffect(eSoundEffect_ArrowRelease);
		}
	}
}

bool Player::CanAttackLeft()
{
	return m_bCanAttackLeft;
}

bool Player::CanAttackRight()
{
	return m_bCanAttackRight;
}

bool Player::GetAttackEnabled()
{
	return m_attackEnabled && (m_attackEnabledDelayTimer <= 0.0f);
}

float Player::GetAttackRadius()
{
	return m_attackRadius;
}

float Player::GetAttackRotation()
{
	return m_attackRotation;
}

float Player::GetAttackSegmentAngle()
{
	return m_attackSegmentAngle;
}

void Player::CheckEnemyDamageRadius(Enemy* pEnemy)
{
	if (IsDead())
	{
		return;
	}

	if (pEnemy->GetAttackEnabled() == false)
	{
		// If attack is not enabled, just return
		return;
	}

	vec3 distance = GetCenter() - pEnemy->GetCenter();
	float lengthToEnemy = length(distance);
	if (lengthToEnemy <= m_radius + pEnemy->GetAttackRadius())
	{
		vec3 distance_minus_y = distance;
		distance_minus_y.y = 0.0f;
		vec3 direction = normalize(distance_minus_y);

		// Figure out the attack vector, based on the attack rotation
		float enemyRotation = pEnemy->GetRotation();
		float attackRotation = pEnemy->GetAttackRotation();
		float angle = DegToRad(enemyRotation + attackRotation);
		vec3 attackDirection = vec3(sin(angle), 0.0f, cos(angle));
		float dotProduct = dot(direction, attackDirection);

		if (dotProduct > pEnemy->GetAttackSegmentAngle()) // Check if we are within the attack segment
		{
			vec3 knockbackDirection = (direction*2.0f) + vec3(0.0f, 1.0f, 0.0f);
			knockbackDirection = normalize(knockbackDirection);
			Colour damageColour = Colour(1.0f, 1.0f, 1.0f);

			float knockbackAmount = 16.0f;

			DoDamage(5.0f, damageColour, knockbackDirection, knockbackAmount, true);
		}
	}
}

void Player::CheckProjectileDamageRadius(Projectile* pProjectile)
{
	if (IsDead())
	{
		return;
	}

	if (pProjectile->CanAttackPlayer() == false && pProjectile->IsReturnToPlayer() == false)
	{
		return;
	}

	vec3 projectileHitboxCenter = GetProjectileHitboxCenter();

	vec3 toProjectile = projectileHitboxCenter - pProjectile->GetCenter();

	bool hitByProjectile = false;
	if (m_eProjectileHitboxType == eProjectileHitboxType_Sphere)
	{
		if (length(toProjectile) < GetRadius() + pProjectile->GetRadius())
		{
			hitByProjectile = true;
		}
	}
	else if (m_eProjectileHitboxType == eProjectileHitboxType_Cube)
	{
		Plane3D planes[6];
		Matrix4x4 rotationMatrix;
		rotationMatrix.SetYRotation(DegToRad(GetRotation()));
		planes[0] = Plane3D(rotationMatrix * vec3(-1.0f, 0.0f, 0.0f), projectileHitboxCenter + (rotationMatrix * vec3(m_projectileHitboxXLength, 0.0f, 0.0f)));
		planes[1] = Plane3D(rotationMatrix * vec3(1.0f, 0.0f, 0.0f), projectileHitboxCenter + (rotationMatrix * vec3(-m_projectileHitboxXLength, 0.0f, 0.0f)));
		planes[2] = Plane3D(rotationMatrix * vec3(0.0f, -1.0f, 0.0f), projectileHitboxCenter + (rotationMatrix * vec3(0.0f, m_projectileHitboxYLength, 0.0f)));
		planes[3] = Plane3D(rotationMatrix * vec3(0.0f, 1.0f, 0.0f), projectileHitboxCenter + (rotationMatrix * vec3(0.0f, -m_projectileHitboxYLength, 0.0f)));
		planes[4] = Plane3D(rotationMatrix * vec3(0.0f, 0.0f, -1.0f), projectileHitboxCenter + (rotationMatrix * vec3(0.0f, 0.0f, m_projectileHitboxZLength)));
		planes[5] = Plane3D(rotationMatrix * vec3(0.0f, 0.0f, 1.0f), projectileHitboxCenter + (rotationMatrix * vec3(0.0f, 0.0f, -m_projectileHitboxZLength)));

		float distance;
		int inside = 0;
		vec3 normal;

		for (int i = 0; i < 6; i++)
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

		if (inside == 6)
		{
			hitByProjectile = true;
		}
	}

	if (hitByProjectile)
	{
		if (pProjectile->IsReturnToPlayer())
		{
			if (pProjectile->CanCatch())
			{
				pProjectile->PlayerCatch();

				m_bCanThrowWeapon = true;

				if (IsBoomerang())
				{
					m_pVoxelCharacter->SetRenderRightWeapon(true);

					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, true, AnimationSections_FullBody, "SwordAttack1", 0.01f);
					m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "SwordAttack1", 0.01f);

					// Start weapon trails
					if (m_pVoxelCharacter->GetRightWeapon())
					{
						if (m_pVoxelCharacter->IsRightWeaponLoaded())
						{
							m_pVoxelCharacter->GetRightWeapon()->StartWeaponTrails();
						}
					}
				}
			}
		}
		else
		{
			vec3 knockbackDirection = (normalize(pProjectile->GetVelocity())*2.0f) + vec3(0.0f, 1.0f, 0.0f);
			knockbackDirection = normalize(knockbackDirection);
			Colour damageColour = Colour(1.0f, 1.0f, 1.0f);

			float knockbackAmount = 16.0f;
			DoDamage(15.0f, damageColour, knockbackDirection, knockbackAmount, false);

			pProjectile->Explode();
		}
	}
}

void Player::DoDamage(float amount, Colour textColour, vec3 knockbackDirection, float knockbackAmount, bool createParticleHit)
{
	if (IsDead())
	{
		return;
	}

	if (m_damageTimer <= 0.0f)
	{
		float healthBefore = m_health;
		float damageDone = healthBefore - m_health;

		m_health -= amount;

		// Figure out if we are dead yet
		if (m_health <= 0.0f)
		{
			m_health = 0.0f;

			// Explode the voxel model
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

		if (createParticleHit && m_health > 0.0f)
		{
			// Do a hit particle effect
			vec3 hitParticlePos = GetCenter() - (normalize(knockbackDirection) * m_radius);
			unsigned int effectId = -1;
			BlockParticleEffect* pBlockParticleEffect = VoxGame::GetInstance()->GetBlockParticleManager()->ImportParticleEffect("media/gamedata/particles/combat_hit.effect", hitParticlePos, &effectId);
			pBlockParticleEffect->PlayEffect();
		}

		// Set face to sad for a short while (until facial hit timer resets);
		m_pVoxelCharacter->PlayFacialExpression("Sad");
		m_hitFacialExpressionTimer = m_hitFacialExpressionTime;
		m_returnToNormalFacialExpressionAfterHit = false;

		m_damageTimer = m_damageTime;

		// Close the interaction GUI compoenents
		VoxGame::GetInstance()->CloseInteractionGUI();
	}

	if (IsDead())
	{
		return;
	}

	if (m_knockbackTimer <= 0.0f)
	{
		m_velocity += knockbackDirection * knockbackAmount;

		m_knockbackTimer = m_knockbackTime;
	}
}

void Player::Explode()
{
	if (IsDead())
	{
		return;
	}

	m_dead = true;

	m_bIsChargingAttack = false;
	m_chargeAmount = 0.0f;

	VoxGame::GetInstance()->GetHUD()->PlayerDied();
	VoxGame::GetInstance()->ReleaseEnemyTarget();
	VoxGame::GetInstance()->CloseAllGUIWindows();

	CalculateWorldTransformMatrix();

	// Explode the qubicle binary voxel file
	for (int explodeCounter = 0; explodeCounter < 3; explodeCounter++)
	{
		const QubicleBinary* pQubicleModel = nullptr;
		int spawnChance = 100;
		if (explodeCounter == 0)
		{
			pQubicleModel = m_pVoxelCharacter->GetQubicleModel();
			spawnChance = 50;
		}
		else if (explodeCounter == 1)
		{
			if (m_pVoxelCharacter->GetRightWeapon() != nullptr)
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
			if (m_pVoxelCharacter->GetLeftWeapon() != nullptr)
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

		if (pQubicleModel != nullptr)
		{
			m_pBlockParticleManager->ExplodeQubicleBinary(pQubicleModel, m_pVoxelCharacter->GetCharacterScale(), spawnChance);
		}
	}

	// Unload weapons
	UnloadWeapon(true);
	UnloadWeapon(false);

	// Create tombstone
	char tombstoneFilename[64];
	if (GetRandomNumber(0, 100) > 35)
	{
		sprintf(tombstoneFilename, "media/gamedata/items/Tombstone/Tombstone1.item");
	}
	else
	{
		sprintf(tombstoneFilename, "media/gamedata/items/Tombstone/Tombstone2.item");
	}
	Item* pTombstone = m_pItemManager->CreateItem(GetCenter(), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), tombstoneFilename, eItem_Tombstone, "Tombstone", false, false, 0.08f);
	pTombstone->SetVelocity(vec3(0.0f, 10.0f, 0.0f));
	pTombstone->SetRotation(vec3(0.0f, GetRandomNumber(0, 360, 1), 0.0f));

	// Create a ghost
	m_createGhost = true;
	m_createGhostTimer = 1.5f;
}

void Player::Respawn()
{
	if (IsDead() == false)
	{
		return;
	}

	m_position = m_respawnPosition;
	
	// Make sure we create a chunk in the respawn position
	UpdateGridPosition();
	m_pChunkManager->CreateNewChunk(GetGridX(), GetGridY(), GetGridZ());

	m_health = m_maxHealth;

	VoxGame::GetInstance()->GetHUD()->UpdatePlayerData();

	// Also go through all the equipped items and equip them
	for (int i = 0; i < EquipSlot_NumSlots; i++)
	{
		InventoryItem* pItem = m_pInventoryManager->GetInventoryItemForEquipSlot((EquipSlot)i);
		if (pItem != nullptr)
		{
			EquipItem(pItem, true);
		}
	}

	m_dead = false;
}

// Enemy target
void Player::SetEnemyDied(Enemy* pEnemy)
{
	if (m_pTargetEnemy == pEnemy)
	{
		SetTargetEnemy(nullptr);
	}
}

void Player::SetTargetEnemy(Enemy* pTargetEnemy)
{
	m_pTargetEnemy = pTargetEnemy;

	if (m_pTargetEnemy != nullptr)
	{
		vec3 toTarget = m_pTargetEnemy->GetCenter() - GetCenter();
		toTarget.y = 0.0f;
		m_forward = normalize(toTarget);

		CalculateWorldTransformMatrix();

		if (Is2HandedSword())
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "2HandedSwordPose", 0.2f);
		}
		else
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "TargetPose", 0.2f);
		}
	}
	else
	{
		SetRandomLookMode();

		if (IsStaff())
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "StaffPose", 0.15f);
		}
		else if (IsSpellHands())
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Right_Arm_Hand, false, AnimationSections_Right_Arm_Hand, "HandSpellCastPose", 0.15f);
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_Left_Arm_Hand, false, AnimationSections_Left_Arm_Hand, "HandSpellCastPose", 0.15f);
		}
		else if (m_bIsChargingAttack == false)
		{
			m_pVoxelCharacter->BlendIntoAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "BindPose", 0.2f);
		}
	}
}

Enemy* Player::GetTargetEnemy()
{
	return m_pTargetEnemy;
}

// Projectile hitbox
eProjectileHitboxType Player::GetProjectileHitboxType()
{
	return m_eProjectileHitboxType;
}

float Player::GetProjectileHitboxRadius()
{
	return m_projectileHitboxRadius;
}

float Player::GetProjectileHitboxXLength()
{
	return m_projectileHitboxXLength;
}

float Player::GetProjectileHitboxYLength()
{
	return m_projectileHitboxYLength;
}

float Player::GetProjectileHitboxZLength()
{
	return m_projectileHitboxZLength;
}

vec3 Player::GetProjectileHitboxCenter()
{
	return GetCenter() + m_projectileHitboxCenterOffset;
}

// World editing
void Player::DestroyBlock()
{
	if (m_blockSelection)
	{
		Chunk* pChunk = m_pChunkManager->GetChunkFromPosition(m_blockSelectionPos.x, m_blockSelectionPos.y, m_blockSelectionPos.z);

		if (pChunk != nullptr)
		{
			int blockX, blockY, blockZ;
			vec3 blockPos;
			bool active = m_pChunkManager->GetBlockActiveFrom3DPosition(m_blockSelectionPos.x, m_blockSelectionPos.y, m_blockSelectionPos.z, &blockPos, &blockX, &blockY, &blockZ, &pChunk);
			if (active)
			{
				float r;
				float g;
				float b;
				float a;
				pChunk->GetColour(blockX, blockY, blockZ, &r, &g, &b, &a);

				pChunk->StartBatchUpdate();
				pChunk->SetColour(blockX, blockY, blockZ, 0);
				pChunk->StopBatchUpdate();

				m_pChunkManager->CreateBlockDestroyParticleEffect(r, g, b, a, m_blockSelectionPos);

				// Create the collectible block item
				BlockType blockType = pChunk->GetBlockType(blockX, blockY, blockZ);
				m_pChunkManager->CreateCollectibleBlock(blockType, m_blockSelectionPos);
			}
		}

		m_blockSelection = false;
	}
}

// Static callbacks
void Player::_AttackEnabledTimerFinished(void *apData)
{
	Player* lpPlayer = (Player*)apData;
	lpPlayer->AttackEnabledTimerFinished();
}

void Player::AttackEnabledTimerFinished()
{
	m_attackEnabled = false;
}

void Player::_AttackEnabledDelayTimerFinished(void *apData)
{
	Player* lpPlayer = (Player*)apData;
	lpPlayer->AttackEnabledDelayTimerFinished();
}

void Player::AttackEnabledDelayTimerFinished()
{
	m_attackEnabledDelayTimer = 0.0f;
}

void Player::_AttackAnimationTimerFinished(void *apData)
{
	Player* lpPlayer = (Player*)apData;
	lpPlayer->AttackAnimationTimerFinished();
}

void Player::AttackAnimationTimerFinished()
{
	if (IsBow())
	{
	}
	else if (IsBoomerang())
	{
		vec3 boomerangSpawnPosition = GetCenter() + (m_forward*0.75f) + (GetRightVector()*-0.4f) + (GetUpVector()*0.5f);

		float cameraModification = (m_cameraForward.y*17.5f);
		if (m_cameraForward.y < 0.0f)
		{
			cameraModification = (m_cameraForward.y*5.0f);
		}

		vec3 boomerangTarget = boomerangSpawnPosition + m_forward*15.0f + (vec3(0.0f, 1.0f, 0.0f) * cameraModification);
		if (m_pTargetEnemy != nullptr)
		{
			boomerangTarget = m_pTargetEnemy->GetProjectileHitboxCenter();
			if (m_pTargetEnemy->IsMoving())
			{
				boomerangTarget += m_pTargetEnemy->GetForwardVector() * (m_pTargetEnemy->GetMovementSpeed() / 3.0f);
			}
		}

		float curveTime = length(boomerangTarget - boomerangSpawnPosition) / 15.0f;
		if (curveTime <= 0.4f)
			curveTime = 0.4f;

		Projectile* pProjectile = m_pProjectileManager->CreateProjectile(boomerangSpawnPosition, vec3(0.0f, 0.0f, 0.0f), 0.0f, "media/gamedata/weapons/Boomerang/BoomerangThrown.weapon", 0.05f);
		pProjectile->SetProjectileType(true, false, false);
		pProjectile->SetOwner(this, nullptr, nullptr);
		pProjectile->SetGravityMultiplier(0.0f);
		pProjectile->SetReturnToPlayer(true);
		pProjectile->SetProjectileCurveParams(m_forward, boomerangTarget, curveTime);
		pProjectile->SetWorldCollisionEnabled(true);

		m_pVoxelCharacter->SetRenderRightWeapon(false);
	}
	else if (IsStaff())
	{
		float powerAmount = 25.0f;
		float cameraMultiplier = 25.0f;

		vec3 spellSpawnPosition = GetCenter() + (m_forward*1.25f) + (GetUpVector()*0.25f);

		if (VoxGame::GetInstance()->GetCameraMode() == CameraMode_FirstPerson)
		{
			cameraMultiplier = 30.0f;
			spellSpawnPosition.y += 0.75f;
		}

		vec3 spellSpawnVelocity = m_forward * powerAmount + vec3(0.0f, 1.0f, 0.0f) * (m_cameraForward.y*cameraMultiplier);

		if (m_pTargetEnemy != nullptr)
		{
			vec3 toTarget = m_pTargetEnemy->GetProjectileHitboxCenter() - GetCenter();
			spellSpawnVelocity = (normalize(toTarget) * powerAmount);
		}

		Projectile* pProjectile = m_pProjectileManager->CreateProjectile(spellSpawnPosition, spellSpawnVelocity, 0.0f, "media/gamedata/items/Fireball/Fireball.item", 0.05f);
		pProjectile->SetProjectileType(true, false, false);
		pProjectile->SetOwner(this, nullptr, nullptr);
		pProjectile->SetGravityMultiplier(0.0f);
	}
	else if (IsWand())
	{
	}
	else if (IsBomb())
	{
		vec3 bombSpawnPosition = GetCenter() + (m_forward*0.75f) + (GetUpVector()*0.5f);

		float liftAmount = 8.0f;
		float powerAmount = 30.0f;
		float cameraMultiplier = 25.0f;

		if (VoxGame::GetInstance()->GetCameraMode() == CameraMode_FirstPerson)
		{
			cameraMultiplier = 30.0f;
		}

		vec3 bombSpawnVelocity;
		if (m_pTargetEnemy)
		{
			// Enemy target
			vec3 toTarget = m_pTargetEnemy->GetCenter() - GetCenter();
			float toTargetDistance = length(toTarget);
			liftAmount += toTargetDistance * 0.04f;
			bombSpawnVelocity = (normalize(toTarget) * powerAmount) + vec3(0.0f, liftAmount, 0.0f);
		}
		else
		{
			bombSpawnVelocity = (m_forward * powerAmount) + (GetUpVector() * liftAmount) + vec3(0.0f, 1.0f, 0.0f) * (m_cameraForward.y*cameraMultiplier);
		}

		Projectile* pProjectile = m_pProjectileManager->CreateProjectile(bombSpawnPosition, bombSpawnVelocity, 0.0f, "media/gamedata/items/Bomb/BombThrown.item", 0.05f);
		pProjectile->SetProjectileType(true, false, false);
		pProjectile->SetOwner(this, nullptr, nullptr);
		pProjectile->SetGravityMultiplier(3.5f);
		float explodeRadius = 3.5f - (GetRandomNumber(-150, 0, 2) * 0.01f);
		pProjectile->SetExplodingProjectile(true, explodeRadius);

		//m_pVoxelCharacter->SetRenderRightWeapon(false);

		InventoryItem* pItem = m_pInventoryManager->GetInventoryItemForEquipSlot(EquipSlot_RightHand);
		if (pItem != nullptr)
		{
			if (pItem->m_quantity != -1)
			{
				pItem->m_quantity -= 1;
			}
			if (pItem->m_quantity == 0)
			{
				// Remove this item from the manager, and remove it from the inventory and GUI
				UnequipItem(EquipSlot_RightHand, false, false);
				m_pInventoryManager->RemoveInventoryItem(EquipSlot_RightHand);
				m_pActionBar->RemoveInventoryItemFromActionBar(pItem->m_title);
			}
		}
	}
	else if (IsConsumable())
	{
	}
	else if (IsDagger())
	{
	}
	else if (IsHammer())
	{
	}
	else if (IsMace())
	{
	}
	else if (IsSickle())
	{
	}
	else if (IsPickaxe())
	{
		Item* pInteractItem = VoxGame::GetInstance()->GetInteractItem();
		if (pInteractItem != nullptr)
		{
			pInteractItem->Interact();
		}
		else
		{
			DestroyBlock();
		}
	}
	else if (IsAxe())
	{
	}
	else if (Is2HandedSword())
	{
	}
	else if (IsSword())
	{
	}
	else if (IsBlockPlacing())
	{
	}
	else if (IsItemPlacing())
	{
	}
	else if (IsSceneryPlacing())
	{
	}
	else if (IsSpellHands())
	{
		float powerAmount = 25.0f;
		float cameraMultiplier = 25.0f;

		vec3 spellSpawnPosition = GetCenter() + (m_forward*0.5f) + (GetUpVector()*0.0f);

		// For right hand
		spellSpawnPosition += -(GetRightVector()*0.4f);

		if (VoxGame::GetInstance()->GetCameraMode() == CameraMode_FirstPerson)
		{
			cameraMultiplier = 30.0f;
			spellSpawnPosition.y += 0.75f;
		}

		vec3 spellSpawnVelocity = m_forward * powerAmount + vec3(0.0f, 1.0f, 0.0f) * (m_cameraForward.y*cameraMultiplier);

		if (m_pTargetEnemy != nullptr)
		{
			vec3 toTarget = m_pTargetEnemy->GetProjectileHitboxCenter() - GetCenter();
			spellSpawnVelocity = (normalize(toTarget) * powerAmount);
		}

		Projectile* pProjectile = m_pProjectileManager->CreateProjectile(spellSpawnPosition, spellSpawnVelocity, 0.0f, "media/gamedata/items/Fireball/FireballBlue.item", 0.05f);
		pProjectile->SetProjectileType(true, false, false);
		pProjectile->SetOwner(this, nullptr, nullptr);
		pProjectile->SetGravityMultiplier(0.0f);
	}
	else if (IsShield())
	{
	}
	else if (IsTorch())
	{
	}
}

void Player::_AttackAnimationTimerFinished_Alternative(void *apData)
{
	Player* lpPlayer = (Player*)apData;
	lpPlayer->AttackAnimationTimerFinished_Alternative();
}

void Player::AttackAnimationTimerFinished_Alternative()
{
	if (IsBow())
	{
	}
	else if (IsBoomerang())
	{
	}
	else if (IsStaff())
	{
	}
	else if (IsWand())
	{
	}
	else if (IsBomb())
	{
	}
	else if (IsConsumable())
	{
	}
	else if (IsDagger())
	{
	}
	else if (IsHammer())
	{
	}
	else if (IsMace())
	{
	}
	else if (IsSickle())
	{
	}
	else if (IsPickaxe())
	{
	}
	else if (IsAxe())
	{
	}
	else if (Is2HandedSword())
	{
	}
	else if (IsSword())
	{
	}
	else if (IsBlockPlacing())
	{
	}
	else if (IsItemPlacing())
	{
	}
	else if (IsSceneryPlacing())
	{
	}
	else if (IsSpellHands())
	{
		float powerAmount = 25.0f;
		float cameraMultiplier = 25.0f;

		vec3 spellSpawnPosition = GetCenter() + (m_forward*0.5f) + (GetUpVector()*0.0f);

		// For left hand
		spellSpawnPosition += (GetRightVector()*0.4f);

		if (VoxGame::GetInstance()->GetCameraMode() == CameraMode_FirstPerson)
		{
			cameraMultiplier = 30.0f;
			spellSpawnPosition.y += 0.75f;
		}

		vec3 spellSpawnVelocity = m_forward * powerAmount + vec3(0.0f, 1.0f, 0.0f) * (m_cameraForward.y*cameraMultiplier);

		if (m_pTargetEnemy != nullptr)
		{
			vec3 toTarget = m_pTargetEnemy->GetProjectileHitboxCenter() - GetCenter();
			spellSpawnVelocity = (normalize(toTarget) * powerAmount);
		}

		Projectile* pProjectile = m_pProjectileManager->CreateProjectile(spellSpawnPosition, spellSpawnVelocity, 0.0f, "media/gamedata/items/Fireball/FireballBlue.item", 0.05f);
		pProjectile->SetProjectileType(true, false, false);
		pProjectile->SetOwner(this, nullptr, nullptr);
		pProjectile->SetGravityMultiplier(0.0f);
	}
	else if (IsShield())
	{
	}
	else if (IsTorch())
	{
	}
}
