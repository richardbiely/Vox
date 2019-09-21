// ******************************************************************************
// Filename:    SceneryManager.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 04/10/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "SceneryManager.h"

#include <vector>
#include <algorithm>


SceneryManager::SceneryManager(Renderer* pRenderer, ChunkManager* pChunkManager)
{
	m_pRenderer = pRenderer;
	m_pChunkManager = pChunkManager;

	m_renderOutlines = false;
	m_renderLabels = false;

	m_numRenderScenery = 0;
}

SceneryManager::~SceneryManager()
{
	ClearSceneryObjects();
}

void SceneryManager::ClearSceneryObjects()
{
	for(unsigned int i = 0; i < m_vpSceneryObjectList.size(); i++)
	{
		delete m_vpSceneryObjectList[i];
		m_vpSceneryObjectList[i] = 0;
	}
	m_vpSceneryObjectList.clear();
}

int SceneryManager::GetNumSceneryObjects()
{
	return (int)m_vpSceneryObjectList.size();
}

void SceneryManager::ResetNumRenderSceneryCounter()
{
	m_numRenderScenery = 0;
}

int SceneryManager::GetNumRenderSceneryObjects()
{
	return m_numRenderScenery;
}

SceneryObject* SceneryManager::GetSceneryObject(int index)
{
	return m_vpSceneryObjectList[index];
}

SceneryObject* SceneryManager::GetSceneryObject(string name)
{
	for(unsigned int i = 0; i < m_vpSceneryObjectList.size(); i++)
	{
		if(m_vpSceneryObjectList[i]->m_name == name)
		{
			return m_vpSceneryObjectList[i];
		}
	}

	return nullptr;
}

SceneryObject* SceneryManager::AddSceneryObject(string name, string filename, vec3 pos, vec3 worldFileOffset, QubicleImportDirection importDirection, QubicleImportDirection parentImportDirection, float scale, float rotation)
{
	// TODO : Should add back in duplicate name check?
	// Check if we already have a scenery object with the same name
	//for(unsigned int i = 0; i < m_vpSceneryObjectList.size(); i++)
	//{
	//	if(m_vpSceneryObjectList[i]->m_name == name)
	//	{
	//		return nullptr; // Don't allow creation of 2 same named scenery objects
	//	}
	//}

	QubicleBinary* pQubicleBinaryFile = new QubicleBinary(m_pRenderer);
	pQubicleBinaryFile->Import(filename.c_str(), true);

	return AddSceneryObject(name, filename, pos, worldFileOffset, importDirection, parentImportDirection, pQubicleBinaryFile, (float)pQubicleBinaryFile->GetQubicleMatrix(0)->m_matrixSizeX, (float)pQubicleBinaryFile->GetQubicleMatrix(0)->m_matrixSizeY, (float)pQubicleBinaryFile->GetQubicleMatrix(0)->m_matrixSizeZ, scale, rotation);
}

SceneryObject* SceneryManager::AddSceneryObject(string name, string filename, vec3 pos, vec3 worldFileOffset, QubicleImportDirection importDirection, QubicleImportDirection parentImportDirection, QubicleBinary* pQubicleBinaryFile, float length, float height, float width, float scale, float rotation)
{
	// Check if we already have a scenery object with the same name
	//for(unsigned int i = 0; i < m_vpSceneryObjectList.size(); i++)
	//{
	//	if(m_vpSceneryObjectList[i]->m_name == name)
	//	{
	//		return nullptr; // Don't allow creation of 2 same named scenery objects
	//	}
	//}

	SceneryObject* pNewSceneryObject = new SceneryObject();
	pNewSceneryObject->m_name = name;
	pNewSceneryObject->m_objectFilename = filename;
	pNewSceneryObject->m_positionOffset = pos;
	pNewSceneryObject->m_worldFileOffset = worldFileOffset;
	pNewSceneryObject->m_importDirection = importDirection;
	pNewSceneryObject->m_parentImportDirection = parentImportDirection;

	pNewSceneryObject->m_pQubicleBinaryFile = pQubicleBinaryFile;
	pNewSceneryObject->m_length = length;
	pNewSceneryObject->m_height = height;
	pNewSceneryObject->m_width = width;

	pNewSceneryObject->m_scale = scale;
	pNewSceneryObject->m_rotation = rotation;

	pNewSceneryObject->m_radius = 1.0f;

	pNewSceneryObject->m_canSelect = true;
	pNewSceneryObject->m_outlineRender = false;
	pNewSceneryObject->m_hoverRender = false;

	m_vpSceneryObjectList.push_back(pNewSceneryObject);

	return pNewSceneryObject;
}

