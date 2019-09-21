// ******************************************************************************
// Filename:    BlockParticle.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 09/11/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "BlockParticle.h"
#include "BlockParticleEmitter.h"
#include "BlockParticleEffect.h"
#include "../blocks/Chunk.h"
#include "../blocks/ChunkManager.h"
#include "../utils/Random.h"


BlockParticle::BlockParticle()
{
	m_erase = false;

	m_allowFloorSliding = false;

	m_gridPositionX = 0;
	m_gridPositionY = 0;
	m_gridPositionZ = 0;

	m_pCachedGridChunk = nullptr;

	m_pParent = nullptr;

	m_hasCollided = false;

	m_createEmitters = false;
	m_pCreatedEmitter = nullptr;

	m_freezeUpdateTimer = 0.0f;
	m_waitAfterUpdateCompleteTimer = 0.0f;
}

BlockParticle::~BlockParticle()
{
	if(m_createEmitters == true && m_pCreatedEmitter != nullptr)
	{
		m_pCreatedEmitter->m_pParentParticle = nullptr;
		m_pCreatedEmitter->m_erase = true;
	}
}

void BlockParticle::ClearParticleChunkCacheForChunk(Chunk* pChunk)
{
	if (m_pCachedGridChunk == pChunk)
	{
		m_pCachedGridChunk = nullptr;
	}
}

void BlockParticle::CreateStartingParams()
{
	m_startScale = m_startScale + ((GetRandomNumber(-1, 1, 2) * m_startScaleVariance) * m_startScale);
	m_endScale = m_endScale + ((GetRandomNumber(-1, 1, 2) * m_endScaleVariance) * m_endScale);
	m_currentScale = m_startScale;

	m_startRed = m_startRed + (GetRandomNumber(-1, 1, 2) * m_startRedVariance);
	m_endRed = m_endRed + (GetRandomNumber(-1, 1, 2) * m_endRedVariance);
	m_currentRed = m_startRed;

	m_startGreen = m_startGreen + (GetRandomNumber(-1, 1, 2) * m_startGreenVariance);
	m_endGreen = m_endGreen + (GetRandomNumber(-1, 1, 2) * m_endGreenVariance);
	m_currentGreen = m_startGreen;

	m_startBlue = m_startBlue + (GetRandomNumber(-1, 1, 2) * m_startBlueVariance);
	m_endBlue = m_endBlue + (GetRandomNumber(-1, 1, 2) * m_endBlueVariance);
	m_currentBlue = m_startBlue;

	m_startAlpha = m_startAlpha + (GetRandomNumber(-1, 1, 2) * m_startAlphaVariance);
	m_endAlpha = m_endAlpha + (GetRandomNumber(-1, 1, 2) * m_endAlphaVariance);
	m_currentAlpha = m_startAlpha;

	m_lifeTime = m_lifeTime + ((GetRandomNumber(-1, 1, 2) * m_lifeTimeVariance) * m_lifeTime);
	m_maxLifeTime = m_lifeTime;

	m_velocity = m_velocity + vec3(GetRandomNumber(-100, 100, 2)*0.01f*m_startVelocityVariance.x, GetRandomNumber(-100, 100, 2)*0.01f*m_startVelocityVariance.y, GetRandomNumber(-100, 100, 2)*0.01f*m_startVelocityVariance.z);
	m_angularVelocity = m_angularVelocity + vec3(GetRandomNumber(-100, 100, 2)*0.01f*m_startAngularVelocityVariance.x, GetRandomNumber(-100, 100, 2)*0.01f*m_startAngularVelocityVariance.y, GetRandomNumber(-100, 100, 2)*0.01f*m_startAngularVelocityVariance.z);

	if(m_randomStartRotation)
	{
		m_rotation = vec3(GetRandomNumber(-360, 360, 2), GetRandomNumber(-360, 360, 2), GetRandomNumber(-360, 360, 2));
	}
	else
	{
		m_rotation = m_startRotation;
	}
}

