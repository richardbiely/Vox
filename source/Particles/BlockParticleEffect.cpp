// ******************************************************************************
// Filename:    BlockParticleEffect.cpp
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

#include "BlockParticleEffect.h"
#include "BlockParticleEmitter.h"
#include "BlockParticleManager.h"

#include "../utils/Random.h"

#include <fstream>
#include <ostream>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
using namespace std;


BlockParticleEffect::BlockParticleEffect(Renderer* pRenderer, BlockParticleManager* pBlockParticleManager)
{
	m_pRenderer = pRenderer;
	m_pBlockParticleManager = pBlockParticleManager;

	m_particleEffectId = -1;

	m_erase = false;

	m_started = false;
	m_paused = false;

	m_endless = true;
	m_lifeTime = 1.0f;
	m_lifeTimer = m_lifeTime;

	m_renderNoWoldOffsetViewport = false;
}

BlockParticleEffect::~BlockParticleEffect()
{
	ClearEmitters();
}

void BlockParticleEffect::ClearEmitters()
{
	for(unsigned int i = 0; i < m_vpBlockParticleEmittersList.size(); i++)
	{
		if(m_vpBlockParticleEmittersList[i]->m_erase == false)
		{
			m_pBlockParticleManager->RemoveEmitterLinkage(m_vpBlockParticleEmittersList[i]);
			m_vpBlockParticleEmittersList[i]->m_pParent = nullptr; // Remove parent link, since we have deleted this effect
			m_vpBlockParticleEmittersList[i]->m_erase = true;
		}
	}
}

void BlockParticleEffect::SetPosition(vec3 pos)
{
	m_position = pos;
}

vec3 BlockParticleEffect::GetPosition() const
{
	return m_position;
}

