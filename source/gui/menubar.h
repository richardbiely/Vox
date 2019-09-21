// ******************************************************************************
//
// Filename:	menubar.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A menu bar is a container of menus.
//
// Revision History:
//   Initial Revision - 15/05/08
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#pragma once

#include "menu.h"


class MenuBar : public Container, public MouseListener
{
public:
	/* Public methods */
	MenuBar(Renderer* pRenderer);

	~MenuBar();

	void Add(Menu* lpMenu);

	void CloseAllMenus();

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
	int mBiggestHeight;
	int mCurrentMenuBarWidth;
};
