// ******************************************************************************
// Filename:    BlockParticleEmitter.cpp
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

#include "BlockParticleEmitter.h"
#include "BlockParticleManager.h"
#include "BlockParticleEffect.h"
#include "BlockParticle.h"

#include "../utils/Random.h"


BlockParticleEmitter::BlockParticleEmitter(Renderer* pRenderer, BlockParticleManager* pBlockParticleManager)
{
	m_pRenderer = pRenderer;
	m_pBlockParticleManager = pBlockParticleManager;

	m_erase = false;

	m_pParent = nullptr;
	m_pParentParticle = nullptr;
	m_isReferenceEmitter = false;

	m_emitterName = "";
	m_creationTime = 0.0f;
	m_creationTimer = 0.0f;
	// Number of particles to spawn
	m_numParticlesToSpawn = 0;
	// Should particles follow emitter movement
	m_particlesFollowEmitter = false;
	// Emitter orbiting
	m_orbit = 0.0f;
	m_orbitReverse = 0.0f;
	m_orbitRadius = 0.0f;
	m_orbitTime = 0.0f;
	m_orbitTimer = 0.0f;
	// Playback controls
	m_started = false;
	m_paused = false;
	// Emitter type
	m_emitterType = EmitterType_Point;
	// Emitter radius and lengths
	m_emitterRadius = 0.0f;
	m_emitterLengthX = 0.0f;
	m_emitterLengthY = 0.0f;
	m_emitterLengthZ = 0.0f;
	// Are we spawning on the outside of the shape
	m_spawnOutline = false;
	// Particle creation params
	m_gravityMultiplier = 0.0f;
	m_startScale = 0.0f;
	m_startScaleVariance = 0.0f;
	m_endScale = 0.0f;
	m_endScaleVariance = 0.0f;
	m_startRed = 0.0f;
	m_startRedVariance = 0.0f;
	m_endRed = 0.0f;
	m_endRedVariance = 0.0f;
	m_startGreen = 0.0f;
	m_startGreenVariance = 0.0f;
	m_endGreen = 0.0f;
	m_endGreenVariance = 0.0f;
	m_startBlue = 0.0f;
	m_startBlueVariance = 0.0f;
	m_endBlue = 0.0f;
	m_endBlueVariance = 0.0f;
	m_startAlpha = 0.0f;
	m_startAlphaVariance = 0.0f;
	m_endAlpha = 0.0f;
	m_endAlphaVariance = 0.0f;
	m_lifeTime = 0.0f;
	m_lifeTimeVariance = 0.0f;
	m_velocityTowardsPoint = 0.0f;
	m_accelerationTowardsPoint = 0.0f;
	m_checkWorldCollisions = false;
	m_destoryOnCollision = false;
	m_startLifeDecayOnCollision = false;
	m_randomStartRotation = false;
	m_tangentialVelocityXY = 0.0f;
	m_tangentialAccelerationXY = 0.0f;
	m_tangentialVelocityXZ = 0.0f;
	m_tangentialAccelerationXZ = 0.0f;
	m_tangentialVelocityYZ = 0.0f;
	m_tangentialAccelerationYZ = 0.0f;
	m_createEmitters = false;
	m_createEmitterName = "";
}

BlockParticleEmitter::~BlockParticleEmitter()
{
	if(m_pParent)
	{
		m_pParent->RemoveEmitterFromParticleEffect(this);
	}

	m_pBlockParticleManager->RemoveEmitterLinkage(this);
}