void BlockParticleEffect::Import(const char* fileName)
{
	ifstream importFile;

	// Open the file
	importFile.open(fileName, ios::in);

	if(importFile.is_open())
	{
		string tempString;

		string effectName;
		importFile >> tempString >> effectName;
		m_effectName = effectName;

		importFile >> tempString >> m_endless;
		importFile >> tempString >> m_lifeTime;
		m_lifeTimer = m_lifeTime;

		int numEmitters;
		importFile >> tempString >> numEmitters;
		for(int i = 0; i < numEmitters; i++)
		{
			string emitterName;
			importFile >> tempString >> emitterName;
			float x, y, z;
			importFile >> tempString >> x >> y >> z;
			vec3 position = vec3(x, y, z);
			float creationTime;
			importFile >> tempString >> creationTime;
			int numParticlesToSpawn;
			importFile >> tempString >> numParticlesToSpawn;
			bool particlesFollowEmitter;
			importFile >> tempString >> particlesFollowEmitter;
			bool orbit;
			importFile >> tempString >> orbit;
			bool  orbitReverse;
			importFile >> tempString >> orbitReverse;
			float orbitRadius;
			importFile >> tempString >> orbitRadius;
			float orbitTime;
			importFile >> tempString >> orbitTime;
			int eType;
			importFile >> tempString >> eType;
			EmitterType emitterType = (EmitterType)eType;
			float emitterRadius;
			importFile >> tempString >> emitterRadius;
			float emitterLengthX;
			importFile >> tempString >> emitterLengthX;
			float emitterLengthY;
			importFile >> tempString >> emitterLengthY;
			float emitterLengthZ;
			importFile >> tempString >> emitterLengthZ;
			bool spawnOutline;
			importFile >> tempString >> spawnOutline;
			importFile >> tempString >> x >> y >> z;
			vec3 gravityDirection = vec3(x, y, z);
			float gravityMultiplier;
			importFile >> tempString >> gravityMultiplier;
			importFile >> tempString >> x >> y >> z;
			vec3 pointOrigin = vec3(x, y, z);
			float startScale;
			importFile >> tempString >> startScale;
			float startScaleVariance;
			importFile >> tempString >> startScaleVariance;
			float endScale;
			importFile >> tempString >> endScale;
			float endScaleVariance;
			importFile >> tempString >> endScaleVariance;
			float startRed;
			importFile >> tempString >> startRed;
			float startRedVariance;
			importFile >> tempString >> startRedVariance;
			float endRed;
			importFile >> tempString >> endRed;
			float endRedVariance;
			importFile >> tempString >> endRedVariance;
			float startGreen;
			importFile >> tempString >> startGreen;
			float startGreenVariance;
			importFile >> tempString >> startGreenVariance;
			float endGreen;
			importFile >> tempString >> endGreen;
			float endGreenVariance;
			importFile >> tempString >> endGreenVariance;
			float startBlue;
			importFile >> tempString >> startBlue;
			float startBlueVariance;
			importFile >> tempString >> startBlueVariance;
			float endBlue;
			importFile >> tempString >> endBlue;
			float endBlueVariance;
			importFile >> tempString >> endBlueVariance;
			float startAlpha;
			importFile >> tempString >> startAlpha;
			float startAlphaVariance;
			importFile >> tempString >> startAlphaVariance;
			float endAlpha;
			importFile >> tempString >> endAlpha;
			float endAlphaVariance;
			importFile >> tempString >> endAlphaVariance;
			float lifeTime;
			importFile >> tempString >> lifeTime;
			float lifeTimeVariance;
			importFile >> tempString >> lifeTimeVariance;
			float velocityTowardsPoint;
			importFile >> tempString >> velocityTowardsPoint;
			float accelerationTowardsPoint;
			importFile >> tempString >> accelerationTowardsPoint;
			bool checkWorldCollisions;
			importFile >> tempString >> checkWorldCollisions;
			bool destoryOnCollision;
			importFile >> tempString >> destoryOnCollision;
			bool startLifeDecayOnCollision;
			importFile >> tempString >> startLifeDecayOnCollision;
			importFile >> tempString >> x >> y >> z;
			vec3 startVelocity = vec3(x, y, z);
			importFile >> tempString >> x >> y >> z;
			vec3 startVelocityVariance = vec3(x, y, z);
			bool randomStartRotation;
			importFile >> tempString >> randomStartRotation;
			importFile >> tempString >> x >> y >> z;
			vec3 startRotation = vec3(x, y, z);
			importFile >> tempString >> x >> y >> z;
			vec3 startAngularVelocity = vec3(x, y, z);
			importFile >> tempString >> x >> y >> z;
			vec3 startAngularVelocityVariance = vec3(x, y, z);
			float tangentialVelocityXY;
			importFile >> tempString >> tangentialVelocityXY;
			float tangentialAccelerationXY;
			importFile >> tempString >> tangentialAccelerationXY;
			float tangentialVelocityXZ;
			importFile >> tempString >> tangentialVelocityXZ;
			float tangentialAccelerationXZ;
			importFile >> tempString >> tangentialAccelerationXZ;
			float tangentialVelocityYZ;
			importFile >> tempString >> tangentialVelocityYZ;
			float tangentialAccelerationYZ;
			importFile >> tempString >> tangentialAccelerationYZ;

			bool createEmitters;
			importFile >> tempString >> createEmitters;
			string createEmitterName;
			importFile >> tempString >> createEmitterName;
			bool isReferenceEmitter;
			importFile >> tempString >> isReferenceEmitter;

			// Create temp emitter
			BlockParticleEmitter* pNewEmitter = m_pBlockParticleManager->CreateBlockParticleEmitter(emitterName, position);

			pNewEmitter->m_emitterName = emitterName;

			pNewEmitter->m_startingPosition = position;
			pNewEmitter->m_position = position;

			pNewEmitter->m_creationTime = creationTime;
			pNewEmitter->m_creationTimer = 0.0f;

			pNewEmitter->m_numParticlesToSpawn = numParticlesToSpawn;

			pNewEmitter->m_particlesFollowEmitter = particlesFollowEmitter;

			pNewEmitter->m_orbit = orbit;
			pNewEmitter->m_orbitReverse = orbitReverse;
			pNewEmitter->m_orbitRadius = orbitRadius;
			pNewEmitter->m_orbitTime = orbitTime;
			pNewEmitter->m_orbitTimer = orbitTime;
			pNewEmitter->m_emitterType = emitterType;

			pNewEmitter->m_emitterRadius = emitterRadius;
			pNewEmitter->m_emitterLengthX = emitterLengthX;
			pNewEmitter->m_emitterLengthY = emitterLengthY;
			pNewEmitter->m_emitterLengthZ = emitterLengthZ;

			pNewEmitter->m_spawnOutline = spawnOutline;
			pNewEmitter->m_gravityDirection = gravityDirection;
			pNewEmitter->m_gravityMultiplier = gravityMultiplier;
			pNewEmitter->m_pointOrigin = pointOrigin;
			pNewEmitter->m_startScale = startScale;
			pNewEmitter->m_startScaleVariance = startScaleVariance;
			pNewEmitter->m_endScale = endScale;
			pNewEmitter->m_endScaleVariance = endScaleVariance;
			pNewEmitter->m_startRed = startRed;
			pNewEmitter->m_startRedVariance = startRedVariance;
			pNewEmitter->m_endRed = endRed;
			pNewEmitter->m_endRedVariance = endRedVariance;
			pNewEmitter->m_startGreen = startGreen;
			pNewEmitter->m_startGreenVariance = startGreenVariance;
			pNewEmitter->m_endGreen = endGreen;
			pNewEmitter->m_endGreenVariance = endGreenVariance;
			pNewEmitter->m_startBlue = startBlue;
			pNewEmitter->m_startBlueVariance = startBlueVariance;
			pNewEmitter->m_endBlue = endBlue;
			pNewEmitter->m_endBlueVariance = endBlueVariance;
			pNewEmitter->m_startAlpha = startAlpha;
			pNewEmitter->m_startAlphaVariance = startAlphaVariance;
			pNewEmitter->m_endAlpha = endAlpha;
			pNewEmitter->m_endAlphaVariance = endAlphaVariance;
			pNewEmitter->m_lifeTime = lifeTime;
			pNewEmitter->m_lifeTimeVariance = lifeTimeVariance;
			pNewEmitter->m_velocityTowardsPoint = velocityTowardsPoint;
			pNewEmitter->m_accelerationTowardsPoint = accelerationTowardsPoint;
			pNewEmitter->m_checkWorldCollisions = checkWorldCollisions;
			pNewEmitter->m_destoryOnCollision = destoryOnCollision;
			pNewEmitter->m_startLifeDecayOnCollision = startLifeDecayOnCollision;
			pNewEmitter->m_startVelocity = startVelocity;
			pNewEmitter->m_startVelocityVariance = startVelocityVariance;
			pNewEmitter->m_randomStartRotation = randomStartRotation;
			pNewEmitter->m_startRotation = startRotation;
			pNewEmitter->m_startAngularVelocity = startAngularVelocity;
			pNewEmitter->m_startAngularVelocityVariance = startAngularVelocityVariance;
			pNewEmitter->m_tangentialVelocityXY = tangentialVelocityXY;
			pNewEmitter->m_tangentialAccelerationXY = tangentialAccelerationXY;
			pNewEmitter->m_tangentialVelocityXZ = tangentialVelocityXZ;
			pNewEmitter->m_tangentialAccelerationXZ = tangentialAccelerationXZ;
			pNewEmitter->m_tangentialVelocityYZ = tangentialVelocityYZ;
			pNewEmitter->m_tangentialAccelerationYZ = tangentialAccelerationYZ;

			pNewEmitter->m_createEmitters = createEmitters;
			pNewEmitter->m_createEmitterName = createEmitterName;
			pNewEmitter->m_isReferenceEmitter = isReferenceEmitter;

			pNewEmitter->m_pParent = this;

			m_vpBlockParticleEmittersList.push_back(pNewEmitter);
		}
	}
}

