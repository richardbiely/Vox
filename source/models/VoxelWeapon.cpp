// ******************************************************************************
// Filename:    VoxelWeapon.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 12/08/14
//
// Copyright (c) 2005-2015, Steven Ball
// ******************************************************************************

#include "VoxelWeapon.h"

#include <fstream>
#include <ostream>
#include <iostream>
#include <string>
using namespace std;


VoxelWeapon::VoxelWeapon(Renderer* pRenderer, QubicleBinaryManager* pQubicleBinaryManager)
{
	m_pRenderer = pRenderer;
	m_pQubicleBinaryManager = pQubicleBinaryManager;

	Reset(); 

	m_numLights = 0;
	m_pLights = nullptr;

	m_numAnimatedSections = 0;
	m_pAnimatedSections = nullptr;

	m_numParticleEffects = 0;
	m_pParticleEffects = nullptr;

	m_numWeaponTrails = 0;
	m_pWeaponTrails = nullptr;
}

VoxelWeapon::~VoxelWeapon()
{
	UnloadWeapon();
	Reset();
}

void VoxelWeapon::Reset()
{
	m_pParentCharacter = nullptr;

	m_boneIndex = -1;

	m_matrixName = "";
	m_matrixIndex = -1;

	m_renderOffset = vec3(0.0f, 0.0f, 0.0f);

	m_renderScale = 1.0f;

	m_numLights = 0;
	m_numAnimatedSections = 0;
	m_numParticleEffects = 0;

	m_weaponTrailsStarted = false;

	m_weaponRadius = 1.0f;
	m_firstPersonMode = false;

	m_cameraYRotation = 0.0f;

	m_loaded = false;
}

bool VoxelWeapon::IsLoaded()
{
	return m_loaded;
}

// Rebuild
void VoxelWeapon::RebuildVoxelModel(bool faceMerge)
{
	for (int i = 0; i < m_numAnimatedSections; i++)
	{
		m_pAnimatedSections[i].m_pVoxelObject->RebuildVoxelModel(faceMerge);
	}
}

void VoxelWeapon::LoadWeapon(const char *weaponFilename, bool useManager)
{
	ifstream file;

	// Open the file
	file.open(weaponFilename, ios::in);
	if(file.is_open())
	{
		string tempString;

		file >> tempString >> m_renderOffset.x >> m_renderOffset.y >> m_renderOffset.z;

		file >> tempString >> m_renderScale;

		// Animated sections
		m_numAnimatedSections = 0;
		file >> tempString >> m_numAnimatedSections;
		if(m_numAnimatedSections > 0)
		{
			m_pAnimatedSections = new AnimatedSection[m_numAnimatedSections];
		}
		for(int i = 0; i < m_numAnimatedSections; i++)
		{
			m_pAnimatedSections[i].m_pVoxelObject = new VoxelObject();
			m_pAnimatedSections[i].m_pVoxelObject->SetRenderer(m_pRenderer);
			m_pAnimatedSections[i].m_pVoxelObject->SetQubicleBinaryManager(m_pQubicleBinaryManager);

			file >> tempString >> m_pAnimatedSections[i].m_fileName;
			m_pAnimatedSections[i].m_pVoxelObject->LoadObject(m_pAnimatedSections[i].m_fileName.c_str(), useManager);

			file >> tempString >> m_pAnimatedSections[i].m_renderScale;

			float offsetX = 0.0f;
			float offsetY = 0.0f;
			float offsetZ = 0.0f;
			file >> tempString >> offsetX >> offsetY >> offsetZ;
			m_pAnimatedSections[i].m_renderOffset = vec3(offsetX, offsetY, offsetZ);

			file >> tempString >> m_pAnimatedSections[i].m_autoStart;
			file >> tempString >> m_pAnimatedSections[i].m_loopingAnimation;
			m_pAnimatedSections[i].m_playingAnimation = m_pAnimatedSections[i].m_autoStart;

			// Translation
			file >> tempString >> m_pAnimatedSections[i].m_translateSpeedX;
			file >> tempString >> m_pAnimatedSections[i].m_translateSpeedY;
			file >> tempString >> m_pAnimatedSections[i].m_translateSpeedZ;

			file >> tempString >> m_pAnimatedSections[i].m_translateRangeXMin >> m_pAnimatedSections[i].m_translateRangeXMax;
			file >> tempString >> m_pAnimatedSections[i].m_translateRangeYMin >> m_pAnimatedSections[i].m_translateRangeYMax;
			file >> tempString >> m_pAnimatedSections[i].m_translateRangeZMin >> m_pAnimatedSections[i].m_translateRangeZMax;

			file >> tempString >> m_pAnimatedSections[i].m_translateSpeedTurnSpeedX;
			file >> tempString >> m_pAnimatedSections[i].m_translateSpeedTurnSpeedY;
			file >> tempString >> m_pAnimatedSections[i].m_translateSpeedTurnSpeedZ;

			m_pAnimatedSections[i].m_translateX = 0.0f;
			m_pAnimatedSections[i].m_translateY = 0.0f;
			m_pAnimatedSections[i].m_translateZ = 0.0f;

			m_pAnimatedSections[i].m_translateMaxSpeedX = m_pAnimatedSections[i].m_translateSpeedX;
			m_pAnimatedSections[i].m_translateMaxSpeedY = m_pAnimatedSections[i].m_translateSpeedY;
			m_pAnimatedSections[i].m_translateMaxSpeedZ = m_pAnimatedSections[i].m_translateSpeedZ;
			m_pAnimatedSections[i].m_translateXUp = true;
			m_pAnimatedSections[i].m_translateXDown = false;
			m_pAnimatedSections[i].m_translateYUp = true;
			m_pAnimatedSections[i].m_translateYDown = false;
			m_pAnimatedSections[i].m_translateZUp = true;
			m_pAnimatedSections[i].m_translateZDown = false;

			// Rotation
			float rotationPointX = 0.0f;
			float rotationPointY = 0.0f;
			float rotationPointZ = 0.0f;
			file >> tempString >> rotationPointX >> rotationPointY >> rotationPointZ;
			m_pAnimatedSections[i].m_rotationPoint = vec3(rotationPointX, rotationPointY, rotationPointZ);

			file >> tempString >> m_pAnimatedSections[i].m_rotationSpeedX;
			file >> tempString >> m_pAnimatedSections[i].m_rotationSpeedY;
			file >> tempString >> m_pAnimatedSections[i].m_rotationSpeedZ;

			file >> tempString >> m_pAnimatedSections[i].m_rotationRangeXMin >> m_pAnimatedSections[i].m_rotationRangeXMax;
			file >> tempString >> m_pAnimatedSections[i].m_rotationRangeYMin >> m_pAnimatedSections[i].m_rotationRangeYMax;
			file >> tempString >> m_pAnimatedSections[i].m_rotationRangeZMin >> m_pAnimatedSections[i].m_rotationRangeZMax;

			file >> tempString >> m_pAnimatedSections[i].m_rotationSpeedTurnSpeedX;
			file >> tempString >> m_pAnimatedSections[i].m_rotationSpeedTurnSpeedY;
			file >> tempString >> m_pAnimatedSections[i].m_rotationSpeedTurnSpeedZ;

			m_pAnimatedSections[i].m_rotationX = 0.0f;
			m_pAnimatedSections[i].m_rotationY = 0.0f;
			m_pAnimatedSections[i].m_rotationZ = 0.0f;

			m_pAnimatedSections[i].m_rotationMaxSpeedX = m_pAnimatedSections[i].m_rotationSpeedX;
			m_pAnimatedSections[i].m_rotationMaxSpeedY = m_pAnimatedSections[i].m_rotationSpeedY;
			m_pAnimatedSections[i].m_rotationMaxSpeedZ = m_pAnimatedSections[i].m_rotationSpeedZ;
			m_pAnimatedSections[i].m_rotationXUp = true;
			m_pAnimatedSections[i].m_rotationXDown = false;
			m_pAnimatedSections[i].m_rotationYUp = true;
			m_pAnimatedSections[i].m_rotationYDown = false;
			m_pAnimatedSections[i].m_rotationZUp = true;
			m_pAnimatedSections[i].m_rotationZDown = false;
		}

		// Dynamic lights
		m_numLights = 0;
		file >> tempString >> m_numLights;
		if(m_numLights > 0)
		{
			m_pLights = new VoxelWeaponLight[m_numLights];
		}
		for(int i = 0; i < m_numLights; i++)
		{
			m_pLights[i].m_lightId = -1;

			float offsetX = 0.0f;
			float offsetY = 0.0f;
			float offsetZ = 0.0f;
			file >> tempString >> offsetX >> offsetY >> offsetZ;
			m_pLights[i].m_lightOffset = vec3(offsetX, offsetY, offsetZ);

			file >> tempString >> m_pLights[i].m_lightRadius;

			file >> tempString >> m_pLights[i].m_lightDiffuseMultiplier;

			float r = 1.0f;
			float g = 1.0f;
			float b = 1.0f;
			float a = 1.0f;
			file >> tempString >> r >> g >> b >> a;
			m_pLights[i].m_lightColour = Colour(r, g, b, a);

			file >> tempString >> m_pLights[i].m_connectedToSectionIndex;
		}

		// Particle effects
		m_numParticleEffects = 0;
		file >> tempString >> m_numParticleEffects;
		if(m_numParticleEffects > 0)
		{
			m_pParticleEffects = new ParticleEffect[m_numParticleEffects];
		}
		for(int i = 0; i < m_numParticleEffects; i++)
		{
			m_pParticleEffects[i].m_particleEffectId = -1;

			file >> tempString >> m_pParticleEffects[i].m_fileName;
			float offsetX = 0.0f;
			float offsetY = 0.0f;
			float offsetZ = 0.0f;
			file >> tempString >> offsetX >> offsetY >> offsetZ;
			m_pParticleEffects[i].m_positionOffset = vec3(offsetX, offsetY, offsetZ);

			file >> tempString >> m_pParticleEffects[i].m_connectedToSectionIndex;
		}

		// Weapon trails
		m_numWeaponTrails = 0;
		file >> tempString >> m_numWeaponTrails;
		if(m_numWeaponTrails > 0)
		{
			m_pWeaponTrails = new WeaponTrail[m_numWeaponTrails];
		}
		for(int i = 0; i < m_numWeaponTrails; i++)
		{
			file >> tempString >> m_pWeaponTrails[i].m_trailTime;

			float startOffsetX = 0.0f;
			float startOffsetY = 0.0f;
			float startOffsetZ = 0.0f;
			file >> tempString >> startOffsetX >> startOffsetY >> startOffsetZ;
			m_pWeaponTrails[i].m_startOffsetPoint = vec3(startOffsetX, startOffsetY, startOffsetZ);

			float endOffsetX = 0.0f;
			float endOffsetY = 0.0f;
			float endOffsetZ = 0.0f;
			file >> tempString >> endOffsetX >> endOffsetY >> endOffsetZ;
			m_pWeaponTrails[i].m_endOffsetPoint = vec3(endOffsetX, endOffsetY, endOffsetZ);

			float r = 1.0f;
			float g = 1.0f;
			float b = 1.0f;
			file >> tempString >> r >> g >> b;
			m_pWeaponTrails[i].m_trailColour = Colour(r, g, b);

			file >> tempString >> m_pWeaponTrails[i].m_followOrigin;

			m_pWeaponTrails[i].m_parentScale = 1.0f;
			m_pWeaponTrails[i].m_numTrailPoints = 50;
			m_pWeaponTrails[i].m_pTrailPoints = new WeaponTrailPoint[m_pWeaponTrails[i].m_numTrailPoints];
			m_pWeaponTrails[i].m_trailNextAddIndex = 0;
			for(int point = 0; point < m_pWeaponTrails[i].m_numTrailPoints; point++)
			{
				m_pWeaponTrails[i].m_pTrailPoints[point].m_pointActive = false;
				m_pWeaponTrails[i].m_pTrailPoints[point].m_animaionTime = 0.0f;
			}
		}

		// Gameplay
		file >> tempString >> m_weaponRadius;

		m_loaded = true;

		file.close();
	}
}