void BlockParticleEmitter::CopyParams(BlockParticleEmitter* EmitterParams)
{
	m_startingPosition = EmitterParams->m_startingPosition;
	m_position = EmitterParams->m_position;

	// Creation Time
	m_creationTime = EmitterParams->m_creationTime;
	m_creationTimer = EmitterParams->m_creationTimer;

	// Number of particles to spawn
	m_numParticlesToSpawn = EmitterParams->m_numParticlesToSpawn;

	// Should particles follow emitter movement
	m_particlesFollowEmitter = EmitterParams->m_particlesFollowEmitter;

	// Emitter orbiting
	m_orbit = EmitterParams->m_orbit;
	m_orbitReverse = EmitterParams->m_orbitReverse;
	m_orbitRadius = EmitterParams->m_orbitRadius;
	m_orbitTime = EmitterParams->m_orbitTime;
	m_orbitTimer = EmitterParams->m_orbitTimer;

	// Playback controls
	m_started = EmitterParams->m_started;
	m_paused = EmitterParams->m_paused;

	// Emitter type
	m_emitterType = EmitterParams->m_emitterType;

	// Emitter radius and lengths
	m_emitterRadius = EmitterParams->m_emitterRadius;
	m_emitterLengthX = EmitterParams->m_emitterLengthX;
	m_emitterLengthY = EmitterParams->m_emitterLengthY;
	m_emitterLengthZ = EmitterParams->m_emitterLengthZ;

	// Are we spawning on the outside of the shape
	m_spawnOutline = EmitterParams->m_spawnOutline;

	// Particle creation params
	m_gravityDirection = EmitterParams->m_gravityDirection;
	m_gravityMultiplier = EmitterParams->m_gravityMultiplier;
	m_pointOrigin = EmitterParams->m_pointOrigin;
	m_startScale = EmitterParams->m_startScale;
	m_startScaleVariance = EmitterParams->m_startScaleVariance;
	m_endScale = EmitterParams->m_endScale;
	m_endScaleVariance = EmitterParams->m_endScaleVariance;
	m_startRed = EmitterParams->m_startRed;
	m_startRedVariance = EmitterParams->m_startRedVariance;
	m_endRed = EmitterParams->m_endRed;
	m_endRedVariance = EmitterParams->m_endRedVariance;
	m_startGreen = EmitterParams->m_startGreen;
	m_startGreenVariance = EmitterParams->m_startGreenVariance;
	m_endGreen = EmitterParams->m_endGreen;
	m_endGreenVariance = EmitterParams->m_endGreenVariance;
	m_startBlue = EmitterParams->m_startBlue;
	m_startBlueVariance = EmitterParams->m_startBlueVariance;
	m_endBlue = EmitterParams->m_endBlue;
	m_endBlueVariance = EmitterParams->m_endBlueVariance;
	m_startAlpha = EmitterParams->m_startAlpha;
	m_startAlphaVariance = EmitterParams->m_startAlphaVariance;
	m_endAlpha = EmitterParams->m_endAlpha;
	m_endAlphaVariance = EmitterParams->m_endAlphaVariance;
	m_lifeTime = EmitterParams->m_lifeTime;
	m_lifeTimeVariance = EmitterParams->m_lifeTimeVariance;
	m_velocityTowardsPoint = EmitterParams->m_velocityTowardsPoint;
	m_accelerationTowardsPoint = EmitterParams->m_accelerationTowardsPoint;
	m_checkWorldCollisions = EmitterParams->m_checkWorldCollisions;
	m_destoryOnCollision = EmitterParams->m_destoryOnCollision;
	m_startLifeDecayOnCollision = EmitterParams->m_startLifeDecayOnCollision;
	m_startVelocity = EmitterParams->m_startVelocity;
	m_startVelocityVariance = EmitterParams->m_startVelocityVariance;
	m_randomStartRotation = EmitterParams->m_randomStartRotation;
	m_startRotation = EmitterParams->m_startRotation;
	m_startAngularVelocity = EmitterParams->m_startAngularVelocity;
	m_startAngularVelocityVariance = EmitterParams->m_startAngularVelocityVariance;
	m_tangentialVelocityXY = EmitterParams->m_tangentialVelocityXY;
	m_tangentialAccelerationXY = EmitterParams->m_tangentialAccelerationXY;
	m_tangentialVelocityXZ = EmitterParams->m_tangentialVelocityXZ;
	m_tangentialAccelerationXZ = EmitterParams->m_tangentialAccelerationXZ;
	m_tangentialVelocityYZ = EmitterParams->m_tangentialVelocityYZ;
	m_tangentialAccelerationYZ = EmitterParams->m_tangentialAccelerationYZ;
	m_createEmitters = EmitterParams->m_createEmitters;
	m_createEmitterName = EmitterParams->m_createEmitterName;
	m_isReferenceEmitter = false;

	// Parent effect
	m_pParent = EmitterParams->m_pParent;

	if(m_pParent)
	{
		m_pParent->AddEmitterToParticleEffect(this);
	}
}

// Update
void BlockParticleEmitter::Update(float dt)
{
	if(m_isReferenceEmitter)
	{
		// If we are a reference emitter, don't do anything
		return;
	}

	if(m_paused)
	{
		// If we are paused, return
		return;
	}

	if(m_erase)
	{
		return;
	}

	if(m_creationTimer <= 0.0f)
	{
		m_creationTimer = m_creationTime;

		if(m_started) // Only create particles if we have been started
		{
			for(int i = 0; i < m_numParticlesToSpawn; i++)
			{
				BlockParticle* pParticle = m_pBlockParticleManager->CreateBlockParticleFromEmitterParams(this);
			}
		}
	}
	else
	{
		m_creationTimer -= dt;
	}

	if(m_orbit)
	{
		m_orbitTimer += dt;
		if(m_orbitTimer >= m_orbitTime)
		{
			m_orbitTimer = 0.0f;
		}

		float ratio = m_orbitTimer / m_orbitTime;
		float rot_angle = DegToRad((ratio * 360.0f));
		if(m_orbitReverse)
			rot_angle = -rot_angle;

		m_position = m_startingPosition + vec3(cos(rot_angle) * m_orbitRadius, 0.0f, sin(rot_angle) * m_orbitRadius);
	}
}

