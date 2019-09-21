// ******************************************************************************
//
// Filename:	mouselistener.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   Mouse listener interface, inherit from this class if you need
//   to listen for and handle mouse events.
//
// Revision History:
//   Initial Revision - 06/10/06
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#pragma once

#include "eventlistener.h"
#include "mouseevent.h"


class MouseListener : public EventListener
{
public:
	/* Public methods */
	MouseListener();
	virtual ~MouseListener();

	virtual void MouseClicked(const MouseEvent& lEvent) {}  // Pressed AND released
	virtual void MousePressed(const MouseEvent& lEvent) {}
	virtual void MouseReleased(const MouseEvent& lEvent) {}
	virtual void MouseReleasedOutside(const MouseEvent& lEvent) {} // Pressed an object, then released outside of the objects bounds
	virtual void MouseEntered(const MouseEvent& lEvent) {}
	virtual void MouseExited(const MouseEvent& lEvent) {}
	virtual void MouseDragged(const MouseEvent& lEvent) {}
	virtual void MouseDraggedOutside(const MouseEvent& lEvent) {} // HACK
	virtual void MouseMotion(const MouseEvent& lEvent) {}

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
};