void VoxelWeapon::SaveWeapon(const char *weaponFilename)
{
	ofstream file;

	// Open the file
	file.open(weaponFilename, ios::out);
	if(file.is_open())
	{
		file << "offset: " << m_renderOffset.x << " " << m_renderOffset.y << " " << m_renderOffset.z << "\n";

		file << "scale: " << m_renderScale << "\n\n";

		// Animated sections
		file << "numAnimatedSections: " << m_numAnimatedSections << "\n";
		for (int i = 0; i < m_numAnimatedSections; i++)
		{
			file << "qubicleFile: " << m_pAnimatedSections[i].m_fileName << "\n";
			file << "renderScale: " << m_pAnimatedSections[i].m_renderScale << "\n";
			file << "renderOffset: " << m_pAnimatedSections[i].m_renderOffset.x << " " << m_pAnimatedSections[i].m_renderOffset.y << " " << m_pAnimatedSections[i].m_renderOffset.z << " " << "\n";
			file << "autoStartAnimation: " << m_pAnimatedSections[i].m_autoStart << "\n";
			file << "loopingAnimation: " << m_pAnimatedSections[i].m_loopingAnimation << "\n";
			file << "translateXSpeed: " << m_pAnimatedSections[i].m_translateSpeedX << "\n";
			file << "translateYSpeed: " << m_pAnimatedSections[i].m_translateSpeedY << "\n";
			file << "translateZSpeed: " << m_pAnimatedSections[i].m_translateSpeedZ << "\n";
			file << "translateXRange: " << m_pAnimatedSections[i].m_translateRangeXMin << " " << m_pAnimatedSections[i].m_translateRangeXMax << "\n";
			file << "translateYRange: " << m_pAnimatedSections[i].m_translateRangeYMin << " " << m_pAnimatedSections[i].m_translateRangeYMax << "\n";
			file << "translateZRange: " << m_pAnimatedSections[i].m_translateRangeZMin << " " << m_pAnimatedSections[i].m_translateRangeZMax << "\n";
			file << "translateXTurnSpeed: " << m_pAnimatedSections[i].m_translateSpeedTurnSpeedX << "\n";
			file << "translateYTurnSpeed: " << m_pAnimatedSections[i].m_translateSpeedTurnSpeedY << "\n";
			file << "translateZTurnSpeed: " << m_pAnimatedSections[i].m_translateSpeedTurnSpeedZ << "\n";
			file << "rotationPoint: " << m_pAnimatedSections[i].m_rotationPoint.x << " " << m_pAnimatedSections[i].m_rotationPoint.y << " " << m_pAnimatedSections[i].m_rotationPoint.z << "\n";
			file << "rotationXSpeed: " << m_pAnimatedSections[i].m_rotationSpeedX << "\n";
			file << "rotationYSpeed: " << m_pAnimatedSections[i].m_rotationSpeedY << "\n";
			file << "rotationZSpeed: " << m_pAnimatedSections[i].m_rotationSpeedZ << "\n";
			file << "rotationXRange: " << m_pAnimatedSections[i].m_rotationRangeXMin << " " << m_pAnimatedSections[i].m_rotationRangeXMax << "\n";
			file << "rotationYRange: " << m_pAnimatedSections[i].m_rotationRangeYMin << " " << m_pAnimatedSections[i].m_rotationRangeYMax << "\n";
			file << "rotationZRange: " << m_pAnimatedSections[i].m_rotationRangeZMin << " " << m_pAnimatedSections[i].m_rotationRangeZMax << "\n";
			file << "rotationXTurnSpeed: " << m_pAnimatedSections[i].m_rotationSpeedTurnSpeedX << "\n";
			file << "rotationYTurnSpeed: " << m_pAnimatedSections[i].m_rotationSpeedTurnSpeedY << "\n";
			file << "rotationZTurnSpeed: " << m_pAnimatedSections[i].m_rotationSpeedTurnSpeedZ << "\n";
		}
		file << "\n";

		// Dynamic lights
		file << "numLights: " << m_numLights << "\n";
		for (int i = 0; i < m_numLights; i++)
		{
			file << "lightOffset: " << m_pLights[i].m_lightOffset.x << " " << m_pLights[i].m_lightOffset.y << " " << m_pLights[i].m_lightOffset.z << " " << "\n";
			file << "lightRadius: " << m_pLights[i].m_lightRadius << "\n";
			file << "lightDiffuseMultiplier: " << m_pLights[i].m_lightDiffuseMultiplier << "\n";
			file << "lightColour: " << m_pLights[i].m_lightColour.GetRed() << " " << m_pLights[i].m_lightColour.GetGreen() << " " << m_pLights[i].m_lightColour.GetBlue() << " " << m_pLights[i].m_lightColour.GetAlpha() << "\n";
			file << "connectedToSection: " << m_pLights[i].m_connectedToSectionIndex << "\n";
		}
		file << "\n";

		// Particle effects
		file << "numParticleEffects: " << m_numParticleEffects << "\n";
		for (int i = 0; i < m_numParticleEffects; i++)
		{
			file << "particleEffect: " << m_pParticleEffects[i].m_fileName << "\n";
			file << "position: " << m_pParticleEffects[i].m_positionOffset.x << " " << m_pParticleEffects[i].m_positionOffset.y << " " << m_pParticleEffects[i].m_positionOffset.z << "\n";
			file << "connectedToSection: " << m_pParticleEffects[i].m_connectedToSectionIndex << "\n";
		}
		file << "\n";

		// Weapon trails
		file << "numWeaponTrails: " << m_numWeaponTrails << "\n";
		for (int i = 0; i < m_numWeaponTrails; i++)
		{
			file << "trailTime: " << m_pWeaponTrails[i].m_trailTime << "\n";
			file << "startOffsetPoint: " << m_pWeaponTrails[i].m_startOffsetPoint.x << " " << m_pWeaponTrails[i].m_startOffsetPoint.y << " " << m_pWeaponTrails[i].m_startOffsetPoint.z << "\n";
			file << "endOffsetPoint: " << m_pWeaponTrails[i].m_endOffsetPoint.x << " " << m_pWeaponTrails[i].m_endOffsetPoint.y << " " << m_pWeaponTrails[i].m_endOffsetPoint.z << "\n";
			file << "trailColour: " << m_pWeaponTrails[i].m_trailColour.GetRed() << " " << m_pWeaponTrails[i].m_trailColour.GetGreen() << " " << m_pWeaponTrails[i].m_trailColour.GetBlue() << " " << m_pWeaponTrails[i].m_trailColour.GetAlpha() << "\n";
			file << "followOrigin: " << m_pWeaponTrails[i].m_followOrigin << "\n";
		}
		file << "\n";

		// Gameplay
		file << "weaponRadius: " << m_weaponRadius << "\n";

		file.close();
	}
}

