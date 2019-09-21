// ******************************************************************************
// Filename:    InstanceManager.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//   Instanced objects are a group of objects that all share the same render call.
//   Thus we can group together all instances into a single call, and just modify
//   the objects matrix and other render properties in the shader, in a single
//   draw call. This is mostly used for rendering large chunks of similar objects
//   where the only difference is the model matrix (scale, position, rotation),
//   For example, scenery, flowers, clutter, etc.
//
// Revision History:
//   Initial Revision - 11/06/12
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once
#include <string>
#include <vector>
#include <glm/vec3.hpp>
#include "../Maths/3dmaths.h"

class Chunk;
class QubicleBinary;
class Renderer;

class InstanceObject
{
public:	
	Matrix4x4 m_worldMatrix;

	Chunk* m_pOwningChunk;
	int m_voxelX;
	int m_voxelY;
	int m_voxelZ;

	bool m_erase;
	bool m_render;

	void UpdateMatrix(vec3 position, vec3 rotation, float scale)
	{
		m_worldMatrix.SetRotation(rotation.x, rotation.y, rotation.z);
		m_worldMatrix.SetTranslation(position);

		Matrix4x4 scaleMat;
		scaleMat.SetScale(vec3(scale, scale, scale));
		m_worldMatrix = scaleMat * m_worldMatrix;
	}
};

typedef std::vector<InstanceObject*> InstanceObjectList;

class InstanceParent
{
public:
	unsigned int m_vertexArray;
	unsigned int m_positionBuffer;
	unsigned int m_normalBuffer;
	unsigned int m_colourBuffer;
	unsigned int m_matrixBuffer;

	InstanceObjectList m_vpInstanceObjectList;

	std::string m_modelName;
	QubicleBinary* m_pQubicleBinary;
};

typedef std::vector<InstanceParent*> InstanceParentList;


class InstanceManager
{
public:
	/* Public methods */
	InstanceManager(Renderer* pRenderer);
	~InstanceManager();

	void ClearInstanceObjects();

	// Counters
	int GetNumInstanceParents();
	int GetNumInstanceObjectsForParent(int parentId);
	int GetNumInstanceRenderObjectsForParent(int parentId);
	int GetTotalNumInstanceObjects();
	int GetTotalNumInstanceRenderObjects();

	// Setup
	void SetupGLBuffers(InstanceParent *pInstanceParent);

	// Creation
	InstanceParent* GetInstanceParent(std::string modelName);
	InstanceObject* AddInstanceObject(std::string modelName, vec3 position, vec3 rotation, float instanceScale, Chunk* pOwningChunk, int voxelX, int voxelY, int voxelZ);

	// Rendering modes
	void SetWireFrameRender(bool wireframe);

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

protected:
	/* Protected members */

private:
	/* Private members */
	Renderer* m_pRenderer;

	// Render modes
	bool m_renderWireFrame;

	// Shader
	unsigned int m_instanceShader;

	// List of instance parents what we render in a single render call for all children instances
	InstanceParentList m_vpInstanceParentList;

	// Timer to check for when to erase instances that are no longer linked to an owning chunk
	float m_checkChunkInstanceTimer;
};
