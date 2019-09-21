// ******************************************************************************
//
// Filename:	draggablerenderrectangle.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 31/07/12
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#pragma once

#include "container.h"
#include "renderrectangle.h"

typedef void(*FunctionCallback)(void* lpData);

class DraggableRenderRectangle : public Container, public MouseListener
{
public:
	/* Public methods */
	DraggableRenderRectangle(Renderer* pRenderer);

	~DraggableRenderRectangle();

	void SetIcon(RenderRectangle* icon);

	void AddText(Renderer* pRenderer, unsigned int GUIFont, unsigned int OutlineGUIFont, const std::string& label, Colour colour, int xOffset, int yOffset, bool outline = false, Colour outlineColour = Colour(1.0f, 1.0f, 1.0f, 1.0f));
	void AddIcon(Renderer* pRenderer, const std::string& fileName, int texWidth, int texHeight, int width, int height, int xOffset, int yOffset, float depth);
	void RemoveIcon(const std::string& fileName);

	void SetDimensions(int x, int y, int width, int height) override;
	void SetDimensions(const Dimensions& r) override;

	void AddEventListeners() override;
	void RemoveEventListeners() override;

	void SetPressedCallBackFunction(FunctionCallback lFunction);
	void SetPressedCallBackData(void* lpData);

	void SetReleasedCallBackFunction(FunctionCallback lFunction);
	void SetReleasedCallBackData(void* lpData);

	void SetEnterCallBackFunction(FunctionCallback lFunction);
	void SetEnterCallBackData(void* lpData);

	void SetExitCallBackFunction(FunctionCallback lFunction);
	void SetExitCallBackData(void* lpData);

	EComponentType GetComponentType() const override;

protected:
	/* Protected methods */
	void MouseEntered(const MouseEvent& lEvent) override;
	void MouseExited(const MouseEvent& lEvent) override;
	void MousePressed(const MouseEvent& lEvent) override;
	void MouseReleased(const MouseEvent& lEvent) override;
	void MouseClicked(const MouseEvent& lEvent) override;
	void MouseDragged(const MouseEvent& lEvent);
	void MouseDraggedOutside(const MouseEvent& lEvent);

	virtual void OnMouseEnter();
	virtual void OnMouseExit();
	virtual void OnMousePressed();
	virtual void OnMouseReleased();
	virtual void OnMouseClicked();
	virtual void OnMouseDragged();
	virtual void OnMouseDraggedOutside();

	void DrawSelf() override;

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	RenderRectangle* m_pIcon;

	ComponentList m_vpAddedComponentList;

	bool m_bDragging;
	int m_nextX;
	int m_nextY;

	// Function callback for when this draggable icon is pressed
	FunctionCallback m_PressedCallback;
	void* m_pPressedCallbackData;

	// Function callback for when this draggable icon is released
	FunctionCallback m_ReleasedCallback;
	void* m_pReleasedCallbackData;

	// Function callback for when this draggable icon is entered
	FunctionCallback m_EnterCallback;
	void* m_pEnterCallbackData;

	// Function callback for when this draggable icon is exited
	FunctionCallback m_ExitCallback;
	void* m_pExitCallbackData;
};