void VoxelWeapon::UnloadWeapon()
{
	if(m_numLights > 0)
	{
		delete[] m_pLights;
		m_pLights = nullptr;
		m_numLights = 0;
	}

	if(m_numAnimatedSections > 0)
	{
		delete[] m_pAnimatedSections;
		m_pAnimatedSections = nullptr;
		m_numAnimatedSections = 0;
	}

	if(m_numParticleEffects > 0)
	{
		delete[] m_pParticleEffects;
		m_pParticleEffects = nullptr;
		m_numParticleEffects = 0;
	}

	if(m_numWeaponTrails > 0)
	{
		for(int i = 0; i < m_numWeaponTrails; i++)
		{
			delete[] m_pWeaponTrails[i].m_pTrailPoints;
			m_pWeaponTrails[i].m_pTrailPoints = nullptr;
		}

		delete[] m_pWeaponTrails;
		m_pWeaponTrails = nullptr;
		m_numWeaponTrails = 0;
	}

	Reset();

	m_loaded = false;
}

void VoxelWeapon::SetVoxelCharacterParent(VoxelCharacter* pParentCharacter)
{
	m_pParentCharacter = pParentCharacter;
}

void VoxelWeapon::SetBoneAttachment(const char* boneName)
{
	if(m_pParentCharacter != nullptr)
	{
		m_boneIndex = m_pParentCharacter->GetBoneIndex(boneName);

		m_matrixName = boneName;
		m_matrixIndex = m_pParentCharacter->GetMatrixIndexForName(boneName);
	}	
}

void VoxelWeapon::SetRenderOffset(vec3 offset)
{
	m_renderOffset = offset;
}

vec3 VoxelWeapon::GetRenderOffset()
{
	return m_renderOffset;
}

void VoxelWeapon::SetRenderScale(float scale)
{
	m_renderScale = scale;
}

float VoxelWeapon::GetRenderScale()
{
	return m_renderScale;
}

vec3 VoxelWeapon::GetCenter()
{
	vec3 centerPos;
	for(int i = 0; i < m_numAnimatedSections; i++)
	{
		centerPos += m_pAnimatedSections[i].m_renderOffset*m_pAnimatedSections[i].m_renderScale;
	}

	centerPos /= (float)m_numAnimatedSections;

	return centerPos + (m_renderOffset*m_renderScale);
}

// Subsection animations
void VoxelWeapon::StartSubSectionAnimation()
{
	for(int i = 0; i < m_numAnimatedSections; i++)
	{
		m_pAnimatedSections[i].m_playingAnimation = true;
	}
}

void VoxelWeapon::StopSubSectionAnimation()
{
	for(int i = 0; i < m_numAnimatedSections; i++)
	{
		m_pAnimatedSections[i].m_playingAnimation = false;
	}
}

bool VoxelWeapon::HasSubSectionAnimationFinished(int index)
{
	return m_pAnimatedSections[index].m_playingAnimation == false;
}

// Weapon trails
void VoxelWeapon::StartWeaponTrails()
{
	m_weaponTrailsStarted = true;

	for(int i = 0; i < m_numWeaponTrails; i++)
	{
		for(int point = 0; point < m_pWeaponTrails[i].m_numTrailPoints; point++)
		{
			m_pWeaponTrails[i].m_pTrailPoints[point].m_pointActive = false;
			m_pWeaponTrails[i].m_pTrailPoints[point].m_animaionTime = 0.0f;
		}
	}
}

void VoxelWeapon::StopWeaponTrails()
{
	m_weaponTrailsStarted = false;
}

bool VoxelWeapon::IsWeaponTrailsActive()
{
	return m_weaponTrailsStarted;
}

// Lighting
int VoxelWeapon::GetNumLights()
{
	return m_numLights;
}

void VoxelWeapon::SetLightingId(int lightIndex, unsigned int lightId)
{
	m_pLights[lightIndex].m_lightId = lightId;
}

void VoxelWeapon::GetLightParams(int lightIndex, unsigned int *lightId, vec3 *position, float *radius, float *diffuseMultiplier, Colour *colour, bool *connectedToSegment)
{
	*lightId = m_pLights[lightIndex].m_lightId;
	*radius = m_pLights[lightIndex].m_lightRadius;
	*diffuseMultiplier = m_pLights[lightIndex].m_lightDiffuseMultiplier;
	*colour = m_pLights[lightIndex].m_lightColour;
	*position = m_pLights[lightIndex].m_lightPosition;
	*connectedToSegment = m_pLights[lightIndex].m_connectedToSectionIndex != -1;

	if(m_pLights[lightIndex].m_connectedToSectionIndex == -1)
	{
		if(m_pParentCharacter != nullptr)
		{
			*position *= m_pParentCharacter->GetCharacterScale();
		}
	}
}

// Particle effects
int VoxelWeapon::GetNumParticleEffects()
{
	return m_numParticleEffects;
}

void VoxelWeapon::SetParticleEffectId(int particleEffectIndex, unsigned int particleEffectId)
{
	m_pParticleEffects[particleEffectIndex].m_particleEffectId = particleEffectId;
}

void VoxelWeapon::GetParticleEffectParams(int particleEffectIndex, unsigned int *particleEffectId, vec3 *position, string* name, bool *connectedToSegment)
{
	*particleEffectId = m_pParticleEffects[particleEffectIndex].m_particleEffectId;
	*position = m_pParticleEffects[particleEffectIndex].m_particleEffectPosition;
	*name = m_pParticleEffects[particleEffectIndex].m_fileName;
	*connectedToSegment = m_pParticleEffects[particleEffectIndex].m_connectedToSectionIndex != -1;

	if(m_pParticleEffects[particleEffectIndex].m_connectedToSectionIndex == -1)
	{
		if(m_pParentCharacter != nullptr)
		{
			*position *= m_pParentCharacter->GetCharacterScale();
		}
	}
}

// Animated sections
int VoxelWeapon::GetNumAimatedSections()
{
	return m_numAnimatedSections;
}

AnimatedSection* VoxelWeapon::GetAnimatedSection(int index)
{
	return &m_pAnimatedSections[index];
}

// Gameplay params
float VoxelWeapon::GetWeaponRadius()
{
	return m_weaponRadius;
}

void VoxelWeapon::SetFirstPersonMode(bool firstPerson)
{
	m_firstPersonMode = firstPerson;
}

// Camera settings
void VoxelWeapon::SetCameraYRotation(float yRot)
{
	m_cameraYRotation = yRot;
}

// Rendering modes
void VoxelWeapon::SetWireFrameRender(bool wireframe)
{
	for(int i = 0; i < m_numAnimatedSections; i++)
	{
		m_pAnimatedSections[i].m_pVoxelObject->SetWireFrameRender(wireframe);
	}
}

void VoxelWeapon::SetMeshAlpha(float alpha)
{
	for(int i = 0; i < m_numAnimatedSections; i++)
	{
		m_pAnimatedSections[i].m_pVoxelObject->SetMeshAlpha(alpha);
	}
}

void VoxelWeapon::SetMeshSingleColour(float r, float g, float b)
{
	for(int i = 0; i < m_numAnimatedSections; i++)
	{
		m_pAnimatedSections[i].m_pVoxelObject->SetMeshSingleColour(r, g, b);
	}
}

// Updating
void VoxelWeapon::SetWeaponTrailsParams(Matrix4x4 originMatrix, float scale)
{
	if(m_loaded == false)
	{
		return;
	}

	for(int i = 0; i < m_numWeaponTrails; i++)
	{
		m_pWeaponTrails[i].m_origin = originMatrix;
		m_pWeaponTrails[i].m_parentScale = scale;
	}
}