void BlockParticle::CalculateWorldTransformMatrix()
{
	// Full world positional matrix
	m_worldMatrix.LoadIdentity();
	m_worldMatrix.SetRotation(DegToRad(m_rotation.x), DegToRad(m_rotation.y), DegToRad(m_rotation.z));

	vec3 pos = m_position;
	if(m_pParent != nullptr && m_pParent->m_particlesFollowEmitter)
	{
		// If we have a parent and we are locked to their position
		pos += m_pParent->m_position;

		if(m_pParent->m_pParent != nullptr)
		{
			// If our emitter's parent effect has a position offset
			pos += m_pParent->m_pParent->m_position;
		}
	}

	m_worldMatrix.SetTranslation(vec3(pos.x, pos.y, pos.z));

	Matrix4x4 scaleMat;
	scaleMat.SetScale(vec3(m_currentScale, m_currentScale, m_currentScale));

	m_worldMatrix = scaleMat * m_worldMatrix;

	// --------------------------------------------------------------------------

	// Non-world matrix that doesn't contain the world positional offset, i.e only local to the particle's emitter and effect
	m_worldMatrix_NoPositionOffset.LoadIdentity();
	m_worldMatrix_NoPositionOffset.SetRotation(DegToRad(m_rotation.x), DegToRad(m_rotation.y), DegToRad(m_rotation.z));

	pos = m_position_NoWorldOffset;
	if (m_pParent != nullptr && m_pParent->m_particlesFollowEmitter)
	{
		// If we have a parent and we are locked to their position
		pos += m_pParent->m_position;

		if (m_pParent->m_pParent != nullptr)
		{
			// If our emitter's parent effect has a position offset
			pos += m_pParent->m_pParent->m_position_NoWorldOffset;
		}
	}

	m_worldMatrix_NoPositionOffset.SetTranslation(vec3(pos.x, pos.y, pos.z));

	scaleMat.SetScale(vec3(m_currentScale, m_currentScale, m_currentScale));

	m_worldMatrix_NoPositionOffset = scaleMat * m_worldMatrix_NoPositionOffset;
}