void BlockParticleEffect::Export(const char* fileName)
{
	ofstream exportFile;

	// Open the file
	exportFile.open(fileName, ios::out);

	if(exportFile.is_open())
	{
		exportFile << "effectName: " << m_effectName << "\n\n";

		int numEmitters = (int)m_vpBlockParticleEmittersList.size();
		exportFile << "numEmitters: " << numEmitters << "\n";
		for (BlockParticleEmitter* pEmitter : m_vpBlockParticleEmittersList)
		{
			exportFile << "emitterName: " << pEmitter->m_emitterName << "\n";
			exportFile << "emitterPosition: " << pEmitter->m_position.x << " " << pEmitter->m_position.y << " " << pEmitter->m_position.z << "\n";
			exportFile << "creationTime: " << pEmitter->m_creationTime << "\n";
			exportFile << "numParticles: " << pEmitter->m_numParticlesToSpawn << "\n";
			exportFile << "followEmitter: " << pEmitter->m_particlesFollowEmitter << "\n";
			exportFile << "orbit: " << pEmitter->m_orbit << "\n";
			exportFile << "orbitReverse: " << pEmitter->m_orbitReverse << "\n";
			exportFile << "orbitRadius: " << pEmitter->m_orbitRadius << "\n";
			exportFile << "orbitTime: " << pEmitter->m_orbitTime << "\n";
			exportFile << "emitterType: " << (int)pEmitter->m_emitterType << "\n";
			exportFile << "emitterRadius: " << pEmitter->m_emitterRadius << "\n";
			exportFile << "emitterLengthX: " << pEmitter->m_emitterLengthX << "\n";
			exportFile << "emitterLengthY: " << pEmitter->m_emitterLengthY << "\n";
			exportFile << "emitterLengthZ: " << pEmitter->m_emitterLengthZ << "\n";
			exportFile << "spawnOutline: " << pEmitter->m_spawnOutline << "\n";
			exportFile << "gravityDir: " << pEmitter->m_gravityDirection.x << " " << pEmitter->m_gravityDirection.y << " " << pEmitter->m_gravityDirection.z << "\n";
			exportFile << "gravityMultiplier: " << pEmitter->m_gravityMultiplier << "\n";
			exportFile << "pointOrigin: " << pEmitter->m_pointOrigin.x << " " << pEmitter->m_pointOrigin.y << " " << pEmitter->m_pointOrigin.z << "\n";
			exportFile << "startScale: " << pEmitter->m_startScale << "\n";
			exportFile << "startScaleVariance: " << pEmitter->m_startScaleVariance << "\n";
			exportFile << "endScale: " << pEmitter->m_endScale << "\n";
			exportFile << "endScaleVariance: " << pEmitter->m_endScaleVariance << "\n";
			exportFile << "startRed: " << pEmitter->m_startRed << "\n";
			exportFile << "startRedVariance: " << pEmitter->m_startRedVariance << "\n";
			exportFile << "endRed: " << pEmitter->m_endRed << "\n";
			exportFile << "endRedVariance: " << pEmitter->m_endRedVariance << "\n";
			exportFile << "startGreen: " << pEmitter->m_startGreen << "\n";
			exportFile << "startGreenVariance: " << pEmitter->m_startGreenVariance << "\n";
			exportFile << "endGreen: " << pEmitter->m_endGreen << "\n";
			exportFile << "endGreenVariance: " << pEmitter->m_endGreenVariance << "\n";
			exportFile << "startBlue: " << pEmitter->m_startBlue << "\n";
			exportFile << "startBlueVariance: " << pEmitter->m_startBlueVariance << "\n";
			exportFile << "endBlue: " << pEmitter->m_endBlue << "\n";
			exportFile << "endBlueVariance: " << pEmitter->m_endBlueVariance << "\n";
			exportFile << "startAlpha: " << pEmitter->m_startAlpha << "\n";
			exportFile << "startAlphaVariance: " << pEmitter->m_startAlphaVariance << "\n";
			exportFile << "endAlpha: " << pEmitter->m_endAlpha << "\n";
			exportFile << "endAlphaVariance: " << pEmitter->m_endAlphaVariance << "\n";
			exportFile << "lifeTime: " << pEmitter->m_lifeTime << "\n";
			exportFile << "lifeTimeVariance: " << pEmitter->m_lifeTimeVariance << "\n";
			exportFile << "velocityTowardsPoint: " << pEmitter->m_velocityTowardsPoint << "\n";
			exportFile << "accelerationTowardsPoint: " << pEmitter->m_accelerationTowardsPoint << "\n";
			exportFile << "worldCollisions: " << pEmitter->m_checkWorldCollisions << "\n";
			exportFile << "destoryOnCollision: " << pEmitter->m_destoryOnCollision << "\n";
			exportFile << "decayOnCollision: " << pEmitter->m_startLifeDecayOnCollision << "\n";
			exportFile << "startVelocity: " << pEmitter->m_startVelocity.x << " " << pEmitter->m_startVelocity.y << " " << pEmitter->m_startVelocity.z << "\n";
			exportFile << "startVelocityVariance: " << pEmitter->m_startVelocityVariance.x << " " << pEmitter->m_startVelocityVariance.y << " " << pEmitter->m_startVelocityVariance.z << "\n";
			exportFile << "randomStartRotation: " << pEmitter->m_randomStartRotation << "\n";
			exportFile << "startAngularVelocity: " << pEmitter->m_startAngularVelocity.x << " " << pEmitter->m_startAngularVelocity.y << " " << pEmitter->m_startAngularVelocity.z << "\n";
			exportFile << "startAngularVelocityVariance: " << pEmitter->m_startAngularVelocityVariance.x << " " << pEmitter->m_startAngularVelocityVariance.y << " " << pEmitter->m_startAngularVelocityVariance.z << "\n";
			exportFile << "tangentialVelocityXY: " << pEmitter->m_tangentialVelocityXY << "\n";
			exportFile << "tangentialAccelerationXY: " << pEmitter->m_tangentialAccelerationXY << "\n";
			exportFile << "tangentialVelocityXZ: " << pEmitter->m_tangentialVelocityXZ << "\n";
			exportFile << "tangentialAccelerationXZ: " << pEmitter->m_tangentialAccelerationXZ << "\n";
			exportFile << "tangentialVelocityYZ: " << pEmitter->m_tangentialVelocityYZ << "\n";
			exportFile << "tangentialAccelerationYZ: " << pEmitter->m_tangentialAccelerationYZ << "\n";
			exportFile << "creteEmitter: " << pEmitter->m_createEmitters << "\n";
			exportFile << "createEmitterName: " << pEmitter->m_createEmitterName << "\n";
			exportFile << "isReferenceEmitter: " << pEmitter->m_isReferenceEmitter << "\n";
		}
	}
}

