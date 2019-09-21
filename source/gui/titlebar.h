// ******************************************************************************
//
// Filename:	titlebar.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A title bar that is used for gui windows, displays the window
//   title and any buttons for close, minimize, etc..
//
// Revision History:
//   Initial Revision - 22/10/06
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#pragma once

#include "container.h"
#include "label.h"
#include "guiwindowclosebutton.h"
#include "guiwindowminimizebutton.h"

class TitleBar : public Container, public MouseListener
{
public:
	/* Public methods */
	TitleBar(Renderer* pRenderer, unsigned int GUIFont, const std::string &title);

	TitleBar(Renderer* pRenderer, unsigned int GUIFont, const std::string &title, GUIWindowCloseButton* pCloseButton, GUIWindowMinimizeButton* pMinimizeButton);

	~TitleBar();

	void AddEventListeners() override;
	void RemoveEventListeners() override;

	GUIWindowCloseButton* GetCloseButton() const;
	GUIWindowMinimizeButton* GetMinimizeButton() const;

	void SetGUIWindowParent(GUIWindow* pParent);

	void SetDimensions(int x, int y, int width, int height) override;
	void SetDimensions(const Dimensions& r) override;

	void SetTitleOffset(int xOffset, int yOffset);

	void SetBackgroundIcon(RenderRectangle *icon);

	void SetTitle(const std::string &title);
	const std::string GetTitle() const;

	void SetOutlineRender(bool outline);

	void AllowMinimizing(bool val);
	void AllowClosing(bool val);

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
	GUIWindow *m_pParent;

	Label m_title;
	GUIWindowCloseButton* m_closeButton;
	GUIWindowMinimizeButton* m_minimizeButton;

	RenderRectangle	*m_pBackgroundIcon;

	bool mb_ownsButtons;

	bool m_outlineRender;

	bool m_bFirstOutsideDrag;
	int m_lastX;
	int m_lastY;

	bool m_bDragging;
	int m_nextX;
	int m_nextY;
};
