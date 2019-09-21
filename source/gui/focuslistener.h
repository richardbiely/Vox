// ******************************************************************************
//
// Filename:	focuslistener.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   Focus listener interface, inherit from this class if you need
//   to listen for and handle focus events.
//
// Revision History:
//   Initial Revision - 09/07/08
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#pragma once

#include "eventlistener.h"
#include "focusevent.h"


class FocusListener : public EventListener
{
public:
	/* Public methods */
	FocusListener();
	virtual ~FocusListener();

	virtual void FocusLost(const FocusEvent& lEvent) {}
	virtual void FocusGained(const FocusEvent& lEvent) {}

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
