// ******************************************************************************
//
// Filename:	icon.h
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

#pragma once

#include "container.h"
#include "renderrectangle.h"
#include "icon.h"
#include "multitextureicon.h"
#include "directdrawrectangle.h"


class Icon : public RenderRectangle
{
public:
	/* Public methods */
	Icon();
	Icon(Renderer* pRenderer);

	Icon(Renderer* pRenderer, const std::string &fileName, int width = 32, int height = 32);

	~Icon();

	std::string GetFileName();

	void SetDimensions(int x, int y, int width, int height) override;

	void SetIcon(RenderRectangle *icon);
	void SetIcon(const std::string &fileName);

	void SetDynamicTexture(unsigned int textureId);

	void SetFlipped(bool x, bool y);

	void CreateDrawBuffer();

	int GetTextureWidth();
	int GetTextureHeight();

	int GetTextureWidthPower2();
	int GetTextureHeightPower2();

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
	RenderRectangle	*m_pIcon;

	std::string m_fileName;

	int m_TextureWidth;
	int m_TextureHeight;

	int m_TextureWidthPower2;
	int m_TextureHeightPower2;

	unsigned int mTextureDrawBuffer;

	unsigned int m_textureID;
	unsigned int m_dynamicTextureID;

	bool m_dynamicTexture;

	bool m_flippedX;
	bool m_flippedY;
};
