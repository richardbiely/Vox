// ******************************************************************************
//
// Filename:	abstractbutton.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   An abstract button class used as a base
//   class for all button related components.
//
// Revision History:
//   Initial Revision - 28/05/07
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#pragma once

#include "container.h"
#include "label.h"
#include "renderrectangle.h"

typedef void(*FunctionCallback)(void *lpData);

class AbstractButton : public Container
{
public:
	/* Public methods */
	AbstractButton(Renderer* pRenderer, unsigned int GUIFont, const std::string &label);

	virtual ~AbstractButton();

	void SetLabel(const string &label);

	virtual void RemoveIcon(RenderRectangle *pRemoveIcon);
	virtual void RemoveIcon(const std::string &fileName);
	virtual void SetDefaultIcons(Renderer* pRenderer);

	void SetDefaultIcon(RenderRectangle *icon);
	void SetSelectedIcon(RenderRectangle *icon);
	void SetHoverIcon(RenderRectangle *icon);
	void SetDisabledIcon(RenderRectangle *icon);

	Label GetLabel() const;

	void SetDisplayLabel(bool display);

	void SetSelected(bool val);
	void SetHover(bool val);
	void SetDisabled(bool val);

	bool IsSelected() const;
	bool IsHover() const;
	bool IsDisabled() const;

	void SetCallBackFunction(FunctionCallback lFunction);
	void SetCallBackData(void *lpData);

	void SetCallBackFunction_Pressed(FunctionCallback lFunction);
	void SetCallBackData_Pressed(void *lpData);

	void SetCallBackFunction_Released(FunctionCallback lFunction);
	void SetCallBackData_Released(void *lpData);

	void SetEnterCallBackFunction(FunctionCallback lFunction);
	void SetEnterCallBackData(void *lpData);

	void SetExitCallBackFunction(FunctionCallback lFunction);
	void SetExitCallBackData(void *lpData);

	EComponentType GetComponentType() const override;

protected:
	/* Protected methods */

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */
	RenderRectangle	*m_pDefaultIcon;
	RenderRectangle	*m_pSelectedIcon;
	RenderRectangle	*m_pHoverIcon;
	RenderRectangle	*m_pDisabledIcon;

	Label m_label;

	bool m_bDisplayLabel;

	int m_labelOffsetX;
	int m_labelOffsetY;

	// Function callback for when this button is pressed and released
	FunctionCallback m_Callback;
	void *m_pCallbackData;

	// Function callback for when this button is pressed
	FunctionCallback m_Callback_Pressed;
	void *m_pCallbackData_Pressed;

	// Function callback for when this button is released
	FunctionCallback m_Callback_Released;
	void *m_pCallbackData_Released;

	// Function callback for when this button is entered
	FunctionCallback m_EnterCallback;
	void *m_pEnterCallbackData;

	// Function callback for when this button is exited
	FunctionCallback m_ExitCallback;
	void *m_pExitCallbackData;

private:
	/* Private members */
	bool	m_bSelected;
	bool	m_bHover;
	bool	m_bDisabled;
};