void VoxelWeapon::CreateWeaponTrailPoint()
{
	for (int i = 0; i < m_numWeaponTrails; i++)
	{
		int index = m_pWeaponTrails[i].m_trailNextAddIndex;

		if (m_weaponTrailsStarted == false)
		{
			m_pWeaponTrails[i].m_pTrailPoints[index].m_pointActive = false;
		}
		else
		{
			vec3 startPosition = m_pWeaponTrails[i].m_startOffsetPoint;
			vec3 endPosition = m_pWeaponTrails[i].m_endOffsetPoint;

			// Scale to render size
			// Translate for initial block offset
			startPosition += (vec3(m_renderOffset.x, m_renderOffset.y, m_renderOffset.z) * m_renderScale);
			endPosition += (vec3(m_renderOffset.x, m_renderOffset.y, m_renderOffset.z) * m_renderScale);

			// Rotation due to the weapon facing forwards for hand directions
			if (m_pParentCharacter != nullptr)
			{
				Matrix4x4 rotationMatrix;
				rotationMatrix.SetRotation(DegToRad(90.0f), 0.0f, 0.0f);
				startPosition = rotationMatrix * startPosition;
				endPosition = rotationMatrix * endPosition;
			}

			if (m_matrixIndex != -1)
			{
				vec3 handBoneOffset = m_pParentCharacter->GetBoneMatrixRenderOffset(m_matrixName.c_str());

				// Translate for external matrix offset value
				startPosition += vec3(handBoneOffset.x, handBoneOffset.y, handBoneOffset.z);
				endPosition += vec3(handBoneOffset.x, handBoneOffset.y, handBoneOffset.z);
			}

			// Rotation due to 3dsmax export affecting the bone rotations
			if (m_pParentCharacter != nullptr)
			{
				Matrix4x4 rotationMatrix;
				rotationMatrix.SetRotation(0.0f, 0.0f, DegToRad(-90.0f));
				startPosition = rotationMatrix * startPosition;
				endPosition = rotationMatrix * endPosition;
			}

			// First person mode modifications
			if (m_firstPersonMode)
			{
				Matrix4x4 rotationMatrix;
				float amountX = 0.75f;
				float amountY = 0.75f;
				float amountZ = 0.5f;
				rotationMatrix.SetTranslation(vec3(amountX, amountY, -amountZ));
				rotationMatrix.SetRotation(0.0f, DegToRad(m_cameraYRotation), 0.0f);
				startPosition = rotationMatrix * startPosition;
				endPosition = rotationMatrix * endPosition;
			}

			if (m_pParentCharacter != nullptr)
			{
				if (m_boneIndex != -1)
				{
					AnimationSections animationSection = AnimationSections_FullBody;
					if (m_boneIndex == m_pParentCharacter->GetHeadBoneIndex() ||
						m_boneIndex == m_pParentCharacter->GetBodyBoneIndex())
					{
						animationSection = AnimationSections_Head_Body;
					}
					else if (m_boneIndex == m_pParentCharacter->GetLeftShoulderBoneIndex() ||
						m_boneIndex == m_pParentCharacter->GetLeftHandBoneIndex())
					{
						animationSection = AnimationSections_Left_Arm_Hand;
					}
					else if (m_boneIndex == m_pParentCharacter->GetRightShoulderBoneIndex() ||
						m_boneIndex == m_pParentCharacter->GetRightHandBoneIndex())
					{
						animationSection = AnimationSections_Right_Arm_Hand;
					}
					else if (m_boneIndex == m_pParentCharacter->GetLegsBoneIndex() ||
						m_boneIndex == m_pParentCharacter->GetRightFootBoneIndex() ||
						m_boneIndex == m_pParentCharacter->GetLeftFootBoneIndex())
					{
						animationSection = AnimationSections_Legs_Feet;
					}

					Matrix4x4 boneMatrix = m_pParentCharacter->GetBoneMatrix(animationSection, m_boneIndex);

					// Translate by attached bone matrix
					startPosition = boneMatrix * startPosition;
					endPosition = boneMatrix * endPosition;

					// Looking direction, since we are attached to a character who is holding us.
					{
						vec3 lForward = normalize(m_pParentCharacter->GetFaceLookingDirection());
						lForward.y = 0.0f;
						lForward = normalize(lForward);
						vec3 forwardDiff = lForward - vec3(0.0f, 0.0f, 1.0f);
						lForward = normalize(vec3(0.0f, 0.0f, 1.0f) + (forwardDiff*0.5f));

						vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
						vec3 lRight = normalize(cross(lUp, lForward));
						lUp = normalize(cross(lForward, lRight));

						float lMatrix[16] =
						{
							lRight.x, lRight.y, lRight.z, 0.0f,
							lUp.x, lUp.y, lUp.z, 0.0f,
							lForward.x, lForward.y, lForward.z, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f
						};
						Matrix4x4 lookingMat;
						lookingMat.SetValues(lMatrix);

						startPosition = lookingMat * startPosition;
						endPosition = lookingMat * endPosition;
					}

					// Breathing animation
					float offsetAmount = m_pParentCharacter->GetBreathingAnimationOffsetForBone(m_boneIndex);
					startPosition += vec3(0.0f, offsetAmount, 0.0f);
					endPosition += vec3(0.0f, offsetAmount, 0.0f);
				}
			}

			if (m_pWeaponTrails[i].m_followOrigin == false)
			{
				startPosition *= m_pWeaponTrails[i].m_parentScale;
				endPosition *= m_pWeaponTrails[i].m_parentScale;

				startPosition = m_pWeaponTrails[i].m_origin * startPosition;
				endPosition = m_pWeaponTrails[i].m_origin * endPosition;
			}

			m_pWeaponTrails[i].m_pTrailPoints[index].m_startPoint = startPosition;
			m_pWeaponTrails[i].m_pTrailPoints[index].m_endPoint = endPosition;

			m_pWeaponTrails[i].m_pTrailPoints[index].m_pointActive = true;
			m_pWeaponTrails[i].m_pTrailPoints[index].m_animaionTime = m_pWeaponTrails[i].m_trailTime;

			m_pWeaponTrails[i].m_trailNextAddIndex++;
			if (m_pWeaponTrails[i].m_trailNextAddIndex == 49)
			{
				m_pWeaponTrails[i].m_trailNextAddIndex = 0;
			}
		}
	}
}

