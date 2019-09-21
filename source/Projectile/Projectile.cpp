// ******************************************************************************
// Filename:    Projectile.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 15/02/15
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "Projectile.h"

#include <algorithm>

#include "../utils/Interpolator.h"
#include "../utils/Random.h"

#include "../Lighting/LightingManager.h"
#include "../Player/Player.h"
#include "../NPC/NPCManager.h"


Projectile::Projectile(Renderer* pRenderer, ChunkManager* pChunkManager, QubicleBinaryManager* pQubicleBinaryManager, const char* objectFilename, float scale)
{
	m_pRenderer = pRenderer;
	m_pChunkManager = pChunkManager;
	m_pQubicleBinaryManager = pQubicleBinaryManager;

	m_erase = false;

	m_radius = 0.25f;

	m_renderScale = scale;

	m_gravityDirection = vec3(0.0f, -1.0f, 0.0f);
	m_gravityMultiplier = 2.5f;

	m_forward = vec3(0.0f, 0.0f, 1.0f);

	m_gridPositionX = 0;
	m_gridPositionY = 0;
	m_gridPositionZ = 0;

	m_pCachedGridChunk = nullptr;

	m_pVoxeProjectile = nullptr;
	LoadItem(objectFilename);

	// World collision flag
	m_worldCollisionEnabled = true;

	// Explosion
	m_explodeWorld = false;
	m_explosionRadius = 1.0f;

	// Projectile type
	m_attackEnemies = false;
	m_attackPlayer = false;
	m_attackNPCs = false;

	// Return to player
	m_returnToPlayer = false;
	m_catchTimer = 0.0f;
	m_curveTime = 1.0f;
	m_curveTimer = m_curveTime;
	m_rightCurve = true;
	m_returningDirectToPlayer = false;

	m_pOwnedPlayer = nullptr;
	m_pOwnedNPC = nullptr;
	m_pOwnedEnemy = nullptr;

	m_pVoxeProjectile->StartWeaponTrails();
}

Projectile::~Projectile()
{
	m_pVoxeProjectile->StopWeaponTrails();

	UnloadEffectsAndLights();

	if(m_pVoxeProjectile != nullptr)
	{
		delete m_pVoxeProjectile;
	}
}


void Projectile::SetLightingManager(LightingManager* pLightingManager)
{
	m_pLightingManager = pLightingManager;
}

void Projectile::SetGameWindow(GameWindow* pGameWindow)
{
	m_pGameWindow = pGameWindow;
}

void Projectile::SetBlockParticleManager(BlockParticleManager* pBlockParticleManager)
{
	m_pBlockParticleManager = pBlockParticleManager;
}

void Projectile::SetPlayer(Player* pPlayer)
{
	m_pPlayer = pPlayer;
}