void SceneryManager::DeleteSceneryObject(string name)
{
	SceneryObject* pDeleteObject = nullptr;

	// Find
	for(unsigned int i = 0; i < m_vpSceneryObjectList.size(); i++)
	{
		if(m_vpSceneryObjectList[i]->m_name == name)
		{
			pDeleteObject = m_vpSceneryObjectList[i];
		}
	}

	// Erase from container
	SceneryObjectList::iterator iter = std::find(m_vpSceneryObjectList.begin(), m_vpSceneryObjectList.end(), pDeleteObject);
	if(iter != m_vpSceneryObjectList.end())
	{
		m_vpSceneryObjectList.erase(iter);
	}

	// Delete
	if(pDeleteObject != nullptr)
	{
		delete pDeleteObject;
	}
}

void SceneryManager::DeleteLayout(string name)
{
	string nameToSearch = name + ".";
	SceneryObjectList vDeleteList;

	for(unsigned int i = 0; i < m_vpSceneryObjectList.size(); i++)
	{
		if(m_vpSceneryObjectList[i]->m_name.find(nameToSearch) != std::string::npos)
		{
			vDeleteList.push_back(m_vpSceneryObjectList[i]);
		}
	}

	for(unsigned int i = 0; i < vDeleteList.size(); i++)
	{
		DeleteSceneryObject(vDeleteList[i]->m_name);
	}
}

void SceneryManager::UpdateLayoutPosition(string name, vec3 newPosition)
{
	string nameToSearch = name + ".";
	for(unsigned int i = 0; i < m_vpSceneryObjectList.size(); i++)
	{
		if(m_vpSceneryObjectList[i]->m_name.find(nameToSearch) != std::string::npos)
		{
			m_vpSceneryObjectList[i]->m_worldFileOffset.x = (float)((int)newPosition.x);
			m_vpSceneryObjectList[i]->m_worldFileOffset.y = (float)((int)newPosition.y);
			m_vpSceneryObjectList[i]->m_worldFileOffset.z = (float)((int)newPosition.z);
		}
	}
}

void SceneryManager::UpdateLayoutDirection(string name, QubicleImportDirection direction)
{
	string nameToSearch = name + ".";
	for(unsigned int i = 0; i < m_vpSceneryObjectList.size(); i++)
	{
		if(m_vpSceneryObjectList[i]->m_name.find(nameToSearch) != std::string::npos)
		{
			m_vpSceneryObjectList[i]->m_parentImportDirection = direction;
		}
	}
}

// Render modes
void SceneryManager::SetRenderOutlines(bool outlines)
{
	m_renderOutlines = outlines;
}

void SceneryManager::SetRenderLabels(bool labels)
{
	m_renderLabels = labels;
}

// Updating
void SceneryManager::Update(float dt)
{
}

// Rendering
void SceneryManager::Render(bool reflection, bool silhouette, bool shadow, bool renderOnlyOutline, bool renderOnlyNormal)
{
	for(unsigned int i = 0; i < m_vpSceneryObjectList.size(); i++)
	{
		SceneryObject* pSceneryObject = m_vpSceneryObjectList[i];

		if(silhouette && (pSceneryObject->m_outlineRender == false && pSceneryObject->m_hoverRender == false))
		{
			continue; // Don't render silhouette unless we are rendering outline
		}

		if(renderOnlyNormal && (pSceneryObject->m_outlineRender || pSceneryObject->m_hoverRender))
		{
			continue;
		}
		if(renderOnlyOutline && pSceneryObject->m_outlineRender == false && pSceneryObject->m_hoverRender == false)
		{
			continue;
		}

		vec3 pos = pSceneryObject->m_worldFileOffset + pSceneryObject->m_positionOffset;

		bool renderBounding = false;
		RenderSceneryObject(pSceneryObject, false, reflection, silhouette, renderBounding, shadow);

		m_numRenderScenery++;
	}
}

