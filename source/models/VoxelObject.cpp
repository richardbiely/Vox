// ******************************************************************************
// Filename:    VoxelObject.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 11/08/14
//
// Copyright (c) 2005-2015, Steven Ball
// ******************************************************************************

#include "VoxelObject.h"


VoxelObject::VoxelObject()
{
	m_usingQubicleManager = false;

	Reset();
}

void VoxelObject::SetRenderer(Renderer* pRenderer)
{
	m_pRenderer = pRenderer;
}

void VoxelObject::SetQubicleBinaryManager(QubicleBinaryManager* pQubicleBinaryManager)
{
	m_pQubicleBinaryManager = pQubicleBinaryManager;
}

VoxelObject::~VoxelObject()
{
	UnloadObject();
	Reset();
}

void VoxelObject::Reset()
{
	m_pVoxelModel = nullptr;

	m_loaded = false;
}

// Rebuild
void VoxelObject::RebuildVoxelModel(bool faceMerge)
{
	m_pVoxelModel->RebuildMesh(faceMerge);
}

QubicleBinary* VoxelObject::GetQubicleModel()
{
	return m_pVoxelModel;
}

Matrix4x4 VoxelObject::GetModelMatrix(int qubicleMatrixIndex)
{
	if(m_pVoxelModel == nullptr || m_loaded == false)
	{
		return Matrix4x4();
	}

	return m_pVoxelModel->GetModelMatrix(qubicleMatrixIndex);
}

vec3 VoxelObject::GetCenter()
{
	if(m_pVoxelModel == nullptr || m_loaded == false)
	{
		return vec3(0.0f, 0.0f, 0.0f);
	}

	vec3 centerPos;
	for(int i = 0; i < m_pVoxelModel->GetNumMatrices(); i++)
	{
		const QubicleMatrix* pMatrix = m_pVoxelModel->GetQubicleMatrix(i);

		centerPos.x += (pMatrix->m_matrixSizeX*0.5f)*pMatrix->m_scale;
		centerPos.y += (pMatrix->m_matrixSizeY*0.5f)*pMatrix->m_scale;
		centerPos.z += (pMatrix->m_matrixSizeZ*0.5f)*pMatrix->m_scale;
	}

	centerPos /= (float)m_pVoxelModel->GetNumMatrices();

	return centerPos;
}

void VoxelObject::LoadObject(const char *qbFilename, bool useManager)
{
	m_usingQubicleManager = useManager;

	if(m_pVoxelModel == nullptr)
	{
		if(useManager)
		{
			m_pVoxelModel = m_pQubicleBinaryManager->GetQubicleBinaryFile(qbFilename, false);
		}
		else
		{
			m_pVoxelModel = new QubicleBinary(m_pRenderer);
			m_pVoxelModel->Import(qbFilename, true);
		}
	}

	m_loaded = true;
}

void VoxelObject::UnloadObject()
{
	if(m_loaded)
	{
		if(m_usingQubicleManager == false)
		{
			delete m_pVoxelModel;
		}

		m_pVoxelModel = nullptr;
	}

	m_loaded = false;
}

// Rendering modes
void VoxelObject::SetWireFrameRender(bool wireframe)
{
	if(m_pVoxelModel != nullptr)
	{
		m_pVoxelModel->SetWireFrameRender(wireframe);
	}
}

void VoxelObject::SetMeshAlpha(float alpha)
{
	if(m_pVoxelModel != nullptr)
	{
		m_pVoxelModel->SetMeshAlpha(alpha);
	}
}

void VoxelObject::SetMeshSingleColour(float r, float g, float b)
{
	if(m_pVoxelModel != nullptr)
	{
		m_pVoxelModel->SetMeshSingleColour(r, g, b);
	}
}

void VoxelObject::Update(float dt)
{
	if(m_loaded == false)
	{
		return;
	}
}

void VoxelObject::Render(bool renderOutline, bool reflection, bool silhouette, Colour OutlineColour)
{
	if(m_pVoxelModel != nullptr)
	{
		m_pVoxelModel->Render(renderOutline, reflection, silhouette, OutlineColour);
	}
}