// Unloading
void Projectile::UnloadEffectsAndLights()
{
	// Lights
	for(int i = 0; i < m_pVoxeProjectile->GetNumLights(); i++)
	{
		unsigned int lightId;
		vec3 lightPos;
		float lightRadius;
		float lightDiffuseMultiplier;
		Colour lightColour;
		bool connectedToSegment;
		m_pVoxeProjectile->GetLightParams(i, &lightId, &lightPos, &lightRadius, &lightDiffuseMultiplier, &lightColour, &connectedToSegment);

		if(lightId != -1)
		{
			m_pLightingManager->RemoveLight(lightId);
			m_pVoxeProjectile->SetLightingId(i, -1);

			if(connectedToSegment == false)
			{
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

			float scale = m_renderScale;
			unsigned int lId;
			m_pLightingManager->AddDyingLight(lightPos, lightRadius * scale, lightDiffuseMultiplier, lightColour, 2.0f, &lId);
		}
	}

	// Particle Effects
	for(int i = 0; i < m_pVoxeProjectile->GetNumParticleEffects(); i++)
	{
		unsigned int particleEffectId;
		vec3 ParticleEffectPos;
		string effectName;
		bool connectedToSegment;
		m_pVoxeProjectile->GetParticleEffectParams(i, &particleEffectId, &ParticleEffectPos, &effectName, &connectedToSegment);

		if(particleEffectId != -1)
		{
			m_pBlockParticleManager->DestroyParticleEffect(particleEffectId);
			m_pVoxeProjectile->SetParticleEffectId(i, -1);
		}
	}
}

// Erase flag
bool Projectile::GetErase()
{
	return m_erase;
}

void Projectile::SetErase(bool erase)
{
	m_erase = erase;
}

// Setup
void Projectile::LoadItem(const char* objectFilename)
{
	if(m_pVoxeProjectile == nullptr)
	{
		m_pVoxeProjectile = new VoxelWeapon(m_pRenderer, m_pQubicleBinaryManager);
	}

	m_pVoxeProjectile->SetVoxelCharacterParent(nullptr);
	m_pVoxeProjectile->LoadWeapon(objectFilename, false);

	// Lights
	for(int i = 0; i < m_pVoxeProjectile->GetNumLights(); i++)
	{
		unsigned int lightId;
		vec3 lightPos;
		float lightRadius;
		float lightDiffuseMultiplier;
		Colour lightColour;
		bool connectedToSegment;
		m_pVoxeProjectile->GetLightParams(i, &lightId, &lightPos, &lightRadius, &lightDiffuseMultiplier, &lightColour, &connectedToSegment);

		if(lightId != -1)
		{
			m_pLightingManager->RemoveLight(lightId);
			m_pVoxeProjectile->SetLightingId(i, -1);
		}
	}

	// Particle effects
	for(int i = 0; i < m_pVoxeProjectile->GetNumParticleEffects(); i++)
	{
		unsigned int particleEffectId;
		vec3 ParticleEffectPos;
		string effectName;
		bool connectedToSegment;
		m_pVoxeProjectile->GetParticleEffectParams(i, &particleEffectId, &ParticleEffectPos, &effectName, &connectedToSegment);

		if(particleEffectId != -1)
		{
			m_pBlockParticleManager->DestroyParticleEffect(particleEffectId);
			m_pVoxeProjectile->SetParticleEffectId(i, -1);
		}
	}
}

// Accessors / Setters
void Projectile::SetPosition(vec3 pos)
{
	m_position = pos;
}

vec3 Projectile::GetPosition()
{
	return m_position;
}

void Projectile::SetVelocity(vec3 vel)
{
	m_velocity = vel;
}

vec3 Projectile::GetVelocity()
{
	return m_velocity;
}

void Projectile::SetRotation(float rot)
{
	float angle = DegToRad(rot);
	m_forward = vec3(sin(angle), 0.0f, cos(angle));
}

float Projectile::GetRotation()
{
	float rotationAngle = acos(dot(vec3(0.0f, 0.0f, 1.0f), m_forward));
	rotationAngle = RadToDeg(rotationAngle);
	if(m_forward.x < 0.0f)
	{
		rotationAngle = (360.0f - rotationAngle);
	}

	return rotationAngle;
}

float Projectile::GetScale()
{
	return m_renderScale;
}

float Projectile::GetRadius()
{
	return m_radius;
}

void Projectile::UpdateRadius()
{
	m_radius = m_pVoxeProjectile->GetRenderScale() / 0.14f;
}

vec3 Projectile::GetCenter()
{
	vec3 center = m_position + vec3(0.0f, m_radius, 0.0f);

	return center;
}

vec3 Projectile::GetForwardVector()
{
	return m_worldMatrix.GetForwardVector();
}

vec3 Projectile::GetRightVector()
{
	return m_worldMatrix.GetRightVector();
}

vec3 Projectile::GetUpVector()
{
	return m_worldMatrix.GetUpVector();
}

void Projectile::SetGravityDirection(vec3 dir)
{
	m_gravityDirection = dir;

	if(length(m_gravityDirection) >= 0.0001f)
	{
		m_gravityDirection = normalize(m_gravityDirection);
	}
}

void Projectile::SetGravityMultiplier(float multiplier)
{
	m_gravityMultiplier = multiplier;
}

void Projectile::SetProjectileCurveParams(vec3 forward, vec3 target, float curveTime)
{	
	bool goRight = true;// (GetRandomNumber(0, 100) > 50); // Randomly go either left or right to start with

	vec3 crossRight = cross(forward, vec3(0.0f, 1.0f, 0.0f));
	vec3 targetDiff = target - m_position;

	// First part of curve
	m_bezierStart_Right = m_position;
	m_bezierEnd_Right = target;
	if(goRight)
	{
		m_bezierControl_Right = target + crossRight*length(targetDiff);
	}
	else
	{
		m_bezierControl_Right = target - crossRight*length(targetDiff);
	}

	// Second part of curve
	m_bezierStart_Left = m_position;
	m_bezierEnd_Left = target;
	if(goRight)
	{
		m_bezierControl_Left = target - crossRight*length(targetDiff);
	}
	else
	{
		m_bezierControl_Left = target + crossRight*length(targetDiff);
	}
	
	m_curveTime = curveTime;
	m_curveTimer = curveTime;
	m_rightCurve = true;

	Interpolator::GetInstance()->AddFloatInterpolation(&m_curveTimer, m_curveTime, 0.0f, m_curveTime, 0.0f, nullptr, _RightCurveTimerFinished, this);
}

void Projectile::SetWorldCollisionEnabled(bool enabled)
{
	m_worldCollisionEnabled = enabled;
}

// Projectile type
void Projectile::SetProjectileType(bool attackEnemies, bool attackPlayer, bool attackNPCs)
{
	m_attackEnemies = attackEnemies;
	m_attackPlayer = attackPlayer;
	m_attackNPCs = attackNPCs;
}

bool Projectile::CanAttackEnemies()
{
	return m_attackEnemies;
}

bool Projectile::CanAttackPlayer()
{
	return m_attackPlayer;
}

bool Projectile::CanAttackNPCs()
{
	return m_attackNPCs;
}

void Projectile::SetReturnToPlayer(bool returnToPlayer)
{
	m_returnToPlayer = returnToPlayer;

	if(m_returnToPlayer)
	{
		m_catchTimer = 0.75f;
	}
}

bool Projectile::IsReturnToPlayer()
{
	return m_returnToPlayer;
}

void Projectile::SetExplodingProjectile(bool exploding, float radius)
{
	m_explodeWorld = exploding;
	m_explosionRadius = radius;
}

void Projectile::SetOwner(Player* pPlayer, NPC* pNPC, Enemy* pEnemy)
{
	m_pOwnedPlayer = pPlayer;
	m_pOwnedNPC = pNPC;
	m_pOwnedEnemy = pEnemy;
}

Player* Projectile::GetPlayerOwner()
{
	return m_pOwnedPlayer;
}

NPC* Projectile::GetNPCOwner()
{
	return m_pOwnedNPC;
}

Enemy* Projectile::GetEnemyOwner()
{
	return m_pOwnedEnemy;
}

// Catching
bool Projectile::CanCatch()
{
	return m_returnToPlayer && m_catchTimer <= 0.0f;
}

void Projectile::PlayerCatch()
{
	m_erase = true;
}

// Explode
void Projectile::Explode()
{
	CalculateWorldTransformMatrix();

	if(m_pVoxeProjectile != nullptr)
	{
		for(int animatedSectionsIndex = 0; animatedSectionsIndex < m_pVoxeProjectile->GetNumAimatedSections(); animatedSectionsIndex++)
		{
			AnimatedSection* pAnimatedSection = m_pVoxeProjectile->GetAnimatedSection(animatedSectionsIndex);
			QubicleBinary* pQubicleModel = pAnimatedSection->m_pVoxelObject->GetQubicleModel();
			m_pBlockParticleManager->ExplodeQubicleBinary(pQubicleModel, m_renderScale, 100);
		}
	}

	if (m_explodeWorld)
	{
		m_pChunkManager->ExplodeSphere(GetCenter(), m_explosionRadius);
	}

	m_erase = true;
}

// Grid
void Projectile::UpdateGridPosition()
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

Chunk* Projectile::GetCachedGridChunkOrFromPosition(vec3 pos)
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

// Rendering Helpers
void Projectile::CalculateWorldTransformMatrix()
{
	// Make sure we are always pointing towards our velocity
	if(length(m_velocity) > 0.01f)
	{
		m_forward = normalize(m_velocity);
	}

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
		GetCenter().x, GetCenter().y, GetCenter().z, 1.0f
	};

	m_worldMatrix.SetValues(lMatrix);
}