void VoxelWeapon::Update(float dt)
{
	if(m_loaded == false)
	{
		return;
	}

	if (dt > 0.1f)
	{
		// Temp fix for when dt is large due to a freeze or temp halt, would cause animated sections to get out-of-sync.
		// due to allowing values to go over min/max and then 're-adjust', but since dt was large they would go WAY over min/max
		return;
	}

	// Update animated sections
	for(int i = 0; i < m_numAnimatedSections; i++)
	{
		if(m_pAnimatedSections[i].m_playingAnimation)
		{
			bool noChangeInAnimation = (m_pAnimatedSections[i].m_translateSpeedX == 0.0f && m_pAnimatedSections[i].m_translateSpeedY == 0.0f && m_pAnimatedSections[i].m_translateSpeedZ == 0.0f &&
										m_pAnimatedSections[i].m_rotationSpeedX == 0.0f && m_pAnimatedSections[i].m_rotationSpeedY == 0.0f && m_pAnimatedSections[i].m_rotationSpeedZ == 0.0f);

			// Translation
			if(m_pAnimatedSections[i].m_translateXUp == true && m_pAnimatedSections[i].m_translateSpeedX < m_pAnimatedSections[i].m_translateMaxSpeedX)
			{
				if(m_pAnimatedSections[i].m_translateSpeedTurnSpeedX != -1)
				{
					m_pAnimatedSections[i].m_translateSpeedX += m_pAnimatedSections[i].m_translateSpeedTurnSpeedX * dt;
				}
				else
				{
					m_pAnimatedSections[i].m_translateX = m_pAnimatedSections[i].m_translateRangeXMin;
					m_pAnimatedSections[i].m_translateSpeedX = -m_pAnimatedSections[i].m_translateSpeedX;
				}
			}
			else if(m_pAnimatedSections[i].m_translateXDown == true && m_pAnimatedSections[i].m_translateSpeedX > -m_pAnimatedSections[i].m_translateMaxSpeedX)
			{
				if(m_pAnimatedSections[i].m_translateSpeedTurnSpeedX != -1)
				{
					m_pAnimatedSections[i].m_translateSpeedX -= m_pAnimatedSections[i].m_translateSpeedTurnSpeedX * dt;
				}
				else
				{
					m_pAnimatedSections[i].m_translateX = m_pAnimatedSections[i].m_translateRangeXMax;
					m_pAnimatedSections[i].m_translateSpeedX = -m_pAnimatedSections[i].m_translateSpeedX;
				}
			}
			if(m_pAnimatedSections[i].m_translateYUp == true && m_pAnimatedSections[i].m_translateSpeedY < m_pAnimatedSections[i].m_translateMaxSpeedY)
			{
				if(m_pAnimatedSections[i].m_translateSpeedTurnSpeedY != -1)
				{
					m_pAnimatedSections[i].m_translateSpeedY += m_pAnimatedSections[i].m_translateSpeedTurnSpeedY * dt;
				}
				else
				{
					m_pAnimatedSections[i].m_translateY = m_pAnimatedSections[i].m_translateRangeYMin;
					m_pAnimatedSections[i].m_translateSpeedY = -m_pAnimatedSections[i].m_translateSpeedY;
				}
			}
			else if(m_pAnimatedSections[i].m_translateYDown == true && m_pAnimatedSections[i].m_translateSpeedY > -m_pAnimatedSections[i].m_translateMaxSpeedY)
			{
				if(m_pAnimatedSections[i].m_translateSpeedTurnSpeedY != -1)
				{
					m_pAnimatedSections[i].m_translateSpeedY -= m_pAnimatedSections[i].m_translateSpeedTurnSpeedY * dt;
				}
				else
				{
					m_pAnimatedSections[i].m_translateY = m_pAnimatedSections[i].m_translateRangeYMax;
					m_pAnimatedSections[i].m_translateSpeedY = -m_pAnimatedSections[i].m_translateSpeedY;
				}
			}
			if(m_pAnimatedSections[i].m_translateZUp == true && m_pAnimatedSections[i].m_translateSpeedZ < m_pAnimatedSections[i].m_translateMaxSpeedZ)
			{
				if(m_pAnimatedSections[i].m_translateSpeedTurnSpeedZ != -1)
				{
					m_pAnimatedSections[i].m_translateSpeedZ += m_pAnimatedSections[i].m_translateSpeedTurnSpeedZ * dt;
				}
				else
				{
					m_pAnimatedSections[i].m_translateZ = m_pAnimatedSections[i].m_translateRangeZMin;
					m_pAnimatedSections[i].m_translateSpeedZ = -m_pAnimatedSections[i].m_translateSpeedZ;
				}
			}
			else if(m_pAnimatedSections[i].m_translateZDown == true && m_pAnimatedSections[i].m_translateSpeedZ > -m_pAnimatedSections[i].m_translateMaxSpeedZ)
			{
				if(m_pAnimatedSections[i].m_translateSpeedTurnSpeedZ != -1)
				{
					m_pAnimatedSections[i].m_translateSpeedZ -= m_pAnimatedSections[i].m_translateSpeedTurnSpeedZ * dt;
				}
				else
				{
					m_pAnimatedSections[i].m_translateZ = m_pAnimatedSections[i].m_translateRangeZMax;
					m_pAnimatedSections[i].m_translateSpeedZ = -m_pAnimatedSections[i].m_translateSpeedZ;
				}
			}

			if(m_pAnimatedSections[i].m_translateX > m_pAnimatedSections[i].m_translateRangeXMax)
			{
				m_pAnimatedSections[i].m_translateXUp = false;
				m_pAnimatedSections[i].m_translateXDown = true;

				if(m_pAnimatedSections[i].m_loopingAnimation == false)
				{
					m_pAnimatedSections[i].m_playingAnimation = false;
				}
			}
			else if(m_pAnimatedSections[i].m_translateX < m_pAnimatedSections[i].m_translateRangeXMin)
			{
				m_pAnimatedSections[i].m_translateXUp = true;
				m_pAnimatedSections[i].m_translateXDown = false;

				if(m_pAnimatedSections[i].m_loopingAnimation == false)
				{
					m_pAnimatedSections[i].m_playingAnimation = false;
				}
			}
			if(m_pAnimatedSections[i].m_translateY > m_pAnimatedSections[i].m_translateRangeYMax)
			{
				m_pAnimatedSections[i].m_translateYUp = false;
				m_pAnimatedSections[i].m_translateYDown = true;

				if(m_pAnimatedSections[i].m_loopingAnimation == false)
				{
					m_pAnimatedSections[i].m_playingAnimation = false;
				}
			}
			else if(m_pAnimatedSections[i].m_translateY < m_pAnimatedSections[i].m_translateRangeYMin)
			{
				m_pAnimatedSections[i].m_translateYUp = true;
				m_pAnimatedSections[i].m_translateYDown = false;

				if(m_pAnimatedSections[i].m_loopingAnimation == false)
				{
					m_pAnimatedSections[i].m_playingAnimation = false;
				}
			}
			if(m_pAnimatedSections[i].m_translateZ > m_pAnimatedSections[i].m_translateRangeZMax)
			{
				m_pAnimatedSections[i].m_translateZUp = false;
				m_pAnimatedSections[i].m_translateZDown = true;

				if(m_pAnimatedSections[i].m_loopingAnimation == false)
				{
					m_pAnimatedSections[i].m_playingAnimation = false;
				}
			}
			else if(m_pAnimatedSections[i].m_translateZ < m_pAnimatedSections[i].m_translateRangeZMin)
			{
				m_pAnimatedSections[i].m_translateZUp = true;
				m_pAnimatedSections[i].m_translateZDown = false;

				if(m_pAnimatedSections[i].m_loopingAnimation == false)
				{
					m_pAnimatedSections[i].m_playingAnimation = false;
				}
			}

			m_pAnimatedSections[i].m_translateX += m_pAnimatedSections[i].m_translateSpeedX * dt;
			m_pAnimatedSections[i].m_translateY += m_pAnimatedSections[i].m_translateSpeedY * dt;
			m_pAnimatedSections[i].m_translateZ += m_pAnimatedSections[i].m_translateSpeedZ * dt;

			// Rotation
			if(m_pAnimatedSections[i].m_rotationXUp == true && m_pAnimatedSections[i].m_rotationSpeedX < m_pAnimatedSections[i].m_rotationMaxSpeedX)
			{
				if(m_pAnimatedSections[i].m_rotationSpeedTurnSpeedX != -1)
				{
					m_pAnimatedSections[i].m_rotationSpeedX += m_pAnimatedSections[i].m_rotationSpeedTurnSpeedX * dt;
				}
				else
				{
					m_pAnimatedSections[i].m_rotationX = m_pAnimatedSections[i].m_rotationRangeXMin;
					m_pAnimatedSections[i].m_rotationSpeedX = -m_pAnimatedSections[i].m_rotationSpeedX;
				}
			}
			else if(m_pAnimatedSections[i].m_rotationXDown == true && m_pAnimatedSections[i].m_rotationSpeedX > -m_pAnimatedSections[i].m_rotationMaxSpeedX)
			{
				if(m_pAnimatedSections[i].m_rotationSpeedTurnSpeedX != -1)
				{
					m_pAnimatedSections[i].m_rotationSpeedX -= m_pAnimatedSections[i].m_rotationSpeedTurnSpeedX * dt;
				}
				else
				{
					m_pAnimatedSections[i].m_rotationX = m_pAnimatedSections[i].m_rotationRangeXMax;
					m_pAnimatedSections[i].m_rotationSpeedX = -m_pAnimatedSections[i].m_rotationSpeedX;
				}
			}
			if(m_pAnimatedSections[i].m_rotationYUp == true && m_pAnimatedSections[i].m_rotationSpeedY < m_pAnimatedSections[i].m_rotationMaxSpeedY)
			{
				if(m_pAnimatedSections[i].m_rotationSpeedTurnSpeedY != -1)
				{
					m_pAnimatedSections[i].m_rotationSpeedY += m_pAnimatedSections[i].m_rotationSpeedTurnSpeedY * dt;
				}
				else
				{
					m_pAnimatedSections[i].m_rotationY = m_pAnimatedSections[i].m_rotationRangeYMin;
					m_pAnimatedSections[i].m_rotationSpeedY = -m_pAnimatedSections[i].m_rotationSpeedY;
				}
			}
			else if(m_pAnimatedSections[i].m_rotationYDown == true && m_pAnimatedSections[i].m_rotationSpeedY > -m_pAnimatedSections[i].m_rotationMaxSpeedY)
			{
				if(m_pAnimatedSections[i].m_rotationSpeedTurnSpeedY != -1)
				{
					m_pAnimatedSections[i].m_rotationSpeedY -= m_pAnimatedSections[i].m_rotationSpeedTurnSpeedY * dt;
				}
				else
				{
					m_pAnimatedSections[i].m_rotationY = m_pAnimatedSections[i].m_rotationRangeYMax;
					m_pAnimatedSections[i].m_rotationSpeedY = -m_pAnimatedSections[i].m_rotationSpeedY;
				}
			}
			if(m_pAnimatedSections[i].m_rotationZUp == true && m_pAnimatedSections[i].m_rotationSpeedZ < m_pAnimatedSections[i].m_rotationMaxSpeedZ)
			{
				if(m_pAnimatedSections[i].m_rotationSpeedTurnSpeedZ != -1)
				{
					m_pAnimatedSections[i].m_rotationSpeedZ += m_pAnimatedSections[i].m_rotationSpeedTurnSpeedZ * dt;
				}
				else
				{
					m_pAnimatedSections[i].m_rotationZ = m_pAnimatedSections[i].m_rotationRangeZMin;
					m_pAnimatedSections[i].m_rotationSpeedZ = -m_pAnimatedSections[i].m_rotationSpeedZ;
				}
			}
			else if(m_pAnimatedSections[i].m_rotationZDown == true && m_pAnimatedSections[i].m_rotationSpeedZ > -m_pAnimatedSections[i].m_rotationMaxSpeedZ)
			{
				if(m_pAnimatedSections[i].m_rotationSpeedTurnSpeedZ != -1)
				{
					m_pAnimatedSections[i].m_rotationSpeedZ -= m_pAnimatedSections[i].m_rotationSpeedTurnSpeedZ * dt;
				}
				else
				{
					m_pAnimatedSections[i].m_rotationZ = m_pAnimatedSections[i].m_rotationRangeZMax;
					m_pAnimatedSections[i].m_rotationSpeedZ = -m_pAnimatedSections[i].m_rotationSpeedZ;
				}
			}

			if(m_pAnimatedSections[i].m_rotationX > m_pAnimatedSections[i].m_rotationRangeXMax)
			{
				m_pAnimatedSections[i].m_rotationXUp = false;
				m_pAnimatedSections[i].m_rotationXDown = true;

				if(m_pAnimatedSections[i].m_loopingAnimation == false)
				{
					m_pAnimatedSections[i].m_playingAnimation = false;
				}
			}
			else if(m_pAnimatedSections[i].m_rotationX < m_pAnimatedSections[i].m_rotationRangeXMin)
			{
				m_pAnimatedSections[i].m_rotationXUp = true;
				m_pAnimatedSections[i].m_rotationXDown = false;

				if(m_pAnimatedSections[i].m_loopingAnimation == false)
				{
					m_pAnimatedSections[i].m_playingAnimation = false;
				}
			}
			if(m_pAnimatedSections[i].m_rotationY > m_pAnimatedSections[i].m_rotationRangeYMax)
			{
				m_pAnimatedSections[i].m_rotationYUp = false;
				m_pAnimatedSections[i].m_rotationYDown = true;

				if(m_pAnimatedSections[i].m_loopingAnimation == false)
				{
					m_pAnimatedSections[i].m_playingAnimation = false;
				}
			}
			else if(m_pAnimatedSections[i].m_rotationY < m_pAnimatedSections[i].m_rotationRangeYMin)
			{
				m_pAnimatedSections[i].m_rotationYUp = true;
				m_pAnimatedSections[i].m_rotationYDown = false;

				if(m_pAnimatedSections[i].m_loopingAnimation == false)
				{
					m_pAnimatedSections[i].m_playingAnimation = false;
				}
			}
			if(m_pAnimatedSections[i].m_rotationZ > m_pAnimatedSections[i].m_rotationRangeZMax)
			{
				m_pAnimatedSections[i].m_rotationZUp = false;
				m_pAnimatedSections[i].m_rotationZDown = true;

				if(m_pAnimatedSections[i].m_loopingAnimation == false)
				{
					m_pAnimatedSections[i].m_playingAnimation = false;
				}
			}
			else if(m_pAnimatedSections[i].m_rotationZ < m_pAnimatedSections[i].m_rotationRangeZMin)
			{
				m_pAnimatedSections[i].m_rotationZUp = true;
				m_pAnimatedSections[i].m_rotationZDown = false;

				if(m_pAnimatedSections[i].m_loopingAnimation == false)
				{
					m_pAnimatedSections[i].m_playingAnimation = false;
				}
			}

			m_pAnimatedSections[i].m_rotationX += m_pAnimatedSections[i].m_rotationSpeedX * dt;
			m_pAnimatedSections[i].m_rotationY += m_pAnimatedSections[i].m_rotationSpeedY * dt;
			m_pAnimatedSections[i].m_rotationZ += m_pAnimatedSections[i].m_rotationSpeedZ * dt;

			if (noChangeInAnimation)
			{
				m_pAnimatedSections[i].m_playingAnimation = false;
			}
		}
	}

	// Update dynamic lights
	for(int i = 0; i < m_numLights; i++)
	{
		if(m_pLights[i].m_connectedToSectionIndex != -1)
		{
			// If we are attached to an animated section, just copy it's position
			m_pLights[i].m_lightPosition = m_pAnimatedSections[m_pLights[i].m_connectedToSectionIndex].m_animatedSectionPosition;

			// Scale to render size
			// Translate for initial block offset
			vec3 lightPosition = m_pLights[i].m_lightOffset;

			// Rotation due to the weapon facing forwards for hand directions
			if(m_pParentCharacter != nullptr)
			{
				Matrix4x4 rotationMatrix;
				rotationMatrix.SetRotation(DegToRad(90.0f), 0.0f, 0.0f);
				lightPosition = rotationMatrix * lightPosition;

				lightPosition *= m_pParentCharacter->GetCharacterScale();
			}
			
			m_pLights[i].m_lightPosition += lightPosition;

			continue;
		}

		// Get the weapon position
		vec3 lightPosition = m_pLights[i].m_lightOffset;

		// Scale to render size
		// Translate for initial block offset
		lightPosition += (vec3(m_renderOffset.x, m_renderOffset.y, m_renderOffset.z) * m_renderScale);

		// Rotation due to the weapon facing forwards for hand directions
		if(m_pParentCharacter != nullptr)
		{
			Matrix4x4 rotationMatrix;
			rotationMatrix.SetRotation(DegToRad(90.0f), 0.0f, 0.0f);
			lightPosition = rotationMatrix * lightPosition;
		}

		if(m_matrixIndex != -1)
		{
			vec3 handBoneOffset = m_pParentCharacter->GetBoneMatrixRenderOffset(m_matrixName.c_str());

			// Translate for external matrix offset value
			lightPosition += vec3(handBoneOffset.x, handBoneOffset.y, handBoneOffset.z);
		}

		// Rotation due to 3dsmax export affecting the bone rotations
		if(m_pParentCharacter != nullptr)
		{
			Matrix4x4 rotationMatrix;
			rotationMatrix.SetRotation(0.0f, 0.0f, DegToRad(-90.0f));
			lightPosition = rotationMatrix * lightPosition;
		}

		// First person mode modifications
		if(m_firstPersonMode)
		{
			Matrix4x4 rotationMatrix;
			float amountX = 0.75f;
			float amountY = 0.75f;
			float amountZ = 0.5f;
			rotationMatrix.SetTranslation(vec3(amountX, amountY, -amountZ));
			rotationMatrix.SetRotation(0.0f, DegToRad(m_cameraYRotation), 0.0f);
			lightPosition = rotationMatrix * lightPosition;
		}

		// Translate for initial block offset
		//lightPosition -= vec3(0.5f, 0.5f, 0.5f);

		if(m_pParentCharacter != nullptr)
		{
			if(m_boneIndex != -1)
			{
				AnimationSections animationSection = AnimationSections_FullBody;
				if(m_boneIndex == m_pParentCharacter->GetHeadBoneIndex() ||
				   m_boneIndex == m_pParentCharacter->GetBodyBoneIndex())
				{
					animationSection = AnimationSections_Head_Body;
				}
				else if(m_boneIndex == m_pParentCharacter->GetLeftShoulderBoneIndex() ||
						m_boneIndex == m_pParentCharacter->GetLeftHandBoneIndex())
				{
					animationSection = AnimationSections_Left_Arm_Hand;
				}
				else if(m_boneIndex == m_pParentCharacter->GetRightShoulderBoneIndex() ||
						m_boneIndex == m_pParentCharacter->GetRightHandBoneIndex())
				{
					animationSection = AnimationSections_Right_Arm_Hand;
				}
				else if(m_boneIndex == m_pParentCharacter->GetLegsBoneIndex() ||
						m_boneIndex == m_pParentCharacter->GetRightFootBoneIndex() ||
						m_boneIndex == m_pParentCharacter->GetLeftFootBoneIndex())
				{
					animationSection = AnimationSections_Legs_Feet;
				}				

				Matrix4x4 boneMatrix = m_pParentCharacter->GetBoneMatrix(animationSection, m_boneIndex);

				// Translate by attached bone matrix
				lightPosition = boneMatrix * lightPosition;

				// Looking direction, since we are attached to a character who is holding us.
				{
					vec3 lForward = normalize(m_pParentCharacter->GetFaceLookingDirection());
					lForward.y = 0.0f;
					lForward = normalize(lForward);
					vec3 forwardDiff = lForward - vec3(0.0f, 0.0f, 1.0f);
					lForward = normalize(vec3(0.0f, 0.0f, 1.0f) + (forwardDiff*0.5f));

					vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
					vec3 lRight = normalize(cross(lUp, lForward));
					lUp = normalize(cross(lForward, lRight));

					float lMatrix[16] =
					{
						lRight.x, lRight.y, lRight.z, 0.0f,
						lUp.x, lUp.y, lUp.z, 0.0f,
						lForward.x, lForward.y, lForward.z, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f
					};
					Matrix4x4 lookingMat;
					lookingMat.SetValues(lMatrix);

					lightPosition = lookingMat * lightPosition;
				}

				// Breathing animation
				float offsetAmount = m_pParentCharacter->GetBreathingAnimationOffsetForBone(m_boneIndex);
				lightPosition += vec3(0.0f, offsetAmount, 0.0f);
			}
		}

		m_pLights[i].m_lightPosition = lightPosition;
	}

	// Update particle effects
	for(int i = 0; i < m_numParticleEffects; i++)
	{
		if(m_pParticleEffects[i].m_connectedToSectionIndex != -1)
		{
			// If we are attached to an animated section, just copy it's position
			m_pParticleEffects[i].m_particleEffectPosition = m_pAnimatedSections[m_pParticleEffects[i].m_connectedToSectionIndex].m_animatedSectionPosition;

			// Scale to render size
			// Translate for initial block offset
			vec3 particleEffectPosition = m_pParticleEffects[i].m_positionOffset;

			// Rotation due to the weapon facing forwards for hand directions
			if(m_pParentCharacter != nullptr)
			{
				Matrix4x4 rotationMatrix;
				rotationMatrix.SetRotation(DegToRad(90.0f), 0.0f, 0.0f);
				particleEffectPosition = rotationMatrix * particleEffectPosition;

				particleEffectPosition *= m_pParentCharacter->GetCharacterScale();	
			}

			m_pParticleEffects[i].m_particleEffectPosition += particleEffectPosition;

			continue;
		}

		vec3 particleEffectPosition = m_pParticleEffects[i].m_positionOffset;

		// Scale to render size
		// Translate for initial block offset
		particleEffectPosition += (vec3(m_renderOffset.x, m_renderOffset.y, m_renderOffset.z) * m_renderScale);

		// Rotation due to the weapon facing forwards for hand directions
		if(m_pParentCharacter != nullptr)
		{
			Matrix4x4 rotationMatrix;
			rotationMatrix.SetRotation(DegToRad(90.0f), 0.0f, 0.0f);
			particleEffectPosition = rotationMatrix * particleEffectPosition;
		}

		if(m_matrixIndex != -1)
		{
			vec3 handBoneOffset = m_pParentCharacter->GetBoneMatrixRenderOffset(m_matrixName.c_str());

			// Translate for external matrix offset value
			particleEffectPosition += vec3(handBoneOffset.x, handBoneOffset.y, handBoneOffset.z);
		}

		// Rotation due to 3dsmax export affecting the bone rotations
		if(m_pParentCharacter != nullptr)
		{
			Matrix4x4 rotationMatrix;
			rotationMatrix.SetRotation(0.0f, 0.0f, DegToRad(-90.0f));
			particleEffectPosition = rotationMatrix * particleEffectPosition;
		}

		// First person mode modifications
		if(m_firstPersonMode)
		{
			Matrix4x4 rotationMatrix;
			float amountX = 0.75f;
			float amountY = 0.75f;
			float amountZ = 0.5f;
			rotationMatrix.SetTranslation(vec3(amountX, amountY, -amountZ));
			rotationMatrix.SetRotation(0.0f, DegToRad(m_cameraYRotation), 0.0f);
			particleEffectPosition = rotationMatrix * particleEffectPosition;
		}

		// Translate for initial block offset
		//particleEffectPosition -= vec3(0.5f, 0.5f, 0.5f);

		if(m_pParentCharacter != nullptr)
		{
			if(m_boneIndex != -1)
			{
				AnimationSections animationSection = AnimationSections_FullBody;
				if(m_boneIndex == m_pParentCharacter->GetHeadBoneIndex() ||
					m_boneIndex == m_pParentCharacter->GetBodyBoneIndex())
				{
					animationSection = AnimationSections_Head_Body;
				}
				else if(m_boneIndex == m_pParentCharacter->GetLeftShoulderBoneIndex() ||
					m_boneIndex == m_pParentCharacter->GetLeftHandBoneIndex())
				{
					animationSection = AnimationSections_Left_Arm_Hand;
				}
				else if(m_boneIndex == m_pParentCharacter->GetRightShoulderBoneIndex() ||
					m_boneIndex == m_pParentCharacter->GetRightHandBoneIndex())
				{
					animationSection = AnimationSections_Right_Arm_Hand;
				}
				else if(m_boneIndex == m_pParentCharacter->GetLegsBoneIndex() ||
					m_boneIndex == m_pParentCharacter->GetRightFootBoneIndex() ||
					m_boneIndex == m_pParentCharacter->GetLeftFootBoneIndex())
				{
					animationSection = AnimationSections_Legs_Feet;
				}				

				Matrix4x4 boneMatrix = m_pParentCharacter->GetBoneMatrix(animationSection, m_boneIndex);

				// Translate by attached bone matrix
				particleEffectPosition = boneMatrix * particleEffectPosition;

				// Looking direction, since we are attached to a character who is holding us.
				{
					vec3 lForward = normalize(m_pParentCharacter->GetFaceLookingDirection());
					lForward.y = 0.0f;
					lForward = normalize(lForward);
					vec3 forwardDiff = lForward - vec3(0.0f, 0.0f, 1.0f);
					lForward = normalize(vec3(0.0f, 0.0f, 1.0f) + (forwardDiff*0.5f));

					vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
					vec3 lRight = normalize(cross(lUp, lForward));
					lUp = normalize(cross(lForward, lRight));

					float lMatrix[16] =
					{
						lRight.x, lRight.y, lRight.z, 0.0f,
						lUp.x, lUp.y, lUp.z, 0.0f,
						lForward.x, lForward.y, lForward.z, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f
					};
					Matrix4x4 lookingMat;
					lookingMat.SetValues(lMatrix);

					particleEffectPosition = lookingMat * particleEffectPosition;
				}

				// Breathing animation
				float offsetAmount = m_pParentCharacter->GetBreathingAnimationOffsetForBone(m_boneIndex);
				particleEffectPosition += vec3(0.0f, offsetAmount, 0.0f);
			}
		}

		m_pParticleEffects[i].m_particleEffectPosition = particleEffectPosition;
	}

	// Update weapon trails timers;
	for (int i = 0; i < m_numWeaponTrails; i++)
	{
		for (int point = 0; point < m_pWeaponTrails[i].m_numTrailPoints; point++)
		{
			m_pWeaponTrails[i].m_pTrailPoints[point].m_animaionTime -= dt;

			if (m_pWeaponTrails[i].m_pTrailPoints[point].m_animaionTime <= 0.0f)
			{
				m_pWeaponTrails[i].m_pTrailPoints[point].m_pointActive = false;
			}
		}
	}
}