void BlockParticleEffect::PlayEffect()
{
	if(m_endless == false)
	{
		m_lifeTimer = m_lifeTime;
	}

	m_started = true;

	for(unsigned int i = 0; i < m_vpBlockParticleEmittersList.size(); i++)
	{
		if(m_vpBlockParticleEmittersList[i]->m_started == false)
		{
			m_vpBlockParticleEmittersList[i]->m_creationTimer = 0.0f;
		}

		m_vpBlockParticleEmittersList[i]->m_started = true;
	}
}

void BlockParticleEffect::StopEffect()
{
	m_started = false;

	for(unsigned int i = 0; i < m_vpBlockParticleEmittersList.size(); i++)
	{
		if(m_vpBlockParticleEmittersList[i]->m_pParentParticle != nullptr)
		{
			// Don't stop emitters that are connected to a particle
			continue;
		}

		m_vpBlockParticleEmittersList[i]->m_started = false;
	}
}

void BlockParticleEffect::PauseEffect()
{
	m_paused = !m_paused;

	for(unsigned int i = 0; i < m_vpBlockParticleEmittersList.size(); i++)
	{
		m_vpBlockParticleEmittersList[i]->m_paused = m_paused;
	}
}

bool BlockParticleEffect::IsStarted() const
{
	return m_started;
}