// Updating
void Projectile::Update(float dt)
{
	if(m_pVoxeProjectile != nullptr)
	{
		CalculateWorldTransformMatrix();

		m_pVoxeProjectile->Update(dt);

		m_pVoxeProjectile->SetWeaponTrailsParams(m_worldMatrix, m_renderScale);
		m_pVoxeProjectile->CreateWeaponTrailPoint();
	}

	if(m_erase == true)
	{
		return;
	}

	// Update catch timer
	if(m_catchTimer > 0.0f)
	{
		m_catchTimer -= dt;
	}

	// Update grid position
	UpdateGridPosition();

	vec3 acceleration = (m_gravityDirection * 9.81f) * m_gravityMultiplier;

	if(m_returnToPlayer)
	{
		float t = 1.0f - (m_curveTimer / m_curveTime);

		m_bezierStart_Left = m_pPlayer->GetCenter();

		Bezier3 curve_right(m_bezierStart_Right, m_bezierEnd_Right, m_bezierControl_Right);
		Bezier3 curve_left(m_bezierStart_Left, m_bezierEnd_Left, m_bezierControl_Left);

		vec3 pos;
		vec3 pos1;
		if(m_rightCurve)
		{
			pos = curve_right.GetInterpolatedPoint(t);
			pos1 = curve_right.GetInterpolatedPoint(t+0.01f);
		}
		else
		{
			pos = curve_left.GetInterpolatedPoint(t);
			pos1 = curve_left.GetInterpolatedPoint(t-0.01f);
		}

		m_position = pos;
		m_velocity = normalize(pos1 - pos);
	}

	// Integrate velocity and position
	m_velocity += acceleration * dt;
	m_position += m_velocity * dt;

	if(m_worldCollisionEnabled)
	{
		vec3 floorPosition;
		if (m_pChunkManager->FindClosestFloor(GetCenter(), &floorPosition) == false)
		{
			if (m_returnToPlayer)
			{
				if (m_returningDirectToPlayer == false)
				{
					Interpolator::GetInstance()->RemoveFloatInterpolationByVariable(&m_curveTimer);

					// Go straight back to player
					m_bezierStart_Left = m_pPlayer->GetCenter();
					m_bezierEnd_Left = m_position;
					m_bezierControl_Left = m_position;
					m_curveTime = 1.0f;
					m_curveTimer = 0.0f;
					m_rightCurve = false;
					Interpolator::GetInstance()->AddFloatInterpolation(&m_curveTimer, 0.0f, m_curveTime, m_curveTime, 0.0f);

					m_returningDirectToPlayer = true;
				}
			}
			else
			{
				Explode();
			}
		}
		else
		{
			int blockX, blockY, blockZ;
			vec3 blockPos;

			Chunk* pChunk = GetCachedGridChunkOrFromPosition(m_position);
			bool active = m_pChunkManager->GetBlockActiveFrom3DPosition(m_position.x, m_position.y, m_position.z, &blockPos, &blockX, &blockY, &blockZ, &pChunk);

			if (pChunk != false && active == true)
			{
				if (m_returnToPlayer)
				{
					if (m_returningDirectToPlayer == false)
					{
						Interpolator::GetInstance()->RemoveFloatInterpolationByVariable(&m_curveTimer);

						// Go straight back to player
						m_bezierStart_Left = m_pPlayer->GetCenter();
						m_bezierEnd_Left = m_position;
						m_bezierControl_Left = m_position;
						m_curveTime = 1.0f;
						m_curveTimer = 0.0f;
						m_rightCurve = false;
						Interpolator::GetInstance()->AddFloatInterpolation(&m_curveTimer, 0.0f, m_curveTime, m_curveTime, 0.0f);

						m_returningDirectToPlayer = true;
					}
				}
				else
				{
					// Roll back the integration, since we will intersect the block otherwise
					m_position -= m_velocity * dt;

					m_velocity = vec3(0.0f, 0.0f, 0.0f);

					Explode();
				}
			}
		}
	}

	m_previousPosition = GetCenter();
}