// Rendering
void VoxelWeapon::Render(bool renderOutline, bool reflection, bool silhouette, Colour OutlineColour)
{
	m_pRenderer->PushMatrix();
		if(m_pParentCharacter != nullptr)
		{
			if(m_boneIndex != -1)
			{
				AnimationSections animationSection = AnimationSections_FullBody;
				if(m_boneIndex == m_pParentCharacter->GetHeadBoneIndex() ||
					m_boneIndex == m_pParentCharacter->GetBodyBoneIndex())
				{
					animationSection = AnimationSections_Head_Body;
				}
				else if(m_boneIndex == m_pParentCharacter->GetLeftShoulderBoneIndex() ||
					m_boneIndex == m_pParentCharacter->GetLeftHandBoneIndex())
				{
					animationSection = AnimationSections_Left_Arm_Hand;
				}
				else if(m_boneIndex == m_pParentCharacter->GetRightShoulderBoneIndex() ||
					m_boneIndex == m_pParentCharacter->GetRightHandBoneIndex())
				{
					animationSection = AnimationSections_Right_Arm_Hand;
				}
				else if(m_boneIndex == m_pParentCharacter->GetLegsBoneIndex() ||
					m_boneIndex == m_pParentCharacter->GetRightFootBoneIndex() ||
					m_boneIndex == m_pParentCharacter->GetLeftFootBoneIndex())
				{
					animationSection = AnimationSections_Legs_Feet;
				}

				Matrix4x4 boneMatrix = m_pParentCharacter->GetBoneMatrix(animationSection, m_boneIndex);

				// Breathing animation
				float offsetAmount = m_pParentCharacter->GetBreathingAnimationOffsetForBone(m_boneIndex);
				m_pRenderer->TranslateWorldMatrix(0.0f, offsetAmount, 0.0f);

				// Body and hands/shoulders looking direction
				if( m_boneIndex == m_pParentCharacter->GetLeftHandBoneIndex() ||
					m_boneIndex == m_pParentCharacter->GetRightHandBoneIndex() )
				{
					vec3 lForward = normalize(m_pParentCharacter->GetFaceLookingDirection());
					lForward.y = 0.0f;
					lForward = normalize(lForward);
					vec3 forwardDiff = lForward - vec3(0.0f, 0.0f, 1.0f);
					lForward = normalize(vec3(0.0f, 0.0f, 1.0f) + (forwardDiff*0.5f));

					vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
					vec3 lRight = normalize(cross(lUp, lForward));
					lUp = normalize(cross(lForward, lRight));

					float lMatrix[16] =
					{
						lRight.x, lRight.y, lRight.z, 0.0f,
						lUp.x, lUp.y, lUp.z, 0.0f,
						lForward.x, lForward.y, lForward.z, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f
					};
					Matrix4x4 lookingMat;
					lookingMat.SetValues(lMatrix);
					m_pRenderer->MultiplyWorldMatrix(lookingMat);
				}

				// Translate by attached bone matrix
				m_pRenderer->MultiplyWorldMatrix(boneMatrix);

				// Rotation due to 3dsmax export affecting the bone rotations
				m_pRenderer->RotateWorldMatrix(0.0f, 0.0f, -90.0f);
			}
		}

		if(m_matrixIndex != -1)
		{
			vec3 handBoneOffset = m_pParentCharacter->GetBoneMatrixRenderOffset(m_matrixName.c_str());

			// Translate for external matrix offset value
			m_pRenderer->TranslateWorldMatrix(handBoneOffset.x, handBoneOffset.y, handBoneOffset.z);

			// Rotation due to the weapon facing forwards for hand directions
			m_pRenderer->RotateWorldMatrix(90.0f, 0.0f, 0.0f);
		}

		// Scale to render size
		m_pRenderer->ScaleWorldMatrix(m_renderScale, m_renderScale, m_renderScale);

		// Translate for initial block offset
		m_pRenderer->TranslateWorldMatrix(m_renderOffset.x, m_renderOffset.y, m_renderOffset.z);

		// Render all animated sections
		for(int i = 0; i < m_numAnimatedSections; i++)
		{
			m_pRenderer->PushMatrix();
				// Scale to render size
				m_pRenderer->ScaleWorldMatrix(m_pAnimatedSections[i].m_renderScale, m_pAnimatedSections[i].m_renderScale, m_pAnimatedSections[i].m_renderScale);

				// Translate for initial block offset
				m_pRenderer->TranslateWorldMatrix(m_pAnimatedSections[i].m_renderOffset.x, m_pAnimatedSections[i].m_renderOffset.y, m_pAnimatedSections[i].m_renderOffset.z);

				// Animated sections
				m_pRenderer->TranslateWorldMatrix(-m_pAnimatedSections[i].m_rotationPoint.x, -m_pAnimatedSections[i].m_rotationPoint.y, -m_pAnimatedSections[i].m_rotationPoint.z);
				m_pRenderer->RotateWorldMatrix(m_pAnimatedSections[i].m_rotationX, m_pAnimatedSections[i].m_rotationY, m_pAnimatedSections[i].m_rotationZ);
				m_pRenderer->TranslateWorldMatrix(m_pAnimatedSections[i].m_rotationPoint.x, m_pAnimatedSections[i].m_rotationPoint.y, m_pAnimatedSections[i].m_rotationPoint.z);
					
				m_pRenderer->TranslateWorldMatrix(m_pAnimatedSections[i].m_translateX, m_pAnimatedSections[i].m_translateY, m_pAnimatedSections[i].m_translateZ);

				//m_pRenderer->ScaleWorldMatrix(1.0f, 1.0f, 1.0f);

				m_pAnimatedSections[i].m_pVoxelObject->Render(renderOutline, reflection, silhouette, OutlineColour);

				// Store the animated section position, since light might be attached to it
				if(reflection == false)
				{
					Matrix4x4 pMat = m_pAnimatedSections[i].m_pVoxelObject->GetModelMatrix(0);
					m_pAnimatedSections[i].m_animatedSectionPosition = pMat.GetTranslationVector();
				}
			m_pRenderer->PopMatrix();
		}
	m_pRenderer->PopMatrix();
}

