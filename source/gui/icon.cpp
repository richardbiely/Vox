// ******************************************************************************
//
// Filename:	icon.cpp
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   An image component, used to display graphics on other gui components.
//
// Revision History:
//   Initial Revision - 04/10/06
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "icon.h"


Icon::Icon()
  : RenderRectangle(nullptr)
{
	m_pIcon = nullptr;
	m_textureID = -1;
	m_dynamicTextureID = -1;
	m_dynamicTexture = false;
	m_flippedX = false;
	m_flippedY = false;
}

Icon::Icon(Renderer* pRenderer)
  : RenderRectangle(pRenderer)
{
	m_pIcon = nullptr;
	m_textureID = -1;
	m_dynamicTextureID = -1;
	m_dynamicTexture = false;
	m_flippedX = false;
	m_flippedY = false;
}

Icon::Icon(Renderer* pRenderer, const std::string &fileName, int width, int height)
  : RenderRectangle(pRenderer),
    m_fileName(fileName)
{
	m_textureID = -1;
	m_dynamicTextureID = -1;
	m_pRenderer->LoadTexture(fileName.c_str(), &m_TextureWidth, &m_TextureHeight, &m_TextureWidthPower2, &m_TextureHeightPower2, &m_textureID);

	m_pIcon = nullptr;

	// Set dimensions
	SetDimensions(0, 0, width, height);

	CreateDrawBuffer();

	m_dynamicTexture = false;
	m_flippedX = false;
	m_flippedY = false;
}

void Icon::CreateDrawBuffer()
{
	// Create the buffer used for drawing the image
	OGLPositionNormalVertex vertices[4] = {
		//X				Y				Z		nX		nY		nZ
		{0.0f,			0.0f,			0.0f,	0.0f,	0.0f,	1.0f },
		{1.0f,			0.0f,			0.0f,	0.0f,	0.0f,	1.0f },
		{1.0f,			1.0f,			0.0f,	0.0f,	0.0f,	1.0f },
		{0.0f,			1.0f,			0.0f,	0.0f,	0.0f,	1.0f }
	};

	OGLUVCoordinate textureCoordinates[4] = {
		//u			v
		{0.0f,		1.0f },
		{1.0f,		1.0f },
		{1.0f,		0.0f },
		{0.0f,		0.0f },
	};

	unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

	m_pRenderer->CreateStaticBuffer(VT_POSITION_NORMAL_UV, -1, -1, 4, 4, 6, &vertices, &textureCoordinates, indices, &mTextureDrawBuffer);
}

Icon::~Icon()
{
	if(m_pIcon != nullptr)
	{
		Remove(m_pIcon);
		delete m_pIcon;
	}
}

std::string Icon::GetFileName()
{
	return m_fileName;
}

void Icon::SetDimensions(int x, int y, int width, int height)
{
	Component::SetDimensions(x, y, width, height);

	if(m_pIcon != nullptr)
	{
		// Also set the dimensions of the icon
		m_pIcon->SetDimensions(0, 0, width, height);
	}
}

void Icon::SetIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pIcon)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pIcon);

		delete m_pIcon;
		m_pIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pIcon = lpNewDirectDraw;
	}

	// Re-add the icon to the component list
	Add(m_pIcon);
}

void Icon::SetIcon(const std::string &fileName)
{
	m_pRenderer->LoadTexture(fileName.c_str(), &m_TextureWidth, &m_TextureHeight, &m_TextureWidthPower2, &m_TextureHeightPower2, &m_textureID);
}

void Icon::SetDynamicTexture(unsigned int textureId)
{
	m_dynamicTextureID = textureId;
	m_dynamicTexture = true;
}

void Icon::SetFlipped(bool x, bool y)
{
	m_flippedX = x;
	m_flippedY = y;
}

int Icon::GetTextureWidth()
{
	return m_TextureWidth;
}

int Icon::GetTextureHeight()
{
	return m_TextureHeight;
}

int Icon::GetTextureWidthPower2()
{
	return m_TextureWidthPower2;
}

int Icon::GetTextureHeightPower2()
{
	return m_TextureHeightPower2;
}

EComponentType Icon::GetComponentType() const
{
	return EComponentType_Icon;
}

void Icon::DrawSelf()
{
	float lPaddingWidth = (float)m_TextureWidthPower2 - (float)m_TextureWidth; // Not used??
	float lPaddingHeight = (float)m_TextureHeightPower2 - (float)m_TextureHeight;

	float lWidthRatio = (float)m_TextureWidthPower2 / (float)m_TextureWidth;
	float lHeightRatio = (float)m_TextureHeightPower2 / (float)m_TextureHeight;

	float lWidth = m_dimensions.m_width * lWidthRatio;
	float lHeight = m_dimensions.m_height * lHeightRatio;

	float lAdjustedPaddingHeight = lHeight - m_dimensions.m_height;

	if(m_dynamicTexture)
	{
		float l_length = (float)m_dimensions.m_width;
		float l_height = (float)m_dimensions.m_height;
		float l_depth = 1.0f;

		float s1 = 0.0f;
		float s2 = 1.0f;
		float t1 = 0.0f;
		float t2 = 1.0f;

		if(m_flippedX)
		{
			s1 = 1.0f;
			s2 = 0.0f;
		}
		if(m_flippedY)
		{
			t1 = 1.0f;
			t2 = 0.0f;
		}

		m_pRenderer->PushMatrix();
			m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, GetDepth());

			m_pRenderer->SetRenderMode(RM_TEXTURED);
			m_pRenderer->BindRawTextureId(m_dynamicTextureID);
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
			m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
			m_pRenderer->EnableImmediateMode(IM_QUADS);
				m_pRenderer->ImmediateTextureCoordinate(s1, t1);
				m_pRenderer->ImmediateVertex(0.0f, 0.0f, l_depth);
				m_pRenderer->ImmediateTextureCoordinate(s2, t1);
				m_pRenderer->ImmediateVertex(l_length, 0.0f, l_depth);
				m_pRenderer->ImmediateTextureCoordinate(s2, t2);
				m_pRenderer->ImmediateVertex(l_length, l_height, l_depth);
				m_pRenderer->ImmediateTextureCoordinate(s1, t2);
				m_pRenderer->ImmediateVertex(0.0f, l_height, l_depth);
			m_pRenderer->DisableImmediateMode();
			m_pRenderer->DisableTransparency();
		m_pRenderer->PopMatrix();
	}
	else
	{
		if(m_textureID != -1)
		{
			m_pRenderer->PushMatrix();
				m_pRenderer->SetRenderMode(RM_TEXTURED);
				m_pRenderer->SetPrimativeMode(PM_TRIANGLES);
				m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
				m_pRenderer->TranslateWorldMatrix(0.0f, -lAdjustedPaddingHeight, GetDepth());
				m_pRenderer->ScaleWorldMatrix(lWidth, lHeight, 0.0f);
				m_pRenderer->BindTexture(m_textureID);
				m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
				m_pRenderer->RenderStaticBuffer(mTextureDrawBuffer);
				m_pRenderer->DisableTransparency();
				m_pRenderer->DisableTexture();
			m_pRenderer->PopMatrix();
		}
	}
}