void Projectile::UpdateProjectileLights(float dt)
{
	if(m_erase)
	{
		return;
	}

	if(m_pVoxeProjectile != nullptr)
	{
		for(int i = 0; i < m_pVoxeProjectile->GetNumLights(); i++)
		{
			unsigned int lightId;
			vec3 lightPos;
			float lightRadius;
			float lightDiffuseMultiplier;
			Colour lightColour;
			bool connectedToSegment;
			m_pVoxeProjectile->GetLightParams(i, &lightId, &lightPos, &lightRadius, &lightDiffuseMultiplier, &lightColour, &connectedToSegment);

			if(lightId == -1)
			{
				m_pLightingManager->AddLight(vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f, 1.0f), &lightId);
				m_pVoxeProjectile->SetLightingId(i, lightId);
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

void Projectile::UpdateProjectileParticleEffects(float dt)
{
	if(m_erase)
	{
		return;
	}

	if(m_pVoxeProjectile != nullptr)
	{
		for(int i = 0; i < m_pVoxeProjectile->GetNumParticleEffects(); i++)
		{
			unsigned int particleEffectId;
			vec3 ParticleEffectPos;
			string effectName;
			bool connectedToSegment;
			m_pVoxeProjectile->GetParticleEffectParams(i, &particleEffectId, &ParticleEffectPos, &effectName, &connectedToSegment);

			if(particleEffectId == -1)
			{
				m_pBlockParticleManager->ImportParticleEffect(effectName, ParticleEffectPos, &particleEffectId);
				m_pVoxeProjectile->SetParticleEffectId(i, particleEffectId);
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
void Projectile::Render()
{
	if(m_erase == true)
	{
		return;
	}

	if(m_pVoxeProjectile != nullptr)
	{
		Colour OutlineColour(1.0f, 1.0f, 0.0f, 1.0f);

		m_pRenderer->PushMatrix();
			m_pRenderer->MultiplyWorldMatrix(m_worldMatrix);
			m_pRenderer->ScaleWorldMatrix(m_renderScale, m_renderScale, m_renderScale);
			m_pVoxeProjectile->Render(false, false, false, OutlineColour);
		m_pRenderer->PopMatrix();
	}
}

void Projectile::RenderWeaponTrails()
{
	m_pRenderer->PushMatrix();
		m_pVoxeProjectile->RenderWeaponTrails();
	m_pRenderer->PopMatrix();
}

void Projectile::RenderDebug()
{
	if(m_erase == true)
	{
		return;
	}

	m_pRenderer->PushMatrix();
		m_pRenderer->TranslateWorldMatrix(GetCenter().x, GetCenter().y, GetCenter().z);

		m_pRenderer->PushMatrix();
			m_pRenderer->SetLineWidth(1.0f);
			m_pRenderer->SetRenderMode(RM_WIREFRAME);
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);

			m_pRenderer->RotateWorldMatrix(0.0f, GetRotation(), 0.0f);

			m_pRenderer->DrawSphere(m_radius, 20, 20);
		m_pRenderer->PopMatrix();
	m_pRenderer->PopMatrix();

	RenderForwardDebug();

	RenderDebugReturnCurve();
}

void Projectile::RenderDebugReturnCurve()
{
	Bezier3 curve_right(m_bezierStart_Right, m_bezierEnd_Right, m_bezierControl_Right);
	Bezier3 curve_left(m_bezierStart_Left, m_bezierEnd_Left, m_bezierControl_Left);

	m_pRenderer->PushMatrix();
		m_pRenderer->SetRenderMode(RM_WIREFRAME);
		m_pRenderer->SetCullMode(CM_NOCULL);
		m_pRenderer->SetLineWidth(1.0f);
		m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
		m_pRenderer->DrawBezier(curve_right, 20);
		m_pRenderer->DrawBezier(curve_left, 20);
	m_pRenderer->PopMatrix();
}

void Projectile::RenderForwardDebug()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->TranslateWorldMatrix(GetCenter().x, GetCenter().y, GetCenter().z);

		m_pRenderer->SetRenderMode(RM_SOLID);
		m_pRenderer->SetLineWidth(3.0f);
		m_pRenderer->EnableImmediateMode(IM_LINES);
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(m_forward.x*1.5f, m_forward.y*1.5f, m_forward.z*1.5f);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();
}

void Projectile::_RightCurveTimerFinished(void *apData)
{
	Projectile* lpProjectile = (Projectile*)apData;
	lpProjectile->RightCurveTimerFinished();
}

void Projectile::RightCurveTimerFinished()
{
	m_curveTimer = 0.0f;
	m_rightCurve = false;

	Interpolator::GetInstance()->AddFloatInterpolation(&m_curveTimer, 0.0f, m_curveTime, m_curveTime, 0.0f);
}