void SceneryManager::RenderDebug()
{
	for(unsigned int i = 0; i < m_vpSceneryObjectList.size(); i++)
	{
		SceneryObject* pSceneryObject = m_vpSceneryObjectList[i];

		vec3 pos = pSceneryObject->m_worldFileOffset + pSceneryObject->m_positionOffset;

		m_pRenderer->PushMatrix();
			m_pRenderer->TranslateWorldMatrix(pos.x, pos.y, pos.z);
			m_pRenderer->SetLineWidth(1.0f);
			m_pRenderer->SetRenderMode(RM_WIREFRAME);

			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
			m_pRenderer->DrawSphere(pSceneryObject->m_radius, 20, 20);
		m_pRenderer->PopMatrix();
	}
}

void SceneryManager::RenderSceneryObject(SceneryObject* pSceneryObject, bool outline, bool reflection, bool silhouette, bool boundingBox, bool shadow)
{
	m_pRenderer->PushMatrix();
		// First translate to world file origin
		m_pRenderer->TranslateWorldMatrix(pSceneryObject->m_worldFileOffset.x, pSceneryObject->m_worldFileOffset.y, pSceneryObject->m_worldFileOffset.z);
		
		bool switchedFaceCulling = false;
		switch(pSceneryObject->m_parentImportDirection)
		{
			case QubicleImportDirection_Normal: {  } break;
			case QubicleImportDirection_MirrorX: { m_pRenderer->ScaleWorldMatrix(-1.0f, 1.0f, 1.0f); switchedFaceCulling = !switchedFaceCulling; } break;
			case QubicleImportDirection_MirrorY: { m_pRenderer->ScaleWorldMatrix(1.0f, -1.0f, 1.0f); switchedFaceCulling = !switchedFaceCulling; } break;
			case QubicleImportDirection_MirrorZ: { m_pRenderer->ScaleWorldMatrix(1.0f, 1.0f, -1.0f); switchedFaceCulling = !switchedFaceCulling; } break;
			case QubicleImportDirection_RotateY90: { m_pRenderer->RotateWorldMatrix(0.0f, -90.0f, 0.0f); } break;
			case QubicleImportDirection_RotateY180: { m_pRenderer->RotateWorldMatrix(0.0f, -180.0f, 0.0f); } break;
			case QubicleImportDirection_RotateY270: { m_pRenderer->RotateWorldMatrix(0.0f, -270.0f, 0.0f); } break;
			case QubicleImportDirection_RotateX90: { m_pRenderer->RotateWorldMatrix(-90.0f, 0.0f, 0.0f); } break;
			case QubicleImportDirection_RotateX180: { m_pRenderer->RotateWorldMatrix(-180.0f, 0.0f, 0.0f); } break;
			case QubicleImportDirection_RotateX270: { m_pRenderer->RotateWorldMatrix(-270.0f, 0.0f, 0.0f); } break;
			case QubicleImportDirection_RotateZ90: { m_pRenderer->RotateWorldMatrix(0.0f, 0.0f, -90.0f); } break;
			case QubicleImportDirection_RotateZ180: { m_pRenderer->RotateWorldMatrix(0.0f, 0.0f, -180.0f); } break;
			case QubicleImportDirection_RotateZ270: { m_pRenderer->RotateWorldMatrix(0.0f, 0.0f, -270.0f); } break;
		}

		// Now local object offset
		m_pRenderer->TranslateWorldMatrix(pSceneryObject->m_positionOffset.x, pSceneryObject->m_positionOffset.y, pSceneryObject->m_positionOffset.z);

		// Translate for block size offset
		m_pRenderer->TranslateWorldMatrix(0.0f, -Chunk::BLOCK_RENDER_SIZE, 0.0f);

		float l_length = pSceneryObject->m_length*0.5f;
		float l_height = pSceneryObject->m_height*0.5f;
		float l_width = pSceneryObject->m_width*0.5f;

		// Rotate the scenery object
		m_pRenderer->RotateWorldMatrix(0.0f, pSceneryObject->m_rotation, 0.0f);

		// Scale the scenery object
		m_pRenderer->ScaleWorldMatrix(pSceneryObject->m_scale, pSceneryObject->m_scale, pSceneryObject->m_scale);

		// Translate to the center
		m_pRenderer->TranslateWorldMatrix(0.0f, l_height, 0.0f);

		switch(pSceneryObject->m_importDirection)
		{
			case QubicleImportDirection_Normal: {  } break;
			case QubicleImportDirection_MirrorX: { m_pRenderer->ScaleWorldMatrix(-1.0f, 1.0f, 1.0f); switchedFaceCulling = !switchedFaceCulling; } break;
			case QubicleImportDirection_MirrorY: { m_pRenderer->ScaleWorldMatrix(1.0f, -1.0f, 1.0f); switchedFaceCulling = !switchedFaceCulling; } break;
			case QubicleImportDirection_MirrorZ: { m_pRenderer->ScaleWorldMatrix(1.0f, 1.0f, -1.0f); switchedFaceCulling = !switchedFaceCulling; } break;
			case QubicleImportDirection_RotateY90: { m_pRenderer->RotateWorldMatrix(0.0f, -90.0f, 0.0f); } break;
			case QubicleImportDirection_RotateY180: { m_pRenderer->RotateWorldMatrix(0.0f, -180.0f, 0.0f); } break;
			case QubicleImportDirection_RotateY270: { m_pRenderer->RotateWorldMatrix(0.0f, -270.0f, 0.0f); } break;
			case QubicleImportDirection_RotateX90: { m_pRenderer->RotateWorldMatrix(-90.0f, 0.0f, 0.0f); } break;
			case QubicleImportDirection_RotateX180: { m_pRenderer->RotateWorldMatrix(-180.0f, 0.0f, 0.0f); } break;
			case QubicleImportDirection_RotateX270: { m_pRenderer->RotateWorldMatrix(-270.0f, 0.0f, 0.0f); } break;
			case QubicleImportDirection_RotateZ90: { m_pRenderer->RotateWorldMatrix(0.0f, 0.0f, -90.0f); } break;
			case QubicleImportDirection_RotateZ180: { m_pRenderer->RotateWorldMatrix(0.0f, 0.0f, -180.0f); } break;
			case QubicleImportDirection_RotateZ270: { m_pRenderer->RotateWorldMatrix(0.0f, 0.0f, -270.0f); } break;
		}

		Colour OulineColour(1.0f, 1.0f, 0.0f, 1.0f);
		if(pSceneryObject->m_hoverRender && pSceneryObject->m_outlineRender == false)
			OulineColour = Colour(1.0f, 0.0f, 1.0f, 1.0f);

		if(shadow)
		{
			m_pRenderer->SetCullMode(switchedFaceCulling ? CM_BACK : CM_FRONT);
		}
		else
		{
			m_pRenderer->SetCullMode(switchedFaceCulling ? CM_FRONT : CM_BACK);
		}

		m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
		pSceneryObject->m_pQubicleBinaryFile->Render(outline, reflection, silhouette, OulineColour);

		if(boundingBox)
		{
			m_pRenderer->SetRenderMode(RM_WIREFRAME);
			m_pRenderer->SetCullMode(CM_NOCULL);
			m_pRenderer->SetLineWidth(1.0f);

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

			m_pRenderer->SetCullMode(CM_BACK);
		}
	m_pRenderer->PopMatrix();
}

void SceneryManager::RenderOutlineScenery()
{
	for(unsigned int i = 0; i < m_vpSceneryObjectList.size(); i++)
	{
		SceneryObject* pSceneryObject = m_vpSceneryObjectList[i];

		if(pSceneryObject->m_outlineRender == false && pSceneryObject->m_hoverRender == false)
		{
			continue;
		}

		RenderSceneryObject(pSceneryObject, true, false, false, false, false);
	}
}
