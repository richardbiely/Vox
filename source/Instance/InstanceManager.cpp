// ******************************************************************************
// Filename:    InstanceManager.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 11/06/12
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "InstanceManager.h"

#include <algorithm>

#include "../Renderer/Renderer.h"
#include "../utils/Random.h"
#include "../blocks/Chunk.h"
#include "../models/QubicleBinary.h"


InstanceManager::InstanceManager(Renderer* pRenderer)
{
	m_pRenderer = pRenderer;

	m_renderWireFrame = false;

	m_instanceShader = -1;
	m_pRenderer->LoadGLSLShader("media/shaders/instance.vertex", "media/shaders/instance.pixel", &m_instanceShader);

	m_checkChunkInstanceTimer = 0.0f;

	// Test data
	/*
	for(int i = 0; i < 100; i++)
	{
		vec3 pos = vec3(GetRandomNumber(-50, 50, 2), 7.5f, GetRandomNumber(-50, 50, 2));
		vec3 rot = vec3(0.0f, GetRandomNumber(0, 360, 2), 0.0f);
		AddInstanceObject("media/gamedata/terrain/plains/flower1.qb", pos, rot, 0.08f, nullptr, 0, 0, 0);
	}

	for(int i = 0; i < 100; i++)
	{
		vec3 pos = vec3(GetRandomNumber(-50, 50, 2), 7.5f, GetRandomNumber(-50, 50, 2));
		vec3 rot = vec3(0.0f, GetRandomNumber(0, 360, 2), 0.0f);
		AddInstanceObject("media/gamedata/terrain/plains/smalltree2.qb", pos, rot, 0.1f, nullptr, 0, 0, 0);
	}
	*/
}

InstanceManager::~InstanceManager()
{
	ClearInstanceObjects();
}

void InstanceManager::ClearInstanceObjects()
{
	for(unsigned int i = 0; i < (int)m_vpInstanceParentList.size(); i++)
	{
		for(unsigned int j = 0; j < (int)m_vpInstanceParentList[i]->m_vpInstanceObjectList.size(); j++)
		{
			delete m_vpInstanceParentList[i]->m_vpInstanceObjectList[j];
			m_vpInstanceParentList[i]->m_vpInstanceObjectList[j] = 0;
		}
		m_vpInstanceParentList[i]->m_vpInstanceObjectList.clear();

		delete m_vpInstanceParentList[i]->m_pQubicleBinary;

		delete m_vpInstanceParentList[i];
		m_vpInstanceParentList[i] = 0;
	}
	m_vpInstanceParentList.clear();
}

// Counters
int InstanceManager::GetNumInstanceParents()
{
	int numInstanceParents = (int)m_vpInstanceParentList.size();

	return numInstanceParents;
}

int InstanceManager::GetNumInstanceObjectsForParent(int parentId)
{
	int numInstanceObjects = (int)m_vpInstanceParentList[parentId]->m_vpInstanceObjectList.size();

	return numInstanceObjects;
}

int InstanceManager::GetNumInstanceRenderObjectsForParent(int parentId)
{
	int renderCounter = 0;

	for(int i = 0; i < (int)m_vpInstanceParentList[parentId]->m_vpInstanceObjectList.size(); i++)
	{
		if(m_vpInstanceParentList[parentId]->m_vpInstanceObjectList[i]->m_render == false)
		{
			continue;
		}

		renderCounter += 1;
	}

	return renderCounter;
}

int InstanceManager::GetTotalNumInstanceObjects()
{
	int counter = 0;
	for(int instanceParentId = 0; instanceParentId < (int)m_vpInstanceParentList.size(); instanceParentId++)
	{
		counter += (int)m_vpInstanceParentList[instanceParentId]->m_vpInstanceObjectList.size();
	}

	return counter;
}

