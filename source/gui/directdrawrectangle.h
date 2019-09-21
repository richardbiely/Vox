// ******************************************************************************
//
// Filename:	directdrawrectangle.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A rendered rectangle that is drawn directly by OpenGL, using primitives.
//
// Revision History:
//   Initial Revision - 07/08/08
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#pragma once

#include "renderrectangle.h"


class DirectDrawRectangle :	public RenderRectangle
{
public:
	/* Public methods */
	DirectDrawRectangle(Renderer* pRenderer);

	~DirectDrawRectangle();

	void SetBackgroundColourTopLeft(Colour col);
	void SetBackgroundColourTopRight(Colour col);
	void SetBackgroundColourBottomLeft(Colour col);
	void SetBackgroundColourBottomRight(Colour col);

	void SetOutlineColourTop(Colour col);
	void SetOutlineColourBottom(Colour col);
	void SetOutlineColourLeft(Colour col);
	void SetOutlineColourRight(Colour col);

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
	float mLineSizeTop;
	float mLineSizeBottom;
	float mLineSizeLeft;
	float mLineSizeRight;

	Colour mBackgroundColourTopLeft;
	Colour mBackgroundColourTopRight;
	Colour mBackgroundColourBottomLeft;
	Colour mBackgroundColourBottomRight;

	Colour mOutlineColourTop;
	Colour mOutlineColourBottom;
	Colour mOutlineColourLeft;
	Colour mOutlineColourRight;
};
