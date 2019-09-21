// ******************************************************************************
//
// Filename:	multitextureicon.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A multi-textured icon that has a seperate graphic for each of the
//	 rectangles regions, and can scale without graphical distortion.
//
// Revision History:
//   Initial Revision - 10/08/08
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#pragma once

#include "renderrectangle.h"


enum ERectanlgeRegion
{
	ERectanlgeRegion_TopLeft = 0,
	ERectanlgeRegion_TopCenter,
	ERectanlgeRegion_TopRight,
	ERectanlgeRegion_MiddleLeft,
	ERectanlgeRegion_MiddleCenter,
	ERectanlgeRegion_MiddleRight,
	ERectanlgeRegion_BottomLeft,
	ERectanlgeRegion_BottomCenter,
	ERectanlgeRegion_BottomRight,

	ERectanlgeRegion_Num,
};

class MultiTextureIcon : public RenderRectangle
{
public:
	/* Public methods */
	MultiTextureIcon(Renderer* pRenderer);

	~MultiTextureIcon();

	void SetTexture(ERectanlgeRegion lRegionTexture, const std::string &fileName);

	int GetTextureWidth(ERectanlgeRegion lRegionTexture) const;
	int GetTextureHeight(ERectanlgeRegion lRegionTexture) const;

	void CreateDrawBuffer();

	EComponentType GetComponentType() const override;

protected:
	/* Protected methods */
	void DrawSelf() override;

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	std::string m_fileName[ERectanlgeRegion_Num];

	int m_TextureWidth[ERectanlgeRegion_Num];
	int m_TextureHeight[ERectanlgeRegion_Num];

	int m_TextureWidthPower2[ERectanlgeRegion_Num];
	int m_TextureHeightPower2[ERectanlgeRegion_Num];

	unsigned int m_textureID[ERectanlgeRegion_Num];

	unsigned int mTextureDrawBuffer;
};