int InstanceManager::GetTotalNumInstanceRenderObjects()
{
	int renderCounter = 0;
	for(int instanceParentId = 0; instanceParentId < (int)m_vpInstanceParentList.size(); instanceParentId++)
	{
		renderCounter += GetNumInstanceRenderObjectsForParent(instanceParentId);
	}

	return renderCounter;
}

// Setup
void InstanceManager::SetupGLBuffers(InstanceParent *pInstanceParent)
{
	pInstanceParent->m_vertexArray = -1;
	pInstanceParent->m_positionBuffer = -1;
	pInstanceParent->m_normalBuffer = -1;
	pInstanceParent->m_colourBuffer = -1;
	pInstanceParent->m_matrixBuffer = -1;

	pInstanceParent->m_pQubicleBinary = new QubicleBinary(m_pRenderer);
	pInstanceParent->m_pQubicleBinary->Import(pInstanceParent->m_modelName.c_str(), true);

	OpenGLTriangleMesh* pMesh = pInstanceParent->m_pQubicleBinary->GetQubicleMatrix(0)->m_pMesh;	

	glShader* pShader = m_pRenderer->GetShader(m_instanceShader);

	GLint in_position = glGetAttribLocation(pShader->GetProgramObject(), "in_position");
	GLint in_normal = glGetAttribLocation(pShader->GetProgramObject(), "in_normal");
	GLint in_color = glGetAttribLocation(pShader->GetProgramObject(), "in_color");
	GLint in_model_matrix = glGetAttribLocation(pShader->GetProgramObject(), "in_model_matrix");

	glBindFragDataLocation(pShader->GetProgramObject(), 0, "outputColor");
	glBindFragDataLocation(pShader->GetProgramObject(), 1, "outputPosition");
	glBindFragDataLocation(pShader->GetProgramObject(), 2, "outputNormal");

	glGenVertexArrays(1, &pInstanceParent->m_vertexArray);
	glBindVertexArray(pInstanceParent->m_vertexArray);

	float* vertices = new float[4 * (int)pMesh->m_vertices.size()];
	float* normals = new float[4 * (int)pMesh->m_vertices.size()];
	float* colours = new float[4 * (int)pMesh->m_vertices.size()];
	int counter = 0;
	for(int i = 0; i < (int)pMesh->m_vertices.size(); i++)
	{
		// Vertices
		vertices[counter+0] = pMesh->m_vertices[i]->vertexPosition[0];
		vertices[counter+1] = pMesh->m_vertices[i]->vertexPosition[1];
		vertices[counter+2] = pMesh->m_vertices[i]->vertexPosition[2];
		vertices[counter+3] = 1.0f;

		// Normals
		normals[counter+0] = pMesh->m_vertices[i]->vertexNormals[0];
		normals[counter+1] = pMesh->m_vertices[i]->vertexNormals[1];
		normals[counter+2] = pMesh->m_vertices[i]->vertexNormals[2];
		normals[counter+3] = 1.0f;

		// Colours
		colours[counter+0] = pMesh->m_vertices[i]->vertexColour[0];
		colours[counter+1] = pMesh->m_vertices[i]->vertexColour[1];
		colours[counter+2] = pMesh->m_vertices[i]->vertexColour[2];
		colours[counter+3] = 1.0f;

		counter += 4;
	}

	if(pInstanceParent->m_positionBuffer != -1)
	{
		glDeleteBuffers(1, &pInstanceParent->m_positionBuffer);
	}
	glGenBuffers(1, &pInstanceParent->m_positionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, pInstanceParent->m_positionBuffer);
	int sizeOfVertices = sizeof(float)*4*(int)pMesh->m_vertices.size();
	glBufferData(GL_ARRAY_BUFFER, sizeOfVertices, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(in_position);
	glVertexAttribPointer(in_position, 4, GL_FLOAT, 0, 0, 0);

	if(pInstanceParent->m_normalBuffer != -1)
	{
		glDeleteBuffers(1, &pInstanceParent->m_normalBuffer);
	}
	glGenBuffers(1, &pInstanceParent->m_normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, pInstanceParent->m_normalBuffer);
	int sizeOfNormals = sizeof(float)*4*(int)pMesh->m_vertices.size();
	glBufferData(GL_ARRAY_BUFFER, sizeOfNormals, normals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(in_normal);
	glVertexAttribPointer(in_normal, 4, GL_FLOAT, 0, 0, 0);

	if(pInstanceParent->m_colourBuffer != -1)
	{
		glDeleteBuffers(1, &pInstanceParent->m_colourBuffer);
	}
	glGenBuffers(1, &pInstanceParent->m_colourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, pInstanceParent->m_colourBuffer);
	int sizeOfColours = sizeof(float)*4*(int)pMesh->m_vertices.size();
	glBufferData(GL_ARRAY_BUFFER, sizeOfColours, colours, GL_STATIC_DRAW);
	glEnableVertexAttribArray(in_color);
	glVertexAttribPointer(in_color, 4, GL_FLOAT, 0, 0, 0);

	delete [] vertices;
	delete [] normals;
	delete [] colours;
}

// Creation
InstanceParent* InstanceManager::GetInstanceParent(string modelName)
{
	for(int instanceParentId = 0; instanceParentId < (int)m_vpInstanceParentList.size(); instanceParentId++)
	{
		if(m_vpInstanceParentList[instanceParentId]->m_modelName == modelName)
		{
			return m_vpInstanceParentList[instanceParentId];
		}
	}

	return nullptr;
}

InstanceObject* InstanceManager::AddInstanceObject(string modelName, vec3 position, vec3 rotation, float instanceScale, Chunk* pOwningChunk, int voxelX, int voxelY, int voxelZ)
{
	InstanceParent* pNewInstanceParent = GetInstanceParent(modelName);
	
	if(pNewInstanceParent == nullptr)
	{
		// Add new instance parent
		pNewInstanceParent = new InstanceParent();

		pNewInstanceParent->m_modelName = modelName;
		pNewInstanceParent->m_pQubicleBinary = new QubicleBinary(m_pRenderer);
		pNewInstanceParent->m_pQubicleBinary->Import(modelName.c_str(), true);

		SetupGLBuffers(pNewInstanceParent);

		m_vpInstanceParentList.push_back(pNewInstanceParent);
	}

	InstanceObject* pInstanceObject = new InstanceObject();
	pInstanceObject->m_erase = false;
	pInstanceObject->m_render = true;
	pInstanceObject->UpdateMatrix(position, rotation, instanceScale);

	pInstanceObject->m_pOwningChunk = pOwningChunk;
	pInstanceObject->m_voxelX = voxelX;
	pInstanceObject->m_voxelY = voxelY;
	pInstanceObject->m_voxelZ = voxelZ;

	pNewInstanceParent->m_vpInstanceObjectList.push_back(pInstanceObject);

	return pInstanceObject;
}

bool instance_object_needs_erasing(InstanceObject* pInstanceObject)
{
	bool needsErase = pInstanceObject->m_erase;

	if(needsErase == true)
	{
		if(pInstanceObject->m_pOwningChunk != nullptr)
		{
			// TODO : Add functionality back in, when chunks own instances
			//pInstanceObject->m_pOwningChunk->RemoveInstanceObjectFromList(pInstanceObject);
		}

		delete pInstanceObject;
	}

	return needsErase;
}

// Rendering modes
void InstanceManager::SetWireFrameRender(bool wireframe)
{
	m_renderWireFrame = wireframe;
}

// Update
void InstanceManager::Update(float dt)
{
	if(m_checkChunkInstanceTimer > 0.0f)
	{
		m_checkChunkInstanceTimer -= dt;
	}

	for(int instanceParentId = 0; instanceParentId < (int)m_vpInstanceParentList.size(); instanceParentId++)
	{
		// Check chunk instances, so that instances linked to voxels erase whhen the voxel is destroyed
		if(m_checkChunkInstanceTimer <= 0.0f)
		{
			for(int instanceobjectId = 0; instanceobjectId < (int)m_vpInstanceParentList[instanceParentId]->m_vpInstanceObjectList.size(); instanceobjectId++)
			{
				InstanceObject* pInstanceObject = m_vpInstanceParentList[instanceParentId]->m_vpInstanceObjectList[instanceobjectId];

				if(pInstanceObject->m_erase)
				{
					continue;
				}

				if(pInstanceObject->m_pOwningChunk != nullptr)
				{
					bool active = pInstanceObject->m_pOwningChunk->GetActive(pInstanceObject->m_voxelX, pInstanceObject->m_voxelY, pInstanceObject->m_voxelZ);

					if(active == false)
					{
						pInstanceObject->m_erase = true;
					}
				}
			}

			m_checkChunkInstanceTimer = 2.0f;
		}
		else
		{
			m_checkChunkInstanceTimer -= dt;
		}

		m_vpInstanceParentList[instanceParentId]->m_vpInstanceObjectList.erase( remove_if(m_vpInstanceParentList[instanceParentId]->m_vpInstanceObjectList.begin(), m_vpInstanceParentList[instanceParentId]->m_vpInstanceObjectList.end(), instance_object_needs_erasing), m_vpInstanceParentList[instanceParentId]->m_vpInstanceObjectList.end() );
	}
}

// Rendering
void InstanceManager::Render()
{
	glShader* pShader = m_pRenderer->GetShader(m_instanceShader);

	GLint in_position = glGetAttribLocation(pShader->GetProgramObject(), "in_position");
	GLint in_color = glGetAttribLocation(pShader->GetProgramObject(), "in_color");
	GLint in_model_matrix = glGetAttribLocation(pShader->GetProgramObject(), "in_model_matrix");

	for(int instanceParentId = 0; instanceParentId < (int)m_vpInstanceParentList.size(); instanceParentId++)
	{
		InstanceParent* pParent = m_vpInstanceParentList[instanceParentId];
		OpenGLTriangleMesh * pMesh = pParent->m_pQubicleBinary->GetQubicleMatrix(0)->m_pMesh;

		unsigned int numIndices = (int)pMesh->m_triangles.size() * 3;
		unsigned int numTriangles = (int)pMesh->m_triangles.size();
		unsigned int* indicesBuffer = new unsigned int[numIndices];
		int lIndexCounter = 0;
		for (auto pTriangle : pMesh->m_triangles)
		{
			indicesBuffer[lIndexCounter+0] = pTriangle->vertexIndices[0];
			indicesBuffer[lIndexCounter+1] = pTriangle->vertexIndices[1];
			indicesBuffer[lIndexCounter+2] = pTriangle->vertexIndices[2];

			lIndexCounter += 3;
		}

		int instanceObjectRenderCounter = 0;
		int numInstanceObjectsRender = GetNumInstanceRenderObjectsForParent(instanceParentId);
		if(numInstanceObjectsRender > 0)
		{
			float* newMatrices = new float[16 * numInstanceObjectsRender];

			int counter = 0;
			
			instanceObjectRenderCounter = 0;
			for (unsigned int i = 0; i < (int)pParent->m_vpInstanceObjectList.size() && instanceObjectRenderCounter < numInstanceObjectsRender; i++)
			{
				InstanceObject* pObject = pParent->m_vpInstanceObjectList[i];
				if (pObject->m_render == false)
				{
					continue;
				}

				newMatrices[counter + 0] = pObject->m_worldMatrix.m[0];
				newMatrices[counter + 1] = pObject->m_worldMatrix.m[1];
				newMatrices[counter + 2] = pObject->m_worldMatrix.m[2];
				newMatrices[counter + 3] = pObject->m_worldMatrix.m[3];
				newMatrices[counter + 4] = pObject->m_worldMatrix.m[4];
				newMatrices[counter + 5] = pObject->m_worldMatrix.m[5];
				newMatrices[counter + 6] = pObject->m_worldMatrix.m[6];
				newMatrices[counter + 7] = pObject->m_worldMatrix.m[7];
				newMatrices[counter + 8] = pObject->m_worldMatrix.m[8];
				newMatrices[counter + 9] = pObject->m_worldMatrix.m[9];
				newMatrices[counter + 10] = pObject->m_worldMatrix.m[10];
				newMatrices[counter + 11] = pObject->m_worldMatrix.m[11];
				newMatrices[counter + 12] = pObject->m_worldMatrix.m[12];
				newMatrices[counter + 13] = pObject->m_worldMatrix.m[13];
				newMatrices[counter + 14] = pObject->m_worldMatrix.m[14];
				newMatrices[counter + 15] = pObject->m_worldMatrix.m[15];
				counter += 16;

				instanceObjectRenderCounter++;
			}

			glBindVertexArray(pParent->m_vertexArray);

			if(m_vpInstanceParentList[instanceParentId]->m_matrixBuffer != -1)
			{
				glDeleteBuffers(1, &pParent->m_matrixBuffer);
			}
			// Bind buffer for matrix and copy data into buffer
			glGenBuffers(1, &pParent->m_matrixBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, pParent->m_matrixBuffer);
			for (int i = 0; i < 4; i++)
			{
				glVertexAttribPointer(in_model_matrix + i,		// Location
					4, GL_FLOAT, GL_FALSE,	// vec4
					4*16,						// Stride
					reinterpret_cast<void *>(16 * i));		// Start offset

				glEnableVertexAttribArray(in_model_matrix + i);
				glVertexAttribDivisor(in_model_matrix + i, 1);
			}
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*16*numInstanceObjectsRender, newMatrices, GL_STATIC_READ);

			delete [] newMatrices;
		}


		// Render the instances
		m_pRenderer->BeginGLSLShader(m_instanceShader);

		GLint projMatrixLoc = glGetUniformLocation(pShader->GetProgramObject(), "projMatrix");
		GLint viewMatrixLoc = glGetUniformLocation(pShader->GetProgramObject(), "viewMatrix");

		Matrix4x4 projMat;
		Matrix4x4 viewMat;
		m_pRenderer->GetProjectionMatrix(&projMat);
		m_pRenderer->GetModelViewMatrix(&viewMat);

		glUniformMatrix4fv(projMatrixLoc,  1, false, projMat.m);
		glUniformMatrix4fv(viewMatrixLoc,  1, false, viewMat.m);

		GLint in_light_position = glGetUniformLocation(pShader->GetProgramObject(), "in_light_position");
		GLint in_light_const_a = glGetUniformLocation(pShader->GetProgramObject(), "in_light_const_a");
		GLint in_light_linear_a = glGetUniformLocation(pShader->GetProgramObject(), "in_light_linear_a");
		GLint in_light_quad_a = glGetUniformLocation(pShader->GetProgramObject(), "in_light_quad_a");
		GLint in_light_ambient = glGetUniformLocation(pShader->GetProgramObject(), "in_light_ambient");
		GLint in_light_diffuse = glGetUniformLocation(pShader->GetProgramObject(), "in_light_diffuse");

		if (m_renderWireFrame)
		{
			m_pRenderer->SetLineWidth(1.0f);
			m_pRenderer->SetRenderMode(RM_WIREFRAME);
			m_pRenderer->SetCullMode(CM_NOCULL);
		}
		else
		{
			m_pRenderer->SetCullMode(CM_BACK);
			m_pRenderer->SetRenderMode(RM_SOLID);
		}

		m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);

		glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indicesBuffer, numInstanceObjectsRender);

		m_pRenderer->DisableTransparency();

		m_pRenderer->EndGLSLShader(m_instanceShader);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		delete [] indicesBuffer;
	}
}