// Rendering
void BlockParticleEmitter::Render()
{
	if(m_isReferenceEmitter)
	{
		// If we are a reference emitter, don't do anything
		return;
	}

	if(m_erase)
	{
		return;
	}

	// Emitter
	m_pRenderer->PushMatrix();
		if(m_pParent != nullptr && m_pParentParticle == nullptr)
		{
			m_pRenderer->TranslateWorldMatrix(m_pParent->m_position.x, m_pParent->m_position.y, m_pParent->m_position.z);
		}
		m_pRenderer->TranslateWorldMatrix(m_position.x, m_position.y, m_position.z);

		float l_length = m_emitterLengthX;
		float l_height = m_emitterLengthY;
		float l_width = m_emitterLengthZ;

		m_pRenderer->SetRenderMode(RM_WIREFRAME);
		m_pRenderer->SetCullMode(CM_NOCULL);
		m_pRenderer->SetLineWidth(1.0f);

		if(m_emitterType == EmitterType_Point)
		{
			float pointLength = 0.5f;
			m_pRenderer->SetRenderMode(RM_SOLID);
			m_pRenderer->EnableImmediateMode(IM_LINES);
				m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
				m_pRenderer->ImmediateVertex(-pointLength, 0.0f, 0.0f);
				m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
				m_pRenderer->ImmediateVertex(pointLength, 0.0f, 0.0f);
				m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
				m_pRenderer->ImmediateVertex(0.0f, -pointLength, 0.0f);
				m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
				m_pRenderer->ImmediateVertex(0.0f, pointLength, 0.0f);
				m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
				m_pRenderer->ImmediateVertex(0.0f, 0.0f, -pointLength);
				m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
				m_pRenderer->ImmediateVertex(0.0f, 0.0f, pointLength);
			m_pRenderer->DisableImmediateMode();
		}
		else if(m_emitterType == EmitterType_Square)
		{
			m_pRenderer->EnableImmediateMode(IM_QUADS);
				m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
				m_pRenderer->ImmediateNormal(0.0f, -1.0f, 0.0f);
				m_pRenderer->ImmediateVertex(-l_length, 0.0f, -l_width);
				m_pRenderer->ImmediateVertex(l_length, 0.0f, -l_width);
				m_pRenderer->ImmediateVertex(l_length, 0.0f, l_width);
				m_pRenderer->ImmediateVertex(-l_length, 0.0f, l_width);
			m_pRenderer->DisableImmediateMode();
		}
		else if(m_emitterType == EmitterType_Cube)
		{
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
		}
		else if(m_emitterType == EmitterType_Circle)
		{
			float radius = m_emitterRadius;

			m_pRenderer->PushMatrix();
				m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
				m_pRenderer->DrawLineCircle(radius, 30);
			m_pRenderer->PopMatrix();
		}
		else if(m_emitterType == EmitterType_Sphere)
		{
			float radius = m_emitterRadius;

			m_pRenderer->PushMatrix();
				m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
				m_pRenderer->DrawSphere(radius, 30, 30);
			m_pRenderer->PopMatrix();
		}
		else if(m_emitterType == EmitterType_Mesh)
		{
		}

		m_pRenderer->SetCullMode(CM_BACK);
	m_pRenderer->PopMatrix();
	
	// Origin Point
	m_pRenderer->PushMatrix();
		if(m_pParent != nullptr && m_pParentParticle == nullptr)
		{
			m_pRenderer->TranslateWorldMatrix(m_pParent->m_position.x, m_pParent->m_position.y, m_pParent->m_position.z);
		}
		if(m_pParentParticle != nullptr)
		{
			m_pRenderer->TranslateWorldMatrix(m_pParentParticle->m_position.x, m_pParentParticle->m_position.y, m_pParentParticle->m_position.z);
		}
		if(m_particlesFollowEmitter)
		{
			m_pRenderer->TranslateWorldMatrix(m_position.x, m_position.y, m_position.z);
		}
		m_pRenderer->TranslateWorldMatrix(m_pointOrigin.x, m_pointOrigin.y, m_pointOrigin.z);

		l_length = 0.5f;

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
}
