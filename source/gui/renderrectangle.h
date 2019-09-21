// ******************************************************************************
//
// Filename:	renderrectangle.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A rectangle drawing base class, used to define abstract behaviour for
//	 drawing of rectangular bounded areas.
//
// Revision History:
//   Initial Revision - 07/08/08
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#pragma once

#include "container.h"

class Renderer;

class RenderRectangle : public Container
{
public:
	/* Public methods */
	RenderRectangle(Renderer* pRenderer);

	virtual ~RenderRectangle();

	void AddText(Renderer* pRenderer, unsigned int GUIFont, unsigned int OutlineGUIFont, const std::string &label, Colour colour, int xOffset, int yOffset, bool outline = false, Colour outlineColour = Colour(1.0f, 1.0f, 1.0f, 1.0f));
	void AddIcon(Renderer* pRenderer, const std::string &fileName, int texWidth, int texHeight, int width, int height, int xOffset, int yOffset, float depth);

	EComponentType GetComponentType() const override;

protected:
	/* Protected methods */
	void DrawSelf() override;
	void DrawChildren() override;

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */
	ComponentList m_vpAddedComponentList;

private:
	/* Private members */
};