void VoxelWeapon::RenderPaperdoll()
{
	m_pRenderer->PushMatrix();
		if(m_pParentCharacter != nullptr)
		{
			if(m_boneIndex != -1)
			{
				bool leftSide = (m_boneIndex == m_pParentCharacter->GetLeftHandBoneIndex());
				Matrix4x4 boneMatrix = m_pParentCharacter->GetBoneMatrixPaperdoll(m_boneIndex, leftSide);

				// Breathing animation
				float offsetAmount = m_pParentCharacter->GetBreathingAnimationOffsetForBone(m_boneIndex);
				m_pRenderer->TranslateWorldMatrix(0.0f, offsetAmount, 0.0f);

				// Body and hands/shoulders looking direction
				if( m_boneIndex == m_pParentCharacter->GetLeftHandBoneIndex() ||
					m_boneIndex == m_pParentCharacter->GetRightHandBoneIndex() )
				{
					vec3 lForward = normalize(m_pParentCharacter->GetFaceLookingDirection());
					lForward.y = 0.0f;
					lForward = normalize(lForward);
					vec3 forwardDiff = lForward - vec3(0.0f, 0.0f, 1.0f);
					lForward = normalize(vec3(0.0f, 0.0f, 1.0f) + (forwardDiff*0.5f));

					vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
					vec3 lRight = normalize(cross(lUp, lForward));
					lUp = normalize(cross(lForward, lRight));

					float lMatrix[16] =
					{
						lRight.x, lRight.y, lRight.z, 0.0f,
						lUp.x, lUp.y, lUp.z, 0.0f,
						lForward.x, lForward.y, lForward.z, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f
					};
					Matrix4x4 lookingMat;
					lookingMat.SetValues(lMatrix);
					m_pRenderer->MultiplyWorldMatrix(lookingMat);
				}

				// Translate by attached bone matrix
				m_pRenderer->MultiplyWorldMatrix(boneMatrix);
			}
		}

		// Rotation due to 3dsmax export affecting the bone rotations
		m_pRenderer->RotateWorldMatrix(0.0f, 0.0f, -90.0f);

		if(m_matrixIndex != -1)
		{
			vec3 handBoneOffset = m_pParentCharacter->GetBoneMatrixRenderOffset(m_matrixName.c_str());

			// Translate for external matrix offset value
			m_pRenderer->TranslateWorldMatrix(handBoneOffset.x, handBoneOffset.y, handBoneOffset.z);

			// Rotation due to the weapon facing forwards for hand directions
			m_pRenderer->RotateWorldMatrix(90.0f, 0.0f, 0.0f);
		}

		// Scale to render size
		m_pRenderer->ScaleWorldMatrix(m_renderScale, m_renderScale, m_renderScale);

		// Translate for initial block offset
		m_pRenderer->TranslateWorldMatrix(m_renderOffset.x, m_renderOffset.y, m_renderOffset.z);

		// Render all animated sections
		for(int i = 0; i < m_numAnimatedSections; i++)
		{
			m_pRenderer->PushMatrix();
				// Scale to render size
				m_pRenderer->ScaleWorldMatrix(m_pAnimatedSections[i].m_renderScale, m_pAnimatedSections[i].m_renderScale, m_pAnimatedSections[i].m_renderScale);

				// Translate for initial block offset
				m_pRenderer->TranslateWorldMatrix(m_pAnimatedSections[i].m_renderOffset.x, m_pAnimatedSections[i].m_renderOffset.y, m_pAnimatedSections[i].m_renderOffset.z);

				// Animated sections
				m_pRenderer->TranslateWorldMatrix(-m_pAnimatedSections[i].m_rotationPoint.x, -m_pAnimatedSections[i].m_rotationPoint.y, -m_pAnimatedSections[i].m_rotationPoint.z);
				m_pRenderer->RotateWorldMatrix(m_pAnimatedSections[i].m_rotationX, m_pAnimatedSections[i].m_rotationY, m_pAnimatedSections[i].m_rotationZ);
				m_pRenderer->TranslateWorldMatrix(m_pAnimatedSections[i].m_rotationPoint.x, m_pAnimatedSections[i].m_rotationPoint.y, m_pAnimatedSections[i].m_rotationPoint.z);

				m_pRenderer->TranslateWorldMatrix(m_pAnimatedSections[i].m_translateX, m_pAnimatedSections[i].m_translateY, m_pAnimatedSections[i].m_translateZ);

				//m_pRenderer->ScaleWorldMatrix(1.0f, 1.0f, 1.0f);

				Colour OutlineColour(1.0f, 1.0f, 0.0f, 1.0f);
				m_pAnimatedSections[i].m_pVoxelObject->Render(false, false, false, OutlineColour);
			m_pRenderer->PopMatrix();
		}
	m_pRenderer->PopMatrix();
}

