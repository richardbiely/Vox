// ******************************************************************************
//
// Filename:	BlockParticle.h
// Project:		Game
// Author:		Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 09/11/14
//
// Copyright (c) 2005-2011, Steven Ball
//
// ******************************************************************************

#pragma once

#include "../Maths/3dmaths.h"

class BlockParticleEmitter;


class BlockParticle
{
public:
	/* Public methods */
	BlockParticle();
	~BlockParticle();

	void CreateStartingParams();

	void CalculateWorldTransformMatrix();

	void SetFreezeDelayTimer(float timer);
	void SetWaitAfterUpdateCompleteTimer(float timer);

	// Update
	void Update(float dt);

protected:
	/* Protected methods */

private:
	/* Private methods */

public:
	/* Public members */
	bool m_erase;

	// Floor Sliding flag (set manually)
	bool m_allowFloorSliding;

	// Position
	Vector3d m_position;

	// Velocity
	Vector3d m_velocity;
	Vector3d m_startVelocityVariance;

	// Rotation
	Vector3d m_rotation;
	Vector3d m_startRotation;
	bool m_randomStartRotation;

	// Angular velocity
	Vector3d m_angularVelocity;
	Vector3d m_startAngularVelocityVariance;

	// Gravity
	Vector3d m_gravityDirection;
	float m_gravityMultiplier;

	// World matrix
	Matrix4x4 m_worldMatrix;

	// Particle colour
	float m_currentRed;
	float m_startRed;
	float m_startRedVariance;
	float m_endRed;
	float m_endRedVariance;
	float m_currentGreen;
	float m_startGreen;
	float m_startGreenVariance;
	float m_endGreen;
	float m_endGreenVariance;
	float m_currentBlue;
	float m_startBlue;
	float m_startBlueVariance;
	float m_endBlue;
	float m_endBlueVariance;
	float m_currentAlpha;
	float m_startAlpha;
	float m_startAlphaVariance;
	float m_endAlpha;
	float m_endAlphaVariance;

	// Particle scale
	float m_currentScale;
	float m_startScale;
	float m_startScaleVariance;
	float m_endScale;
	float m_endScaleVariance;

	// Velocity towards point origin
	Vector3d m_pointOrigin;
	Vector3d m_pointVelocity;
	float m_velocityTowardsPoint;
	float m_accelerationTowardsPoint;

	// Tangential velocity
	Vector3d m_tangentialVelocity;
	float m_tangentialVelocityXY;
	float m_tangentialAccelerationXY;
	float m_tangentialVelocityXZ;
	float m_tangentialAccelerationXZ;
	float m_tangentialVelocityYZ;
	float m_tangentialAccelerationYZ;

	// Lifetime
	float m_lifeTime;
	float m_maxLifeTime;
	float m_lifeTimeVariance;

	// Freeze update / delay
	float m_freezeUpdateTimer;

	// Wait after complete
	float m_waitAfterUpdateCompleteTimer;

	// Does particle collide with the world
	bool m_checkWorldCollisions;

	// Do we destroy this particle instantly on collision with world
	bool m_destoryOnCollision;
	
	// Do we only start the life decay after we collide
	bool m_startLifeDecayOnCollision;
	bool m_hasCollided;

	// Grid
	int m_gridPositionX;
	int m_gridPositionY;
	int m_gridPositionZ;

	// Emitter parent
	BlockParticleEmitter* m_pParent;

	// Create Emitters instead of particles
	bool m_createEmitters;
	BlockParticleEmitter* m_pCreatedEmitter;

protected:
	/* Protected members */

public:
	/* Private members */
};
