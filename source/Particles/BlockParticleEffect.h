// ******************************************************************************
// Filename:    BlockParticleEffect.h
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

#pragma once

#include "../Maths/3dmaths.h"
#include "../Renderer/Renderer.h"

#include <vector>
using namespace std;

class BlockParticleManager;
class BlockParticleEmitter;

typedef vector<BlockParticleEmitter*> BlockParticlesEmitterList;

class BlockParticleEffect
{
public:
	/* Public methods */
	BlockParticleEffect(Renderer* pRenderer, BlockParticleManager* pBlockParticleManager);
	~BlockParticleEffect();

	void ClearEmitters();

	void SetPosition(vec3 pos);
	vec3 GetPosition() const;

	void Import(const char* fileName);
	void Export(const char* fileName);

	void PlayEffect();
	void StopEffect();
	void PauseEffect();

	bool IsStarted() const;
	bool IsPaused() const;

	void AddEmitterToParticleEffect(BlockParticleEmitter* pEmitter);
	void RemoveEmitterFromParticleEffect(BlockParticleEmitter* pEmitter);

	BlockParticleEmitter* GetEmitter(string emitterName);

	// Update
	void Update(float dt);

	// Rendering
	void Render();

protected:
	/* Protected methods */

private:
	/* Private methods */

public:
	/* Public members */
	bool m_erase;

	// Effect Id
	unsigned int m_particleEffectId;

	// Name
	string m_effectName;

	// Position
	vec3 m_position;

	// Position with no world offset
	vec3 m_position_NoWorldOffset;

	// Flag to indicate if we should render particles created from this particle effect in the no world offset viewports (characterGUI)
	bool m_renderNoWoldOffsetViewport;

	// Life
	bool m_endless;
	float m_lifeTime;
	float m_lifeTimer;

	// Playback controls
	bool m_started;
	bool m_paused;

protected:
	/* Protected members */

public:
	/* Private members */
	Renderer* m_pRenderer;
	BlockParticleManager* m_pBlockParticleManager;

	BlockParticlesEmitterList m_vpBlockParticleEmittersList;
};