void BlockParticle::UpdateGridPosition()
{
	int gridPositionX = (int)((m_position.x + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);
	int gridPositionY = (int)((m_position.y + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);
	int gridPositionZ = (int)((m_position.z + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);

	if (m_position.x <= -0.5f)
		gridPositionX -= 1;
	if (m_position.y <= -0.5f)
		gridPositionY -= 1;
	if (m_position.z <= -0.5f)
		gridPositionZ -= 1;

	if (gridPositionX != m_gridPositionX || gridPositionY != m_gridPositionY || gridPositionZ != m_gridPositionZ || m_pCachedGridChunk == nullptr)
	{
		m_gridPositionX = gridPositionX;
		m_gridPositionY = gridPositionY;
		m_gridPositionZ = gridPositionZ;

		m_pCachedGridChunk = m_pChunkManager->GetChunk(m_gridPositionX, m_gridPositionY, m_gridPositionZ);
	}
}

Chunk* BlockParticle::GetCachedGridChunkOrFromPosition(vec3 pos)
{
	// First check if the position is in the same grid as the cached chunk
	int gridPositionX = (int)((pos.x + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);
	int gridPositionY = (int)((pos.y + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);
	int gridPositionZ = (int)((pos.z + Chunk::BLOCK_RENDER_SIZE) / Chunk::CHUNK_SIZE);

	if (pos.x <= -0.5f)
		gridPositionX -= 1;
	if (pos.y <= -0.5f)
		gridPositionY -= 1;
	if (pos.z <= -0.5f)
		gridPositionZ -= 1;

	if (gridPositionX != m_gridPositionX || gridPositionY != m_gridPositionY || gridPositionZ != m_gridPositionZ)
	{
		return nullptr;
	}
	else
	{
		return m_pCachedGridChunk;
	}
}

void BlockParticle::SetFreezeDelayTimer(float timer)
{
	m_freezeUpdateTimer = timer;
}

void BlockParticle::SetWaitAfterUpdateCompleteTimer(float timer)
{
	m_waitAfterUpdateCompleteTimer = timer;
}

// Update
void BlockParticle::Update(float dt)
{
	if(m_erase)
	{
		return;
	}

	if(m_pParent != nullptr && m_pParent->m_paused == true)
	{
		// If our parent emitter is paused, return
		return;
	}

	// Update particle life
	if(m_startLifeDecayOnCollision == false || m_hasCollided == true)
	{
		m_lifeTime -= dt;

		if(m_lifeTime < 0.0f)
		{
			m_lifeTime = 0.0f;
		}
	}

	// Update grid position and cached chunk pointer
	if (m_checkWorldCollisions)
	{
		UpdateGridPosition();

		if (m_pCachedGridChunk == nullptr)
		{
			m_hasCollided = true;

			if (m_destoryOnCollision)
			{
				m_erase = true;
				return;
			}

			return;
		}
	}

	// If we are frozen, return and don't do any physics updating
	if(m_freezeUpdateTimer >= 0.0f)
	{
		m_freezeUpdateTimer -= dt;

		return;
	}

	if(m_lifeTime > 0.0f)
	{
		if(m_pParent != nullptr)
		{
			// Velocity towards point origin
			vec3 pointOrigin = m_pointOrigin;
			if(m_pParent != nullptr && m_pParent->m_particlesFollowEmitter == false)
			{
				if(m_pParent->m_pParent != nullptr)
				{
					pointOrigin += m_pParent->m_pParent->m_position; // Add on parent's particle effect position
				}
			}
			vec3 toPoint = pointOrigin - m_position;
			if(length(toPoint) > 0.001f)
			{
				m_velocityTowardsPoint += m_accelerationTowardsPoint * dt;
				vec3 velToPoint = toPoint * m_velocityTowardsPoint;
				m_pointVelocity += (velToPoint) * dt;

				// Tangential velocity
				vec3 x_axis = vec3(m_velocity.y<0.0f?-1.0f:1.0f, 0.0f, 0.0f);
				vec3 cross_x = cross(toPoint, x_axis);
				vec3 y_axis = vec3(0.0f, m_velocity.z<0.0f?-1.0f:1.0f, 0.0f);
				vec3 cross_y = cross(toPoint, y_axis);
				vec3 z_axis = vec3(0.0f, 0.0f, m_velocity.y<0.0f?-1.0f:1.0f);
				vec3 cross_z = cross(toPoint, z_axis);

				m_tangentialVelocityXY += m_tangentialAccelerationXY * dt;
				m_tangentialVelocityXZ += m_tangentialAccelerationXZ * dt;
				m_tangentialVelocityYZ += m_tangentialAccelerationYZ * dt;
				vec3 velTangentXY = (cross_z/* * length(toPoint)*/) * m_tangentialVelocityXY;
				vec3 velTangentXZ = (cross_y/* * length(toPoint)*/) * m_tangentialVelocityXZ;
				vec3 velTangentYZ = (cross_x/* * length(toPoint)*/) * m_tangentialVelocityYZ;
				
				m_tangentialVelocity = velTangentXY+velTangentXZ+velTangentYZ;
			}
		}

		// Position integration
		vec3 acceleration = (m_gravityDirection * 9.81f) * m_gravityMultiplier;
		m_velocity += acceleration * dt;
		m_position += m_velocity * dt;
		m_position_NoWorldOffset += m_velocity * dt;

		// Point origin and tangential velocity
		m_position += m_tangentialVelocity * dt;
		m_position += m_pointVelocity * dt;
		m_position_NoWorldOffset += m_tangentialVelocity * dt;
		m_position_NoWorldOffset += m_pointVelocity * dt;

		// Rotation integration
		vec3 angularAcceleration(0.0f, 0.0f, 0.0f);
		m_angularVelocity += angularAcceleration * dt;
		m_rotation += m_angularVelocity * dt;

		if (m_checkWorldCollisions)
		{
			int blockX, blockY, blockZ;
			vec3 blockPos;
			vec3 particlePos = m_position;
			if (m_pParent != nullptr && m_pParent->m_particlesFollowEmitter)
			{
				particlePos += m_pParent->m_position;
			}

			Chunk* pChunk = GetCachedGridChunkOrFromPosition(particlePos);
			bool active = m_pChunkManager->GetBlockActiveFrom3DPosition(particlePos.x, particlePos.y, particlePos.z, &blockPos, &blockX, &blockY, &blockZ, &pChunk);

			if (active == true)
			{
				m_hasCollided = true;

				if (m_destoryOnCollision)
				{
					m_erase = true;
					return;
				}

				if (m_allowFloorSliding)
				{
					// Roll back the integration, since we will intersect the block otherwise
					m_position -= vec3(0.0f, m_velocity.y, 0.0f) * dt;
					m_position -= m_tangentialVelocity * dt;
					m_position -= m_pointVelocity * dt;
					m_velocity -= acceleration * dt;

					// Apply some damping to the rotation and velocity
					m_angularVelocity *= 0.96f;
					m_velocity *= 0.96f;

					if (m_velocity.y <= 0.05f)
					{
						m_velocity = vec3(m_velocity.x, 0.0f, m_velocity.z);
					}
				}
				else
				{
					// Roll back the integration, since we will intersect the block otherwise
					m_position -= m_velocity * dt;
					m_position -= m_tangentialVelocity * dt;
					m_position -= m_pointVelocity * dt;

					// Apply some damping to the rotation and velocity
					m_angularVelocity *= 0.96f;
					m_velocity *= 0.96f;

					if (m_velocity.y <= 0.05f)
					{
						m_velocity = vec3(0.0f, 0.0f, 0.0f);
					}
				}
			}
		}
	}
	else
	{
		if(m_waitAfterUpdateCompleteTimer > 0.0f)
		{
			m_waitAfterUpdateCompleteTimer -= dt;
		}
	}

	float timeRatio = (m_lifeTime+m_freezeUpdateTimer) / (m_maxLifeTime+m_freezeUpdateTimer);

	// Update colour
	m_currentRed = m_startRed + ((m_endRed - m_startRed) * (1.0f - timeRatio));
	m_currentGreen = m_startGreen + ((m_endGreen - m_startGreen) * (1.0f - timeRatio));
	m_currentBlue = m_startBlue + ((m_endBlue - m_startBlue) * (1.0f - timeRatio));
	m_currentAlpha = m_startAlpha + ((m_endAlpha - m_startAlpha) * (1.0f - timeRatio));

	// Update scale
	m_currentScale = m_startScale + ((m_endScale - m_startScale) * (1.0f - timeRatio));

	if(m_createEmitters == true && m_pCreatedEmitter != nullptr)
	{
		m_pCreatedEmitter->m_pParentParticle = this;
		m_pCreatedEmitter->m_position = m_position;
	}
}