void VoxelWeapon::RenderWeaponTrails()
{
	for(int i = 0; i < m_numWeaponTrails; i++)
	{
		m_pRenderer->PushMatrix();
			if(m_pWeaponTrails[i].m_followOrigin)
			{
				m_pRenderer->MultiplyWorldMatrix(m_pWeaponTrails[i].m_origin);

				m_pRenderer->ScaleWorldMatrix(m_pWeaponTrails[i].m_parentScale, m_pWeaponTrails[i].m_parentScale, m_pWeaponTrails[i].m_parentScale);
			}

			m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
			//m_pRenderer->DisableDepthTest();
			m_pRenderer->SetCullMode(CM_NOCULL);
			m_pRenderer->SetRenderMode(RM_SOLID);
			m_pRenderer->SetLineWidth(3.0f);

			m_pRenderer->EnableImmediateMode(IM_QUADS);
				for (int j = 0; j < m_pWeaponTrails[i].m_numTrailPoints-1; j++)
				{
					int index1 = j;
					int index2 = j + 1;
					if (index2 >= m_pWeaponTrails[i].m_numTrailPoints-1)
					{
						index2 = 0;
					}

					if (m_pWeaponTrails[i].m_pTrailPoints[index1].m_pointActive == true &&
						m_pWeaponTrails[i].m_pTrailPoints[index2].m_pointActive == true)
					{
						float alpha1 = m_pWeaponTrails[i].m_pTrailPoints[index1].m_animaionTime / m_pWeaponTrails[i].m_trailTime;
						float alpha2 = m_pWeaponTrails[i].m_pTrailPoints[index2].m_animaionTime / m_pWeaponTrails[i].m_trailTime;

						m_pRenderer->ImmediateColourAlpha(m_pWeaponTrails[i].m_trailColour.GetRed(), m_pWeaponTrails[i].m_trailColour.GetGreen(), m_pWeaponTrails[i].m_trailColour.GetBlue(), alpha1);
						m_pRenderer->ImmediateVertex(m_pWeaponTrails[i].m_pTrailPoints[index1].m_startPoint.x, m_pWeaponTrails[i].m_pTrailPoints[index1].m_startPoint.y, m_pWeaponTrails[i].m_pTrailPoints[index1].m_startPoint.z);

						m_pRenderer->ImmediateColourAlpha(m_pWeaponTrails[i].m_trailColour.GetRed(), m_pWeaponTrails[i].m_trailColour.GetGreen(), m_pWeaponTrails[i].m_trailColour.GetBlue(), alpha1);
						m_pRenderer->ImmediateVertex(m_pWeaponTrails[i].m_pTrailPoints[index1].m_endPoint.x, m_pWeaponTrails[i].m_pTrailPoints[index1].m_endPoint.y, m_pWeaponTrails[i].m_pTrailPoints[index1].m_endPoint.z);

						m_pRenderer->ImmediateColourAlpha(m_pWeaponTrails[i].m_trailColour.GetRed(), m_pWeaponTrails[i].m_trailColour.GetGreen(), m_pWeaponTrails[i].m_trailColour.GetBlue(), alpha2);
						m_pRenderer->ImmediateVertex(m_pWeaponTrails[i].m_pTrailPoints[index2].m_endPoint.x, m_pWeaponTrails[i].m_pTrailPoints[index2].m_endPoint.y, m_pWeaponTrails[i].m_pTrailPoints[index2].m_endPoint.z);

						m_pRenderer->ImmediateColourAlpha(m_pWeaponTrails[i].m_trailColour.GetRed(), m_pWeaponTrails[i].m_trailColour.GetGreen(), m_pWeaponTrails[i].m_trailColour.GetBlue(), alpha2);
						m_pRenderer->ImmediateVertex(m_pWeaponTrails[i].m_pTrailPoints[index2].m_startPoint.x, m_pWeaponTrails[i].m_pTrailPoints[index2].m_startPoint.y, m_pWeaponTrails[i].m_pTrailPoints[index2].m_startPoint.z);
					}
				}
			m_pRenderer->DisableImmediateMode();
			m_pRenderer->DisableTransparency();
			m_pRenderer->SetCullMode(CM_BACK);
			//m_pRenderer->EnableDepthTest(DT_LESS);
		m_pRenderer->PopMatrix();
	}
}