bool BlockParticleEffect::IsPaused() const
{
	return m_paused;
}

void BlockParticleEffect::AddEmitterToParticleEffect(BlockParticleEmitter* pEmitter)
{
	m_vpBlockParticleEmittersList.push_back(pEmitter);
}

void BlockParticleEffect::RemoveEmitterFromParticleEffect(BlockParticleEmitter* pEmitter)
{
	BlockParticlesEmitterList::iterator iter = std::find(m_vpBlockParticleEmittersList.begin(), m_vpBlockParticleEmittersList.end(), pEmitter);
	if(iter != m_vpBlockParticleEmittersList.end())
	{
		m_vpBlockParticleEmittersList.erase(iter);
	}
}

BlockParticleEmitter* BlockParticleEffect::GetEmitter(string emitterName)
{
	for(unsigned int i = 0; i < m_vpBlockParticleEmittersList.size(); i++)
	{
		if(m_vpBlockParticleEmittersList[i]->m_emitterName == emitterName)
		{
			BlockParticleEmitter* pReturn = m_vpBlockParticleEmittersList[i];

			return pReturn;
		}
	}

	return nullptr;
}

// Update
void BlockParticleEffect::Update(float dt)
{
	if(m_started == false || m_paused == true)
	{
		return;
	}

	if(m_endless == false)
	{
		if(m_lifeTimer <= 0.0f)
		{
			StopEffect();
			
			m_erase = true;
		}
		else
		{
			m_lifeTimer -= dt;
		}
	}
}

// Rendering
void BlockParticleEffect::Render()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->TranslateWorldMatrix(m_position.x, m_position.y, m_position.z);

		float l_length = 0.5f;

		m_pRenderer->SetRenderMode(RM_SOLID);
		m_pRenderer->SetLineWidth(2.0f);
		m_pRenderer->EnableImmediateMode(IM_LINES);
			m_pRenderer->ImmediateColourAlpha(0.0f, 1.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateVertex(-l_length, 0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(l_length, 0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(0.0f, -l_length, 0.0f);
			m_pRenderer->ImmediateVertex(0.0f, l_length, 0.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, -l_length);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, l_length);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();
}
